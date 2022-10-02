// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#include "AdvancedPreviewScene.h"
#include "SSubobjectEditor.h"
#include "Slate/SceneViewport.h"
#include "BlueprintEditorSettings.h"
#include "PLATEAUExtentEditor.h"
#include "ExtentEditor/MeshCodeGizmoComponent.h"

#include "plateau/udx/mesh_code.h"
#include "plateau/udx/udx_file_collection.h"

SPLATEAUExtentEditorViewport::SPLATEAUExtentEditorViewport()
    : PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()))) {}


void SPLATEAUExtentEditorViewport::Construct(const FArguments& InArgs) {
    bIsActiveTimerRegistered = false;

    // Save off the Blueprint editor reference, we'll need this later
    ExtentEditorPtr = InArgs._ExtentEditor;

    SEditorViewport::Construct(SEditorViewport::FArguments());
    
    if (ViewportClient.IsValid()) {
        UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
        if (World != nullptr) {
            World->ChangeFeatureLevel(GWorld->FeatureLevel);
        }
        std::shared_ptr<plateau::udx::UdxFileCollection> FileCollection;
        const auto& SourcePath = ExtentEditorPtr.Pin()->GetSourcePath();
        try {
            FileCollection = plateau::udx::UdxFileCollection::find(TCHAR_TO_UTF8(*SourcePath));
        }
        catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Failed to open udx source path: %s"), *SourcePath);
        }

        if (FileCollection == nullptr)
            return;

        if (FileCollection->getMeshCodes().size() == 0)
            return;

        ViewportClient->InitCamera();

        plateau::geometry::GeoReference GeoReference(TVec3d(0, 0, 0), 1, plateau::geometry::CoordinateSystem::NWU);
        const auto ReferencePoint = GeoReference.project(FileCollection->getMeshCodes().begin()->getExtent().min);
        GeoReference.setReferencePoint(ReferencePoint);

        for (const auto& MeshCode : FileCollection->getMeshCodes()) {
            const auto MeshCodeGizmo = NewObject<UMeshCodeGizmoComponent>();
            MeshCodeGizmo->Init(MeshCode, GeoReference);
            ViewportClient->GetPreviewScene()->AddComponent(MeshCodeGizmo, FTransform::Identity);
        }
    }

    UEditorEngine* Editor = (UEditorEngine*)GEngine;
    PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda([this](ERHIFeatureLevel::Type NewFeatureLevel) {
        if (ViewportClient.IsValid()) {
            UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
            if (World != nullptr) {
                World->ChangeFeatureLevel(NewFeatureLevel);

                RequestRefresh(false);
            }
        }
        });
    
    RequestRefresh(true);
}

SPLATEAUExtentEditorViewport::~SPLATEAUExtentEditorViewport() {
    UEditorEngine* Editor = (UEditorEngine*)GEngine;
    Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);

    if (ViewportClient.IsValid()) {
        // Reset this to ensure it's no longer in use after destruction
        ViewportClient->Viewport = NULL;
    }
}

bool SPLATEAUExtentEditorViewport::IsVisible() const {
    // We consider the viewport to be visible if the reference is valid
    return ViewportWidget.IsValid() && SEditorViewport::IsVisible();
}

TSharedRef<FEditorViewportClient> SPLATEAUExtentEditorViewport::MakeEditorViewportClient() {

    // Construct a new viewport client instance.
    ViewportClient = MakeShareable(new FPLATEAUExtentEditorViewportClient(SharedThis(this), PreviewScene.ToSharedRef()));
    ViewportClient->SetRealtime(true);
    ViewportClient->bSetListenerPosition = false;
    ViewportClient->VisibilityDelegate.BindSP(this, &SPLATEAUExtentEditorViewport::IsVisible);

    return ViewportClient.ToSharedRef();
}

void SPLATEAUExtentEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) {
    SEditorViewport::PopulateViewportOverlays(Overlay);

    // add the feature level display widget
    Overlay->AddSlot()
        .VAlign(VAlign_Bottom)
        .HAlign(HAlign_Right)
        .Padding(5.0f)
        [
            BuildFeatureLevelWidget()
        ];
}

void SPLATEAUExtentEditorViewport::BindCommands() {}

void SPLATEAUExtentEditorViewport::Invalidate() {
    //ViewportClient->Invalidate();
}

void SPLATEAUExtentEditorViewport::ToggleIsSimulateEnabled() {
    // Make the viewport visible if the simulation is starting.
    //if (!ViewportClient->GetIsSimulateEnabled()) {
    //    if (GetDefault<UBlueprintEditorSettings>()->bShowViewportOnSimulate) {
    //        BlueprintEditorPtr.Pin()->GetTabManager()->TryInvokeTab(FBlueprintEditorTabs::SCSViewportID);
    //    }
    //}

    //ViewportClient->ToggleIsSimulateEnabled();
}

void SPLATEAUExtentEditorViewport::EnablePreview(bool bEnable) {
    //const FText SystemDisplayName = NSLOCTEXT("BlueprintEditor", "RealtimeOverrideMessage_Blueprints", "the active blueprint mode");
    //if (bEnable) {
    //    // Restore the previously-saved realtime setting
    //    ViewportClient->RemoveRealtimeOverride(SystemDisplayName);
    //} else {
    //    // Disable and store the current realtime setting. This will bypass real-time rendering in the preview viewport (see UEditorEngine::UpdateSingleViewportClient).
    //    const bool bShouldBeRealtime = false;
    //    ViewportClient->AddRealtimeOverride(bShouldBeRealtime, SystemDisplayName);
    //}
}

void SPLATEAUExtentEditorViewport::RequestRefresh(bool bResetCamera, bool bRefreshNow) {
    //if (bRefreshNow) {
    //    if (ViewportClient.IsValid()) {
    //        ViewportClient->InvalidatePreview(bResetCamera);
    //    }
    //} else {
    //    // Defer the update until the next tick. This way we don't accidentally spawn the preview actor in the middle of a transaction, for example.
    //    if (!bIsActiveTimerRegistered) {
    //        bIsActiveTimerRegistered = true;
    //        RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SPLATEAUExtentEditorViewport::DeferredUpdatePreview, bResetCamera));
    //    }
    //}
}

void SPLATEAUExtentEditorViewport::OnComponentSelectionChanged() {
    // When the component selection changes, make sure to invalidate hit proxies to sync with the current selection
    SceneViewport->Invalidate();
}

void SPLATEAUExtentEditorViewport::OnFocusViewportToSelection() {
    //ViewportClient->FocusViewportToSelection();
}

bool SPLATEAUExtentEditorViewport::ShouldShowViewportCommands() const {
    // Hide if actively debugging
    return !GIntraFrameDebuggingGameThread;
}

bool SPLATEAUExtentEditorViewport::GetIsSimulateEnabled() {
    // return ViewportClient->GetIsSimulateEnabled();
    return false;
}

void SPLATEAUExtentEditorViewport::SetOwnerTab(TSharedRef<SDockTab> Tab) {
    OwnerTab = Tab;
}

TSharedPtr<SDockTab> SPLATEAUExtentEditorViewport::GetOwnerTab() const {
    return OwnerTab.Pin();
}

FReply SPLATEAUExtentEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    //TSharedPtr<SSubobjectEditor> SubobjectEditor = BlueprintEditorPtr.Pin()->GetSubobjectEditor();

    //return SubobjectEditor->TryHandleAssetDragDropOperation(DragDropEvent);
    return FReply::Handled();
}

EActiveTimerReturnType SPLATEAUExtentEditorViewport::DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera) {
    if (ViewportClient.IsValid()) {
        //ViewportClient->InvalidatePreview(bResetCamera);
    }

    bIsActiveTimerRegistered = false;
    return EActiveTimerReturnType::Stop;
}
