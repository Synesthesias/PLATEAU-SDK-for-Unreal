#include "SPLATEAUServerDatasetSelectPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "SlateOptMacros.h"

#include <plateau/dataset/city_model_package.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>

#define LOCTEXT_NAMESPACE "SPLATEAUServerDatasetSelectPanel"

void SPLATEAUServerDatasetSelectPanel::LoadClientData(const std::string& InServerURL) {
    if (!bLoadedClientData) {
        const auto ServerLoadTask = FFunctionGraphTask::CreateAndDispatchWhenReady([&, InServerURL] {
            ClientRef = plateau::network::Client(InServerURL);
            const auto tempDatasets = ClientRef.getMetadata();

            FFunctionGraphTask::CreateAndDispatchWhenReady([&, tempDatasets] {
                DataSets = tempDatasets;
                bLoadedClientData = true;
                InitUITexts();
                }, TStatId(), nullptr, ENamedThreads::GameThread_Local);

            }, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
    }
}

void SPLATEAUServerDatasetSelectPanel::InitServerData() {
    if (!bLoadedClientData && !bServerInitialized) {
        bServerInitialized = true;
        LoadClientData(DefaultServerURL);
    }
}

void SPLATEAUServerDatasetSelectPanel::LoadServerDataWithURL(const std::string& InServerURL) {
    LoadClientData(InServerURL.c_str());
}

void SPLATEAUServerDatasetSelectPanel::InitUITexts() {
    PrefectureTexts.Empty();
    MunicipalityTexts.Empty();
    if (DataSets->size() > 0) {
        for (int i = 0; i < DataSets->size(); i++) {
            std::string TmpStr = DataSets->at(i).title;
            PrefectureTexts.Add(i, FText::FromString(UTF8_TO_TCHAR(TmpStr.c_str())));
        }
        if (DataSets->at(0).datasets.size()) {
            for (int j = 0; j < DataSets->at(0).datasets.size(); j++) {
                std::string TmpStr2 = DataSets->at(0).datasets[j].title;
                MunicipalityTexts.Add(j, FText::FromString(UTF8_TO_TCHAR(TmpStr2.c_str())));
            }
            std::string TmpStr3 = DataSets->at(0).datasets[0].description;
            DescriptionText = FText::FromString(UTF8_TO_TCHAR(TmpStr3.c_str()));
            std::string TmpStr4 = std::to_string(DataSets->at(0).datasets[0].max_lod);
            MaxLODText = FText::FromString(UTF8_TO_TCHAR(TmpStr4.c_str()));
        }
    }
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUServerDatasetSelectPanel::Construct(const FArguments& InArgs) {
    OwnerWindow = InArgs._OwnerWindow;

    ChildSlot[
        SNew(SVerticalBox)
            .Visibility_Lambda([this]() {
            return bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
                })
            + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        ConstructServerDataPanel().ToSharedRef()
                    ]
                + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        ConstructPrefectureSelectPanel().ToSharedRef()
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
        .Padding(FMargin(0, 0, 0, 12))
        [
            SNew(SHorizontalBox) +
            SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 0)
        .FillWidth(0.1f) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Right)
        .Padding(FMargin(0, 0, 0, 0))
        .FillWidth(0.25f)
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Right)
        .Text(LOCTEXT("Server URL", "サーバーURL"))
        ] +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 0)
        .FillWidth(0.1f) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 0)
        .FillWidth(1)
        [
            SAssignNew(ServerURL, SEditableTextBox)
            .Justification(ETextJustify::Left)
        .Text(FText::FromString(DefaultServerURL.c_str()))
        ]
    + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 0)
        .FillWidth(0.1f)]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 0, 32, 20))[
            SNew(SBox)
                .WidthOverride(5)
                .HeightOverride(30)
                [SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ForegroundColor(FColor::White)
                .ButtonColorAndOpacity(FColor(132, 132, 132))
                .OnClicked_Lambda([&]() {
                if (bLoadedClientData) {
                    bLoadedClientData = false;
                    PrefectureID = 0;
                    MunicipalityID = 0;
                    const std::string TmpString = TCHAR_TO_UTF8(*ServerURL->GetText().ToString());
                    LoadServerDataWithURL(TmpString);
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

TSharedPtr<SVerticalBox> SPLATEAUServerDatasetSelectPanel::ConstructPrefectureSelectPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SHorizontalBox) +
            SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(0.1f) +
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
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(1)
        [
            SNew(SComboButton)
            .OnGetMenuContent_Lambda(
                [&]() {
                    FMenuBuilder MenuBuilder(true, nullptr);
                    TMap<int, FText> Items;
                    if (bLoadedClientData) {
                        Items = PrefectureTexts;
                    } else {
                        Items.Add(0, FText::FromString("Loading..."));
                    }
                    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                        const auto ItemText = ItemIter->Value;
                        const auto ID = ItemIter->Key;
                        FUIAction ItemAction(FExecuteAction::CreateLambda(
                            [this, ID]() {
                                PrefectureID = ID;
                                MunicipalityID = 0;
                                MunicipalityTexts.Empty();
                                for (int i = 0; i < DataSets->at(ID).datasets.size(); i++) {
                                    std::string TmpStr = DataSets->at(ID).datasets[i].title;
                                    MunicipalityTexts.Add(i, FText::FromString(UTF8_TO_TCHAR(TmpStr.c_str())));
                                }
                            }));
                        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                    }
                    return MenuBuilder.MakeWidget();
                })
        .ContentPadding(0.0f)
                    .VAlign(VAlign_Center)
                    .ButtonContent()
                    [
                        SNew(STextBlock).Text_Lambda(
                            [this]() {
                                if (bLoadedClientData && PrefectureTexts.Num() > PrefectureID)
                                    return PrefectureTexts[PrefectureID];
                                else
                                    return FText::FromString("Loading...");
                            })
                    .Justification(ETextJustify::Left)
                    ]
        ]
    + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 15)
        .FillWidth(0.1f)];
}

