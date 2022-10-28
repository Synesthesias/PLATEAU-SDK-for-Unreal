#include "PLATEAUMeshLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "citygml/citygml.h"

#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "ImageUtils.h"

namespace {
    void ComputeFlatNormals(const TArray<uint32_t>& Indices, TArray<FStaticMeshBuildVertex>& Vertices) {
        for (int i = 0; i < Indices.Num(); i += 3) {
            //法線計算
            FStaticMeshBuildVertex& V0 = Vertices[Indices[i]];
            FStaticMeshBuildVertex& V1 = Vertices[Indices[i + 1]];
            FStaticMeshBuildVertex& V2 = Vertices[Indices[i + 2]];
            FVector3f V01 = V1.Position - V0.Position;
            FVector3f V02 = V2.Position - V0.Position;
            FVector3f Normal = FVector3f::CrossProduct(V01, V02);
            V0.TangentX = V1.TangentX = V2.TangentX = FVector3f(0.0f);
            V0.TangentY = V1.TangentY = V2.TangentY = FVector3f(0.0f);
            V0.TangentZ = V1.TangentZ = V2.TangentZ = Normal.GetSafeNormal();
        }
    }
}

void FPLATEAUMeshLoader::CreateMesh(AActor* ModelActor, USceneComponent* ParentComponent, std::shared_ptr<plateau::polygonMesh::Model> ModelData) {
    auto Result = Async(EAsyncExecution::Thread,
        [=] {
            for (int i = 0; i < ModelData->getRootNodeCount(); i++) {
                LoadNodes_InModel(ParentComponent, &ModelData->getRootNodeAt(i), *ModelActor);
            }
        });
}

void FPLATEAUMeshLoader::LoadNodes_InModel(USceneComponent* ParentComponent, const plateau::polygonMesh::Node* Node, AActor& Actor) {
    USceneComponent* Comp = nullptr;
    if (Node->getMesh() == std::nullopt) {
        //SceneComponentを付与
        FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&] {
                Comp = NewObject<USceneComponent>(&Actor, NAME_None);
                const auto DesiredName = UTF8_TO_TCHAR(Node->getName().c_str());
                FString NewUniqueName = DesiredName;
                if (!Comp->Rename(*NewUniqueName, nullptr, REN_Test)) {
                    NewUniqueName = MakeUniqueObjectName(&Actor, USceneComponent::StaticClass(), FName(DesiredName)).ToString();
                }
                Comp->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

                check(Comp != nullptr);
                Comp->Mobility = EComponentMobility::Static;
                Actor.AddInstanceComponent(Comp);
                Comp->RegisterComponent();
                Comp->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
            }, TStatId(), nullptr, ENamedThreads::GameThread);
        Task->Wait();
    } else {
        // TODO: 空のMeshが入っている問題
        if (Node->getMesh()->getVertices().size() == 0)
            return;

        //StaticMeshComponentを付与
        TArray<FVector> VertArr;
        for (int j = 0; j < Node->getMesh()->getVertices().size(); j++) {
            VertArr.Add(FVector(Node->getMesh()->getVertices()[j].x, Node->getMesh()->getVertices()[j].y, Node->getMesh()->getVertices()[j].z));
        }

        //テクスチャ読み込み(無ければnullptrを入れる)
        TArray<UTexture2D*> SubmeshTextures;
        for (const auto& SubMesh : Node->getMesh()->getSubMeshes()) {
            FString TexturePath = UTF8_TO_TCHAR(SubMesh.getTexturePath().c_str());

            FGraphEventRef Result = FFunctionGraphTask::CreateAndDispatchWhenReady(
                [&] {
                    SubmeshTextures.Add(LoadTextureFromPath(TexturePath));
                }, TStatId(), nullptr, ENamedThreads::GameThread);
            Result->Wait();
        }

        std::vector UVs = { Node->getMesh()->getUV1(), Node->getMesh()->getUV2(), Node->getMesh()->getUV3() };
        const std::vector<int> Indices = Node->getMesh()->getIndices();

        const FString CompName = UTF8_TO_TCHAR(Node->getName().c_str());
        Comp = CreateStaticMeshComponent(
            Actor, *ParentComponent, Indices, VertArr,
            CompName, SubmeshTextures, Node->getMesh()->getSubMeshes(), UVs);
    }

    TArray<TFuture<void>> Results;
    for (int i = 0; i < Node->getChildCount(); i++) {
        const auto& TargetNode = Node->getChildAt(i);

        // TODO: NodeのLifetimeへの依存解消
        Results.Add(Async(EAsyncExecution::Thread,
            [this, Comp, &TargetNode, &Actor] {
                LoadNodes_InModel(Comp, &TargetNode, Actor);
            }));
    }

    for (const auto& Result : Results) {
        Result.Wait();
    }
}

