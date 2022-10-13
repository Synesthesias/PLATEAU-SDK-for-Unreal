// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "citygml/citygml.h"

#include "Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"

using namespace plateau::udx;
using namespace plateau::polygonMesh;

APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
}

void APLATEAUCityModelLoader::Load() {
#if WITH_EDITOR
    UE_LOG(LogTemp, Log, TEXT("-----Load Start-----"));
    // 仮の範囲情報(53392642の地域メッシュ)
    Extent.Min.Latitude = 35.5335751;
    Extent.Min.Longitude = 139.7755041;
    Extent.Min.Height = -10000;
    Extent.Max.Latitude = 35.54136964;
    Extent.Max.Longitude = 139.78712557;
    Extent.Max.Height = 10000;

    // GeoReferenceを選択範囲の中心に更新
    const auto MinPoint = GeoReference.GetData().project(Extent.GetNativeData().min);
    const auto MaxPoint = GeoReference.GetData().project(Extent.GetNativeData().max);
    const auto NativeReferencePoint = (MinPoint + MaxPoint) / 2.0;
    GeoReference.ReferencePoint.X += NativeReferencePoint.x;
    GeoReference.ReferencePoint.Y += NativeReferencePoint.y;
    GeoReference.ReferencePoint.Z += NativeReferencePoint.z;

    // ファイル検索
    const auto UdxFileCollection =
        UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
        ->filter(Extent.GetNativeData());
    const auto GmlFiles = UdxFileCollection->getGmlFiles(PredefinedCityModelPackage::Building);

    if (GmlFiles->size() == 0)
        return;
    UE_LOG(LogTemp, Log, TEXT("GmlFiles size : %zu"), GmlFiles->size());

    // 都市モデルパース
    citygml::ParserParams ParserParams;
    ParserParams.tesselate = true;
    const auto CityModel = citygml::load(*GmlFiles->begin(), ParserParams);
    UE_LOG(LogTemp, Log, TEXT("CityModel ID : %s"), CityModel->getId().c_str());

    // ポリゴンメッシュ抽出
    const MeshExtractOptions MeshExtractOptions(
        GeoReference.GetData().getReferencePoint(), CoordinateSystem::NWU,
        MeshGranularity::PerPrimaryFeatureObject,
        3, 0, true,
        1, 0.01, GeoReference.ZoneID, Extent.GetNativeData());
    const auto Model = MeshExtractor::extract(*CityModel, MeshExtractOptions);
    UE_LOG(LogTemp, Log, TEXT("Model RootNode Count : %d"), Model->getRootNodeCount());

    // アクター生成
    AActor* modelActor = GetWorld()->SpawnActor<AActor>(FVector(0, 0, 0), FRotator(0, 0, 0));
    CreateRootComponent(*modelActor);
    modelActor->SetActorLabel("Model");

    //ノード走査
    for (int i = 0; i < Model->getRootNodeCount(); i++)
    {
        LoadNodes_InModel(modelActor->GetRootComponent(), &Model->getRootNodeAt(i), *modelActor, i, 0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("-----Load End-----"));
#endif
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();

}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

void APLATEAUCityModelLoader::CreateRootComponent(AActor& Actor) {
    USceneComponent* ActorRootComponent = NewObject<USceneComponent>(&Actor,
        USceneComponent::GetDefaultSceneRootVariableName());

    check(ActorRootComponent != nullptr);
    ActorRootComponent->Mobility = EComponentMobility::Static;
    ActorRootComponent->bVisualizeComponent = true;
    Actor.SetRootComponent(ActorRootComponent);
    Actor.AddInstanceComponent(ActorRootComponent);
    ActorRootComponent->RegisterComponent();
    Actor.SetFlags(RF_Transactional);
    ActorRootComponent->SetFlags(RF_Transactional);
    GEngine->BroadcastLevelActorListChanged();
}

TUniquePtr<FStaticMeshRenderData> APLATEAUCityModelLoader::CreateRenderData(std::vector<int> _inIndices, TArray<FVector> _vertices) {
    UE_LOG(LogTemp, Log, TEXT("-----CreateRenderData Start-----"));
    std::vector<int> InIndices = _inIndices;
    TArray<FVector> vertices = _vertices;

    
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
    UE_LOG(LogTemp, Log, TEXT("vertices size : %zu"), vertices.Num());

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
    StaticMeshBuildVertices.SetNum(vertices.Num());

    for (int i = 0; i < StaticMeshBuildVertices.Num(); ++i) {
        auto& Vertex = StaticMeshBuildVertices[i];
        //uint32 vertexIndex = indices[i];
        Vertex.Position = FVector3f(vertices[i]);
        //UE_LOG(LogTemp, Log, TEXT("Vertex position : %s"), *Vertex.Position.ToString());
        Vertex.UVs[0] = { 0.0f, 0.0f };
        Vertex.UVs[2] = { 0.0f, 0.0f };
        RenderData->Bounds.SphereRadius = FMath::Max(
            (Vertex.Position - FVector3f(RenderData->Bounds.Origin)).Size(),
            RenderData->Bounds.SphereRadius);
        RenderData->Bounds.Origin = FVector(0, 0, 0);
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

void APLATEAUCityModelLoader::LoadNodes_InModel(USceneComponent* _parentComponent, plateau::polygonMesh::Node* _node, AActor& _actor, int index, int count)
{
    USceneComponent* comp = nullptr;
    FString compName;

    UE_LOG(LogTemp, Log, TEXT("LoadNodes_InModel NodeName : %s"), _node->getName().c_str());

    if (_node->getMesh() == std::nullopt)
    {
        //SceneComponentを付与
        comp = NewObject<USceneComponent>(&_actor, FName(_node->getName().c_str()));
        UE_LOG(LogTemp, Log, TEXT("Node doesn't have Mesh"));
        check(comp != nullptr);
        comp->Mobility = EComponentMobility::Static;
        _actor.AddInstanceComponent(comp);
        comp->RegisterComponent();
        comp->AttachToComponent(_parentComponent, FAttachmentTransformRules::KeepWorldTransform);
    }
    else
    {
        //StaticMeshComponentを付与
        compName = "Mesh" + FString::FromInt(index) + "_" + FString::FromInt(count);
        TArray<FVector> vertArr;
        for (int j = 0; j < _node->getMesh()->getVertices().size(); j++)
        {
            vertArr.Add(FVector(_node->getMesh()->getVertices()[j].x, _node->getMesh()->getVertices()[j].y, _node->getMesh()->getVertices()[j].z));
            UE_LOG(LogTemp, Log, TEXT("vertArr[%d] position : %s"), j, *vertArr[j].ToString());
            
        }
#if true
        for (int k = 0; k < _node->getMesh()->getIndices().size(); k++)
        {
            UE_LOG(LogTemp, Log, TEXT("Indices[%d] value : %d"), k, _node->getMesh()->getIndices()[k]);

        }
#endif
        UE_LOG(LogTemp, Log, TEXT("Node has Mesh"));
        comp = CreateStaticMeshComponent(_actor, *_parentComponent, _node->getMesh()->getIndices(), vertArr, compName);
    }
    
    for (int i = 0; i < _node->getChildCount(); i++)
    {
        ++count;
        LoadNodes_InModel(comp, &_node->getChildAt(i), _actor, index, count);
    }
}

void APLATEAUCityModelLoader::SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData) {
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

UStaticMeshComponent* APLATEAUCityModelLoader::CreateStaticMeshComponent(AActor& Actor, USceneComponent& _parentComponent, std::vector<int> _indices,
                                                                TArray<FVector> _vertArr, FString _name) 
{
    UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent Start-----"));
    // RenderData作成(ここは非同期で出来るはず)
    auto RenderData = CreateRenderData(_indices, _vertArr);

    // コンポーネント作成
    const auto Component = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
    Component->Mobility = EComponentMobility::Static;
    Component->bVisualizeComponent = true;

    // StaticMesh作成
    const auto StaticMesh = NewObject<UStaticMesh>(Component, FName(_name));
    Component->SetStaticMesh(StaticMesh);
    SetRenderData(StaticMesh, RenderData);
    StaticMesh->SetFlags(
        RF_Transient | RF_DuplicateTransient | RF_TextExportTransient);
    UMaterial* mat = UMaterial::GetDefaultMaterial(MD_Surface);
    mat->TwoSided = false;
    StaticMesh->AddMaterial(mat);
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
    Component->AttachToComponent(&_parentComponent, FAttachmentTransformRules::KeepWorldTransform);
    Component->PostEditChange();
    UE_LOG(LogTemp, Log, TEXT("-----CreateStaticMeshComponent End-----"));
    return Component;
}

void APLATEAUCityModelLoader::computeFlatNormals(const TArray<uint32_t>& indices, TArray<FStaticMeshBuildVertex>& vertices) {
    // Compute flat normals
    for (int i = 0; i < indices.Num(); i += 3) {
        int acc[3] = { indices[i],indices[i + 1],indices[i + 2] };
        FStaticMeshBuildVertex& v0 = vertices[acc[0]];
        FStaticMeshBuildVertex& v1 = vertices[acc[1]];
        FStaticMeshBuildVertex& v2 = vertices[acc[2]];

        FVector3f v01 = v1.Position - v0.Position;
        FVector3f v02 = v2.Position - v0.Position;
        FVector3f normal = FVector3f::CrossProduct(v01, v02);

        v0.TangentX = v1.TangentX = v2.TangentX = FVector3f(0.0f);
        v0.TangentY = v1.TangentY = v2.TangentY = FVector3f(0.0f);
        v0.TangentZ = v1.TangentZ = v2.TangentZ = normal.GetSafeNormal();
    }
}