// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUInstancedCityModel.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "CityGML/PLATEAUCityGmlProxy.h"
#include "Misc/DefaultValueHelper.h"
#include "PLATEAUEditor/Private/Widgets/SPLATEAUFilteringPanel.h"

using namespace plateau::dataset;

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

plateau::dataset::PredefinedCityModelPackage APLATEAUInstancedCityModel::GetExistPackage() const {
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
    SetIsFiltering(true);

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
    TMap<FString, USceneComponent*> GMLMaps;

    for (const auto& GmlComponent : GetRootComponent()->GetAttachChildren()) {
        //一度全てのメッシュを不可視にする
        GMLMaps.Add(GmlComponent->GetFName().ToString(), GmlComponent);
        GmlComponent->SetVisibility(false, true);

        const auto GmlFileName = GmlComponent->GetName() + ".gml";
        const auto Package = UdxSubFolder::getPackage(GmlFile(TCHAR_TO_UTF8(*GmlFileName)).getFeatureType());

        // 選択されていないパッケージを除外
        bool IsPackageIncluded = static_cast<uint32_t>(InPackage) & static_cast<uint32_t>(Package);
        if (!IsPackageIncluded)
            continue;

        TMap<int, TSet<FString>> LodToFeatureIDMap;
        for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
            auto LodComponentName = LodComponent->GetName();
            int Lod;
            FString LodString = LodComponentName.RightChop(3);
            LodString = LodString.LeftChop(LodString.Len() - 1);
            FDefaultValueHelper::ParseInt(LodString, Lod);

            if (Lod < MinLOD || Lod > MaxLOD)
                continue;

            auto& FeatureIDs = LodToFeatureIDMap.Add(Lod);
            for (const auto& FeatureComponent : LodComponent->GetAttachChildren()) {
                auto ComponentName = FeatureComponent->GetName();
                int Index = 0;
                if (ComponentName.FindLastChar('_', Index)) {
                    if (ComponentName.RightChop(Index + 1).IsNumeric()) {
                        ComponentName = ComponentName.LeftChop(ComponentName.Len() - Index);
                    }
                }

                FeatureIDs.Add(ComponentName);
            }
        }

        for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
            auto LodComponentName = LodComponent->GetName();
            int Lod;
            FString LodString = LodComponentName.RightChop(3);
            LodString = LodString.LeftChop(LodString.Len() - 1);
            FDefaultValueHelper::ParseInt(LodString, Lod);

            if (Lod < MinLOD || Lod > MaxLOD)
                continue;

            // 全てのLODを描画する場合は単純
            if (!bSingleLOD) {
                LodComponent->SetVisibility(true, true);
                continue;
            }

            TArray<USceneComponent*> Components;
            LodComponent->GetChildrenComponents(true, Components);
            for (const auto& Component : Components) {
                auto ComponentName = Component->GetName();
                int Index = 0;
                if (ComponentName.FindLastChar('_', Index)) {
                    if (ComponentName.RightChop(Index + 1).IsNumeric()) {
                        ComponentName = ComponentName.LeftChop(ComponentName.Len() - Index);
                    }
                }

                // 最大LOD取得
                TArray<int> Keys;
                LodToFeatureIDMap.GetKeys(Keys);
                int MaxLod = Lod;
                for (const auto Key : Keys) {
                    if (!LodToFeatureIDMap[Key].Contains(ComponentName))
                        continue;
                    MaxLod = FMath::Max(MaxLod, Key);
                }
                if (Lod == MaxLod)
                    Component->SetVisibility(true, true);
            }
        }
    }

    SetIsFiltering(false);
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypes(const citygml::CityObject::CityObjectsType InCityObjectType) {
    SetIsFiltering(true);

    const auto Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, InCityObjectType] {
        for (const auto& GmlComponent : GetRootComponent()->GetAttachChildren()) {
            const auto GmlName = GmlComponent->GetName();
            FPLATEAUCityObjectInfo GmlInfo;
            GmlInfo.DatasetName = DatasetName;
            GmlInfo.GmlName = GmlName + ".gml";
            const auto CityModel = UPLATEAUCityGmlProxy::Load(GmlInfo);
        }

        const auto GameThreadTask =
            FFunctionGraphTask::CreateAndDispatchWhenReady([this, InCityObjectType] {

            for (const auto& GmlComponent : GetRootComponent()->GetAttachChildren()) {
                for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
                    TArray<USceneComponent*> AllComponents;
                    LodComponent->GetChildrenComponents(true, AllComponents);
                    for (const auto& Component : AllComponents) {
                        //この時点で不可視状態ならLODフィルタリングで不可視化されたことになるので無視
                        if (Component->IsVisible() == false)
                            continue;

                        auto ID = Component->GetName();

                        // TODO: よりロバストな手法?末尾に_{数字}が付いた地物IDが存在するため、
                        // 主要な地物の場合のみ
                        if (Component->GetAttachParent() == LodComponent) {
                            int Index = 0;
                            if (ID.FindLastChar('_', Index)) {
                                if (ID.RightChop(Index + 1).IsNumeric()) {
                                    ID = ID.LeftChop(ID.Len() - Index);
                                }
                            }
                        }

                        //BillboardComponentも混ざってるので無視
                        if (ID.Contains("BillboardComponent"))
                            continue;

                        FPLATEAUCityObjectInfo GmlInfo;
                        GmlInfo.DatasetName = DatasetName;
                        GmlInfo.GmlName = GmlComponent->GetName() + ".gml";
                        const auto CityModel = UPLATEAUCityGmlProxy::Load(GmlInfo);

                        if (CityModel == nullptr) {
                            UE_LOG(LogTemp, Error, TEXT("Invalid Dataset or Gml : %s, %s"), *GmlInfo.DatasetName, *GmlInfo.GmlName);
                            continue;
                        }

                        auto CityObject = CityModel->getCityObjectById(TCHAR_TO_UTF8(*ID));
                        if (CityObject == nullptr) {
                            UE_LOG(LogTemp, Error, TEXT("Invalid ID : %s"), *ID);
                            continue;
                        }
                        auto CityObjectType = CityObject->getType();
                        if (!((uint64_t)InCityObjectType & (uint64_t)CityObjectType)) {
                            //マッチしていないので不可視に
                            Component->SetVisibility(false);
                        }
                    }
                }
            }

            SetIsFiltering(false);
                }, TStatId(), nullptr, ENamedThreads::GameThread);
        GameThreadTask->Wait();
        }, TStatId(), nullptr, ENamedThreads::AnyBackgroundHiPriTask);
    return this;
}

TArray<PLATEAUPackageLOD> APLATEAUInstancedCityModel::GetPackageLODs() const {
    TArray<PLATEAUPackageLOD> ReturnLODs;

    int MaxLOD = 0, MinLOD = 0;

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
            if (GMLFile->at(j).getMaxLod() > MaxLOD)
                MaxLOD = GMLFile->at(j).getMaxLod();
        }
        PLATEAUPackageLOD PackageLOD;
        PackageLOD.MaxLOD = MaxLOD;
        PackageLOD.MinLOD = MinLOD;
        ReturnLODs.Add(PackageLOD);
    }

    return ReturnLODs;
}

bool APLATEAUInstancedCityModel::IsFiltering() {
    FScopeLock Lock(&FilterSection);
    return bIsFiltering;
}

void APLATEAUInstancedCityModel::SetIsFiltering(const bool InValue) {
    FScopeLock Lock(&FilterSection);
    bIsFiltering = InValue;
}
