
#include "PLATEAUFeatureInfoDisplay.h"
#include "PLATEAUGeometry.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>
#include <plateau/dataset/mesh_code.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "Components/StaticMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshResources.h"
#include "PLATEAUTextureLoader.h"
#include "Materials/MaterialInstanceDynamic.h"

DECLARE_STATS_GROUP(TEXT("PLATEAUFeatureInfoDisplay"), STATGROUP_PanelMesh, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Dataset.Filter"), STAT_Dataset_Filter, STATGROUP_PanelMesh);
DECLARE_CYCLE_STAT(TEXT("VisibilityUpdate"), STAT_VisibilityUpdate, STATGROUP_PanelMesh);

#define PANEL_PATH_WITHTEXT "AreaIcon_WithText/"
#define PANEL_PATH_NOTEXT "AreaIcon_NoText/"

#define PANEL_PATH_LOD01 "LOD01/"
#define PANEL_PATH_LOD2 "LOD2/"
#define PANEL_PATH_LOD3 "LOD3/"

#define PANEL_PATH_BUILDING "3dicon_Building.png"
#define PANEL_PATH_CITY "3dicon_CityFurniture.png"
#define PANEL_PATH_ROAD "3dicon_Road.png"
#define PANEL_PATH_VEGITATION "3dicon_Vegetation.png"

#define PANEL_SCALE_MULTIPLYER 1.5

namespace {
    void FindGmlFile(
        const plateau::dataset::IDatasetAccessor& InDatasetAccessor,
        const plateau::dataset::MeshCode& InMeshCode,
        const plateau::dataset::PredefinedCityModelPackage InPackage,
        FString& OutGmlFilePath,
        int& OutMaxLod) {
        const auto Files = InDatasetAccessor.filterByMeshCodes({ InMeshCode })->getGmlFiles(InPackage);
        if (Files->size() == 0) {
            OutGmlFilePath = "";
            OutMaxLod = -1;
            return;
        }
        OutGmlFilePath = UTF8_TO_TCHAR(Files->at(0).getPath().c_str());
        OutMaxLod = Files->at(0).isMaxLodCalculated() ? Files->at(0).getMaxLod() : -1;
    }

    int GetMaxLod(const FString& GmlFile) {
        return plateau::dataset::GmlFile(TCHAR_TO_UTF8(*GmlFile)).getMaxLod();
    }

    UStaticMeshComponent* CreatePanelMeshComponent(UTexture* Texture, bool bGrayOut, bool bUseAsBackPanel) {
        UStaticMeshComponent* PanelComponent;
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&] {
                //mesh component作成，テクスチャを適用
                const FName MeshName = MakeUniqueObjectName(
                    GetTransientPackage(),
                    UStaticMeshComponent::StaticClass(),
                    TEXT("Panel"));

                PanelComponent =
                    NewObject<UStaticMeshComponent>(
                        GetTransientPackage(),
                        MeshName, RF_Transient);

                const auto Mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PLATEAU-SDK-for-Unreal/FeatureInfoPanel_PanelIcon")));
                const auto DynMat = UMaterialInstanceDynamic::Create(Mat, GetTransientPackage());
                DynMat->SetTextureParameterValue(TEXT("Texture"), Texture);
                if (bUseAsBackPanel) {
                    DynMat->SetScalarParameterValue(TEXT("Multiplyer"), 0.01f);
                    DynMat->SetScalarParameterValue(TEXT("Opacity"), 0.6f);
                } else if (bGrayOut) {
                    DynMat->SetScalarParameterValue(TEXT("Multiplyer"), 0.7f);
                    DynMat->SetScalarParameterValue(TEXT("Opacity"), 0.3f);
                } else {
                    DynMat->SetScalarParameterValue(TEXT("Multiplyer"), 0.7f);
                    DynMat->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
                }
                PanelComponent->SetMaterial(0, DynMat);
                const auto StaticMeshName = TEXT("/Engine/BasicShapes/Plane");
                const auto Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, StaticMeshName));
                Mesh->AddMaterial(DynMat);
                PanelComponent->SetStaticMesh(Mesh);
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();

            return PanelComponent;
    }
}

FPLATEAUFeatureInfoDisplay::FPLATEAUFeatureInfoDisplay(
    const FPLATEAUGeoReference& InGeoReference,
    const TSharedPtr<FPLATEAUExtentEditorViewportClient> InViewportClient)
    : GeoReference(InGeoReference)
    , ViewportClient(InViewportClient) {}

