// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "SPLATEAUServerDatasetSelectPanel.h"

#include <PLATEAUServerConnectionSettings.h>

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "SlateOptMacros.h"

#include <plateau/dataset/city_model_package.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "PLATEAUServerConnectionSettingsDetails.h"
#include "Async/Async.h"

#define LOCTEXT_NAMESPACE "SPLATEAUServerDatasetSelectPanel"

std::string SPLATEAUServerDatasetSelectPanel::GetServerDatasetID() const {
    return SelectedDataset.ID;
}

void SPLATEAUServerDatasetSelectPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
    const float InDeltaTime) {
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    // サーバーのデータにアクセス可能でかつ読み込まれていなければ読み込み
    if (bIsNativeDatasetMetadataAvailable && !bHasDatasetMetadataLoaded) {
        LoadDatasetMetadataFromNative();

        // 初期状態は1つ目のグループを選択
        if (!DatasetMetadataByGroup.IsEmpty()) {
            SelectedGroupTitle = DatasetMetadataByGroup.begin().Key();

            if (DatasetMetadataByGroup.begin().Value().IsEmpty())
                SelectedDataset = FPLATEAUServerDatasetMetadata();
            else {
                SelectedDataset = DatasetMetadataByGroup.begin().Value()[0];
                OnSelectDataset();
            }
        }
    }
}

TFuture<void> SPLATEAUServerDatasetSelectPanel::GetDatasetMetadataAsync(const std::string& InServerURL, const std::string& InToken) {
    bIsNativeDatasetMetadataAvailable = false;
    bIsGettingNativeDatasetMetadata = true;
    bHasDatasetMetadataLoaded = false;
    SelectedGroupTitle = "";
    SelectedDataset = FPLATEAUServerDatasetMetadata();

    return Async(EAsyncExecution::Thread, [this, InServerURL, InToken] {
        {
            FScopeLock Lock(&GetDatasetMetadataSection);
            ClientPtr = std::make_shared<plateau::network::Client>(InServerURL, InToken);
            NativeDatasetMetadataGroups = ClientPtr->getMetadata();
        }

        bIsNativeDatasetMetadataAvailable = true;
        bIsGettingNativeDatasetMetadata = false;
        });
}

void SPLATEAUServerDatasetSelectPanel::LoadDatasetMetadataFromNative() {
    if (!bIsNativeDatasetMetadataAvailable) {
        bHasDatasetMetadataLoaded = false;
        return;
    }

    DatasetMetadataByGroup.Empty();

    for (const auto& DatasetGroup : *NativeDatasetMetadataGroups) {
        const auto GroupTitle = UTF8_TO_TCHAR(DatasetGroup.title.c_str());
        auto& DatasetInfoArray = DatasetMetadataByGroup.FindOrAdd(GroupTitle);
        for (const auto& Dataset : DatasetGroup.datasets) {
            FPLATEAUServerDatasetMetadata DatasetInfo;
            DatasetInfo.Title = UTF8_TO_TCHAR(Dataset.title.c_str());
            DatasetInfo.Description = UTF8_TO_TCHAR(Dataset.description.c_str());
            DatasetInfo.ID = Dataset.id;
            DatasetInfoArray.Add(DatasetInfo);
        }
    }

    bHasDatasetMetadataLoaded = true;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUServerDatasetSelectPanel::Construct(const FArguments& InArgs) {
    if (!bIsGettingNativeDatasetMetadata)
        GetDatasetMetadataAsync("", "");

    OwnerWindow = InArgs._OwnerWindow;

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bAllowSearch = false;

    ConnectionSettingsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    ConnectionSettingsView->RegisterInstancedCustomPropertyLayout(
        UPLATEAUServerConnectionSettings::StaticClass(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUServerConnectionSettingsDetails::MakeInstance));
    ConnectionSettingsView->SetObject(GetMutableDefault<UPLATEAUServerConnectionSettings>());

    ChildSlot
        [SNew(SVerticalBox)
        .Visibility_Lambda([this] { return bIsVisible ? EVisibility::Visible : EVisibility::Collapsed; })
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [
            ConnectionSettingsView.ToSharedRef()
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructServerDataPanel().ToSharedRef()
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructDatasetGroupSelectPanel().ToSharedRef()
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructDatasetSelectPanel().ToSharedRef()
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructDescriptionPanel().ToSharedRef()
        ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUServerDatasetSelectPanel::ConstructServerDataPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 0, 32, 20))
        [SNew(SBox)
        .WidthOverride(5)
        .HeightOverride(30)
        [SNew(SButton)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(132, 132, 132))
        .OnClicked_Lambda([&]() {
        if (!bIsGettingNativeDatasetMetadata) {
            const auto ConnectionSettings = GetMutableDefault<UPLATEAUServerConnectionSettings>();

            const auto Url = TCHAR_TO_UTF8(*ConnectionSettings->Url);
            const auto Token = TCHAR_TO_UTF8(*ConnectionSettings->Token);
            GetDatasetMetadataAsync(Url, Token);
        }
        return FReply::Handled();
            })
        .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Text(LOCTEXT("Update", "サーバーデータ更新"))
                ]
        ]
        ];
}

