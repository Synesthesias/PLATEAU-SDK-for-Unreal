// Copyright © 2023 Ministry of Land、Infrastructure and Transport


#include "SPLATEAUMainTab.h"
#include "PLATEAUEditorStyle.h"

#define LOCTEXT_NAMESPACE "SPLATEAUMainTab"
#define COLOR_TABSELECT_BG FColor(255, 255, 255, 90)

void SPLATEAUMainTab::Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;


    ChildSlot
        [
            SNew(SOverlay)
            .RenderTransformPivot(FVector2D(0.0f, 0.0f))
        + SOverlay::Slot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Center)
        [
            CreateTabBackground()
        ]
    + SOverlay::Slot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Center)
        [
            CreateTabSelectBackground()
        ]
    + SOverlay::Slot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Center)
        [
            CreateTabButtons()
        ]

    // ロゴ
    + SOverlay::Slot()
        .Padding(FMargin(0, 105, 0, 0))
        [
            SNew(SBorder)
            .BorderImage(Style->GetBrush(TEXT("PLATEAUEditor.LogoBackground")))
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.LogoImage"))
        ]
        ]
        ];
}

bool SPLATEAUMainTab::IsCurrentIndex(const int ID) {
    return ID == SelectingID;
}

TSharedRef<SHorizontalBox> SPLATEAUMainTab::CreateTabButtons() {
    return
        //スペーサーおよび画像を生成
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(1)
        .Padding(FMargin(0, 0, 0, 0))
        [SNew(SSpacer)]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(16.5f, 23.75f, 16.5f, 23.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(72)
        .HeightOverride(56)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabImportIcon"))
        .OnMouseButtonDown_Lambda([&](const FGeometry&, const FPointerEvent&) {
        OnButtonClicked(1);
        return FReply::Handled();
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(16.5f, 23.75f, 16.5f, 23.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(72)
        .HeightOverride(56)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabAdjustIcon"))
        .OnMouseButtonDown_Lambda([&](const FGeometry&, const FPointerEvent&) {
        OnButtonClicked(2);
        return FReply::Handled();
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(16.5f, 23.75f, 16.5f, 23.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(72)
        .HeightOverride(56)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabExportIcon"))
        .OnMouseButtonDown_Lambda([&](const FGeometry&, const FPointerEvent&) {
        OnButtonClicked(3);
        return FReply::Handled();
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .FillWidth(1)
        .Padding(FMargin(0, 0, 0, 0))
        [SNew(SSpacer)];
}

TSharedRef<SHorizontalBox> SPLATEAUMainTab::CreateTabSelectBackground() {
    return
        //TabButtonと同じようにスペーサーおよび画像を生成
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(1)
        .Padding(FMargin(0, 0, 0, 0))
        [SNew(SSpacer)]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(12.5f, 14.75f, 12.5f, 14.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(80)
        .HeightOverride(70.5f)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabSelectBack"))
        .ColorAndOpacity_Lambda([=]() {
        return GetTabSelectBGColor(1);
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(12.5f, 14.75f, 12.5f, 14.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(80)
        .HeightOverride(70.5f)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabSelectBack"))
        .ColorAndOpacity_Lambda([=]() {
        return GetTabSelectBGColor(2);
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(FMargin(12.5f, 14.75f, 12.5f, 14.75f))
        [
            SNew(SBox)
            .Padding(FMargin(0, 0, 0, 0))
        .HAlign(HAlign_Left)
        .VAlign(VAlign_Top)
        .WidthOverride(80)
        .HeightOverride(70.5f)
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabSelectBack"))
        .ColorAndOpacity_Lambda([=]() {
        return GetTabSelectBGColor(3);
            })
        ]
        ]
    + SHorizontalBox::Slot()
        .FillWidth(1)
        .Padding(FMargin(0, 0, 0, 0))
        [SNew(SSpacer)];
}

TSharedRef<SBox> SPLATEAUMainTab::CreateTabBackground() {
    return
        SNew(SBox)
        .HeightOverride(100)
        .WidthOverride(400)
        .RenderTransformPivot(FVector2D(0.0f, 0.0f))
        [
            SNew(SImage)
            .Image(Style->GetBrush("PLATEAUEditor.TabBackground"))
        .ColorAndOpacity(FSlateColor(FColor(0, 0, 0, 100)))
        ];
}

void SPLATEAUMainTab::OnButtonClicked(const int ID) {
    SelectingID = ID;
}

const FSlateColor SPLATEAUMainTab::GetTabSelectBGColor(const int ID) {
    return SelectingID == ID ? FSlateColor(COLOR_TABSELECT_BG) : FSlateColor(FColor(0, 0, 0, 0));
}

#undef LOCTEXT_NAMESPACE
