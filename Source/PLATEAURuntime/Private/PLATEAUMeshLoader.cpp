// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUMeshLoader.h"
#include "PLATEAUTextureLoader.h"

#include "plateau/polygon_mesh/mesh_extractor.h"
#include "citygml/citygml.h"

#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"
#include "PhysicsEngine/BodySetup.h"
#include "ImageUtils.h"
#include "MeshElementRemappings.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"
#include "PLATEAUInstancedCityModel.h"
#include "StaticMeshAttributes.h"
#include "Misc/DefaultValueHelper.h"


#if WITH_EDITOR

DECLARE_STATS_GROUP(TEXT("PLATEAUMeshLoader"), STATGROUP_PLATEAUMeshLoader, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Mesh.Build"), STAT_Mesh_Build, STATGROUP_PLATEAUMeshLoader);

FSubMeshMaterialSet::FSubMeshMaterialSet()
{
}

FSubMeshMaterialSet::FSubMeshMaterialSet(std::shared_ptr<const citygml::Material> mat, FString texPath)
{
    hasMaterial = mat != nullptr;
    if (hasMaterial)
    {
        auto dif = mat->getDiffuse();
        Diffuse = FVector3f(dif.x, dif.y, dif.z);
        auto spc = mat->getSpecular();
        Specular = FVector3f(spc.x, spc.y, spc.z);
        auto ems = mat->getEmissive();
        Emissive = FVector3f(ems.x, ems.y, ems.z);
        Shininess = mat->getShininess();
        Transparency = mat->getTransparency();
        Ambient = mat->getAmbientIntensity();
        isSmooth = mat->isSmooth();
    }
    TexturePath = texPath;
}

bool FSubMeshMaterialSet::operator==(const FSubMeshMaterialSet& Other) const
{
    return Equals(Other);
}

bool FSubMeshMaterialSet::Equals(const FSubMeshMaterialSet& Other) const
{
    float tl = 0.0001f;
    return Diffuse.Equals(Other.Diffuse, tl) &&
        Specular.Equals(Other.Specular, tl) &&
        Emissive.Equals(Other.Emissive, tl) &&
        FMath::IsNearlyEqual(Shininess, Other.Shininess, tl) &&
        FMath::IsNearlyEqual(Transparency, Other.Transparency, tl) &&
        FMath::IsNearlyEqual(Ambient, Other.Ambient, tl) &&
        isSmooth == Other.isSmooth &&
        hasMaterial == Other.hasMaterial &&
        TexturePath.Equals(Other.TexturePath);
}

FORCEINLINE uint32 GetTypeHash(const FSubMeshMaterialSet& Value)
{
    TArray<uint32> HashArray;
    HashArray.Add(FCrc::MemCrc32(&Value.Diffuse, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Specular, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Emissive, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Shininess, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.Transparency, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.Ambient, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.isSmooth, sizeof(bool)));
    HashArray.Add(FCrc::MemCrc32(&Value.TexturePath, sizeof(FString)));
    uint32 Hash = 0;
    for (auto h : HashArray)
    {
        Hash = HashCombine(Hash, h);
    }
    return Hash;
}

namespace
{
    void ComputeNormals(FStaticMeshAttributes& Attributes)
    {
        const auto Normals = Attributes.GetVertexInstanceNormals();
        const auto Indices = Attributes.GetVertexInstanceVertexIndices();
        const auto Vertices = Attributes.GetVertexPositions();

        const uint32 NumFaces = Indices.GetNumElements() / 3;
        for (uint32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
        {
            const int32 FaceOffset = FaceIndex * 3;

            FVector3f VertexPositions[3];
            int32 VertexIndices[3];

            // Retrieve vertex indices and positions
            VertexIndices[0] = Indices[FaceOffset];
            VertexPositions[0] = Vertices[VertexIndices[0]];

            VertexIndices[1] = Indices[FaceOffset + 1];
            VertexPositions[1] = Vertices[VertexIndices[1]];

            VertexIndices[2] = Indices[FaceOffset + 2];
            VertexPositions[2] = Vertices[VertexIndices[2]];

            // Calculate normal for triangle face			
            FVector3f N = FVector3f::CrossProduct((VertexPositions[0] - VertexPositions[1]),
                                                  (VertexPositions[0] - VertexPositions[2]));
            N.Normalize();

            // Unrolled loop
            Normals[FaceOffset + 0] += N;
            Normals[FaceOffset + 1] += N;
            Normals[FaceOffset + 2] += N;
        }

        for (int i = 0; i < Normals.GetNumElements(); ++i)
        {
            Normals[i].Normalize();
        }
    }

    bool ConvertMesh(const plateau::polygonMesh::Mesh& InMesh, FMeshDescription& OutMeshDescription,
                     TSet<FSubMeshMaterialSet>& SubMeshMaterialSets)
    {
        FStaticMeshAttributes Attributes(OutMeshDescription);

        // UVチャンネル数を3に設定
        const auto VertexInstanceUVs = Attributes.GetVertexInstanceUVs();
        if (VertexInstanceUVs.GetNumChannels() < 4)
        {
            VertexInstanceUVs.SetNumChannels(4);
        }

        const auto& InVertices = InMesh.getVertices();
        const auto& InIndices = InMesh.getIndices();

        const auto FaceCount = InIndices.size() / 3;
        // 同じ頂点は複数の面に利用されないように複製されるため、頂点数はインデックス数と同じサイズになる。
        const auto VertexCount = InIndices.size();

        OutMeshDescription.ReserveNewVertices(VertexCount);
        OutMeshDescription.ReserveNewPolygons(FaceCount);
        OutMeshDescription.ReserveNewVertexInstances(VertexCount);
        OutMeshDescription.ReserveNewEdges(VertexCount);

        const auto VertexPositions = Attributes.GetVertexPositions();
        for (const auto& Vertex : InVertices)
        {
            const auto VertexID = OutMeshDescription.CreateVertex();
            VertexPositions[VertexID] = FVector3f(Vertex.x, Vertex.y, Vertex.z);
        }

        // 頂点の再利用を防ぐため使用済みの頂点を保持
        TSet<unsigned> UsedVertexIDs;

        for (const auto& SubMesh : InMesh.getSubMeshes())
        {
            const auto& TexturePath = SubMesh.getTexturePath();
            const auto MaterialValue = SubMesh.getMaterial();
            FPolygonGroupID PolygonGroupID = 0;
            FSubMeshMaterialSet MaterialSet(MaterialValue,
                                            TexturePath.empty() ? FString() : FString(TexturePath.c_str()));

            if (!SubMeshMaterialSets.Contains(MaterialSet))
            {
                // マテリアル設定
                PolygonGroupID = OutMeshDescription.CreatePolygonGroup();
                FString MaterialName = "DefaultMaterial";
                if (TexturePath != "")
                {
                    MaterialName = FPaths::GetBaseFilename(UTF8_TO_TCHAR(TexturePath.c_str()));
                }
                else if (MaterialValue != nullptr)
                {
                    MaterialName = FString(MaterialValue->getId().c_str());
                }
                MaterialSet.PolygonGroupID = PolygonGroupID;
                MaterialSet.MaterialSlot = MaterialName;
                SubMeshMaterialSets.Add(MaterialSet);
            }
            else
            {
                FSubMeshMaterialSet* Found = SubMeshMaterialSets.Find(MaterialSet);
                check(Found != nullptr);
                PolygonGroupID = Found->PolygonGroupID;
                MaterialSet = *Found;
            }

            //BPのUStaticMeshDescriptionのSetPolygonGroupMaterialSlotNameと同様の処理
            if (OutMeshDescription.IsPolygonGroupValid(PolygonGroupID))
            {
                const FName& SlotName = *MaterialSet.MaterialSlot;
                OutMeshDescription.PolygonGroupAttributes().SetAttribute(
                    PolygonGroupID, MeshAttribute::PolygonGroup::ImportedMaterialSlotName, 0, SlotName);
            }

            // インデックス、UV設定
            const auto& StartIndex = SubMesh.getStartIndex();
            const auto& EndIndex = SubMesh.getEndIndex();
            TArray<FVertexInstanceID> VertexInstanceIDs;
            for (int InIndexIndex = StartIndex; InIndexIndex <= EndIndex; ++InIndexIndex)
            {
                auto VertexID = InIndices[InIndexIndex];

                // 頂点が使用済みの場合は複製
                if (UsedVertexIDs.Contains(VertexID))
                {
                    const auto NewVertexID = OutMeshDescription.CreateVertex();
                    VertexPositions[NewVertexID] = VertexPositions[VertexID];
                    VertexID = NewVertexID;
                }

                const auto NewVertexInstanceID = OutMeshDescription.CreateVertexInstance(VertexID);
                VertexInstanceIDs.Add(NewVertexInstanceID);

                const auto InUV1 = InMesh.getUV1()[InIndices[InIndexIndex]];
                const auto UV1 = FVector2f(InUV1.x, 1.0f - InUV1.y);
                VertexInstanceUVs.Set(NewVertexInstanceID, 0, UV1);

                const auto InUV4 = InMesh.getUV4()[InIndices[InIndexIndex]];
                const auto UV4 = FVector2f(InUV4.x, InUV4.y);
                VertexInstanceUVs.Set(NewVertexInstanceID, 3, UV4);

                UsedVertexIDs.Add(VertexID);
            }

            // 3頂点毎にPolygonを生成
            TArray<FVertexInstanceID> VertexInstanceIDsCache;
            VertexInstanceIDsCache.SetNumUninitialized(3);
            TArray<FVector3f> TriangleVerticesCache;
            TriangleVerticesCache.SetNumUninitialized(3);
            for (int32 TriangleIndex = 0; TriangleIndex < (EndIndex - StartIndex + 1) / 3; ++TriangleIndex)
            {
                FMemory::Memcpy(VertexInstanceIDsCache.GetData(), VertexInstanceIDs.GetData() + TriangleIndex * 3,
                                sizeof(FVertexInstanceID) * 3);

                // Invert winding order for triangles
                VertexInstanceIDsCache.Swap(0, 2);

                const FPolygonID NewPolygonID = OutMeshDescription.
                    CreatePolygon(PolygonGroupID, VertexInstanceIDsCache);
                // Fill in the polygon's Triangles - this won't actually do any polygon triangulation as we always give it triangles
                OutMeshDescription.ComputePolygonTriangulation(NewPolygonID);

                check(OutMeshDescription.GetPolygonPolygonGroup(NewPolygonID) == PolygonGroupID); //Polygon Group IDチェック
            }
        }

        ComputeNormals(Attributes);

        //Compact the MeshDescription, if there was visibility mask or some bounding box clip, it need to be compacted so the sparse array are from 0 to n with no invalid data in between. 
        FElementIDRemappings ElementIDRemappings;
        OutMeshDescription.Compact(ElementIDRemappings);

        return OutMeshDescription.Polygons().Num() > 0;
    }

    UStaticMesh* CreateStaticMesh(const plateau::polygonMesh::Mesh& InMesh, UObject* InOuter, FName Name)
    {
        const auto StaticMesh = NewObject<UStaticMesh>(InOuter, Name);

        StaticMesh->InitResources();
        // make sure it has a new lighting guid
        StaticMesh->SetLightingGuid();

        // Set it to use textured lightmaps. Note that Build Lighting will do the error-checking (texcoordindex exists for all LODs, etc).
        StaticMesh->SetLightMapResolution(64);
        StaticMesh->SetLightMapCoordinateIndex(1);

        FStaticMeshSourceModel& SrcModel = StaticMesh->AddSourceModel();
        /*Don't allow the engine to recalculate normals*/
        SrcModel.BuildSettings.bRecomputeNormals = false;
        SrcModel.BuildSettings.bRecomputeTangents = false;
        SrcModel.BuildSettings.bRemoveDegenerates = false;
        SrcModel.BuildSettings.bUseHighPrecisionTangentBasis = false;
        SrcModel.BuildSettings.bUseFullPrecisionUVs = false;
        SrcModel.BuildSettings.bBuildReversedIndexBuffer = false;

        return StaticMesh;
    }
}

void FPLATEAUMeshLoader::LoadModel(AActor* ModelActor, USceneComponent* ParentComponent,
                                   const std::shared_ptr<plateau::polygonMesh::Model> Model,
                                   const FLoadInputData& LoadInputData,
                                   const std::shared_ptr<const citygml::CityModel> CityModel, TAtomic<bool>* bCanceled)
{
    UE_LOG(LogTemp, Log, TEXT("Model->getRootNodeCount(): %d"), Model->getRootNodeCount());
    this->PathToTexture = FPathToTexture();
    for (int i = 0; i < Model->getRootNodeCount(); i++)
    {
        if (bCanceled->Load(EMemoryOrder::Relaxed))
            break;

        LoadNodeRecursive(ParentComponent, Model->getRootNodeAt(i), LoadInputData, CityModel, *ModelActor);

        // メッシュをワールド内にビルド
        const auto CopiedStaticMeshes = StaticMeshes;
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [CopiedStaticMeshes, &bCanceled]()
            {
                UStaticMesh::BatchBuild(CopiedStaticMeshes, true, [&bCanceled](UStaticMesh* mesh)
                {
                    return bCanceled->Load(EMemoryOrder::Relaxed);
                });
            }, TStatId(), nullptr, ENamedThreads::GameThread);
        StaticMeshes.Reset();
    }

    // 最大LOD以外の形状を非表示化
    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [ParentComponent]()
        {
            APLATEAUInstancedCityModel::FilterLowLods(ParentComponent);
        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();
}

void FPLATEAUMeshLoader::LoadNodeRecursive(
    USceneComponent* InParentComponent,
    const plateau::polygonMesh::Node& InNode,
    const FLoadInputData& InLoadInputData,
    const std::shared_ptr<const citygml::CityModel> InCityModel,
    AActor& InActor)
{
    UStaticMeshComponent* Component = LoadNode(InParentComponent, InNode, InLoadInputData, InCityModel, InActor);
    const size_t ChildNodeCount = InNode.getChildCount();
    for (int i = 0; i < ChildNodeCount; i++)
    {
        const auto& TargetNode = InNode.getChildAt(i);

        LoadNodeRecursive(Component, TargetNode, InLoadInputData, InCityModel, InActor);
    }
}

UStaticMeshComponent* FPLATEAUMeshLoader::CreateStaticMeshComponent(AActor& Actor, USceneComponent& ParentComponent,
                                                                    const plateau::polygonMesh::Mesh& InMesh,
                                                                    const FLoadInputData& LoadInputData,
                                                                    const std::shared_ptr<const citygml::CityModel>
                                                                    CityModel, const std::string& InNodeName)
{
    // コンポーネント作成
    const FString NodeName = UTF8_TO_TCHAR(InNodeName.c_str());
    UStaticMesh* StaticMesh;
    UStaticMeshComponent* Component = nullptr;
    UStaticMeshComponent* ComponentRef = nullptr;
    TSet<FSubMeshMaterialSet> SubMeshMaterialSets;
    FMeshDescription* MeshDescription;
    {
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [this, &LoadInputData, &InNodeName, &InMesh, &CityModel, &Component, &Actor, &StaticMesh, &MeshDescription,
                &NodeName]()
            {
                if (LoadInputData.bIncludeAttrInfo)
                {
                    const auto& PLATEAUCityObjectGroup = NewObject<UPLATEAUCityObjectGroup>(&Actor, NAME_None);
                    PLATEAUCityObjectGroup->SerializeCityObject(InNodeName, InMesh, LoadInputData, CityModel);
                    Component = PLATEAUCityObjectGroup;
                }
                else
                {
                    Component = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
                }

                if (bAutomationTest)
                {
                    Component->Mobility = EComponentMobility::Movable;
                }
                else
                {
                    Component->Mobility = EComponentMobility::Static;
                }
                Component->bVisualizeComponent = true;

                // StaticMesh作成
                StaticMesh = CreateStaticMesh(InMesh, Component, FName(NodeName));
                MeshDescription = StaticMesh->CreateMeshDescription(0);
            }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();
    }

    ConvertMesh(InMesh, *MeshDescription, SubMeshMaterialSets);

    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&StaticMesh]()
        {
            StaticMesh->CommitMeshDescription(0);
        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();

    StaticMeshes.Add(StaticMesh);
    StaticMesh->OnPostMeshBuild().AddLambda(
        [Component](UStaticMesh* Mesh)
        {
            if (Component == nullptr)
                return;
            Component->SetStaticMesh(Mesh);

            // Collision情報設定
            Mesh->CreateBodySetup();
            Mesh->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
            // レイキャスト時にブロック状態ではマルチヒットしない
            // ヒエラルキー上にLODが複数存在する場合は全てにレイキャスト結果がヒットするようオーバーラップする
            Component->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
        });

    const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&StaticMesh]
    {
        // ビルド前にImportVersionを設定する必要がある。
        StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

        // TODO: 適切なフラグの設定
        // https://docs.unrealengine.com/4.26/ja/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Objects/Creation/
        //StaticMesh->SetFlags();
    }, TStatId(), nullptr, ENamedThreads::GameThread);
    Task->Wait();

    //PolygonGroup数の整合性チェック
    check(SubMeshMaterialSets.Num() == MeshDescription->PolygonGroups().Num());

    const auto ComponentSetupTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&SubMeshMaterialSets, this, &Component, &StaticMesh, &MeshDescription, &Actor, &ParentComponent, &ComponentRef]
        {
            for (const auto& SubMeshValue : SubMeshMaterialSets)
            {
                UMaterialInstanceDynamic** SharedMatPtr = CachedMaterials.Find(SubMeshValue);
                if (SharedMatPtr == nullptr)
                {
                    // マテリアル作成
                    UMaterialInstanceDynamic* DynMaterial;
                    FString TexturePath = SubMeshValue.TexturePath;
                    UTexture2D* Texture;
                    if (TexturePath.IsEmpty())
                    {
                        Texture = nullptr;
                    }
                    else
                    {
                        const bool TextureInCache = PathToTexture.Contains(TexturePath);
                        if (TextureInCache) // テクスチャをすでにロード済みの場合、使い回します。
                        {
                            Texture = PathToTexture[TexturePath]; // nullptrの場合もあります。
                        }
                        else // テクスチャ未ロードの場合、ロードします。
                        {
                            Texture = FPLATEAUTextureLoader::Load(TexturePath);
                            // なければnullptrを返します。
                            PathToTexture.Add(TexturePath, Texture);
                        }
                    }

                    if (SubMeshValue.hasMaterial)
                    {
                        //Material情報が存在する場合
                        const auto SourceMaterialPath = SubMeshValue.Transparency > 0
                                                            ? TEXT(
                                                                "/PLATEAU-SDK-for-Unreal/Materials/PLATEAUX3DMaterial_Transparent")
                                                            : TEXT(
                                                                "/PLATEAU-SDK-for-Unreal/Materials/PLATEAUX3DMaterial");

                        UMaterial* Mat = Cast<UMaterial>(
                            StaticLoadObject(UMaterial::StaticClass(), nullptr, SourceMaterialPath));
                        DynMaterial = UMaterialInstanceDynamic::Create(Mat, Component);

                        DynMaterial->SetVectorParameterValue("BaseColor", SubMeshValue.Diffuse);
                        DynMaterial->SetVectorParameterValue("EmissiveColor", SubMeshValue.Emissive);
                        DynMaterial->SetVectorParameterValue("SpecularColor", SubMeshValue.Specular);
                        DynMaterial->SetScalarParameterValue("Ambient", SubMeshValue.Ambient);
                        DynMaterial->SetScalarParameterValue("Shininess", SubMeshValue.Shininess);
                        DynMaterial->SetScalarParameterValue("Transparency", SubMeshValue.Transparency);

                        //base color とスペキュラの R:G:B の比率がほぼ同じ場合
                        if (FMath::IsNearlyEqual(SubMeshValue.Diffuse.X, SubMeshValue.Diffuse.Y)
                            && FMath::IsNearlyEqual(SubMeshValue.Diffuse.X, SubMeshValue.Diffuse.Z)
                            && FMath::IsNearlyEqual(SubMeshValue.Specular.X, SubMeshValue.Specular.Y)
                            && FMath::IsNearlyEqual(SubMeshValue.Specular.X, SubMeshValue.Specular.Z))
                            DynMaterial->SetScalarParameterValue("Specular/Metallic", 1.0f);
                    }
                    else
                    {
                        //デフォルトマテリアル設定
                        const auto SourceMaterialPath =
                            Texture != nullptr
                                ? TEXT("/PLATEAU-SDK-for-Unreal/Materials/DefaultMaterial")
                                : TEXT("/PLATEAU-SDK-for-Unreal/Materials/DefaultMaterial_No_Texture");
                        UMaterial* Mat = Cast<UMaterial>(
                            StaticLoadObject(UMaterial::StaticClass(), nullptr, SourceMaterialPath));
                        DynMaterial = UMaterialInstanceDynamic::Create(Mat, Component);
                    }
                    //Textureが存在する場合
                    if (Texture != nullptr)
                        DynMaterial->SetTextureParameterValue("Texture", Texture);

                    DynMaterial->TwoSided = false;
                    StaticMesh->AddMaterial(DynMaterial);
                    //Materialをキャッシュに保存
                    CachedMaterials.Add(SubMeshValue, DynMaterial);

                    //SubMeshのPolygonGroupIDとMeshDescriptionのPolygonGroupIDの整合性チェック
                    TAttributesSet<FPolygonGroupID> PolygonGroupAttributes = MeshDescription->PolygonGroupAttributes();
                    if (PolygonGroupAttributes.HasAttribute(MeshAttribute::PolygonGroup::ImportedMaterialSlotName))
                    {
                        FName AttributeValue = PolygonGroupAttributes.GetAttribute<FName>(
                            SubMeshValue.PolygonGroupID, MeshAttribute::PolygonGroup::ImportedMaterialSlotName, 0);
                        check(SubMeshValue.MaterialSlot == AttributeValue.ToString());
                    }
                }
                else
                {
                    //キャッシュのMaterialを使用
                    StaticMesh->AddMaterial(*SharedMatPtr);
                }
            }

            // 名前設定、ヒエラルキー設定など
            Component->DepthPriorityGroup = SDPG_World;
            FString NewUniqueName = StaticMesh->GetName();
            if (!Component->Rename(*NewUniqueName, nullptr, REN_Test))
            {
                NewUniqueName = MakeUniqueObjectName(&Actor, UPLATEAUCityObjectGroup::StaticClass(),
                                                     FName(StaticMesh->GetName())).ToString();
            }
            Component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);
            Actor.AddInstanceComponent(Component);
            Component->RegisterComponent();
            Component->AttachToComponent(&ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
            Component->PostEditChange();
            ComponentRef = Component;
        }, TStatId(), nullptr, ENamedThreads::GameThread);
    ComponentSetupTask->Wait();

    return ComponentRef;
}

