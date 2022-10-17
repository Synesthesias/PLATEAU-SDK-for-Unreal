#include "PLATEAUMeshLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "citygml/citygml.h"

#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "ImageUtils.h"

void FPLATEAUMeshLoader::CreateMesh(AActor* ModelActor, std::shared_ptr<plateau::polygonMesh::Model> ModelData)
{
	for (int i = 0; i < ModelData->getRootNodeCount(); i++)
	{
		LoadNodes_InModel(ModelActor->GetRootComponent(), &ModelData->getRootNodeAt(i), *ModelActor, i, 0);
	}
}

void FPLATEAUMeshLoader::LoadNodes_InModel(USceneComponent* ParentComponent, plateau::polygonMesh::Node* Node, AActor& Actor, int Index, int Count)
{
    USceneComponent* Comp = nullptr;
    FString CompName;

    UE_LOG(LogTemp, Log, TEXT("LoadNodes_InModel NodeName : %s"), Node->getName().c_str());

    if (Node->getMesh() == std::nullopt)
    {
        //SceneComponentを付与
        Comp = NewObject<USceneComponent>(&Actor, FName(Node->getName().c_str()));
        UE_LOG(LogTemp, Log, TEXT("Node doesn't have Mesh"));
        check(Comp != nullptr);
        Comp->Mobility = EComponentMobility::Static;
        Actor.AddInstanceComponent(Comp);
        Comp->RegisterComponent();
        Comp->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        //StaticMeshComponentを付与
        CompName = "Mesh" + FString::FromInt(Index) + "_" + FString::FromInt(Count);
        TArray<FVector> VertArr;
        for (int j = 0; j < Node->getMesh()->getVertices().size(); j++)
        {
            VertArr.Add(FVector(Node->getMesh()->getVertices()[j].x, Node->getMesh()->getVertices()[j].y, Node->getMesh()->getVertices()[j].z));
            //UE_LOG(LogTemp, Log, TEXT("vertArr[%d] position : %s"), j, *VertArr[j].ToString());
        }
#if false
        for (int k = 0; k < Node->getMesh()->getIndices().size(); k++)
        {
            UE_LOG(LogTemp, Log, TEXT("Indices[%d] value : %d"), k, Node->getMesh()->getIndices()[k]);
        }
#endif
        UE_LOG(LogTemp, Log, TEXT("Node has Mesh"));
        //テクスチャ読み込み
        FString TexturePath;
        for (int index = 0; index <  Node->getMesh()->getSubMeshes().size(); index++)
        {
            if (Node->getMesh()->getSubMeshes()[index].getTexturePath() != "")
            {
                //そのまま引っ張ってくると文字化けしているので文字コード変換処理
                std::size_t converted{};
                std::string src = Node->getMesh()->getSubMeshes()[index].getTexturePath();
                std::vector<wchar_t> dest(src.size(), L'\0');
                if (::_mbstowcs_s_l(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE, ::_create_locale(LC_ALL, "jpn")) != 0) {
                    //必要ならエラーチェックなど
                }
                dest.resize(std::char_traits<wchar_t>::length(dest.data()));
                dest.shrink_to_fit();
                std::wstring text = std::wstring(dest.begin(), dest.end()) + L"f";
                UE_LOG(LogTemp, Log, TEXT("Index : %d , Texture Path : %s"), index, text.c_str());
                TexturePath = FString(text.c_str());
            }
        }
        UTexture2D* Texture = LoadTextureFromPath(TexturePath);

        std::vector<TVec2f> UVs[3] = { Node->getMesh()->getUV1(), Node->getMesh()->getUV2(), Node->getMesh()->getUV3()};
        Comp = CreateStaticMeshComponent(Actor, *ParentComponent, Node->getMesh()->getIndices(), VertArr, CompName, Texture, UVs);
        if(Texture != nullptr)
        UE_LOG(LogTemp, Log, TEXT("Texture Size : %d , %d"), Texture->GetSizeX() , Texture->GetSizeY());
    }

    for (int i = 0; i < Node->getChildCount(); i++)
    {
        ++Count;
        LoadNodes_InModel(Comp, &Node->getChildAt(i), Actor, Index, Count);
    }
}

