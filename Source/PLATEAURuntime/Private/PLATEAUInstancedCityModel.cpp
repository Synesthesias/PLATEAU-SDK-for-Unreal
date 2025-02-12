// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUInstancedCityModel.h"
#include "Misc/DefaultValueHelper.h"
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>
#include "CityGML/PLATEAUCityGmlProxy.h"
#include <PLATEAUMeshExporter.h>
#include <PLATEAUMeshLoader.h>
#include <PLATEAUExportSettings.h>
#include "Reconstruct/PLATEAUModelReconstruct.h"
#include <Reconstruct/PLATEAUModelClassificationByType.h>
#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <Reconstruct/PLATEAUModelLandscape.h>
#include <Reconstruct/PLATEAUMeshLoaderForLandscapeMesh.h>
#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <PLATEAUModelFiltering.h>
#include <Util/PLATEAUReconstructUtil.h>
#include <Util/PLATEAUComponentUtil.h>
#include <Util/PLATEAUGmlUtil.h>
#include "Tasks/Pipe.h"

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

// Sets default values
APLATEAUInstancedCityModel::APLATEAUInstancedCityModel() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

double APLATEAUInstancedCityModel::GetLatitude() {
    return GeoReference.GetData().unproject(TVec3d(0, 0, 0)).latitude;
}

double APLATEAUInstancedCityModel::GetLongitude() {
    return GeoReference.GetData().unproject(TVec3d(0, 0, 0)).longitude;
}

FPLATEAUCityObjectInfo APLATEAUInstancedCityModel::GetCityObjectInfo(USceneComponent* Component) {
    FPLATEAUCityObjectInfo Result;
    Result.DatasetName = DatasetName;

    if (Component == nullptr)
        return Result;

    Result.ID = FPLATEAUComponentUtil::GetOriginalComponentName(Component);

    auto GmlComponent = Component;
    while (GmlComponent->GetAttachParent() != RootComponent) {
        GmlComponent = GmlComponent->GetAttachParent();

        // TODO: エラーハンドリング
        if (GmlComponent == nullptr)
            return Result;
    }

    Result.GmlName = FPLATEAUGmlUtil::GetGmlFileName(GmlComponent);

    return Result;
}

TArray<FPLATEAUCityObject>& APLATEAUInstancedCityModel::GetAllRootCityObjects() {
    if (0 < RootCityObjects.Num()) {
        return RootCityObjects;
    }
    RootCityObjects = FPLATEAUComponentUtil::GetRootCityObjects(GetRootComponent());
    return RootCityObjects;
}

void APLATEAUInstancedCityModel::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUInstancedCityModel::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