FPLATEAUFeatureInfoDisplay::~FPLATEAUFeatureInfoDisplay() {}


void FPLATEAUFeatureInfoDisplay::UpdateAsync(const FPLATEAUExtent& InExtent, plateau::dataset::IDatasetAccessor& InDatasetAccessor, const bool bShow, const bool bDetailed) {
    // 読み込み済みの全てのPanelについて、表示非表示を切り替える。
    for (auto& Entry : AsyncLoadedPanels) {
        if (!Entry.Value->GetFullyLoaded())
            continue;
        for (int i = 0; i < 4; i++) {
            const auto PanelComponent = Entry.Value->GetPanelComponent(i);
            const auto DetailedPanelComponent = Entry.Value->GetDetailedPanelComponent(i);
            if (PanelComponent == nullptr)
                continue;
            if (DetailedPanelComponent == nullptr)
                continue;

            // 全てのPanelを非表示化する。使用するPanelだけ後で再表示
            PanelComponent->SetVisibility(false);
            DetailedPanelComponent->SetVisibility(false);

            // 既にPanelがシーンに生成されている場合はスキップする
            if (PanelsInScene.Contains(PanelComponent))
                continue;
            if (PanelsInScene.Contains(DetailedPanelComponent))
                continue;
            if (!ViewportClient.IsValid())
                continue;

            //Panelの座標を取得
            auto TileExtent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*Entry.Key)).getExtent();
            const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
            const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
            FBox Box(FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
                FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z));
            ViewportClient.Pin()->GetPreviewScene()->AddComponent(
                PanelComponent,
                FTransform(FRotator(0, 0, 0),
                    Box.GetCenter() + FVector::UpVector * 2 + FVector((80 * PANEL_SCALE_MULTIPLYER * i) - 120 * PANEL_SCALE_MULTIPLYER, 0, 0),
                    FVector3d(0.8, 0.8, 0.8) * PANEL_SCALE_MULTIPLYER
                ));
            PanelsInScene.Add(PanelComponent);

            //詳細パネル表示
            ViewportClient.Pin()->GetPreviewScene()->AddComponent(
                DetailedPanelComponent,
                FTransform(FRotator(0, 0, 0),
                    Box.GetCenter() + FVector::UpVector * 2 + FVector((80 * PANEL_SCALE_MULTIPLYER * i) - 120 * PANEL_SCALE_MULTIPLYER, 0, 0),
                    FVector3d(0.8, 0.8, 0.8) * PANEL_SCALE_MULTIPLYER
                ));
            PanelsInScene.Add(DetailedPanelComponent);
        }
        const auto BackPanelComponent = Entry.Value->GetBackPanelComponent();

        //パネル背景表示の制御
        if (BackPanelComponent == nullptr)
            continue;
        BackPanelComponent->SetVisibility(false);
        if (PanelsInScene.Contains(BackPanelComponent))
            continue;
        auto TileExtent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*Entry.Key)).getExtent();
        const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
        const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
        FBox Box(FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
            FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z));
        ViewportClient.Pin()->GetPreviewScene()->AddComponent(
            BackPanelComponent,
            FTransform(FRotator(0, 0, 0),
                Box.GetCenter() + FVector::UpVector,
                FVector3d(4, 1.3, 1) * PANEL_SCALE_MULTIPLYER
            ));
        PanelsInScene.Add(BackPanelComponent);
    }
    // もし広範囲を表示しすぎていたら全て非表示のまま(この後の処理は行わない)
    if (!bShow)
        return;

    const auto MeshCodes = plateau::dataset::MeshCode::getThirdMeshes(InExtent.GetNativeData());

    std::shared_ptr<plateau::dataset::IDatasetAccessor> FilteredDatasetAccessor;
    {
        SCOPE_CYCLE_COUNTER(STAT_Dataset_Filter);

        FilteredDatasetAccessor = InDatasetAccessor.filterByMeshCodes(*MeshCodes);
    }

    SCOPE_CYCLE_COUNTER(STAT_VisibilityUpdate);

    for (const auto& RawMeshCode : *MeshCodes) {
        // Panelが生成されていない場合は生成
        const auto MeshCode = UTF8_TO_TCHAR(RawMeshCode.get().c_str());
        if (!AsyncLoadedPanels.Find(MeshCode)) {
            const auto& AsyncLoadedTile = AsyncLoadedPanels.Add(MeshCode, MakeShared<FPLATEAUAsyncLoadedFeatureInfoPanel>());

            FPLATEAUMeshCodeFeatureInfoInput Input{};

            FindGmlFile(
                *FilteredDatasetAccessor, RawMeshCode,
                plateau::dataset::PredefinedCityModelPackage::Building,
                Input.BldgGmlPath,
                Input.BldgMaxLod
            );

            FindGmlFile(
                *FilteredDatasetAccessor, RawMeshCode,
                plateau::dataset::PredefinedCityModelPackage::Road,
                Input.RoadGmlPath,
                Input.RoadMaxLod
            );

            FindGmlFile(
                *FilteredDatasetAccessor, RawMeshCode,
                plateau::dataset::PredefinedCityModelPackage::CityFurniture,
                Input.FrnGmlPath,
                Input.FrnMaxLod
            );

            FindGmlFile(
                *FilteredDatasetAccessor, RawMeshCode,
                plateau::dataset::PredefinedCityModelPackage::Vegetation,
                Input.VegGmlPath,
                Input.VegMaxLod
            );

            AsyncLoadedTile->LoadAsync(Input);
            continue;
        }

        // 範囲内のPanelについて表示をONにする
        const auto& AsyncLoadedPanel = AsyncLoadedPanels[MeshCode];
        if (AsyncLoadedPanel->GetFullyLoaded()) {
            for (int i = 0; i < 4; i++) {
                if (bDetailed) {
                    const auto PanelComponent = AsyncLoadedPanel->GetDetailedPanelComponent(i);
                    PanelComponent->SetVisibility(true);
                } else {
                    const auto PanelComponent = AsyncLoadedPanel->GetPanelComponent(i);
                    PanelComponent->SetVisibility(true);
                }
            }
            const auto BackPanelComponent = AsyncLoadedPanel->GetBackPanelComponent();
            BackPanelComponent->SetVisibility(true);
        }
    }
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadAsync(const FPLATEAUMeshCodeFeatureInfoInput& Input) {
    Async(EAsyncExecution::Thread,
        [this, Input]() {
            const bool BldgExists = Input.BldgGmlPath != "";
            int BldgLODNum = Input.BldgMaxLod;
            if (BldgLODNum == -1 && BldgExists)
                BldgLODNum = GetMaxLod(Input.BldgGmlPath);

            FString FilePathBldg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Building, BldgLODNum, false);
            FString DetailedFilePathBldg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Building, BldgLODNum, true);

            const bool CityExists = Input.FrnGmlPath != "";
            int CityLODNum = Input.FrnMaxLod;
            if (CityLODNum == -1 && CityExists)
                CityLODNum = GetMaxLod(Input.FrnGmlPath);

            FString FilePathCity = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::CityFurniture, CityLODNum, false);
            FString DetailedFilePathCity = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::CityFurniture, CityLODNum, true);

            const bool RoadExists = Input.RoadGmlPath != "";
            int RoadLODNum = Input.RoadMaxLod;
            if (RoadLODNum == -1 && RoadExists)
                RoadLODNum = GetMaxLod(Input.RoadGmlPath);

            FString FilePathRoad = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Road, RoadLODNum, false);
            FString DetailedFilePathRoad = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Road, RoadLODNum, true);

            const bool VegExists = Input.VegGmlPath != "";
            int VegLODNum = Input.VegMaxLod;
            if (VegLODNum == -1 && VegExists)
                VegLODNum = GetMaxLod(Input.VegGmlPath);

            FString FilePathVeg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Vegetation, VegLODNum, false);
            FString DetailedFilePathVeg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::dataset::PredefinedCityModelPackage::Vegetation, VegLODNum, true);


            USceneComponent* TempBldgPanelComponent = nullptr;
            const auto BldgTexture = FPLATEAUTextureLoader::LoadTransient(FilePathBldg);
            TempBldgPanelComponent = CreatePanelMeshComponent(BldgTexture, !BldgExists, false);
            USceneComponent* TempCityPanelComponent = nullptr;
            const auto CityTexture = FPLATEAUTextureLoader::LoadTransient(FilePathCity);
            TempCityPanelComponent = CreatePanelMeshComponent(CityTexture, !CityExists, false);
            USceneComponent* TempRoadPanelComponent = nullptr;
            const auto RoadTexture = FPLATEAUTextureLoader::LoadTransient(FilePathRoad);
            TempRoadPanelComponent = CreatePanelMeshComponent(RoadTexture, !RoadExists, false);
            USceneComponent* TempVegPanelComponent = nullptr;
            const auto VegTexture = FPLATEAUTextureLoader::LoadTransient(FilePathVeg);
            TempVegPanelComponent = CreatePanelMeshComponent(VegTexture, !VegExists, false);

            USceneComponent* TempDetailedBldgPanelComponent = nullptr;
            const auto DetailedBldgTexture = FPLATEAUTextureLoader::LoadTransient(DetailedFilePathBldg);
            TempDetailedBldgPanelComponent = CreatePanelMeshComponent(DetailedBldgTexture, !BldgExists, false);
            USceneComponent* TempDetailedCityPanelComponent = nullptr;
            const auto DetailedCityTexture = FPLATEAUTextureLoader::LoadTransient(DetailedFilePathCity);
            TempDetailedCityPanelComponent = CreatePanelMeshComponent(DetailedCityTexture, !CityExists, false);
            USceneComponent* TempDetailedRoadPanelComponent = nullptr;
            const auto DetailedRoadTexture = FPLATEAUTextureLoader::LoadTransient(DetailedFilePathRoad);
            TempDetailedRoadPanelComponent = CreatePanelMeshComponent(DetailedRoadTexture, !RoadExists, false);
            USceneComponent* TempDetailedVegPanelComponent = nullptr;
            const auto DetailedVegTexture = FPLATEAUTextureLoader::LoadTransient(DetailedFilePathVeg);
            TempDetailedVegPanelComponent = CreatePanelMeshComponent(DetailedVegTexture, !VegExists, false);

            const auto Texture = FPLATEAUTextureLoader::LoadTransient(FPaths::ProjectPluginsDir() + "PLATEAU-SDK-for-Unreal/Content/round-button.png");
            USceneComponent* TempBackPanelComponent = CreatePanelMeshComponent(Texture, true, true);
            {
                FScopeLock Lock(&CriticalSection);
                PanelComponents.Add(TempBldgPanelComponent);
                PanelComponents.Add(TempCityPanelComponent);
                PanelComponents.Add(TempRoadPanelComponent);
                PanelComponents.Add(TempVegPanelComponent);

                DetailedPanelComponents.Add(TempDetailedBldgPanelComponent);
                DetailedPanelComponents.Add(TempDetailedCityPanelComponent);
                DetailedPanelComponents.Add(TempDetailedRoadPanelComponent);
                DetailedPanelComponents.Add(TempDetailedVegPanelComponent);

                BackPanelComponent = TempBackPanelComponent;
                IsFullyLoaded = true;
            }
        });
}

