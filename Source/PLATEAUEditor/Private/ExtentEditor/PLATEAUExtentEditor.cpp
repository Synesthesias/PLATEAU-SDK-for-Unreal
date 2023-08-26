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

FPLATEAUExtent FPLATEAUExtentEditor::GetExtent() const {
    
    for (auto ItemIter = AreaMeshCodeMap.CreateConstIterator(); ItemIter; ++ItemIter) {
        // const auto& MeshCodeGizmo = ItemIter->Value;
        // if (!MeshCodeGizmo.GetbSelected())
        //     continue;
        //
        // const TVec3d Min(MeshCodeGizmo.GetMin().X, MeshCodeGizmo.GetMin().Y, 0);
        // const TVec3d Max(MeshCodeGizmo.GetMax().X, MeshCodeGizmo.GetMax().Y, 0);
        // auto RawMin = GetGeoReference().GetData().unproject(Min);
        // auto RawMax = GetGeoReference().GetData().unproject(Max);
        //
        // // 座標系変換時に緯度の大小が逆転するので再設定を行う。
        // const auto Tmp = RawMin.latitude;
        // RawMin.latitude = FMath::Min(RawMin.latitude, RawMax.latitude);
        // RawMax.latitude = FMath::Max(Tmp, RawMax.latitude);
    }

    return FPLATEAUExtent(plateau::geometry::Extent(plateau::geometry::GeoCoordinate(), plateau::geometry::GeoCoordinate()));
    // return FPLATEAUExtent(plateau::geometry::Extent(RawMin, RawMax));
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

void FPLATEAUExtentEditor::SetServerPackageMask(const plateau::dataset::PredefinedCityModelPackage& InPackageMask) {
    ServerPackageMask = InPackageMask;
}

#undef LOCTEXT_NAMESPACE
