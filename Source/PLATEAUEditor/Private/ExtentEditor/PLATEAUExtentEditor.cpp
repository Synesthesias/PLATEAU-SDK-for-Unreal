// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"

#include "Misc/ScopedSlowTask.h"

#include "Widgets/Docking/SDockTab.h"
#include "EditorViewportTabContent.h"
#include "PLATEAUMeshCodeGizmo.h"
#include "Engine/Selection.h"
#include "Algo/AnyOf.h"

#define LOCTEXT_NAMESPACE "FPLATEUExtentEditor"

const FName FPLATEAUExtentEditor::TabId(TEXT("PLATEAUExtentEditor"));

void FPLATEAUExtentEditor::RegisterTabSpawner(const TSharedRef<class FTabManager>& InTabManager) {
    InTabManager->RegisterTabSpawner(TabId, FOnSpawnTab::CreateSP(this, &FPLATEAUExtentEditor::SpawnTab))
        .SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FPLATEAUExtentEditor::UnregisterTabSpawner(const TSharedRef<class FTabManager>& InTabManager) {
    InTabManager->UnregisterTabSpawner(TabId);
}

FPLATEAUExtentEditor::FPLATEAUExtentEditor() {
    AreaMeshCodeMap.Reset();
}

FPLATEAUExtentEditor::~FPLATEAUExtentEditor() {}

TSharedRef<SDockTab> FPLATEAUExtentEditor::SpawnTab(const FSpawnTabArgs& Args) {
    TWeakPtr<FPLATEAUExtentEditor> WeakSharedThis(SharedThis(this));

    const auto Viewport = SNew(SPLATEAUExtentEditorViewport)
        .ExtentEditor(WeakSharedThis);

    TSharedRef< SDockTab > DockableTab =
        SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [Viewport];

    Viewport->SetOwnerTab(DockableTab);

    return DockableTab;
}

const FString& FPLATEAUExtentEditor::GetSourcePath() const {
    return SourcePath;
}

void FPLATEAUExtentEditor::SetSourcePath(const FString& Path) {
    SourcePath = Path;
}

const FString& FPLATEAUExtentEditor::GetAreaSourcePath() const {
    return AreaSourcePath;
}

void FPLATEAUExtentEditor::SetAreaSourcePath(const FString& InAreaSourcePath) {
    AreaSourcePath = InAreaSourcePath;
}

bool FPLATEAUExtentEditor::bSelectedArea() const {
    return Algo::AnyOf(GetAreaMeshCodeMap(), [](const TTuple<FString, FPLATEAUMeshCodeGizmo>& MeshCodeGizmoTuple) {
        return MeshCodeGizmoTuple.Value.bSelectedArea();
    });
}

TArray<FString> FPLATEAUExtentEditor::GetSelectedCodes() const {
    TArray<FString> Codes;
    for (auto [_, Value] : AreaMeshCodeMap) {
        if (Value.bSelectedArea()) {
            Codes.Append(Value.GetSelectedMeshIds());
        }
    }
    return Codes;
}

TMap<FString, FPLATEAUMeshCodeGizmo> FPLATEAUExtentEditor::GetAreaMeshCodeMap() const {
    return AreaMeshCodeMap;
}

void FPLATEAUExtentEditor::SetAreaMeshCodeMap(const FString& MeshCode, const FPLATEAUMeshCodeGizmo& MeshCodeGizmo) {
    AreaMeshCodeMap.Emplace(MeshCode, MeshCodeGizmo);
}

void FPLATEAUExtentEditor::ResetAreaMeshCodeMap() {
    AreaMeshCodeMap.Reset();
}

FPLATEAUGeoReference FPLATEAUExtentEditor::GetGeoReference() const {
    return GeoReference;
}

void FPLATEAUExtentEditor::SetGeoReference(const FPLATEAUGeoReference& InGeoReference) {
    GeoReference = InGeoReference;
}

const bool FPLATEAUExtentEditor::IsImportFromServer() const {
    return bImportFromServer;
}

void FPLATEAUExtentEditor::SetImportFromServer(bool InBool) {
    bImportFromServer = InBool;
}

std::shared_ptr<plateau::network::Client> FPLATEAUExtentEditor::GetClientPtr() const {
    return ClientPtr;
}

void FPLATEAUExtentEditor::SetClientPtr(const std::shared_ptr<plateau::network::Client>& InClientPtr) {
    ClientPtr = InClientPtr;
}

const std::string& FPLATEAUExtentEditor::GetServerDatasetID() const {
    return ServerDatasetID;
}

void FPLATEAUExtentEditor::SetServerDatasetID(const std::string& InID) {
    ServerDatasetID = InID;
}

const plateau::dataset::PredefinedCityModelPackage& FPLATEAUExtentEditor::GetLocalPackageMask() const {
    return LocalPackageMask;
}

void FPLATEAUExtentEditor::SetLocalPackageMask(const plateau::dataset::PredefinedCityModelPackage& InPackageMask) {
    LocalPackageMask = InPackageMask;
}

const plateau::dataset::PredefinedCityModelPackage& FPLATEAUExtentEditor::GetServerPackageMask() const {
    return ServerPackageMask;
}

const plateau::geometry::GeoCoordinate FPLATEAUExtentEditor::GetSelectedCenterLatLon() const {
    const auto& SelectedCodes = GetSelectedCodes();

    if (SelectedCodes.Num() == 0)
        return plateau::geometry::GeoCoordinate(0.0, 0.0, 0.0);

    // 選択された地域メッシュ全てを囲む範囲を計算
    plateau::geometry::Extent NativeExtent(
        plateau::geometry::GeoCoordinate(180.0, 180.0, 0.0),
        plateau::geometry::GeoCoordinate(-180.0, -180.0, 0.0));

    for (const auto& Code : SelectedCodes) {
        const auto NativePartialExtent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*Code)).getExtent();
        NativeExtent.min.latitude = std::min(NativeExtent.min.latitude, NativePartialExtent.min.latitude);
        NativeExtent.min.longitude = std::min(NativeExtent.min.longitude, NativePartialExtent.min.longitude);
        NativeExtent.max.latitude = std::max(NativeExtent.max.latitude, NativePartialExtent.max.latitude);
        NativeExtent.max.longitude = std::max(NativeExtent.max.longitude, NativePartialExtent.max.longitude);
    }
    return NativeExtent.centerPoint();
}