TSharedPtr<SVerticalBox> SPLATEAUServerDatasetSelectPanel::ConstructDatasetSelectPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 5))
        [
            SNew(SHorizontalBox) +
            SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 15)
        .FillWidth(0.1f) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Right)
        .Padding(FMargin(0, 0, 0, 5))
        .FillWidth(0.25f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Select DataSet", "データセット"))
        .Justification(ETextJustify::Right)
        ] +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(0.1f) +
        SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(1)
        [
            SAssignNew(MunicipalityComboButton, SComboButton)
            .OnGetMenuContent_Lambda(
                [&]() {
                    FMenuBuilder MenuBuilder(true, nullptr);
                    TMap<int, FText> Items;
                    {
                        if (bLoadedClientData) {
                            Items = TMap(MunicipalityTexts);
                        } else {
                            Items.Add(0, FText::FromString("Loading..."));
                        }
                    }
                    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                        const auto ItemText = ItemIter->Value;
                        const auto ID = ItemIter->Key;
                        FUIAction ItemAction(FExecuteAction::CreateLambda(
                            [this, ID]() {
                                MunicipalityID = ID;
                                if (bLoadedClientData) {
                                    std::string TmpStr = DataSets->at(PrefectureID).datasets[MunicipalityID].description;
                                    DescriptionText = FText::FromString(UTF8_TO_TCHAR(TmpStr.c_str()));
                                    std::string TmpStr2 = std::to_string(DataSets->at(PrefectureID).datasets[MunicipalityID].max_lod);
                                    MaxLODText = FText::FromString(UTF8_TO_TCHAR(TmpStr2.c_str()));
                                } else {
                                    DescriptionText = FText::FromString(UTF8_TO_TCHAR("Loading..."));
                                    MaxLODText = FText::FromString(UTF8_TO_TCHAR("Loading..."));
                                }
                            }));
                        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                    }
                    return MenuBuilder.MakeWidget();
                })
        .ContentPadding(0.0f)
                    .VAlign(VAlign_Center)
                    .ButtonContent()
                    [
                        SNew(STextBlock).Text_Lambda(
                            [this]() {
                                if (bLoadedClientData && MunicipalityTexts.Num() > MunicipalityID)
                                    return MunicipalityTexts[MunicipalityID];
                                else
                                    return FText::FromString("Loading...");
                            })
                    .Justification(ETextJustify::Left)
                    ]
        ]
    + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 0, 5)
        .FillWidth(0.1f)
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUServerDatasetSelectPanel::ConstructDescriptionPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 10, 0, 5))
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Left)
        .Text(LOCTEXT("Max LOD", "最大LOD"))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(32, 0, 32, 0))
        [
            SNew(SEditableTextBox)
            .Justification(ETextJustify::Left)
        .Text_Lambda([this]() {
        if (!bLoadedClientData)
            return LOCTEXT("Loading", "Loading...");
        else
            return MaxLODText;
            })
        .IsReadOnly(true)
        ]
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
        .Text_Lambda([this]() {
        if (!bLoadedClientData)
            return LOCTEXT("Loading", "Loading...");
        else
            return DescriptionText;
            })
        ]
        ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