plateau::dataset::PredefinedCityModelPackage APLATEAUInstancedCityModel::GetCityModelPackages() const {
    auto Packages = plateau::dataset::PredefinedCityModelPackage::None;
    for (const auto& GmlComponent : GetGmlComponents()) {
        Packages = Packages | FPLATEAUGmlUtil::GetCityModelPackage(GmlComponent);
    }
    return Packages;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByLods(const plateau::dataset::PredefinedCityModelPackage InPackage, const TMap<plateau::dataset::PredefinedCityModelPackage, FPLATEAUMinMaxLod>& PackageToLodRangeMap, const bool bOnlyMaxLod) {
    bIsFiltering = true;
    FPLATEAUModelFiltering Filter;
    Filter.FilterByLods(GetGmlComponents(), InPackage, PackageToLodRangeMap, bOnlyMaxLod);
    bIsFiltering = false;
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypes(const citygml::CityObject::CityObjectsType InCityObjectType) {
    if (!HasAttributeInfo())
        return FilterByFeatureTypesLegacy(InCityObjectType);
    bIsFiltering = true;
    FPLATEAUModelFiltering Filter;
    Filter.FilterByFeatureTypes(GetGmlComponents(), InCityObjectType);
    bIsFiltering = false;
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypesLegacy(const citygml::CityObject::CityObjectsType InCityObjectType) {
    bIsFiltering = true;
    FPLATEAUModelFiltering Filter;
    Launch(
        TEXT("ParseGmlsTask"),
        [this, InCityObjectType, GmlComponents = GetGmlComponents(), &Filter] {
            // 処理が重いため先にCityGMLのパースを行って内部的にキャッシュしておく。
            Filter.FilterByFeatureTypesLegacyCacheCityGml(GmlComponents, InCityObjectType, DatasetName);
            // フィルタリング実行。スレッドセーフでない関数を使用するためメインスレッドで実行する。
            const auto GameThreadTask =
                FFunctionGraphTask::CreateAndDispatchWhenReady(
                    [this, InCityObjectType, &Filter] {
                        Filter.FilterByFeatureTypesLegacyMain(GetGmlComponents(), InCityObjectType, DatasetName);
                    }, TStatId(), nullptr, ENamedThreads::GameThread);
            GameThreadTask->Wait();
            bIsFiltering = false;
        },
        ETaskPriority::BackgroundHigh);
    return this;
}

FPLATEAUMinMaxLod APLATEAUInstancedCityModel::GetMinMaxLod(const plateau::dataset::PredefinedCityModelPackage InPackage) const {
    return FPLATEAUComponentUtil::GetMinMaxLod(GetGmlComponents(), InPackage);
}

bool APLATEAUInstancedCityModel::IsFiltering() {
    return bIsFiltering;
}

const TArray<TObjectPtr<USceneComponent>>& APLATEAUInstancedCityModel::GetGmlComponents() const {
    return GetRootComponent()->GetAttachChildren();
}

TArray<UActorComponent*> APLATEAUInstancedCityModel::GetComponentsByPackage(EPLATEAUCityModelPackage Pkg) const {
    return FPLATEAUComponentUtil::GetComponentsByPackage(GetGmlComponents(), Pkg);
}

bool APLATEAUInstancedCityModel::HasAttributeInfo() {
    TArray<USceneComponent*> Components;
    GetRootComponent()->GetChildrenComponents(true, Components);
    return Components.ContainsByPredicate([](const auto& Comp) {
        return Comp->IsA(UPLATEAUCityObjectGroup::StaticClass());
        });
}

TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ReconstructModel(const TArray<USceneComponent*>& TargetComponents, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal)  {

    UE_LOG(LogTemp, Log, TEXT("ReconstructModel: %d %d %s"), TargetComponents.Num(), static_cast<int>(ReconstructType), bDestroyOriginal ? TEXT("True") : TEXT("False"));
    TTask<TArray<USceneComponent*>> ReconstructModelTask = Launch(TEXT("ReconstructModelTask"), [this, TargetComponents, ReconstructType, bDestroyOriginal] {       
        FPLATEAUModelReconstruct ModelReconstruct(this, FPLATEAUReconstructUtil::GetConvertGranularityFromReconstructType(ReconstructType));
        const auto& TargetCityObjects = ModelReconstruct.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);
        auto Task = ReconstructTask(ModelReconstruct, TargetCityObjects, bDestroyOriginal);
        AddNested(Task);
        Task.Wait();
        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //終了イベント通知
            OnReconstructFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);
            
        return Task.GetResult();
    });
    return ReconstructModelTask;
}

TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ClassifyModel(const TArray<USceneComponent*>& TargetComponents, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal, UMaterialInterface* DefaultMaterial) {
    
    UE_LOG(LogTemp, Log, TEXT("ClassifyModelByType: %d %d %s"), TargetComponents.Num(), static_cast<int>(ReconstructType), bDestroyOriginal ? TEXT("True") : TEXT("False"));
    TTask<TArray<USceneComponent*>> ClassifyModelByTypeTask = Launch(TEXT("ClassifyModelByTypeTask"), [&, this, TargetComponents, bDestroyOriginal, Materials, ReconstructType, DefaultMaterial] {

        FPLATEAUModelClassificationByType ModelClassification(this, Materials, DefaultMaterial);
        const auto& TargetCityObjects = ModelClassification.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);
        auto Task = ClassifyTask(ModelClassification, TargetCityObjects, ReconstructType, bDestroyOriginal);
        AddNested(Task);
        Task.Wait();
        
        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //終了イベント通知
            OnClassifyFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);
        
        return Task.GetResult();
    });
    return ClassifyModelByTypeTask;
}