UStaticMeshComponent* FPLATEAUMeshLoader::CreateStaticMeshComponent(AActor& Actor, USceneComponent& ParentComponent, std::vector<int> Indices, TArray<FVector> Vertices, 
    FString Name, UTexture2D* Texture, std::vector<TVec2f> UVs[])
{
    UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent Start-----"));
    // RenderData作成(ここは非同期で出来るはず)
    auto RenderData = CreateRenderData(Indices, Vertices, UVs);

    // コンポーネント作成
    const auto Component = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
    Component->Mobility = EComponentMobility::Static;
    Component->bVisualizeComponent = true;

    // StaticMesh作成
    const auto StaticMesh = NewObject<UStaticMesh>(Component, FName(Name));
    Component->SetStaticMesh(StaticMesh);
    SetRenderData(StaticMesh, RenderData);
    StaticMesh->SetFlags(
        RF_Transient | RF_DuplicateTransient | RF_TextExportTransient);
    //UMaterial* Mat = UMaterial::GetDefaultMaterial(MD_Surface);
    UMaterial* Mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PlateauSDK/DefaultMaterial")));
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Mat, Component);
    if (Texture != nullptr)
    {
        DynMaterial->SetTextureParameterValue("Texture", Texture);
    }
    DynMaterial->TwoSided = false;
    StaticMesh->AddMaterial(DynMaterial);
    // RenderData適用
    StaticMesh->NeverStream = true;
    StaticMesh->InitResources();
    StaticMesh->CalculateExtendedBounds();
    StaticMesh->GetRenderData()->ScreenSize[0].Default = 1.0f;
    StaticMesh->CreateBodySetup();

    // 名前設定、ヒエラルキー設定など
    Component->DepthPriorityGroup = SDPG_World;
    // TODO: SetStaticMeshComponentOverrideMaterial(StaticMeshComponent, NodeInfo);
    FString NewUniqueName = StaticMesh->GetName();
    if (!Component->Rename(*NewUniqueName, nullptr, REN_Test)) {
        NewUniqueName = MakeUniqueObjectName(&Actor, USceneComponent::StaticClass(), FName(StaticMesh->GetName())).ToString();
    }
    Component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);
    Actor.AddInstanceComponent(Component);
    Component->RegisterComponent();
    Component->AttachToComponent(&ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
    Component->PostEditChange();
    UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent End-----"));
    return Component;
}