UStaticMeshComponent* FPLATEAUMeshLoader::CreateStaticMeshComponent(
    AActor& Actor, USceneComponent& ParentComponent,
    const std::vector<int>& Indices,
    const TArray<FVector>& Vertices,
    FString Name,
    const TArray<UTexture2D*>& SubmeshTextures,
    const std::vector<plateau::polygonMesh::SubMesh>& SubMeshes,
    const std::vector<std::vector<TVec2f>>& UVs) {
    UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent Start-----"));

    // RenderData作成(ここは非同期で出来るはず)
    auto RenderData = CreateRenderData(Indices, Vertices, UVs[0], UVs[1], UVs[2], SubMeshes);;

    // コンポーネント作成
    UStaticMeshComponent* ComponentRef = nullptr;
    const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&] {
        const auto Component = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
        Component->Mobility = EComponentMobility::Static;
        Component->bVisualizeComponent = true;

        // StaticMesh、Material作成
        const auto StaticMesh = NewObject<UStaticMesh>(Component, FName(Name));
        Component->SetStaticMesh(StaticMesh);
        SetRenderData(StaticMesh, RenderData);

        // TODO: 適切なフラグの設定
        // https://docs.unrealengine.com/4.26/ja/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Objects/Creation/
        //StaticMesh->SetFlags();

        for (const auto& Texture : SubmeshTextures) {
            UMaterial* Mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PlateauSDK/DefaultMaterial")));
            UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Mat, Component);
            if (Texture != nullptr) {
                DynMaterial->SetTextureParameterValue("Texture", Texture);
            }
            DynMaterial->TwoSided = false;
            StaticMesh->AddMaterial(DynMaterial);
        }

        // StaticMeshセットアップ
        StaticMesh->NeverStream = true;
        StaticMesh->InitResources();
        StaticMesh->CalculateExtendedBounds();
        StaticMesh->GetRenderData()->ScreenSize[0].Default = 1.0f;
        StaticMesh->CreateBodySetup();

        // 名前設定、ヒエラルキー設定など
        Component->DepthPriorityGroup = SDPG_World;
        FString NewUniqueName = StaticMesh->GetName();
        if (!Component->Rename(*NewUniqueName, nullptr, REN_Test)) {
            NewUniqueName = MakeUniqueObjectName(&Actor, USceneComponent::StaticClass(), FName(StaticMesh->GetName())).ToString();
        }
        Component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);
        Actor.AddInstanceComponent(Component);
        Component->RegisterComponent();
        Component->AttachToComponent(&ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
        Component->PostEditChange();
        ComponentRef = Component;
        UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent End-----"));
        }, TStatId(), nullptr, ENamedThreads::GameThread);
    Task->Wait();

    return ComponentRef;
}

