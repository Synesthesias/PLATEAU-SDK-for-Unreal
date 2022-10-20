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

    // アクター生成
    AActor* ModelActor = GetWorld()->SpawnActor<AActor>(FVector(0, 0, 0), FRotator(0, 0, 0));
    CreateRootComponent(*ModelActor);
    ModelActor->SetActorLabel("Model");

    auto Result = Async(EAsyncExecution::Thread, [&] {
        ThreadLoad(ModelActor);
        });
#endif
}

void APLATEAUCityModelLoader::ThreadLoad(AActor* ModelActor) {
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

    //ノード走査開始
    MeshLoader.CreateMesh(ModelActor, Model);
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