TUniquePtr<FStaticMeshRenderData> FPLATEAUMeshLoader::CreateRenderData(std::vector<int> InIndicesVector, TArray<FVector> VerticesArray, std::vector<TVec2f> UVs[])
{
    UE_LOG(LogTemp, Log, TEXT("-----CreateRenderData Start-----"));
    std::vector<int> InIndices = InIndicesVector;
    TArray<FVector> Vertices = VerticesArray;


    //デバッグ用 以下を有効化すると三角ポリゴンがちゃんと出る
#if false
    InIndices.clear();
    InIndices.push_back(0);
    InIndices.push_back(1);
    InIndices.push_back(2);
    vertices.Empty();
    vertices.Add(FVector(100, 0, 0));
    vertices.Add(FVector(0, 0, 0));
    vertices.Add(FVector(0, 0, 100));
#endif


    UE_LOG(LogTemp, Log, TEXT("InIndices size : %zu"), InIndices.size());
    UE_LOG(LogTemp, Log, TEXT("vertices size : %zu"), Vertices.Num());

    auto RenderData = MakeUnique<FStaticMeshRenderData>();
    RenderData->AllocateLODResources(1);
    FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
    LODResources.bHasColorVertexData = false;

    //TODO: AABB

    TArray<uint32> indices;
    indices.SetNum(static_cast<TArray<uint32>::SizeType>(InIndices.size()));

    for (int32 i = 0; i < InIndices.size(); ++i) {
        indices[i] = InIndices[i];
    }

    TArray<FStaticMeshBuildVertex> StaticMeshBuildVertices;
    StaticMeshBuildVertices.SetNum(Vertices.Num());

    for (int i = 0; i < StaticMeshBuildVertices.Num(); ++i) {
        auto& Vertex = StaticMeshBuildVertices[i];
        //uint32 vertexIndex = indices[i];
        Vertex.Position = FVector3f(Vertices[i]);
        //UE_LOG(LogTemp, Log, TEXT("Vertex position : %s"), *Vertex.Position.ToString());
        Vertex.UVs[0] = FVector2f(UVs[0][i].x, UVs[0][i].y);
        Vertex.UVs[1] = FVector2f(UVs[1][i].x, UVs[1][i].y);
        Vertex.UVs[2] = FVector2f(UVs[2][i].x, UVs[2][i].y);
        RenderData->Bounds.SphereRadius = FMath::Max(
            (Vertex.Position - FVector3f(RenderData->Bounds.Origin)).Size(),
            RenderData->Bounds.SphereRadius);
        //RenderData->Bounds.Origin = FVector(0, 0, 0);
        RenderData->Bounds.BoxExtent = FVector(10000000000000, 10000000000000, 10000000000000);
    }

    computeFlatNormals(indices, StaticMeshBuildVertices);

    LODResources.VertexBuffers.PositionVertexBuffer.Init(StaticMeshBuildVertices, false);
    FColorVertexBuffer& ColorVertexBuffer = LODResources.VertexBuffers.ColorVertexBuffer;
    //テクスチャ貼るときに変えないといけないかも
    LODResources.VertexBuffers.StaticMeshVertexBuffer.Init(StaticMeshBuildVertices, 1, false);
    for (int i = 2; i < indices.Num(); i += 3) {
        std::swap(indices[i - 2], indices[i]);
    }
#if ENGINE_MAJOR_VERSION == 5
    FStaticMeshSectionArray& Sections = LODResources.Sections;
#else
    FStaticMeshLODResources::FStaticMeshSectionArray& Sections =
        LODResources.Sections;
#endif

    FStaticMeshSection& section = Sections.AddDefaulted_GetRef();
    section.bEnableCollision = false;

    section.NumTriangles = indices.Num() / 3;
    section.FirstIndex = 0;
    section.MinVertexIndex = 0;
    section.MaxVertexIndex = StaticMeshBuildVertices.Num() - 1;
    section.bEnableCollision = true;
    section.bCastShadow = true;
    section.MaterialIndex = 0;

    LODResources.IndexBuffer.SetIndices(indices,
        StaticMeshBuildVertices.Num() >= std::numeric_limits<uint16>::max()
        ? EIndexBufferStride::Type::Force32Bit
        : EIndexBufferStride::Type::Force16Bit);

    LODResources.bHasDepthOnlyIndices = false;
    LODResources.bHasReversedIndices = false;
    LODResources.bHasReversedDepthOnlyIndices = false;

#if ENGINE_MAJOR_VERSION < 5
    LODResources.bHasAdjacencyInfo = false;
#endif

    // TODO: テクスチャ適用

    //std::unordered_map<uint32_t, uint32_t>& textureCoordinateMap;
    // baseColorTexture = loadTexture(model, pbrMetallicRoughness.baseColorTexture, true);

    //primitiveResult
    //    .textureCoordinateParameters["baseColorTextureCoordinateIndex"] =
    //    updateTextureCoordinates(
    //        model,
    //        primitive,
    //        duplicateVertices,
    //        StaticMeshBuildVertices,
    //        indices,
    //        pbrMetallicRoughness.baseColorTexture,
    //        textureCoordinateMap);

    UE_LOG(LogTemp, Log, TEXT("-----CreateRenderData End-----"));
    return RenderData;
}

void FPLATEAUMeshLoader::SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27
    StaticMesh->bIsBuiltAtRuntime = true;
    StaticMesh->RenderData = std::move(RenderData);
#elif ENGINE_MAJOR_VERSION == 4
    StaticMesh->SetIsBuiltAtRuntime(true);
    StaticMesh->SetRenderData(std::move(RenderData));
#else // UE5
    StaticMesh->SetRenderData(std::move(RenderData));
#endif
}

void FPLATEAUMeshLoader::computeFlatNormals(const TArray<uint32_t>& Indices, TArray<FStaticMeshBuildVertex>& Vertices)
{
    for (int i = 0; i < Indices.Num(); i += 3) {
        int acc[3] = { Indices[i],Indices[i + 1],Indices[i + 2] };
        FStaticMeshBuildVertex& v0 = Vertices[acc[0]];
        FStaticMeshBuildVertex& v1 = Vertices[acc[1]];
        FStaticMeshBuildVertex& v2 = Vertices[acc[2]];

        FVector3f v01 = v1.Position - v0.Position;
        FVector3f v02 = v2.Position - v0.Position;
        FVector3f normal = FVector3f::CrossProduct(v01, v02);

        v0.TangentX = v1.TangentX = v2.TangentX = FVector3f(0.0f);
        v0.TangentY = v1.TangentY = v2.TangentY = FVector3f(0.0f);
        v0.TangentZ = v1.TangentZ = v2.TangentZ = normal.GetSafeNormal();
    }
}

UTexture2D* FPLATEAUMeshLoader::LoadTextureFromPath(const FString& Path)
{
    if (Path.IsEmpty()) return NULL; 

    return FImageUtils::ImportFileAsTexture2D(Path);
}

