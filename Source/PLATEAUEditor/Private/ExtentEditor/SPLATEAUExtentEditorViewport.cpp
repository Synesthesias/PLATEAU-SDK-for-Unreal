// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "PLATEAUBasemap.h"
#include "PLATEAUTextureLoader.h"

#include "AdvancedPreviewScene.h"
#include "SSubobjectEditor.h"
#include "Slate/SceneViewport.h"
#include "BlueprintEditorSettings.h"
#include "SlateOptMacros.h"

#include "plateau/dataset/dataset_source.h"
#include "plateau/dataset/i_dataset_accessor.h"
#include "plateau/basemap/tile_projection.h"
#include "plateau/basemap/vector_tile_downloader.h"
#include "plateau/geometry/geo_coordinate.h"
#include "plateau/geometry/geo_reference.h"

#include <filesystem>
#include <fstream>
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "AssetRegistryModule.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Async/Async.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"


#define LOCTEXT_NAMESPACE "SPLATEAUExtentEditorViewport"

SPLATEAUExtentEditorViewport::SPLATEAUExtentEditorViewport()
    : PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()))) {}

SPLATEAUExtentEditorViewport::~SPLATEAUExtentEditorViewport() {
    if (ViewportClient.IsValid()) {
        ViewportClient->Viewport = nullptr;
    }
}

void SPLATEAUExtentEditorViewport::Construct(const FArguments& InArgs) {
    ExtentEditorPtr = InArgs._ExtentEditor;

    SEditorViewport::Construct(SEditorViewport::FArguments());

    if (ViewportClient.IsValid()) {
        UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
        if (World != nullptr) {
            World->ChangeFeatureLevel(GWorld->FeatureLevel);
        }
        const auto& SourcePath = ExtentEditorPtr.Pin()->GetSourcePath();
        const auto ClientRef = ExtentEditorPtr.Pin()->GetClientRef();
        const auto ID = ExtentEditorPtr.Pin()->GetServerDatasetID();
        const auto bImportFromServer = ExtentEditorPtr.Pin()->IsImportFromServer();
        std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
        if (bImportFromServer) {
            try {
                const auto DatasetSource = plateau::dataset::DatasetSource::createServer(ID, ClientRef);
                DatasetAccessor = DatasetSource.getAccessor();
            }
            catch (...) {
                UE_LOG(LogTemp, Error, TEXT("Failed to open source ID: %s"), *ID.c_str());
            }
        }
        else {
            try {
                const auto DatasetSource = plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*SourcePath));
                DatasetAccessor = DatasetSource.getAccessor();
            }
            catch (...) {
                UE_LOG(LogTemp, Error, TEXT("Failed to open udx source path: %s"), *SourcePath);
            }
        }

        if (DatasetAccessor == nullptr)
            return;

        if (DatasetAccessor->getMeshCodes().size() == 0)
            return;

        auto GeoReference = ExtentEditorPtr.Pin()->GetGeoReference();
        const auto RawCenterPoint = DatasetAccessor->calculateCenterPoint(GeoReference.GetData());
        GeoReference.ReferencePoint.X = RawCenterPoint.x;
        GeoReference.ReferencePoint.Y = RawCenterPoint.y;
        GeoReference.ReferencePoint.Z = RawCenterPoint.z;
        ExtentEditorPtr.Pin()->SetGeoReference(GeoReference);
        
        ViewportClient->Initialize(DatasetAccessor);
    }
}

bool SPLATEAUExtentEditorViewport::IsVisible() const {
    // We consider the viewport to be visible if the reference is valid
    return ViewportWidget.IsValid() && SEditorViewport::IsVisible();
}

TSharedRef<FEditorViewportClient> SPLATEAUExtentEditorViewport::MakeEditorViewportClient() {
    // Construct a new viewport client instance.
    ViewportClient = MakeShareable(
        new FPLATEAUExtentEditorViewportClient(
            ExtentEditorPtr,
            SharedThis(this),
            PreviewScene.ToSharedRef()));
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

#undef LOCTEXT_NAMESPACE
