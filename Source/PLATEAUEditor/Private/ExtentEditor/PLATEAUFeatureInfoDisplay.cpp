
#include "PLATEAUFeatureInfoDisplay.h"
#include "PLATEAUGeometry.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>
#include <plateau/udx/mesh_code.h>
#include <plateau/udx/udx_file_collection.h>
#include <plateau/udx/lod_searcher.h>

#include "Components/StaticMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "StaticMeshResources.h"
#include "PLATEAUTextureLoader.h"
#include "Materials/MaterialInstanceDynamic.h"

DECLARE_STATS_GROUP(TEXT("PLATEAUFeatureDisplay"), STATGROUP_PanelMesh, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Mesh.Build"), STAT_Mesh_Build, STATGROUP_PanelMesh);

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
    FString FindGmlFile(const plateau::udx::UdxFileCollection& InFileCollection, const plateau::udx::MeshCode& InMeshCode, const plateau::udx::PredefinedCityModelPackage InPackage) {
        const auto Files = InFileCollection.filterByMeshCodes({ InMeshCode })->getGmlFiles(InPackage);
        if (Files->size() == 0)
            return "";
        return UTF8_TO_TCHAR(Files->at(0).c_str());
    }

    int GetMaxLod(const FString& GmlFile) {
        const auto Flag = plateau::udx::LodSearcher::searchLodsInFile2(TCHAR_TO_UTF8(*GmlFile));
        return Flag.getFlag() >= 1 << 3
            ? 3 : Flag.getFlag() >= 1 << 2
            ? 2 : 1;
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
                }
                else if (bGrayOut) {
                    DynMat->SetScalarParameterValue(TEXT("Multiplyer"), 0.7f);
                    DynMat->SetScalarParameterValue(TEXT("Opacity"), 0.3f);
                }
                else {
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
    , ViewportClient(InViewportClient) {
}

FPLATEAUFeatureInfoDisplay::~FPLATEAUFeatureInfoDisplay() {}


void FPLATEAUFeatureInfoDisplay::UpdateAsync(const FPLATEAUExtent& InExtent, const plateau::udx::UdxFileCollection& InFileCollection, const bool bShow, const bool bDetailed) {
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
            auto TileExtent = plateau::udx::MeshCode(TCHAR_TO_UTF8(*Entry.Key)).getExtent();
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
                    Box.GetCenter() + FVector::UpVector * 2 + FVector((80  * PANEL_SCALE_MULTIPLYER * i) - 120 * PANEL_SCALE_MULTIPLYER, 0, 0),
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
        auto TileExtent = plateau::udx::MeshCode(TCHAR_TO_UTF8(*Entry.Key)).getExtent();
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

    const auto MeshCodes = plateau::udx::MeshCode::getThirdMeshes(InExtent.GetNativeData());

    for (const auto& RawMeshCode : *MeshCodes) {
        // Panelが生成されていない場合は生成
        const auto MeshCode = UTF8_TO_TCHAR(RawMeshCode.get().c_str());
        if (!AsyncLoadedPanels.Find(MeshCode)) {
            const auto& AsyncLoadedTile = AsyncLoadedPanels.Add(MeshCode, MakeShared<FPLATEAUAsyncLoadedFeatureInfoPanel>());

            FPLATEAUMeshCodeFeatureInfoInput Input{};
            Input.BldgGmlFile =
                FindGmlFile(InFileCollection, RawMeshCode,
                    plateau::udx::PredefinedCityModelPackage::Building);
            Input.RoadGmlFile =
                FindGmlFile(InFileCollection, RawMeshCode,
                    plateau::udx::PredefinedCityModelPackage::Road);
            Input.UrfGmlFile =
                FindGmlFile(InFileCollection, RawMeshCode,
                    plateau::udx::PredefinedCityModelPackage::CityFurniture);
            Input.VegGmlFile =
                FindGmlFile(InFileCollection, RawMeshCode,
                    plateau::udx::PredefinedCityModelPackage::Vegetation);

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
                }
                else {
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
            const bool BldgExists = Input.BldgGmlFile != "";
            int BldgLODNum = 1;
            if (BldgExists)
                BldgLODNum = GetMaxLod(Input.BldgGmlFile);
            FString FilePathBldg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Building, BldgLODNum, false);
            FString DetailedFilePathBldg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Building, BldgLODNum, true);

            const bool CityExists = Input.UrfGmlFile != "";
            int CityLODNum = 1;
            if (CityExists)
                CityLODNum = GetMaxLod(Input.UrfGmlFile);
            FString FilePathCity = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::CityFurniture, CityLODNum, false);
            FString DetailedFilePathCity = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::CityFurniture, CityLODNum, true);

            const bool RoadExists = Input.RoadGmlFile != "";
            int RoadLODNum = 1;
            if (RoadExists)
                RoadLODNum = GetMaxLod(Input.RoadGmlFile);
            FString FilePathRoad = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Road, RoadLODNum, false);
            FString DetailedFilePathRoad = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Road, RoadLODNum, true);

            const bool VegExists = Input.VegGmlFile != "";
            int VegLODNum = 1;
            if (VegExists)
                VegLODNum = GetMaxLod(Input.VegGmlFile);
            FString FilePathVeg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Vegetation, VegLODNum, false);
            FString DetailedFilePathVeg = FPaths::ProjectDir() + "Plugins/PLATEAU-SDK-for-Unreal/Content/"
                + MakeTexturePath(plateau::udx::PredefinedCityModelPackage::Vegetation, VegLODNum, true);


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

const FString FPLATEAUAsyncLoadedFeatureInfoPanel::MakeTexturePath(const plateau::udx::PredefinedCityModelPackage Type, const int LOD, const bool bEnableText) {
    FString Path = "";
    if (bEnableText) {
        Path += PANEL_PATH_WITHTEXT;
    }
    else {
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
    case plateau::udx::PredefinedCityModelPackage::Building:
        Path += PANEL_PATH_BUILDING;
        break;
    case plateau::udx::PredefinedCityModelPackage::CityFurniture:
        Path += PANEL_PATH_CITY;
        break;
    case plateau::udx::PredefinedCityModelPackage::Road:
        Path += PANEL_PATH_ROAD;
        break;
    case plateau::udx::PredefinedCityModelPackage::Vegetation:
        Path += PANEL_PATH_VEGITATION;
        break;
    default:
        Path += PANEL_PATH_BUILDING;
        break;
    }
    return Path;
}
