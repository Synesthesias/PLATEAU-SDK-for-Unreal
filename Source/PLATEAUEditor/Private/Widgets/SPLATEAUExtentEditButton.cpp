#include "SPLATEAUExtentEditButton.h"
#include <plateau/udx/udx_file_collection.h>

#include "AssetSelection.h"
#include "PLATEAUEditor.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUFeatureImportSettingsDetails.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "SlateOptMacros.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#define LOCTEXT_NAMESPACE "SPLATEAUExtentEditButton"

using namespace plateau::udx;

SPLATEAUExtentEditButton::SPLATEAUExtentEditButton() {
    ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();

    ZoneIDCache = ExtentEditor->GetGeoReference().ZoneID;
    SourcePathCache = ExtentEditor->GetSourcePath();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUExtentEditButton::Construct(const FArguments& InArgs) {
    ChildSlot
        [SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(0, 255, 255, 255))
        .OnClicked_Lambda(
            [this, ZoneID = InArgs._ZoneID,
            SourcePath = InArgs._SourcePath]() {
        IPLATEAUEditorModule::Get().GetExtentEditor()->SetSourcePath(SourcePath.Get());

        // TODO: ExtentEditorに委譲
        // ビューポートの操作性向上のため100分の1スケールで設定
        const plateau::geometry::GeoReference RawGeoReference(ZoneID.Get(), {}, 1, plateau::geometry::CoordinateSystem::NWU);
        ExtentEditor->SetGeoReference(RawGeoReference);

        const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
        GlobalTabManager->TryInvokeTab(FPLATEAUExtentEditor::TabId);

        return FReply::Handled();
    })
        .Content()
        [SNew(STextBlock)
        .Justification(ETextJustify::Center)
        .Margin(FMargin(80, 14, 80, 14))
        .Text(LOCTEXT("Edit Extent Button", "範囲選択"))
        ]]

    + SVerticalBox::Slot()
        [SNew(STextBlock)
        .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 0, 0, 10))
        .Text_Lambda(
            [this, ZoneID = InArgs._ZoneID,
            SourcePath = InArgs._SourcePath]{
                // TODO: 別の場所にロジック置くべき？
                if (ZoneIDCache != ZoneID.Get() || SourcePathCache != SourcePath.Get()) {
                    ExtentEditor->ResetExtent();
                    ZoneIDCache = ZoneID.Get();
                    SourcePathCache = SourcePath.Get();
                }

                return ExtentEditor->GetExtent().IsSet()
                    ? LOCTEXT("Edit Extent Configured", "範囲選択 : セット済")
                    : LOCTEXT("Edit Extent Unconfigured", "範囲選択 : 未セット");
            })]

        ];
}

bool SPLATEAUExtentEditButton::IsExtentSet() const {
    return ExtentEditor->GetExtent().IsSet();
}

TOptional<FPLATEAUExtent> SPLATEAUExtentEditButton::GetExtent() const {
    return ExtentEditor->GetExtent();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