TUniquePtr<FStaticMeshRenderData> FPLATEAUMeshLoader::CreateRenderData(
    const std::vector<int>& InIndicesVector, const TArray<FVector>& VerticesArray,
    const std::vector<TVec2f>& UV1, const std::vector<TVec2f>& UV2, const std::vector<TVec2f>& UV3,
    const std::vector<plateau::polygonMesh::SubMesh>& SubMeshes
) {
    //RenderData生成
    auto RenderData = MakeUnique<FStaticMeshRenderData>();
    RenderData->AllocateLODResources(1);
    FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
    LODResources.bHasColorVertexData = false;

    //頂点、インデックス、UVなどの反映
    TArray<uint32> indices;
    indices.SetNum(static_cast<TArray<uint32>::SizeType>(InIndicesVector.size()));
    for (int32 i = 0; i < InIndicesVector.size(); ++i) {
        indices[i] = InIndicesVector[i];
    }
    TArray<FStaticMeshBuildVertex> StaticMeshBuildVertices;
    StaticMeshBuildVertices.SetNum(VerticesArray.Num());
    constexpr double Infinity = std::numeric_limits<double>::max();
    FVector MinPosition(Infinity, Infinity, Infinity);
    FVector MaxPosition(-Infinity, -Infinity, -Infinity);
    for (int i = 0; i < StaticMeshBuildVertices.Num(); ++i) {
        auto& Vertex = StaticMeshBuildVertices[i];
        Vertex.Position = FVector3f(VerticesArray[i]);
        // UEだと恐らくUV原点が画像左上
        Vertex.UVs[0] = FVector2f(UV1[i].x, 1.0f - UV1[i].y);
        Vertex.UVs[1] = FVector2f(UV2[i].x, UV2[i].y);
        Vertex.UVs[2] = FVector2f(UV3[i].x, UV3[i].y);

        MinPosition.X = FMath::Min(Vertex.Position.X, MinPosition.X);
        MinPosition.Y = FMath::Min(Vertex.Position.Y, MinPosition.Y);
        MinPosition.Z = FMath::Min(Vertex.Position.Z, MinPosition.Z);
        MaxPosition.X = FMath::Max(Vertex.Position.X, MaxPosition.X);
        MaxPosition.Y = FMath::Max(Vertex.Position.Y, MaxPosition.Y);
        MaxPosition.Z = FMath::Max(Vertex.Position.Z, MaxPosition.Z);
    }

    // Bounds
    RenderData->Bounds.SphereRadius = 0.0f;
    RenderData->Bounds.Origin = (MinPosition + MaxPosition) / 2.0;
    RenderData->Bounds.BoxExtent = MaxPosition - MinPosition;

    ComputeFlatNormals(indices, StaticMeshBuildVertices);
    LODResources.VertexBuffers.PositionVertexBuffer.Init(StaticMeshBuildVertices, false);
    LODResources.VertexBuffers.StaticMeshVertexBuffer.Init(StaticMeshBuildVertices, 1, false);
    for (int i = 2; i < indices.Num(); i += 3) {
        std::swap(indices[i - 2], indices[i]);
    }

    LODResources.Sections.Reset();
#if ENGINE_MAJOR_VERSION == 5
    FStaticMeshSectionArray& Sections = LODResources.Sections;
#else
    FStaticMeshLODResources::FStaticMeshSectionArray& Sections =
        LODResources.Sections;
#endif

    //セクションへのデータ設定
    for (int MaterialIndex = 0; MaterialIndex < SubMeshes.size(); ++MaterialIndex) {
        const auto SubMesh = SubMeshes[MaterialIndex];
        const auto StartIndex = SubMesh.getStartIndex();
        const auto EndIndex = SubMesh.getEndIndex();
        const auto IndexCount = EndIndex - StartIndex + 1;

        FStaticMeshSection& Section = Sections.AddDefaulted_GetRef();
        Section.bEnableCollision = false;
        Section.NumTriangles = IndexCount / 3;
        Section.FirstIndex = StartIndex;

        // インデックスの最小、最大値
        Section.MinVertexIndex = InIndicesVector[StartIndex];
        Section.MaxVertexIndex = InIndicesVector[StartIndex];
        for (int i = StartIndex + 1; i <= EndIndex; ++i) {
            Section.MinVertexIndex = FMath::Min<uint32>(InIndicesVector[i], Section.MinVertexIndex);
            Section.MaxVertexIndex = FMath::Max<uint32>(InIndicesVector[i], Section.MaxVertexIndex);
        }

        Section.bEnableCollision = true;
        Section.bCastShadow = true;
        Section.MaterialIndex = MaterialIndex;
    }

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

    UE_LOG(LogTemp, Log, TEXT("-----CreateRenderData End-----"));
    return RenderData;
}

void FPLATEAUMeshLoader::SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData) {
    //レンダーデータを設定
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

UTexture2D* FPLATEAUMeshLoader::LoadTextureFromPath(const FString& Path) {
    if (Path.IsEmpty()) return nullptr;
    // TODO: 非同期化
    return FImageUtils::ImportFileAsTexture2D(Path);
}
