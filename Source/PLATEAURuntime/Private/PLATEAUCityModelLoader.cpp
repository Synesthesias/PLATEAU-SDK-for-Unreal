// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "PLATEAUInstancedCityModel.h"
#include "plateau/dataset/dataset_source.h"
#include "plateau/dataset/city_model_package.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "PLATEAUMeshLoader.h"
#include "citygml/citygml.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityModelLoader"

using namespace plateau::udx;
using namespace plateau::polygonMesh;


struct FLoadInputData {
    plateau::polygonMesh::MeshExtractOptions ExtractOptions;
    FString GmlPath;
};

class FCityModelLoaderImpl {
public:
    static TArray<FLoadInputData> PrepareInputData(
        const UPLATEAUImportSettings* ImportSettings, const FString& Source,
        const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference, const bool bImportFromServer, const plateau::network::Client ClientRef) {
        // ファイル検索
        const auto DatasetSource = LoadDataset(bImportFromServer, Source, ClientRef);

        TArray<FLoadInputData> LoadInputDataArray;

        for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
            const auto Settings = ImportSettings->GetFeatureSettings(Package);
            if (!Settings.bImport)
                continue;

            const auto GmlFiles =
                DatasetSource.getAccessor()
                ->filter(Extent.GetNativeData())
                ->getGmlFiles(Package);

            for (const auto& GmlFile : *GmlFiles) {
                auto& LoadInputData = LoadInputDataArray.AddDefaulted_GetRef();
                LoadInputData.GmlPath = UTF8_TO_TCHAR(GmlFile.getPath().c_str());
                auto& ExtractOptions = LoadInputData.ExtractOptions;
                ExtractOptions.reference_point = GeoReference.GetData().getReferencePoint();
                ExtractOptions.mesh_axes = plateau::geometry::CoordinateSystem::ESU;
                ExtractOptions.coordinate_zone_id = GeoReference.GetData().getZoneID();
                ExtractOptions.mesh_granularity = UPLATEAUImportSettings::ConvertGranularity(Settings.MeshGranularity);
                ExtractOptions.max_lod = Settings.MaxLod;
                ExtractOptions.min_lod = Settings.MinLod;
                ExtractOptions.export_appearance = Settings.bImportTexture;
                ExtractOptions.grid_count_of_side = 10;
                ExtractOptions.unit_scale = 0.01f;
                ExtractOptions.extent = Extent.GetNativeData();
                if (Package == plateau::dataset::PredefinedCityModelPackage::Relief || Package == plateau::dataset::PredefinedCityModelPackage::DisasterRisk) {
                    ExtractOptions.exclude_city_object_outside_extent = false;
                    ExtractOptions.exclude_triangles_outside_extent = true;
                } else {
                    ExtractOptions.exclude_city_object_outside_extent = true;
                    ExtractOptions.exclude_triangles_outside_extent = false;
                }
            }
        }
        return LoadInputDataArray;
    }

    static plateau::dataset::DatasetSource LoadDataset(bool bImportFromServer, FString Source, plateau::network::Client ClientRef) {
        if (bImportFromServer) {
            return plateau::dataset::DatasetSource::createServer(TCHAR_TO_UTF8(*Source), ClientRef);
        }
        else {
            return plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*Source));
        }
    }

    static FString CopyGmlFile(const FString& Source, const FString& GmlPath, const bool bImportFromServer) {
        const auto Destination = FPaths::ProjectContentDir() + "PLATEAU/Datasets";

        // ファイルコピー
        try {
            const auto SourceGml = bImportFromServer
                ? plateau::dataset::GmlFile(TCHAR_TO_UTF8(*GmlPath), plateau::network::Client("", ""))
                : plateau::dataset::GmlFile(TCHAR_TO_UTF8(*GmlPath));
            const auto CopiedGml = SourceGml.fetch(TCHAR_TO_UTF8(*Destination));
            return UTF8_TO_TCHAR(CopiedGml->getPath().c_str());
        }
        catch (std::exception& e) {
            UE_LOG(LogTemp, Error, TEXT("Failed to copy %s"), *GmlPath);
            UE_LOG(LogTemp, Error, TEXT("%s"), UTF8_TO_TCHAR(e.what()));
            return TEXT("");
        }
    }

    static std::shared_ptr<const citygml::CityModel> ParseCityGml(const FString& GmlPath) {
        std::shared_ptr<const citygml::CityModel> CityModel = nullptr;
        try {
            citygml::ParserParams ParserParams;
            ParserParams.tesselate = true;
            CityModel = citygml::load(TCHAR_TO_UTF8(*GmlPath), ParserParams);
        }
        catch (...) {
            CityModel = nullptr;
        }
        if (CityModel == nullptr)
            UE_LOG(LogTemp, Error, TEXT("Failed to parse %s"), *GmlPath);

        return CityModel;
    }

    static USceneComponent* CreateComponentInGameThread(
        AActor* Actor, const FString& Name) {
        USceneComponent* Component;
        const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&Component, &Actor, &Name] {
                Component = NewObject<USceneComponent>(Actor, NAME_None);
                // コンポーネント名設定(拡張子無しgml名)
                FString NewUniqueName = Name;
                if (!Component->Rename(*NewUniqueName, nullptr, REN_Test)) {
                    NewUniqueName = MakeUniqueObjectName(Actor, USceneComponent::StaticClass(), FName(Name)).ToString();
                }
                Component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

                check(Component != nullptr);
                Component->Mobility = EComponentMobility::Static;
                Actor->AddInstanceComponent(Component);
                Component->RegisterComponent();
                Component->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

            }, TStatId(), nullptr, ENamedThreads::GameThread);
        Task->Wait();

        return Component;
    }