UStaticMeshComponent* FPLATEAUMeshLoader::LoadNode(USceneComponent* ParentComponent,
                                                   const plateau::polygonMesh::Node& Node,
                                                   const FLoadInputData& LoadInputData,
                                                   const std::shared_ptr<const citygml::CityModel> CityModel,
                                                   AActor& Actor)
{
    if (Node.getMesh() == nullptr)
    {
        const auto& CityObject = CityModel->getCityObjectById(Node.getName());
        UStaticMeshComponent* Comp = nullptr;
        UClass* StaticClass;
        const FString DesiredName = FString(UTF8_TO_TCHAR(Node.getName().c_str()));
        const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&, DesiredName]
        {
            // CityObjectがある場合はUPLATEAUCityObjectGroupとする
            if (CityObject != nullptr && LoadInputData.bIncludeAttrInfo)
            {
                StaticClass = UPLATEAUCityObjectGroup::StaticClass();
                const auto& PLATEAUCityObjectGroup = NewObject<UPLATEAUCityObjectGroup>(&Actor, NAME_None);
                PLATEAUCityObjectGroup->SerializeCityObject(Node, CityObject);
                Comp = PLATEAUCityObjectGroup;
            }
            else
            {
                StaticClass = UStaticMeshComponent::StaticClass();
                Comp = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
            }
            FString NewUniqueName = FString(DesiredName);
            if (!Comp->Rename(*NewUniqueName, nullptr, REN_Test))
            {
                NewUniqueName = MakeUniqueObjectName(&Actor, StaticClass, FName(DesiredName)).ToString();
            }
            Comp->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

            check(Comp != nullptr);
            if (bAutomationTest)
            {
                Comp->Mobility = EComponentMobility::Movable;
            }
            else
            {
                Comp->Mobility = EComponentMobility::Static;
            }

            Actor.AddInstanceComponent(Comp);
            Comp->RegisterComponent();
            Comp->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
        }, TStatId(), nullptr, ENamedThreads::GameThread);
        Task->Wait();
        return Comp;
    }

    // TODO: 空のMeshが入っている問題
    if (Node.getMesh()->getVertices().size() == 0)
        return nullptr;

    return CreateStaticMeshComponent(Actor, *ParentComponent, *Node.getMesh(), LoadInputData, CityModel,
                                     Node.getName());
}

#endif