UE::Tasks::TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ClassifyModel(const TArray<USceneComponent*>& TargetComponents, const FString& AttributeKey, TMap<FString, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal, UMaterialInterface* DefaultMaterial) {
    
    UE_LOG(LogTemp, Log, TEXT("ClassifyModelByAttr: %d %d %s"), TargetComponents.Num(), static_cast<int>(ReconstructType), bDestroyOriginal ? TEXT("True") : TEXT("False"));
    TTask<TArray<USceneComponent*>> ClassifyModelByAttrTask = Launch(TEXT("ClassifyModelByAttrTask"), [&, this, TargetComponents, AttributeKey, bDestroyOriginal, Materials, ReconstructType, DefaultMaterial] {

        FPLATEAUModelClassificationByAttribute ModelClassification(this, AttributeKey, Materials, DefaultMaterial);
        const auto& TargetCityObjects = ModelClassification.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);
        auto Task = ClassifyTask(ModelClassification, TargetCityObjects, ReconstructType, bDestroyOriginal);
        AddNested(Task);
        Task.Wait();

        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //終了イベント通知
            OnClassifyFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);

        return Task.GetResult();
        });
    return ClassifyModelByAttrTask;
}

UE::Tasks::TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ClassifyTask(FPLATEAUModelClassification& ModelClassification, const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal) {

    TTask<TArray<USceneComponent*>> ClassifyTask = Launch(TEXT("ClassifyTask"), [&, TargetCityObjects, ReconstructType, bDestroyOriginal] {

        if (ReconstructType == EPLATEAUMeshGranularity::DoNotChange) {

            //粒度ごとにターゲットを取得して実行
            TArray<USceneComponent*> JoinedResults;
            const TArray<ConvertGranularity> GranularityList{
                ConvertGranularity::PerAtomicFeatureObject,
                ConvertGranularity::PerPrimaryFeatureObject,
                ConvertGranularity::PerCityModelArea,
                ConvertGranularity::MaterialInPrimary
            };

            for (const auto& Granularity : GranularityList) {
                const auto& Targets = ModelClassification.FilterComponentsByConvertGranularity(TargetCityObjects, Granularity);
                if (Targets.Num() > 0) {
                    ModelClassification.SetConvertGranularity(Granularity);
                    auto GranularityTask = ReconstructTask(ModelClassification, Targets, bDestroyOriginal);
                    AddNested(GranularityTask);
                    GranularityTask.Wait();
                    JoinedResults.Append(GranularityTask.GetResult());
                }
            }
            return JoinedResults;
        }
        else {
            const auto& ConvertGranularity = FPLATEAUReconstructUtil::GetConvertGranularityFromReconstructType(ReconstructType);
            ModelClassification.SetConvertGranularity(ConvertGranularity);
            auto Task = ReconstructTask(ModelClassification, TargetCityObjects, bDestroyOriginal);
            return Task.GetResult();
        }

        });
    return ClassifyTask;
}


UE::Tasks::TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ReconstructTask(FPLATEAUModelReconstruct& ModelReconstruct, const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects, bool bDestroyOriginal) {

    TTask<TArray<USceneComponent*>> ConvertTask = Launch(TEXT("ReconstructTask"), [&, TargetCityObjects, bDestroyOriginal] {
        std::shared_ptr<plateau::polygonMesh::Model> converted = ModelReconstruct.ConvertModelForReconstruct(TargetCityObjects);
        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //コンポーネント削除
            FPLATEAUComponentUtil::DestroyOrHideComponents(TargetCityObjects, bDestroyOriginal);
            }, TStatId(), NULL, ENamedThreads::GameThread)
            ->Wait();

        const auto ResultComponents = ModelReconstruct.ReconstructFromConvertedModel(converted);
        return ResultComponents;
    });
    return ConvertTask;
}