const FVector3d FPLATEAUExtentEditor::GetSelectedCenterPoint(const int ZoneID) const {
    // 中心点の緯度経度計算
    const auto CenterLatLon = GetSelectedCenterLatLon();

    // 平面直角座標系への変換
    auto GeoReferenceWithoutOffset = FPLATEAUGeoReference();
    GeoReferenceWithoutOffset.ZoneID = ZoneID;
    GeoReferenceWithoutOffset.UpdateNativeData();

    const auto CenterPoint = GeoReferenceWithoutOffset.GetData().project(CenterLatLon);
    return FVector3d(CenterPoint.x, CenterPoint.y, CenterPoint.z);
}

void FPLATEAUExtentEditor::SetServerPackageMask(const plateau::dataset::PredefinedCityModelPackage& InPackageMask) {
    ServerPackageMask = InPackageMask;
}

const FVector3d FPLATEAUExtentEditor::GetCenterByMeshCode(const FString& Code) const {
    const auto Extent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*Code)).getExtent();
    const auto CenterLatLon = Extent.centerPoint();
    auto GeoRef= GetGeoReference();
    const auto CenterPoint = GeoRef.GetData().project(CenterLatLon);
    return FVector3d(CenterPoint.x, CenterPoint.y, CenterPoint.z);
}

const FBox FPLATEAUExtentEditor::GetBoxByMeshCode(const FString& Code) const {
    const auto Extent = plateau::dataset::MeshCode(TCHAR_TO_UTF8(*Code)).getExtent();
    auto GeoRef = GetGeoReference();
    const auto Min = GeoRef.GetData().project(Extent.min);
    const auto Max = GeoRef.GetData().project(Extent.max);    
    return FBox(FVector3d(Min.x, Min.y, Min.z), FVector3d(Max.x, Max.y, Max.z));
}

#undef LOCTEXT_NAMESPACE
