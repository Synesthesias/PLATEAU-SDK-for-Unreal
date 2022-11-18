
#include "PLATEAUFeatureInfoDisplay.h"
#include "PLATEAUGeometry.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>
#include <plateau/udx/mesh_code.h>
#include <plateau/udx/udx_file_collection.h>
#include <plateau/udx/lod_searcher.h>

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
}

FPLATEAUFeatureInfoDisplay::FPLATEAUFeatureInfoDisplay(
    const FPLATEAUGeoReference& InGeoReference,
    const TSharedPtr<FPLATEAUExtentEditorViewportClient> InViewportClient)
    : GeoReference(InGeoReference)
    , ViewportClient(InViewportClient) {}

FPLATEAUFeatureInfoDisplay::~FPLATEAUFeatureInfoDisplay() {}


void FPLATEAUFeatureInfoDisplay::UpdateAsync(const FPLATEAUExtent& InExtent, const plateau::udx::UdxFileCollection& InFileCollection, const bool bShow) {
    // 読み込み済みの全てのPanelについて、表示非表示を切り替える。
    for (auto& Entry : AsyncLoadedPanels) {
        if (!Entry.Value->GetFullyLoaded())
            continue;

        const auto PanelComponent = Entry.Value->GetComponent();
        if (PanelComponent == nullptr)
            continue;

        // 全てのPanelを非表示化する。使用するPanelだけ後で再表示
        PanelComponent->SetVisibility(false);

        // 既にPanelがシーンに生成されている場合はスキップする
        if (PanelsInScene.Contains(PanelComponent))
            continue;
        if (!ViewportClient.IsValid())
            continue;

        //Panelの座標を取得
        auto TileExtent = plateau::udx::MeshCode(TCHAR_TO_UTF8(*Entry.Key)).getExtent();
        const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
        const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
        FBox Box(FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
            FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z));

        // TODO: Panel表示位置の調整
        ViewportClient.Pin()->GetPreviewScene()->AddComponent(
            PanelComponent,
            FTransform(FRotator(0, 0, 0),
                Box.GetCenter() - FVector::UpVector,
                Box.GetExtent() / 50
            ));
        PanelsInScene.Add(PanelComponent);
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
            const auto PanelComponent = AsyncLoadedPanel->GetComponent();
            PanelComponent->SetVisibility(true);
        }
    }
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadAsync(const FPLATEAUMeshCodeFeatureInfoInput& Input) {
    Async(EAsyncExecution::Thread,
        [this, Input]() {
            const bool BldgExists = Input.BldgGmlFile != "";
            // TODO: !BldgExistsならグレーアウト
            if (BldgExists) {
                const int BldgMaxLod = GetMaxLod(Input.BldgGmlFile);
                // TODO: BldgMaxLodに応じて表示変更
                UE_LOG(LogTemp, Log, TEXT("%d"), BldgMaxLod);
            }

            // TODO: 他地物(Road, Veg, CityFurniture)についても実装

            USceneComponent* TempPanelComponent = nullptr;
            FFunctionGraphTask::CreateAndDispatchWhenReady(
                [&] {
                    //TODO: Panel表示用コンポーネントの作成
                }, TStatId(), nullptr, ENamedThreads::GameThread)
                ->Wait();

                PanelComponent = TempPanelComponent;
        });
}