private:
    FCriticalSection SynchronizationObject;

};


APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
}

namespace {
    void ExecuteInGameThread(TWeakObjectPtr<APLATEAUCityModelLoader> Loader, TFunctionRef<void(TWeakObjectPtr<APLATEAUCityModelLoader>)> Lambda) {
        if (!Loader.IsValid())
            return;
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [Loader, &Lambda] {
                if (!Loader.IsValid())
                    return;
                Lambda(Loader);
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }


    void CreateRootComponent(AActor& Actor) {
#if WITH_EDITOR
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
#endif
    }
}

void APLATEAUCityModelLoader::LoadAsync() {
#if WITH_EDITOR

    // アクター生成
    APLATEAUInstancedCityModel* ModelActor = GetWorld()->SpawnActor<APLATEAUInstancedCityModel>();
    CreateRootComponent(*ModelActor);

    ModelActor->SetActorLabel(FPaths::GetCleanFilename(Source));
    ModelActor->GeoReference = GeoReference;
    ModelActor->DatasetName = FPaths::GetCleanFilename(Source);

    Async(EAsyncExecution::Thread,
        [
            ModelActor,
            Source = Source,
            Extent = Extent,
            GeoReference = GeoReference,
            ImportSettings = ImportSettings,
            bImportFromServer = bImportFromServer,
            Client = *ClientPtr,
            OwnerLoader = TWeakObjectPtr<APLATEAUCityModelLoader>(this)
        ]() mutable {

        auto LoadInputDataArray = FCityModelLoaderImpl::PrepareInputData(
            ImportSettings, Source, Extent, GeoReference, bImportFromServer, Client);

        ExecuteInGameThread(OwnerLoader,
            [GmlCount = LoadInputDataArray.Num()](auto Loader){
            Loader->Status.TotalGmlCount = GmlCount;
        });

        FCriticalSection LoadMeshSection;

        TArray<TFuture<bool>> Futures;
        TArray<FString> GmlNames;

        for (int Index = 0; Index < LoadInputDataArray.Num(); ++Index) {
            FGenericPlatformProcess::ConditionalSleep(
                [&Futures, &GmlNames, OwnerLoader]() {
                    TArray<FString> CurrentLoadingGmls;
                    int LoadCompletedCount = 0;
                    for (int i = 0; i < Futures.Num(); ++i) {
                        if (!Futures[i].IsReady())
                            CurrentLoadingGmls.Add(GmlNames[i]);
                        else
                            ++LoadCompletedCount;

                        ExecuteInGameThread(OwnerLoader,
                            [&CurrentLoadingGmls, &LoadCompletedCount](TWeakObjectPtr<APLATEAUCityModelLoader> Loader) {
                                Loader->Status.LoadedGmlCount = LoadCompletedCount;
                                Loader->Status.LoadingGmls = CurrentLoadingGmls;
                            });
                    }
                    return CurrentLoadingGmls.Num() < 4;
                }, 3);

            FLoadInputData InputData = LoadInputDataArray[Index];

            const auto CopiedGmlPath = FCityModelLoaderImpl::CopyGmlFile(Source, InputData.GmlPath, bImportFromServer);
            const auto GmlName = FPaths::GetCleanFilename(InputData.GmlPath);

            // TODO: fldでgml名被る

            GmlNames.Add(GmlName);
            Futures.Add(Async(EAsyncExecution::Thread,
                [InputData, &LoadInputDataArray, Source,
                ModelActor, GmlName, OwnerLoader, CopiedGmlPath, &LoadMeshSection]() {
                    const auto CityModel = FCityModelLoaderImpl::ParseCityGml(CopiedGmlPath);

                    if (CityModel == nullptr) {
                        ExecuteInGameThread(OwnerLoader,
                            [GmlName](auto Loader) {
                                ++Loader->Status.LoadedGmlCount;
                                Loader->Status.LoadingGmls.Remove(GmlName);
                                Loader->Status.FailedGmls.Add(GmlName);
                            });
                        return false;
                    }

                    const auto Model = MeshExtractor::extract(*CityModel, InputData.ExtractOptions);

                    // 各GMLについて親Componentを作成
                    // コンポーネントは拡張子無しgml名に設定
                    const auto GmlRootComponentName = FPaths::GetBaseFilename(CopiedGmlPath);
                    const auto GmlRootComponent = FCityModelLoaderImpl::CreateComponentInGameThread(ModelActor, GmlRootComponentName);

                    {
                        FScopeLock Lock(&LoadMeshSection);

                        FPLATEAUMeshLoader().LoadModel(ModelActor, GmlRootComponent, Model);
                    }
                    return true;
                }));
        }

        FGenericPlatformProcess::ConditionalSleep(
            [&Futures, &GmlNames, OwnerLoader]() {
                TArray<FString> CurrentLoadingGmls;
                int LoadCompletedCount = 0;
                for (int i = 0; i < Futures.Num(); ++i) {
                    if (!Futures[i].IsReady())
                        CurrentLoadingGmls.Add(GmlNames[i]);
                    else
                        ++LoadCompletedCount;

                    ExecuteInGameThread(OwnerLoader,
                        [&CurrentLoadingGmls, &LoadCompletedCount](TWeakObjectPtr<APLATEAUCityModelLoader> Loader) {
                            Loader->Status.LoadedGmlCount = LoadCompletedCount;
                            Loader->Status.LoadingGmls = CurrentLoadingGmls;
                        });
                }
                return CurrentLoadingGmls.Num() == 0;
            }, 3);
    });
#endif
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

#undef LOCTEXT_NAMESPACE
