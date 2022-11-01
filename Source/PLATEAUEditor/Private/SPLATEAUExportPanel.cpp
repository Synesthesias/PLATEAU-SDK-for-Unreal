// Fill out your copyright notice in the Description page of Project Settings.

#include "SPLATEAUExportPanel.h"

#include "PLATEAUEditorStyle.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "PLATEAUFeatureExportSettingsDetails.h"
#include "PLATEAUExportSettings.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "AssetSelection.h"
#include "DesktopPlatformModule.h"

#define LOCTEXT_NAMESPACE "SPLATEAUExportPanel"

namespace {
    static FText GetDisplayName(ExportModelType ModelType) {
        switch (ModelType) {
        case ExportModelType::OBJ: return LOCTEXT("OBJ", "OBJ");
        case ExportModelType::FBX: return LOCTEXT("FBX", "FBX");
        case ExportModelType::GLTF: return LOCTEXT("GLTF", "GLTF");
        default: return LOCTEXT("Others", "その他");
        }
    }
}

void SPLATEAUExportPanel::Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bAllowSearch = false;

    BuildingImportSettingsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    BuildingImportSettingsView->RegisterInstancedCustomPropertyLayout(
        UPLATEAUExportSettings::StaticClass(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUFeatureExportSettingsDetails::MakeInstance));
    BuildingImportSettingsView->SetObject(GetMutableDefault<UPLATEAUExportSettings>());

    ChildSlot
        [SNew(SVerticalBox)

    //エクスポート表示ヘッダー
    + SVerticalBox::Slot()
        .Padding(FMargin(0, 20.5, 0, 5))
        .AutoHeight()[
            SNew(SHeader)
            .HAlign(HAlign_Center)
            .Content()[
                SNew(STextBlock)
                .TextStyle(Style, "PLATEAUEditor.Heading1")
                .Text(LOCTEXT("Export ModelData", "モデルデータのエクスポートを行います。"))
            ]
        ]
        
    //選択オブジェクト表示ヘッダー
    + SVerticalBox::Slot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()[
            SNew(SHeader)
            .Content()[
                SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)[
                    SNew(STextBlock)
                    .TextStyle(Style, "PLATEAUEditor.Heading2")
                    .Text(LOCTEXT("Selecting Object", "選択オブジェクト"))
                ]
            ]
        ]

    //オブジェクト名表示部
    //TODO:アウトライナーで選択しているオブジェクト名の反映
    + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Fill)
        .Padding(FMargin(0, 15, 0, 15))[
            SNew(STextBlock)
            .Text(LOCTEXT("Object Name Here", "Object Name Here"))
            ]

    //選択オブジェクト表示ヘッダー
    + SVerticalBox::Slot()
    .Padding(FMargin(0, 10, 0, 10))
    .AutoHeight()[
        SNew(SHeader)
        .Content()[
            SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)[
                    SNew(STextBlock)
                    .TextStyle(Style, "PLATEAUEditor.Heading2")
                    .Text(LOCTEXT("Export Type", "出力形式"))
                    ]
                ]
            ]

    // 基準座標系の選択
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 10)[
        SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .Padding(19, 0, 0, 0)[
            SNew(STextBlock)
            .Text(LOCTEXT("Export Type", "出力形式"))] +
            SHorizontalBox::Slot()
            .Padding(0, 0, 19, 0)[
                SNew(SComboButton)
                .OnGetMenuContent_Lambda([this]() {
                    FMenuBuilder MenuBuilder(true, nullptr);
                    for (int i = 0;  i < (int)ExportModelType::ExportModelType_MAX; i++) {
                        const auto ItemText = GetDisplayName((ExportModelType)i);
                        const auto ID = i;
                        FUIAction ItemAction(FExecuteAction::CreateLambda([this, ID]() {
                            //TODO;拡張子選択時のコールバック
                            CurrentModelType = (ExportModelType)ID;
                            }));
                        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                    }
                    return MenuBuilder.MakeWidget();
                })
                .ContentPadding(0.0f)
                .VAlign(VAlign_Center)
                .ButtonContent()[
                    SNew(STextBlock).Text_Lambda([this]() {
                        return GetDisplayName(CurrentModelType);
                })]]]
    // 各種設定
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(0, 0, 0, 0)[
        BuildingImportSettingsView.ToSharedRef()]
    + SVerticalBox::Slot()
    .Padding(FMargin(84, 5, 86, 20))[
        CreateExportPathSelectPanel()]

    // モデルをエクスポート
    + SVerticalBox::Slot()
    .AutoHeight()
    .Padding(FMargin(84, 5, 86, 20))
            [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda([this]() {
            //TODO:エクスポート処理
            return FReply::Handled();
        })
        .Content()[
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
            .Margin(FMargin(0, 5, 0, 5))
            .Text(LOCTEXT("Export Button", "モデルをエクスポート"))
            ]
        ]
        ];
}

TSharedRef<SVerticalBox> SPLATEAUExportPanel::CreateExportPathSelectPanel()
{
    return
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [SNew(SEditableTextBox)
        .Padding(FMargin(3, 3, 0, 3))
        .IsReadOnly(true)
        .Text(LOCTEXT("SelectExportPath", "出力先フォルダを選択"))
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(20, 5, 20, 5))
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &SPLATEAUExportPanel::OnBtnSelectGmlFileClicked)
        .Content()
        [SNew(STextBlock)
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
            .Text_Lambda([this]() {
                return FText::FromString(ExportPath);
            })
            .MinDesiredWidth(400)
        ];
}

FReply SPLATEAUExportPanel::OnBtnSelectGmlFileClicked() {
    const void* WindowHandle = nullptr;

    IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
    TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

    if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
        WindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();
    }
    const FString DialogTitle("Select folder.");
    FString OutFolderName;

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (DesktopPlatform->OpenDirectoryDialog(
        WindowHandle,
        DialogTitle,
        ExportPath,
        OutFolderName)) {
        ExportPath = OutFolderName;
        //UpdateWindow(MyWindow);
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
