// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#include "AdvancedPreviewScene.h"
#include "SSubobjectEditor.h"
#include "Slate/SceneViewport.h"
#include "BlueprintEditorSettings.h"
#include "SlateOptMacros.h"

#include "plateau/udx/udx_file_collection.h"


#define LOCTEXT_NAMESPACE "SPLATEAUExtentEditorViewport"

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

        auto GeoReference = ExtentEditorPtr.Pin()->GetGeoReference();
        const auto RawCenterPoint = FileCollection->calculateCenterPoint(GeoReference.GetData());
        GeoReference.ReferencePoint.X = RawCenterPoint.x;
        GeoReference.ReferencePoint.Y = RawCenterPoint.y;
        GeoReference.ReferencePoint.Z = RawCenterPoint.z;
        ExtentEditorPtr.Pin()->SetGeoReference(GeoReference);

        ViewportClient->Initialize(*FileCollection);
    }
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
    ViewportClient = MakeShareable(new FPLATEAUExtentEditorViewportClient(ExtentEditorPtr, SharedThis(this), PreviewScene.ToSharedRef()));
    ViewportClient->SetRealtime(true);
    ViewportClient->bSetListenerPosition = false;
    ViewportClient->VisibilityDelegate.BindSP(this, &SPLATEAUExtentEditorViewport::IsVisible);

    return ViewportClient.ToSharedRef();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUExtentEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) {
    SEditorViewport::PopulateViewportOverlays(Overlay);

    // add the feature level display widget
    Overlay->AddSlot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Left)
        .Padding(5.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("FloatingBorder"))
        .Padding(4.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                if (GetOwnerTab())
                    GetOwnerTab()->RequestCloseTab();
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Cancel Button", "キャンセル"))
                ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                const auto Extent = ViewportClient->GetExtent();
                ExtentEditorPtr.Pin()->SetExtent(Extent);
                ExtentEditorPtr.Pin()->HandleClickOK();

                if (GetOwnerTab())
                    GetOwnerTab()->RequestCloseTab();
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("OK Button", "決定"))
                ]
        ]
        ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUExtentEditorViewport::BindCommands() {}

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

#undef LOCTEXT_NAMESPACE
