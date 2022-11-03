// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "PLATEAUInstancedCityModel.h"
#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "PLATEAUMeshLoader.h"
#include "citygml/citygml.h"
#include "Kismet/GameplayStatics.h"

#include <atomic>

#define LOCTEXT_NAMESPACE "PLATEAUCityModelLoader"

using namespace plateau::udx;
using namespace plateau::polygonMesh;

APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
}

namespace {
    void ExecuteInGameThread(const FString& LoaderName, TFunctionRef<void(APLATEAUCityModelLoader*)> Lambda) {
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [LoaderName, &Lambda] {
                TArray<AActor*> ActorsToFind;
                UGameplayStatics::GetAllActorsOfClass(GWorld, APLATEAUCityModelLoader::StaticClass(), ActorsToFind);
                for (AActor* LoaderActor : ActorsToFind) {
                    const auto LoaderActorCast = Cast<APLATEAUCityModelLoader>(LoaderActor);
                    if (LoaderActorCast == nullptr)
                        continue;

                    if (LoaderActorCast->GetActorNameOrLabel() == LoaderName) {
                        Lambda(LoaderActorCast);
                        return;
                    }
                }
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }

    int ExecuteInGameThreadWithReturn(const FString& LoaderName, TFunctionRef<int(APLATEAUCityModelLoader*)> Lambda) {
        int Result;
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [LoaderName, &Lambda, &Result] {
                TArray<AActor*> ActorsToFind;
                UGameplayStatics::GetAllActorsOfClass(GWorld, APLATEAUCityModelLoader::StaticClass(), ActorsToFind);
                for (AActor* LoaderActor : ActorsToFind) {
                    const auto LoaderActorCast = Cast<APLATEAUCityModelLoader>(LoaderActor);
                    if (LoaderActorCast == nullptr)
                        continue;

                    if (LoaderActorCast->GetActorNameOrLabel() == LoaderName) {
                        Result = Lambda(LoaderActorCast);
                    }
                }
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
            return Result;
    }
}

void APLATEAUCityModelLoader::LoadAsync() {
#if WITH_EDITOR
    struct FLoadInputData {
        MeshExtractOptions ExtractOptions;
        FString GmlPath;
    };

    // GeoReferenceを選択範囲の中心に更新
    const auto MinPoint = GeoReference.GetData().project(Extent.GetNativeData().min);
    const auto MaxPoint = GeoReference.GetData().project(Extent.GetNativeData().max);
    const auto NativeReferencePoint = (MinPoint + MaxPoint) / 2.0;
    GeoReference.ReferencePoint.X += NativeReferencePoint.x;
    GeoReference.ReferencePoint.Y += NativeReferencePoint.y;
    GeoReference.ReferencePoint.Z += NativeReferencePoint.z;

    // アクター生成
    APLATEAUInstancedCityModel* ModelActor = GetWorld()->SpawnActor<APLATEAUInstancedCityModel>();
    CreateRootComponent(*ModelActor);
    ModelActor->SetActorLabel(FPaths::GetCleanFilename(Source));
    ModelActor->GeoReference = GeoReference;

    Async(EAsyncExecution::Thread,
        [
            ModelActor,
            Source = Source,
            ExtentData = Extent.GetNativeData(),
            GeoReferenceData = GeoReference.GetData(),
            ImportSettings = ImportSettings,
            OwnerLoaderName = this->GetActorNameOrLabel()
        ]{
            // ファイル検索
            const auto UdxFileCollection =
            UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
            ->filter(ExtentData);


    TArray<FLoadInputData> LoadInputDataArray;

    for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
        const auto Settings = ImportSettings->GetFeatureSettings(Package);
        if (!Settings.bImport)
            continue;

        const auto GmlFiles = UdxFileCollection->getGmlFiles(Package);

        for (const auto& GmlFile : *GmlFiles) {
            auto& LoadInputData = LoadInputDataArray.AddDefaulted_GetRef();
            LoadInputData.GmlPath = UTF8_TO_TCHAR(GmlFile.c_str());
            auto& ExtractOptions = LoadInputData.ExtractOptions;
            ExtractOptions.reference_point = GeoReferenceData.getReferencePoint();
            ExtractOptions.mesh_axes = plateau::geometry::CoordinateSystem::NWU;
            ExtractOptions.coordinate_zone_id = GeoReferenceData.getZoneID();
            ExtractOptions.mesh_granularity = UPLATEAUImportSettings::ConvertGranularity(Settings.MeshGranularity);
            ExtractOptions.max_lod = Settings.MaxLod;
            ExtractOptions.min_lod = Settings.MinLod;
            ExtractOptions.export_appearance = Settings.bImportTexture;
            ExtractOptions.grid_count_of_side = 10;
            ExtractOptions.unit_scale = 0.01f;
            ExtractOptions.extent = ExtentData;
            if (Package == PredefinedCityModelPackage::Relief || Package == PredefinedCityModelPackage::DisasterRisk) {
                ExtractOptions.exclude_city_object_outside_extent = false;
                ExtractOptions.exclude_triangles_outside_extent = true;
            } else {
                ExtractOptions.exclude_city_object_outside_extent = true;
                ExtractOptions.exclude_triangles_outside_extent = false;
            }
        }
    }

    ExecuteInGameThread(OwnerLoaderName,
        [GmlCount = LoadInputDataArray.Num()](APLATEAUCityModelLoader* Loader){
        Loader->Status.TotalGmlCount = GmlCount;
    });

    for (int Index = 0; Index < LoadInputDataArray.Num(); ++Index) {
        // TODO: RW Lock
        FGenericPlatformProcess::ConditionalSleep(
            [OwnerLoaderName]() {
            const auto RunningTaskCount = ExecuteInGameThreadWithReturn(OwnerLoaderName,
                [OwnerLoaderName](APLATEAUCityModelLoader* Loader) {
                    return Loader->Status.LoadingGmls.Num();
                });
            return RunningTaskCount < 4;
        }
        , 2);

        const auto Destination = FPaths::ProjectContentDir() + "PLATEAU/Datasets";

        FLoadInputData InputData = LoadInputDataArray[Index];

        // ファイルコピー
        auto FetchResult = true;
        try {
            // TODO: staticメソッド化
            UdxFileCollection->fetch(TCHAR_TO_UTF8(*Destination), GmlFileInfo(TCHAR_TO_UTF8(*InputData.GmlPath)));
        }
        catch (...) {
            //TODO: Error Handling
            FetchResult = false;
            UE_LOG(LogTemp, Error, TEXT("Failed to copy %s"), *InputData.GmlPath);
        }

        const auto GmlName = FPaths::GetCleanFilename(InputData.GmlPath);

        if (!FetchResult) {
            ExecuteInGameThread(OwnerLoaderName,
                [&GmlName](APLATEAUCityModelLoader* Loader) {
                    Loader->Status.LoadedGmlCount++;
                    Loader->Status.LoadingGmls.Remove(GmlName);
                    Loader->Status.FailedGmls.Add(GmlName);
                });

            continue;
        }

        Async(EAsyncExecution::Thread,
        [InputData, Destination, UdxFileCollection, &LoadInputDataArray, Source,
            ModelActor, GmlName, OwnerLoaderName] {
                ExecuteInGameThread(OwnerLoaderName,
                    [&GmlName](APLATEAUCityModelLoader* Loader) {
                        Loader->Status.LoadingGmls.Add(GmlName);
                    });

                // TODO: libplateauに委譲
                FString RelativeGmlPath = InputData.GmlPath;
                // 末尾に/が無いなら追加(MakePathRelativeToで末尾のディレクトリ名を含めるため)
                FString RootPath = Source;
                if (RootPath[RootPath.Len() - 1] != *TEXT("/")) {
                    RootPath.AppendChar(*TEXT("/"));
                }

                if (!FPaths::MakePathRelativeTo(RelativeGmlPath, *RootPath)) {
                    //TODO: Error Handling
                    UE_LOG(LogTemp, Error, TEXT("Invalid source: %s"), *Source);
                        ExecuteInGameThread(OwnerLoaderName,
                            [&GmlName](APLATEAUCityModelLoader* Loader) {
                                Loader->Status.LoadedGmlCount++;
                                Loader->Status.LoadingGmls.Remove(GmlName);
                                Loader->Status.FailedGmls.Add(GmlName);
                            });
                    return;
                }
                auto CopiedGmlPath = Destination;
                CopiedGmlPath.PathAppend(*FPaths::GetBaseFilename(Source), FPaths::GetBaseFilename(Source).Len());
                CopiedGmlPath.PathAppend(*RelativeGmlPath, RelativeGmlPath.Len());

                std::shared_ptr<const citygml::CityModel> CityModel;
                bool ParseResult = true;
                try {
                    citygml::ParserParams ParserParams;
                    ParserParams.tesselate = true;
                    CityModel = citygml::load(TCHAR_TO_UTF8(*CopiedGmlPath), ParserParams);
                }
                catch (...) {
                    ParseResult = false;
                }

                if (CityModel == nullptr || !ParseResult) {
                    //TODO: Error Handling
                    UE_LOG(LogTemp, Error, TEXT("Failed to parse %s"), *CopiedGmlPath);
                    ExecuteInGameThread(OwnerLoaderName,
                        [&GmlName](APLATEAUCityModelLoader* Loader) {
                            Loader->Status.LoadedGmlCount++;
                            Loader->Status.LoadingGmls.Remove(GmlName);
                            Loader->Status.FailedGmls.Add(GmlName);
                        });
                    return;
                }

                const auto Model = MeshExtractor::extract(*CityModel, InputData.ExtractOptions);

                // 各GMLについて親Componentを作成
                USceneComponent* GmlRootComponent;
                const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
                    [&GmlRootComponent, &ModelActor, &CopiedGmlPath] {
                        GmlRootComponent = NewObject<USceneComponent>(ModelActor, NAME_None);
                        // コンポーネント名設定(拡張子無しgml名)
                        const auto DesiredName = FPaths::GetBaseFilename(CopiedGmlPath);
                        FString NewUniqueName = DesiredName;
                        if (!GmlRootComponent->Rename(*NewUniqueName, nullptr, REN_Test)) {
                            NewUniqueName = MakeUniqueObjectName(ModelActor, USceneComponent::StaticClass(), FName(DesiredName)).ToString();
                        }
                        GmlRootComponent->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

                        check(GmlRootComponent != nullptr);
                        GmlRootComponent->Mobility = EComponentMobility::Static;
                        ModelActor->AddInstanceComponent(GmlRootComponent);
                        GmlRootComponent->RegisterComponent();
                        GmlRootComponent->AttachToComponent(ModelActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

                    }, TStatId(), nullptr, ENamedThreads::GameThread);
                Task->Wait();

                FPLATEAUMeshLoader().LoadModel(ModelActor, GmlRootComponent, Model);

                ExecuteInGameThread(OwnerLoaderName,
                    [&GmlName](APLATEAUCityModelLoader* Loader) {
                        Loader->Status.LoadedGmlCount++;
                        Loader->Status.LoadingGmls.Remove(GmlName);
                    });
            });
    }
        });
#endif
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

void APLATEAUCityModelLoader::CreateRootComponent(AActor& Actor) const {
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

#undef LOCTEXT_NAMESPACE