const FString FPLATEAUAsyncLoadedFeatureInfoPanel::MakeTexturePath(const plateau::dataset::PredefinedCityModelPackage Type, const int LOD, const bool bEnableText) {
    FString Path = "";
    if (bEnableText) {
        Path += PANEL_PATH_WITHTEXT;
    } else {
        Path += PANEL_PATH_NOTEXT;
    }

    switch (LOD) {
    case 1:
        Path += UTF8_TO_TCHAR(PANEL_PATH_LOD01);
        break;
    case 2:
        Path += UTF8_TO_TCHAR(PANEL_PATH_LOD2);
        break;
    case 3:
        Path += UTF8_TO_TCHAR(PANEL_PATH_LOD3);
        break;
    default:
        Path += UTF8_TO_TCHAR(PANEL_PATH_LOD01);
        break;
    }

    switch (Type) {
    case plateau::dataset::PredefinedCityModelPackage::Building:
        Path += PANEL_PATH_BUILDING;
        break;
    case plateau::dataset::PredefinedCityModelPackage::CityFurniture:
        Path += PANEL_PATH_CITY;
        break;
    case plateau::dataset::PredefinedCityModelPackage::Road:
        Path += PANEL_PATH_ROAD;
        break;
    case plateau::dataset::PredefinedCityModelPackage::Vegetation:
        Path += PANEL_PATH_VEGITATION;
        break;
    default:
        Path += PANEL_PATH_BUILDING;
        break;
    }
    return Path;
}