TSharedPtr<SHorizontalBox> SPLATEAUServerDatasetSelectPanel::ConstructDatasetGroupSelectPanel() {
    return SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Right)
        .Padding(FMargin(0, 0, 0, 5))
        .FillWidth(0.25f)
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Right)
        .Text(LOCTEXT("Select Prefecture", "都道府県"))
        ] +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(0.1f) +

        // 都道府県(DatasetGroup)選択UI
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(1)
        [SNew(SComboButton)
        .OnGetMenuContent_Raw(this, &SPLATEAUServerDatasetSelectPanel::OnGetDatasetGroupMenuContent)
        .ContentPadding(0.0f)
        .VAlign(VAlign_Center)
        .ButtonContent()
        [SNew(STextBlock)
        .Text_Raw(this, &SPLATEAUServerDatasetSelectPanel::OnGetDatasetGroupText)
        .Justification(ETextJustify::Left)
        ]
        ];
}

TSharedPtr<SHorizontalBox> SPLATEAUServerDatasetSelectPanel::ConstructDatasetSelectPanel() {
    return SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Right)
        .Padding(FMargin(0, 0, 0, 5))
        .FillWidth(0.25f)
        [SNew(STextBlock)
        .Text(LOCTEXT("Select DataSet", "データセット"))
        .Justification(ETextJustify::Right)
        ] +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(1)
        [SNew(SComboButton)
        .OnGetMenuContent_Raw(this, &SPLATEAUServerDatasetSelectPanel::OnGetDatasetMenuContent)
        .ContentPadding(0.0f)
        .VAlign(VAlign_Center)
        .ButtonContent()
        [SNew(STextBlock)
        .Text_Raw(this, &SPLATEAUServerDatasetSelectPanel::OnGetDatasetText)
        .Justification(ETextJustify::Left)
        ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUServerDatasetSelectPanel::ConstructDescriptionPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 15, 0, 5))
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Left)
        .Text(LOCTEXT("Model Description", "説明"))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 0, 32, 0))
        [
            SNew(SBox)
            .WidthOverride(180)
        .HeightOverride(90)
        [
            SNew(SMultiLineEditableTextBox)
            .Justification(ETextJustify::Left)
        .IsReadOnly(true)
        .AutoWrapText(true)
        .Text_Raw(this, &SPLATEAUServerDatasetSelectPanel::OnGetDescriptionText)
        ]
        ];
}

/**
 * @brief 毎フレーム都道府県(DatasetGroup)のテキストを描画する際に呼び出されます。
 */
FText SPLATEAUServerDatasetSelectPanel::OnGetDatasetGroupText() const {
    if (!bIsNativeDatasetMetadataAvailable)
        return LOCTEXT("Loading", "Loading...");

    return FText::FromString(SelectedGroupTitle);
}

/**
 * @brief 毎フレームデータセットのテキストを描画する際に呼び出されます。
 */
FText SPLATEAUServerDatasetSelectPanel::OnGetDatasetText() const {
    if (!bIsNativeDatasetMetadataAvailable)
        return LOCTEXT("Loading", "Loading...");

    return FText::FromString(SelectedDataset.Title);
}


/**
 * @brief 都道府県(DatasetGroup)のドロップダウンを開いた際に呼び出されます。
 */
TSharedRef<SWidget> SPLATEAUServerDatasetSelectPanel::OnGetDatasetGroupMenuContent() {

    FMenuBuilder MenuBuilder(true, nullptr);

    TArray<FString> GroupTitles;
    DatasetMetadataByGroup.GetKeys(GroupTitles);

    for (const auto& GroupTitle : GroupTitles) {
        // 各要素がクリックされた時の挙動
        FUIAction ItemAction(FExecuteAction::CreateLambda(
            [this, GroupTitle]() {
                SelectedGroupTitle = GroupTitle;

                const auto SelectedGroup = DatasetMetadataByGroup.Find(GroupTitle);

                if (SelectedGroup == nullptr || SelectedGroup->IsEmpty()) {
                    SelectedDataset.Title = "";
                    SelectedDataset.Description = "";
                    SelectedDataset.ID = "";
                }

                SelectedDataset = (*SelectedGroup)[0];
                OnSelectDataset();
            }));
        MenuBuilder.AddMenuEntry(FText::FromString(GroupTitle),
            TAttribute<FText>(), FSlateIcon(), ItemAction);
    }
    return MenuBuilder.MakeWidget();
}

/**
 * @brief データセットのドロップダウンを開いた際に呼び出されます。
 */
TSharedRef<SWidget> SPLATEAUServerDatasetSelectPanel::OnGetDatasetMenuContent() {
    FMenuBuilder MenuBuilder(true, nullptr);

    if (!bHasDatasetMetadataLoaded) {
        MenuBuilder.AddMenuEntry(
            LOCTEXT("Loading", "Loading..."),
            TAttribute<FText>(), FSlateIcon(),
            FExecuteAction::CreateLambda([] {}));
        return MenuBuilder.MakeWidget();
    }

    const auto DatasetMetadataArray = DatasetMetadataByGroup.Find(SelectedGroupTitle);

    if (DatasetMetadataArray == nullptr) {
        return MenuBuilder.MakeWidget();
    }

    for (const auto& DatasetInfo : *DatasetMetadataArray) {
        FUIAction ItemAction(FExecuteAction::CreateLambda(
            [this, DatasetInfo]() {
                SelectedDataset = DatasetInfo;
                OnSelectDataset();
            }));
        MenuBuilder.AddMenuEntry(
            FText::FromString(DatasetInfo.Title),
            TAttribute<FText>(), FSlateIcon(), ItemAction);
    }
    return MenuBuilder.MakeWidget();
}

FText SPLATEAUServerDatasetSelectPanel::OnGetDescriptionText() const {
    if (!bHasDatasetMetadataLoaded)
        return LOCTEXT("Loading", "Loading...");

    return FText::FromString(SelectedDataset.Description);
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
