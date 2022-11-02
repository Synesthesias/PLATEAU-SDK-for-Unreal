// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "PLATEAUCityModelLoader.h"

#include "Misc/ScopedSlowTask.h"

#include "Widgets/Docking/SDockTab.h"
#include "EditorViewportTabContent.h"
#include "Engine/Selection.h"



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

FPLATEAUExtentEditor::FPLATEAUExtentEditor() {}

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

FPLATEAUGeoReference FPLATEAUExtentEditor::GetGeoReference() const {
    return GeoReference;
}

void FPLATEAUExtentEditor::SetGeoReference(const FPLATEAUGeoReference& InGeoReference) {
    GeoReference = InGeoReference;
}

const TOptional<FPLATEAUExtent>& FPLATEAUExtentEditor::GetExtent() const {
    return Extent;
}

void FPLATEAUExtentEditor::SetExtent(const FPLATEAUExtent& InExtent) {
    Extent = InExtent;
}

void FPLATEAUExtentEditor::RegisterLoaderActor(TWeakObjectPtr<APLATEAUCityModelLoader> InLoader) {
    Loader = InLoader;
}

void FPLATEAUExtentEditor::UnregisterLoaderActor() {
    Loader = nullptr;
}

void FPLATEAUExtentEditor::HandleClickOK() const {
    if (!Extent.IsSet() || !Loader.IsValid())
        return;

    Loader->Extent = Extent.GetValue();
}

#undef LOCTEXT_NAMESPACE