//Landscape
UE::Tasks::FTask APLATEAUInstancedCityModel::CreateLandscape(const TArray<USceneComponent*>& TargetComponents, FPLATEAULandscapeParam Param, bool bDestroyOriginal) {

    UE_LOG(LogTemp, Log, TEXT("CreateLandscape: %d %s"), TargetComponents.Num(), bDestroyOriginal ? TEXT("True") : TEXT("False"));
    FTask CreateLandscapeTask = Launch(TEXT("CreateLandscapeTask"), [&, TargetComponents, Param, bDestroyOriginal] {

        FPLATEAUModelLandscape Landscape(this);
        const auto& TargetCityObjects = Landscape.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);

        FPLATEAUMeshExportOptions ExtOptions;
        ExtOptions.bExportHiddenObjects = false;
        ExtOptions.bExportTexture = true;
        ExtOptions.TransformType = EMeshTransformType::Local;
        ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;
        FPLATEAUMeshExporter MeshExporter;
        std::shared_ptr<plateau::polygonMesh::Model> basemodel = MeshExporter.CreateModelFromComponents(this, TargetCityObjects, ExtOptions);

        auto Results = Landscape.CreateHeightMap(basemodel, Param);

        // 高さを地形に揃える (LOD3Roadの場合は、ResultのHeightmap書き換え)
        if (Param.AlignLand || Param.InvertRoadLod3) {
            const auto& AlignedComponents = AlignLand(Results, Param, bDestroyOriginal);
            FFunctionGraphTask::CreateAndDispatchWhenReady([&, AlignedComponents, bDestroyOriginal]() {
                // Align コンポーネント削除
                FPLATEAUComponentUtil::DestroyOrHideComponents(AlignedComponents, bDestroyOriginal);
                }, TStatId(), NULL, ENamedThreads::GameThread)->Wait();
        }

        for (const auto Result : Results) {
            //　平滑化Mesh / Landscape生成
            if (Param.ConvertTerrain) {
                if (!Param.ConvertToLandscape) {
                    //平滑化Mesh生成
                    FPLATEAUMeshLoaderForLandscapeMesh MeshLoader;
                    MeshLoader.CreateMeshFromHeightMap(*this, Param.TextureWidth, Param.TextureHeight, Result.Min, Result.Max, Result.MinUV, Result.MaxUV, Result.Data->data(), Result.NodeName);
                }
                else {
                    //Landscape生成
                    TArray<uint16> HeightData(Result.Data->data(), Result.Data->size());
                    //LandScape  
                    FFunctionGraphTask::CreateAndDispatchWhenReady(
                        [&, HeightData, Result, Param ] {
                            auto LandActor = Landscape.CreateLandScape(GetWorld(), Param.NumSubsections, Param.SubsectionSizeQuads,
                            Param.ComponentCountX, Param.ComponentCountY,
                            Param.TextureWidth, Param.TextureHeight,
                            Result.Min, Result.Max, Result.MinUV, Result.MaxUV, Result.TexturePath, HeightData, Result.NodeName);
                            Landscape.CreateLandScapeReference(LandActor, this, Result.NodeName);
                        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();
                }
            }
        }

        FFunctionGraphTask::CreateAndDispatchWhenReady([&, TargetCityObjects, bDestroyOriginal, Results]() {

            // Landscape コンポーネント削除
            if (Param.ConvertTerrain)
                FPLATEAUComponentUtil::DestroyOrHideComponents(TargetCityObjects, bDestroyOriginal);

            //終了イベント通知
            EPLATEAULandscapeCreationResult Res = Results.Num() > 0 ? EPLATEAULandscapeCreationResult::Success : EPLATEAULandscapeCreationResult::Fail;
            OnLandscapeCreationFinished.Broadcast(Res);
        }, TStatId(), NULL, ENamedThreads::GameThread)->Wait();

    });
    return CreateLandscapeTask;
}

TArray<UPLATEAUCityObjectGroup*> APLATEAUInstancedCityModel::AlignLand(TArray<HeightmapCreationResult>& Results, const FPLATEAULandscapeParam& Param, bool bDestroyOriginal) {

    FPLATEAUModelAlignLand ModelAlign(this);
    ModelAlign.SetResults(Results, Param);
    TArray<UPLATEAUCityObjectGroup*> TargetCityObjects = ModelAlign.GetTargetCityObjectsForAlignLand();
    //Lod3Roadの場合はLandscape生成前にResultのHeightmap情報書き換え&TargetCityObjectsからLod3Road除外
    if (Param.InvertRoadLod3) 
        Results = ModelAlign.UpdateHeightMapForLod3Road(TargetCityObjects);
    if (Param.AlignLand) 
        ModelAlign.Align(TargetCityObjects);
    return TargetCityObjects;
}