// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUInstancedCityModel.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "PLATEAUEditor/Private/Widgets/SPLATEAUFilteringPanel.h"

using namespace plateau::dataset;

namespace {
    FString RemoveSuffix(const FString ComponentName) {
        int Index = 0;
        if (ComponentName.FindLastChar('_', Index)) {
            if (ComponentName.RightChop(Index + 1).IsNumeric()) {
                return ComponentName.LeftChop(ComponentName.Len() - Index);
            }
            else {
                return ComponentName;
            }
        }
        else
            return ComponentName;
    }
}

// Sets default values
APLATEAUInstancedCityModel::APLATEAUInstancedCityModel() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

FPLATEAUCityObjectInfo APLATEAUInstancedCityModel::GetCityObjectInfo(USceneComponent* Component) {
    FPLATEAUCityObjectInfo Result;
    Result.DatasetName = DatasetName;

    if (Component == nullptr)
        return Result;

    auto ID = Component->GetName();
    {
        int Index = 0;
        if (ID.FindLastChar('_', Index)) {
            if (ID.RightChop(Index + 1).IsNumeric()) {
                ID = ID.LeftChop(ID.Len() - Index);
            }
        }
    }
    Result.ID = ID;

    auto GmlComponent = Component;
    while (GmlComponent->GetAttachParent() != RootComponent) {
        GmlComponent = GmlComponent->GetAttachParent();

        // TODO: エラーハンドリング
        if (GmlComponent == nullptr)
            return Result;
    }

    Result.GmlName = GmlComponent->GetName();

    {
        int Index = 0;
        if (Result.GmlName.FindLastChar('_', Index)) {
            if (Result.GmlName.RightChop(Index + 1).IsNumeric()) {
                Result.GmlName = Result.GmlName.LeftChop(Result.GmlName.Len() - Index);
            }
        }
    }
    Result.GmlName += TEXT(".gml");

    return Result;
}

// Called when the game starts or when spawned
void APLATEAUInstancedCityModel::BeginPlay() {
    Super::BeginPlay();

}

// Called every frame
void APLATEAUInstancedCityModel::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

}

