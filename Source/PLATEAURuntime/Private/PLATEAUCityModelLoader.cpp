// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "PLATEAUMeshLoader.h"
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

    // GeoReferenceを選択範囲の中心に更新
    const auto MinPoint = GeoReference.GetData().project(Extent.GetNativeData().min);
    const auto MaxPoint = GeoReference.GetData().project(Extent.GetNativeData().max);
    const auto NativeReferencePoint = (MinPoint + MaxPoint) / 2.0;
    GeoReference.ReferencePoint.X += NativeReferencePoint.x;
    GeoReference.ReferencePoint.Y += NativeReferencePoint.y;
    GeoReference.ReferencePoint.Z += NativeReferencePoint.z;

    LoadAsync();
#endif
}

void APLATEAUCityModelLoader::LoadAsync() {
    for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
        const auto Settings = ImportSettings->GetFeatureSettings(Package);
        if (!Settings.bImport)
            continue;

        // アクター生成
        AActor* ModelActor = GetWorld()->SpawnActor<AActor>(FVector(0, 0, 0), FRotator(0, 0, 0));
        CreateRootComponent(*ModelActor);

        // キャプチャ用ローカル変数
        const auto& GeoReferenceData = GeoReference.GetData();
        const MeshExtractOptions MeshExtractOptions(
            GeoReferenceData.getReferencePoint(), CoordinateSystem::NWU,
            UPLATEAUImportSettings::ConvertGranularity(Settings.MeshGranularity),
            Settings.MaxLod, Settings.MinLod, Settings.bImportTexture,
            10, 0.01, GeoReferenceData.getZoneID(), Extent.GetNativeData());

        Async(EAsyncExecution::Thread,
            [
                Package, Source = Source,
                ExtentData = Extent.GetNativeData(),
                ModelActor,
                MeshExtractOptions
            ]{
                // ファイル検索
                const auto UdxFileCollection =
                    UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
                    ->filter(ExtentData);
                const auto GmlFiles = UdxFileCollection->getGmlFiles(Package);

                // 都市モデルパース、ポリゴンメッシュ抽出、ノード走査 各ファイルに対して行う
                citygml::ParserParams ParserParams;
                ParserParams.tesselate = true;

                for (const auto& GmlFile : *GmlFiles) {
                    // TODO: fetch
                    // UdxFileCollection->fetch(TCHAR_TO_UTF8(*(FPaths::ProjectContentDir() + "PLATEAU")), GmlFileInfo(GmlFile));

                    std::shared_ptr<const citygml::CityModel> CityModel;
                    try {
                        CityModel = citygml::load(GmlFile, ParserParams);
                    }
                    catch (...) {
                        //TODO: Error Handling
                        UE_LOG(LogTemp, Error, TEXT("Failed to parse %s"), GmlFile.c_str());

                        continue;
                    }
                    const auto Model = MeshExtractor::extract(*CityModel, MeshExtractOptions);

                    const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
                        [&ModelActor, &GmlFile] {
                            // アクター名設定
                            FString PathPart, GmlFileNameWithoutExtension, Extension;
                            FPaths::Split(UTF8_TO_TCHAR(GmlFile.c_str()), PathPart, GmlFileNameWithoutExtension, Extension);
                            ModelActor->SetActorLabel(GmlFileNameWithoutExtension);
                        }, TStatId(), nullptr, ENamedThreads::GameThread);
                    Task->Wait();

                    FPLATEAUMeshLoader().CreateMesh(ModelActor, Model);
                }
            });
    }
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
