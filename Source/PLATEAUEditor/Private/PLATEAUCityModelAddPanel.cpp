// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelAddPanel.h"

#include "SlateOptMacros.h"
#include "AssetSelection.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "PLATEAUCityModelLoader.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "FPLATEAUEditorModule"


FPLATEAUCityModelAddPanel::FPLATEAUCityModelAddPanel() {}

FPLATEAUCityModelAddPanel::~FPLATEAUCityModelAddPanel() {}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FPLATEAUCityModelAddPanel::UpdateWindow(TWeakPtr<SWindow> MyWindow) {
    const auto ScrollBox = SNew(SScrollBox);
    ScrollBox->AddSlot()[
        CreateSourcePathSelectPanel(MyWindow)
    ];
    MyWindow.Pin()->SetContent(ScrollBox);
}

TSharedRef<SVerticalBox> FPLATEAUCityModelAddPanel::CreateSourcePathSelectPanel(TWeakPtr<SWindow> MyWindow) {
    return
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .IsReadOnly(true)
        .Text(LOCTEXT("SelectSource", "インポート元フォルダ選択"))
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(20, 5, 20, 5))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &FPLATEAUCityModelAddPanel::OnBtnSelectGmlFileClicked, MyWindow)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Ref Button", "参照..."))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 10))
        [
            SNew(SEditableTextBox)
            .IsReadOnly(true)
        .Text(FText::FromString(SourcePath))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(20, 5, 20, 5))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                FAssetData EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
                UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
                auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
                Cast<APLATEAUCityModelLoader>(Actor)->Source = SourcePath;
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("AddButton2", "追加"))
                ]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply FPLATEAUCityModelAddPanel::OnBtnSelectGmlFileClicked(TWeakPtr<SWindow> MyWindow) {
    const void* WindowHandle = MyWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    const FString DialogTitle("Select folder.");
    FString OutFolderName;

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (DesktopPlatform->OpenDirectoryDialog(
        WindowHandle,
        DialogTitle,
        SourcePath,
        OutFolderName)) {
        SourcePath = OutFolderName;
        UpdateWindow(MyWindow);
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
