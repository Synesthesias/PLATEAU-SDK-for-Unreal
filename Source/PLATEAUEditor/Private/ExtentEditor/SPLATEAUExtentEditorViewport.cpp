// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#include "AdvancedPreviewScene.h"
#include "SSubobjectEditor.h"
#include "Slate/SceneViewport.h"
#include "BlueprintEditorSettings.h"
#include "SlateOptMacros.h"

#include "plateau/dataset/dataset_source.h"
#include "plateau/dataset/i_dataset_accessor.h"
#include "plateau/basemap/vector_tile_downloader.h"
#include "plateau/geometry/geo_reference.h"

#include <filesystem>
#include <fstream>
#include "Misc/FileHelper.h"
#include "IImageWrapperModule.h"
#include "PLATEAUEditor.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUWindow.h"
#include "Async/Async.h"
#include "Widgets/PLATEAUSDKEditorUtilityWidget.h"


#define LOCTEXT_NAMESPACE "SPLATEAUExtentEditorViewport"


SPLATEAUExtentEditorViewport::SPLATEAUExtentEditorViewport() : PreviewScene(
    MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()))) {
}

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
        const auto ClientRef = ExtentEditorPtr.Pin()->GetClientPtr();
        const auto ID = ExtentEditorPtr.Pin()->GetServerDatasetID();
        const auto bImportFromServer = ExtentEditorPtr.Pin()->IsImportFromServer();
        std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
        if (bImportFromServer) {
            try {
                const auto DatasetSource = plateau::dataset::DatasetSource::createServer(ID, *ClientRef);
                DatasetAccessor = DatasetSource.getAccessor();
            } catch (...) {
                UE_LOG(LogTemp, Error, TEXT("Failed to open source ID: %s"), *ID.c_str());
            }
        } else {
            try {
                const auto DatasetSource = plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*SourcePath));
                DatasetAccessor = DatasetSource.getAccessor();
            } catch (...) {
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
    ViewportClient = MakeShareable(new FPLATEAUExtentEditorViewportClient(ExtentEditorPtr, SharedThis(this), PreviewScene.ToSharedRef()));
    ViewportClient->SetRealtime(true);
    ViewportClient->bSetListenerPosition = false;
    ViewportClient->VisibilityDelegate.BindSP(this, &SPLATEAUExtentEditorViewport::IsVisible);

    return ViewportClient.ToSharedRef();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUExtentEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) {
    SEditorViewport::PopulateViewportOverlays(Overlay);

    Overlay->AddSlot().VAlign(VAlign_Top).HAlign(HAlign_Left).Padding(5.f)
    [
        SNew(SBorder).BorderImage(FAppStyle::Get().GetBrush("FloatingBorder")).Padding(10.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(FMargin(5.f, 5.f, 5.f, 8.f))
            [
                SNew(STextBlock).Text(LOCTEXT("OverlayAreaSelectionText", "範囲選択")).
                TextStyle(ExtentEditorPtr.Pin()->GetEditorStyle(), "PLATEAUEditor.Bold.13").ColorAndOpacity(FLinearColor::White)                
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(FMargin(5.f))
            [
                SNew(SButton).VAlign(VAlign_Center).ForegroundColor(FColor::White).ButtonColorAndOpacity(FColor(10, 90, 80, 255)).
                OnClicked_Lambda([this]() {
                    if (GetOwnerTab())
                        GetOwnerTab()->RequestCloseTab();
                    return FReply::Handled();
                }).Content()
                [
                    SNew(STextBlock).Justification(ETextJustify::Center).Margin(FMargin(0, 5.f, 0, 5.f)).Text(LOCTEXT("Cancel Button", "キャンセル"))
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(FMargin(5.f))
            [
                SNew(SButton).VAlign(VAlign_Center).ForegroundColor(FColor::White).ButtonColorAndOpacity(FColor(10, 90, 80, 255))
                .OnClicked_Lambda([this] {
                    ViewportClient->InitHandlePosition();
                    return FReply::Handled();
                }).Content()
                [
                    SNew(STextBlock).Justification(ETextJustify::Center).Margin(FMargin(0, 5.f, 0, 5.f)).Text(LOCTEXT("Area Reset Button", "範囲選択リセット"))
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(FMargin(5.f))
            [
                SNew(SButton).VAlign(VAlign_Center).ForegroundColor(FColor::White).ButtonColorAndOpacity(FColor(10, 90, 80, 255)).
                OnClicked_Lambda([this]() {
                    const auto Extent = ViewportClient->GetExtent();
                    ExtentEditorPtr.Pin()->SetExtent(Extent);
                    const auto ReferencePoint = GetReferencePoint(ViewportClient->GetExtent().GetNativeData(), ExtentEditorPtr.Pin()->GetGeoReference().ZoneID);
                    const auto PackageMask = GetPackageMask();
                    const auto& EditorUtilityWidget = IPLATEAUEditorModule::Get().GetWindow()->GetEditorUtilityWidget();
                    if (EditorUtilityWidget != nullptr) {
                        const auto& PLATEAUSDKEditorUtilityWidget = dynamic_cast<UPLATEAUSDKEditorUtilityWidget*>(EditorUtilityWidget);
                        if (PLATEAUSDKEditorUtilityWidget != nullptr) {
                            PLATEAUSDKEditorUtilityWidget->AreaSelectSuccessInvoke(ReferencePoint, PackageMask);
                        }
                    } else {
                        UE_LOG(LogTemp, Warning, TEXT("PLATEAU SDK Widget Error"));
                        const FText Title = LOCTEXT("Warning", "警告");
                        const FText DialogText = LOCTEXT("WidgetError", "PLATEAU SDKに問題が発生しました。PLATEAU SDKを再起動して下さい。");
                        FMessageDialog::Open(EAppMsgType::Ok, DialogText, &Title);
                    }                                               
                    if (GetOwnerTab())
                        GetOwnerTab()->RequestCloseTab();
                    return FReply::Handled();
                }).Content()
                [
                    SNew(STextBlock).Justification(ETextJustify::Center).Margin(FMargin(0, 5.f, 0, 5.f)).Text(LOCTEXT("OK Button", "決定"))
                ]
            ]
        ]
    ];

    Overlay->AddSlot().VAlign(VAlign_Bottom).HAlign(HAlign_Left).Padding(5.f)
    [
        SNew(SBorder).BorderImage(FAppStyle::Get().GetBrush("FloatingBorder")).Padding(10.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(FMargin(5.f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoHeight().Padding(FMargin(5.f))
                [
                    SNew(STextBlock).Text(LOCTEXT("OverlayLodText", "LOD")).
                    TextStyle(ExtentEditorPtr.Pin()->GetEditorStyle(), "PLATEAUEditor.Bold.14").ColorAndOpacity(FLinearColor::White)
                ]
                + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(FMargin(5.f))
                [
                    SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
                    [
                        SNew(SImage).Image(ExtentEditorPtr.Pin()->GetEditorStyle()->GetBrush("PLATEAUEditor.Lod01"))
                    ]
                ]
                + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(FMargin(5.f))
                [
                    SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
                    [
                        SNew(SImage).Image(ExtentEditorPtr.Pin()->GetEditorStyle()->GetBrush("PLATEAUEditor.Lod02"))
                    ]
                ]
                + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(FMargin(5.f))
                [
                    SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
                    [
                        SNew(SImage).Image(ExtentEditorPtr.Pin()->GetEditorStyle()->GetBrush("PLATEAUEditor.Lod03"))
                    ]
                ]
                + SVerticalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Center).Padding(FMargin(5.f))
                [
                    SNew(SBox).WidthOverride(30.f).HeightOverride(30.f)
                    [
                        SNew(SImage).Image(ExtentEditorPtr.Pin()->GetEditorStyle()->GetBrush("PLATEAUEditor.Lod04"))
                    ]
                ]                
            ]
        ]
    ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUExtentEditorViewport::BindCommands() {
}

void SPLATEAUExtentEditorViewport::SetOwnerTab(TSharedRef<SDockTab> Tab) {
    OwnerTab = Tab;
}

TSharedPtr<SDockTab> SPLATEAUExtentEditorViewport::GetOwnerTab() const {
    return OwnerTab.Pin();
}

FVector3d SPLATEAUExtentEditorViewport::GetReferencePoint(const plateau::geometry::Extent Extent, const int ZoneID) {
    // 平面直角座標系への変換
    auto GeoReferenceWithoutOffset = FPLATEAUGeoReference();
    GeoReferenceWithoutOffset.ZoneID = ZoneID;
    GeoReferenceWithoutOffset.UpdateNativeData();

    const auto MinPoint = GeoReferenceWithoutOffset.GetData().project(Extent.min);
    const auto MaxPoint = GeoReferenceWithoutOffset.GetData().project(Extent.max);
    const auto NewExtentCenterRaw = (MinPoint + MaxPoint) / 2.0;
    return FVector3d(NewExtentCenterRaw.x, NewExtentCenterRaw.y, NewExtentCenterRaw.z);
}

int64 SPLATEAUExtentEditorViewport::GetPackageMask() const {
    const auto& Extent = ExtentEditorPtr.Pin()->GetExtent();
    if (ExtentEditorPtr.Pin()->IsImportFromServer()) {
        const auto ClientRef = ExtentEditorPtr.Pin()->GetClientPtr();
        const auto InDatasetSource = plateau::dataset::DatasetSource::createServer(ExtentEditorPtr.Pin()->GetServerDatasetID(), *ClientRef);
        const auto FilteredDatasetAccessor = InDatasetSource.getAccessor()->filter(Extent.GetValue().GetNativeData());
        const auto PackageMask = FilteredDatasetAccessor->getPackages();
        ExtentEditorPtr.Pin()->SetServerPackageMask(PackageMask);
        return static_cast<int64>(PackageMask);
    }

    const auto InDatasetSource = plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*ExtentEditorPtr.Pin()->GetSourcePath()));
    const auto FilteredDatasetAccessor = InDatasetSource.getAccessor()->filter(Extent.GetValue().GetNativeData());
    const auto PackageMask = FilteredDatasetAccessor->getPackages();
    ExtentEditorPtr.Pin()->SetLocalPackageMask(PackageMask);
    return static_cast<int64>(PackageMask);
}

#undef LOCTEXT_NAMESPACE