plateau::dataset::PredefinedCityModelPackage APLATEAUInstancedCityModel::GetExistPackage() {
    //キャッシュが取られている前提
    const auto LocalCachePath = FPaths::ProjectContentDir() + "PLATEAU/Datasets/" + DatasetName;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
    try {
        const auto InDatasetSource = DatasetSource::createLocal(TCHAR_TO_UTF8(*LocalCachePath));
        DatasetAccessor = InDatasetSource.getAccessor();
    }
    catch (...) {
        DatasetAccessor = nullptr;
        UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *LocalCachePath);
    }
    return DatasetAccessor->getPackages();
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByLODs(const plateau::dataset::PredefinedCityModelPackage InPackage, const int MinLOD, const int MaxLOD, const bool bSingleLOD) {
    MeshComponents.Empty();

    const auto LocalCachePath = FPaths::ProjectContentDir() + "PLATEAU/Datasets/" + DatasetName;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
    try {
        const auto InDatasetSource = DatasetSource::createLocal(TCHAR_TO_UTF8(*LocalCachePath));
        DatasetAccessor = InDatasetSource.getAccessor();
    }
    catch (...) {
        DatasetAccessor = nullptr;
        UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *LocalCachePath);
        return this;
    }
    const auto PackageData = DatasetAccessor->getPackages();
    const auto GMLRootComponent = this->GetRootComponent();
    TArray<USceneComponent*> GMLComponents;
    TMap<FString, USceneComponent*> GMLMaps;
    GMLRootComponent->GetChildrenComponents(false, GMLComponents);
    for (int index = 0; index < GMLComponents.Num(); index++) {
        //一度全てのメッシュを不可視にする
        GMLMaps.Add(GMLComponents[index]->GetFName().ToString(), GMLComponents[index]);
        GMLComponents[index]->SetVisibility(false, true);
    }

    for (int i = 0; i < 9; i++) {
        TArray<USceneComponent*> TargetComponents;
        std::shared_ptr<std::vector<plateau::dataset::GmlFile>> GMLFile;
        bool bContinueLoop = false;
        switch (i) {
        case 0:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Building);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::Building);
            break;
        case 1:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Road);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::Road);
            break;
        case 2:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision);
            break;
        case 3:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::LandUse);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::LandUse);
            break;
        case 4:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::CityFurniture);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::CityFurniture);
            break;
        case 5:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Vegetation);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::Vegetation);
            break;
        case 6:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Relief);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::Relief);
            break;
        case 7:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::DisasterRisk);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::DisasterRisk);
            break;
        case 8:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Unknown);
            bContinueLoop = !((uint32_t)InPackage & (uint32_t)plateau::dataset::PredefinedCityModelPackage::Unknown);
            break;
        default:
            break;
        }
        if (bContinueLoop)
            continue;
        for (int j = 0; j < GMLFile->size(); j++) {
            const auto GMLName = FPaths::GetCleanFilename(GMLFile->at(j).getPath().c_str()).LeftChop(4);
            const auto GMLMaxLOD = GMLFile->at(j).getMaxLod();
            if (!GMLMaps.Contains(GMLName))
                continue;
            auto GMLComponent = *GMLMaps.Find(GMLName);
            if (GMLComponent != nullptr) {
                GMLComponent->GetChildrenComponents(false, TargetComponents);
                for (int k = 0; k < TargetComponents.Num(); k++) {
                    //各LODについて子供になっているメッシュコンポーネントをすべて保持
                    TArray<USceneComponent*> ChildMeshComponents;
                    TargetComponents[k]->GetChildrenComponents(true, ChildMeshComponents);
                    for (int ChildrenIndex = 0; ChildrenIndex < ChildMeshComponents.Num(); ChildrenIndex++) {
                        MeshComponents.Add(ChildMeshComponents[ChildrenIndex]);
                    }

                    //指定されたLODとその子供を可視状態に
                    //設定によっては最大のLODだけ可視状態に
                    if (bSingleLOD) {
                        auto TargetLOD = FMath::Min(GMLMaxLOD, MaxLOD);
                        if (TargetComponents[k]->GetFName().ToString() == ("LOD" + std::to_string(TargetLOD)).c_str()) {
                            TargetComponents[k]->SetVisibility(true, true);
                        }
                    }
                    else {
                        FString LODLevelStr = TargetComponents[k]->GetFName().ToString().RightChop(3);
                        auto LODLevel = FCString::Atoi(*LODLevelStr);
                        if (MinLOD <= LODLevel && LODLevel <= MaxLOD) {
                            TargetComponents[k]->SetVisibility(true, true);
                        }
                    }
                }
            }
        }
    }
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypes(const citygml::CityObject::CityObjectsType InCityObjectType) {
    for (int i = 0; i < MeshComponents.Num(); i++) {
        //この時点で不可視状態ならLODフィルタリングで不可視化されたことになるので無視
        if (MeshComponents[i]->IsVisible() == false)
            continue;

        //メッシュコンポーネントの親の親がGMLのファイル名を握っている
        FString SubFolderName;
        auto GMLName = MeshComponents[i]->GetAttachParent()->GetAttachParent()->GetFName().ToString();
        auto ID = MeshComponents[i]->GetFName().ToString();
        ID = RemoveSuffix(ID);

        //BillboardComponentも混ざってるので無視
        if (ID.Contains("BillboardComponent"))
            continue;

        //フォルダの名前などを取得
        int Index = 0;
        if (GMLName.FindChar('_', Index))
            SubFolderName = GMLName.RightChop(Index + 1);
        if (SubFolderName.FindChar('_', Index))
            SubFolderName = SubFolderName.LeftChop(SubFolderName.Len() - Index);

        const auto FullGmlPath =
            FPaths::ProjectContentDir() +
            "PLATEAU/Datasets/" +
            DatasetName +
            "/udx/" +
            SubFolderName + "/" +
            GMLName +
            ".gml";

        std::shared_ptr<const citygml::CityModel> CityModel = nullptr;
        try {
            citygml::ParserParams ParserParams;
            ParserParams.tesselate = true;
            CityModel = citygml::load(TCHAR_TO_UTF8(*FullGmlPath), ParserParams);
        }
        catch (...) {
            CityModel = nullptr;
            UE_LOG(LogTemp, Error, TEXT("Invalid cache path : %s"), *FullGmlPath);
            return this;
        }
        auto CityObject = CityModel->getCityObjectById(TCHAR_TO_UTF8(*ID));
        if (CityObject == nullptr) {
            UE_LOG(LogTemp, Error, TEXT("Invalid ID : %s"), *ID);
            return this;
        }
        auto CityObjectType = CityObject->getType();
        if (!((uint64_t)InCityObjectType & (uint64_t)CityObjectType)) {
            //マッチしていないので不可視に
            MeshComponents[i]->SetVisibility(false);
        }
    }
    return this;
}

TArray<PLATEAUPackageLOD> APLATEAUInstancedCityModel::GetPackageLODs() {
    TArray<PLATEAUPackageLOD> ReturnLODs;

    int MaxLOD, MinLOD = 0;

    const auto LocalCachePath = FPaths::ProjectContentDir() + "PLATEAU/Datasets/" + DatasetName;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
    try {
        const auto InDatasetSource = DatasetSource::createLocal(TCHAR_TO_UTF8(*LocalCachePath));
        DatasetAccessor = InDatasetSource.getAccessor();
    }
    catch (...) {
        DatasetAccessor = nullptr;
        UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *LocalCachePath);
        return ReturnLODs;
    }

    for (int i = 0; i < 9; i++) {
        std::shared_ptr<std::vector<plateau::dataset::GmlFile>> GMLFile;
        switch (i) {
        case 0:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Building);
            break;
        case 1:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Road);
            MinLOD = 1;
            break;
        case 2:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision);
            MinLOD = 1;
            break;
        case 3:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::LandUse);
            MinLOD = 1;
            break;
        case 4:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::CityFurniture);
            MinLOD = 1;
            break;
        case 5:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Vegetation);
            MinLOD = 1;
            break;
        case 6:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Relief);
            MinLOD = 1;
            break;
        case 7:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::DisasterRisk);
            MinLOD = 1;
            break;
        case 8:
            GMLFile = DatasetAccessor->getGmlFiles(plateau::dataset::PredefinedCityModelPackage::Unknown);
            break;
        default:
            break;
        }
        for (int j = 0; j < GMLFile->size(); j++) {
            if(GMLFile->at(j).getMaxLod() > MaxLOD)
                MaxLOD = GMLFile->at(j).getMaxLod();
        }
        PLATEAUPackageLOD PackageLOD;
        PackageLOD.MaxLOD = MaxLOD;
        PackageLOD.MinLOD = MinLOD;
        ReturnLODs.Add(PackageLOD);
    }

    return ReturnLODs;
}
