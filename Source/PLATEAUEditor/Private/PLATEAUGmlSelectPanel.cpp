// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUGmlSelectPanel.h"

#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "PLATEAUFileUtils.h"
#include <algorithm>

#define LOCTEXT_NAMESPACE "FPLATEUEditorModule"


FPLATEAUGmlSelectPanel::FPLATEAUGmlSelectPanel() {
    ExistFeatures.Init(false, Features.Num());
    SelectFeatures.Init(false, Features.Num());
}

TSharedRef<SVerticalBox> FPLATEAUGmlSelectPanel::CreateSelectGmlFilePanel(TWeakPtr<SWindow> MyWindow) {
    return  SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .Text(LOCTEXT("Block1", "1. インポート元フォルダ選択"))
        .IsReadOnly(true)
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
        .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnSelectGmlFileClicked, MyWindow)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Button1", "参照..."))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 10))
        [
            SNew(SEditableTextBox)
            .IsReadOnly(true)
        .Text(FText::FromString(GmlFolderPath))
        ];
}

FReply FPLATEAUGmlSelectPanel::OnBtnSelectGmlFileClicked(TWeakPtr<SWindow> MyWindow) {
    void* WindowHandle = MyWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    FString DialogTitle = FString("Select folder.");
    FString DefaultPath = GmlFolderPath;
    FString OutFolderName;

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    GmlFileSelected = DesktopPlatform->OpenDirectoryDialog(
        WindowHandle,
        DialogTitle,
        DefaultPath,
        OutFolderName);

    if (GmlFileSelected) {
        GmlFolderPath = OutFolderName;
        Collection = UdxFileCollection::find(TCHAR_TO_UTF8(*(GmlFolderPath + "/udx")));
        MeshCodes = Collection.getMeshCodes();

        SelectRegion.Reset();
        SelectRegion.Init(false, (*MeshCodes).size());

        UpdateWindow(MyWindow);
    }

    return FReply::Handled();
}

void FPLATEAUGmlSelectPanel::UpdateWindow(TWeakPtr<SWindow> MyWindow) {
    auto ScrollBox = SNew(SScrollBox);

    ScrollBox->AddSlot()[
        CreateSelectGmlFilePanel(MyWindow)
    ];
    if (GmlFileSelected) {
        ScrollBox->AddSlot()[
            CreateSelectRegionMesh()
        ];
        ScrollBox->AddSlot()[
            CreateSelectFeatureMesh()
        ];
    }

    ScrollBox->AddSlot()[
        ConvertSettingsPanelInstance.CreateConvertSettingsPanel()
    ];

    MyWindow.Pin()->SetContent(ScrollBox);
}

TSharedRef<SVerticalBox> FPLATEAUGmlSelectPanel::CreateSelectRegionMesh() {
    auto VbMeshCodes = SNew(SVerticalBox).Visibility_Lambda(
        [this]() {
            if (GmlFileSelected && (*MeshCodes).size() != 0) {
                return EVisibility::Visible;
            }
            return EVisibility::Collapsed;
        }
    );
    int IndexRegion = 0;
    TArray<MeshCode> ThirdMesh;
    std::vector<MeshCode> TempMeshCodes;

    SecondMesh.Reset();
    for (auto MeshCode : *MeshCodes) {
        if (FString(MeshCode.get().c_str()).Len() == 6) {
            SecondMesh.Add(MeshCode);
        }
        else {
            ThirdMesh.Add(MeshCode);
        }
    }

    //疑似2次メッシュ作成
    for (auto Mesh3 : ThirdMesh) {
        auto SimMesh = MeshCode(Mesh3.getAsLevel2());
        if (SecondMesh.Find(SimMesh) == INDEX_NONE) {
            SecondMesh.Add(SimMesh);
            SelectRegion.Add(false);
        }
    }

    SecondMesh = SortMeshCodes(SecondMesh);
    ThirdMesh = SortMeshCodes(ThirdMesh);

    VbMeshCodes->AddSlot()
        .Padding(FMargin(0, 0, 0, 3))[
            SNew(STextBlock)
                .Text(FText::FromString((FString(TEXT("含める地域")))))
        ];

    VbMeshCodes->AddSlot()
        .Padding(FMargin(0, 0, 0, 3))
        .AutoHeight()[
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(0, 0, 15, 0))
                [
                    SNew(SButton)
                    .Text(FText::FromString(FString(TEXT("全選択"))))
                .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnAllSecondMeshSelectClicked)
                ]
            + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(FText::FromString(FString(TEXT("全除外"))))
                .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnAllSecondMeshRelieveClicked)
                ]
        ];

    for (auto Mesh2 : SecondMesh) {
        int SecondIndex = IndexRegion;
        TempMeshCodes.push_back(Mesh2);
        VbMeshCodes->AddSlot()
            .AutoHeight()
            .Padding(FMargin(0, 0, 0, 3))[
                SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(FString(Mesh2.get().c_str())))
                    ]
                + SHorizontalBox::Slot()
                    [
                        SNew(SCheckBox)
                        .IsChecked_Lambda(
                            [this, IndexRegion]()
                            {
                                if (SelectRegion[IndexRegion]) {
                                    return ECheckBoxState::Checked;
                                }
                                return ECheckBoxState::Unchecked;
                            })
                    .OnCheckStateChanged_Raw(this, &FPLATEAUGmlSelectPanel::OnToggleCbSelectRegion, IndexRegion, Mesh2.get())
                    ]
            ];
        IndexRegion++;

        ThirdMesh = SortMeshCodes(ThirdMesh);
        bool BtnDisplayed = false;
        for (auto TMesh : ThirdMesh) {
            if (FString(TMesh.get().c_str()).Contains(FString(Mesh2.get().c_str()))) {
                TempMeshCodes.push_back(TMesh);

                if (!BtnDisplayed) {
                    VbMeshCodes->AddSlot()
                        .Padding(FMargin(0, 0, 0, 3))
                        .AutoHeight()[
                            SNew(SHorizontalBox).Visibility_Lambda(
                                [this, SecondIndex]() {
                                    if (SelectRegion[SecondIndex]) {
                                        return EVisibility::Visible;
                                    }
                                    return EVisibility::Collapsed;
                                })
                                + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(FMargin(30, 0, 15, 0))
                                    [
                                        SNew(SButton)
                                        .Text(FText::FromString(FString(TEXT("全選択"))))
                                    .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnThirdMeshSelectClicked, Mesh2.get())
                                    ]
                                + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    [
                                        SNew(SButton)
                                        .Text(FText::FromString(FString(TEXT("全除外"))))
                                    .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnThirdMeshRelieveClicked, Mesh2.get())
                                    ]
                        ];

                    BtnDisplayed = true;
                }

                VbMeshCodes->AddSlot()
                    .AutoHeight()
                    .Padding(FMargin(30, 0, 0, 3))[
                        SNew(SHorizontalBox).Visibility_Lambda(
                            [this, SecondIndex]() {
                                if (SelectRegion[SecondIndex]) {
                                    return EVisibility::Visible;
                                }
                                else {
                                    return EVisibility::Collapsed;
                                }
                            })
                            + SHorizontalBox::Slot()
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(FString(TMesh.get().c_str())))
                                ]
                            + SHorizontalBox::Slot()
                                [
                                    SNew(SCheckBox)
                                    .IsChecked_Lambda(
                                        [this, IndexRegion]()
                                        {
                                            if (SelectRegion[IndexRegion]) {
                                                return ECheckBoxState::Checked;
                                            }
                                            return ECheckBoxState::Unchecked;
                                        }).OnCheckStateChanged_Raw(this, &FPLATEAUGmlSelectPanel::OnToggleCbSelectRegion, IndexRegion, TMesh.get())
                                ]
                    ];
                IndexRegion++;
            }
        }
    }
    MeshCodes = std::make_shared<std::vector<MeshCode>>(TempMeshCodes);

    return VbMeshCodes;
}

TArray<MeshCode> FPLATEAUGmlSelectPanel::SortMeshCodes(TArray<MeshCode> MeshArray) {
    std::vector<int> IntVecArray;
    TArray<MeshCode> ReturnArray;
    for (auto Mesh : MeshArray) {
        IntVecArray.push_back(atoi(Mesh.get().c_str()));
    }

    std::sort(IntVecArray.begin(), IntVecArray.end());
    for (int MeshNum : IntVecArray) {
        for (auto Mesh_ : MeshArray) {
            if (std::to_string(MeshNum) == Mesh_.get()) {
                ReturnArray.Add(Mesh_);
            }
        }
    }
    return ReturnArray;
}

FReply FPLATEAUGmlSelectPanel::OnBtnAllSecondMeshSelectClicked() {
    for (int i = 0; i < SelectRegion.Num(); i++) {
        if (SecondMesh.Find((*MeshCodes)[i]) != INDEX_NONE) {
            SelectRegion[i] = true;
        }
    }
    CheckRegionMesh();
    return FReply::Handled();
}

FReply FPLATEAUGmlSelectPanel::OnBtnAllSecondMeshRelieveClicked() {
    for (int i = 0; i < SelectRegion.Num(); i++) {
        SelectRegion[i] = false;
    }
    CheckRegionMesh();
    return FReply::Handled();
}

FReply FPLATEAUGmlSelectPanel::OnBtnThirdMeshSelectClicked(std::string SecondMeshName) {
    for (int i = 0; i < SelectRegion.Num(); i++) {
        if ((*MeshCodes)[i].getAsLevel2() == SecondMeshName && (*MeshCodes)[i].get() != SecondMeshName) {
            SelectRegion[i] = true;
        }
    }
    CheckRegionMesh();
    return FReply::Handled();
}

FReply FPLATEAUGmlSelectPanel::OnBtnThirdMeshRelieveClicked(std::string SecondMeshName) {
    for (int i = 0; i < SelectRegion.Num(); i++) {
        if ((*MeshCodes)[i].getAsLevel2() == SecondMeshName && (*MeshCodes)[i].get() != SecondMeshName) {
            SelectRegion[i] = false;
        }
    }
    CheckRegionMesh();
    return FReply::Handled();
}

void FPLATEAUGmlSelectPanel::OnToggleCbSelectRegion(ECheckBoxState CheckState, int Num, std::string MeshName) {
    SelectRegion[Num] = (CheckState == ECheckBoxState::Checked);
    if (FString(MeshName.c_str()).Len() == 6 && CheckState == ECheckBoxState::Unchecked) {
        for (int i = 0; i < SelectRegion.Num(); i++) {
            if ((*MeshCodes)[i].getAsLevel2() == MeshName) {
                SelectRegion[i] = false;
            }
        }
    }
    CheckRegionMesh();
}

void FPLATEAUGmlSelectPanel::CheckRegionMesh() {
    std::vector<MeshCode> TargetMeshCodes;

    int i = 0;
    for (auto meshCode : *MeshCodes) {
        if (SelectRegion[i]) {
            TargetMeshCodes.push_back(meshCode);
        }
        i++;
    }
    ExistFeatures.Init(false, Features.Num());
    if (TargetMeshCodes.size() != 0) {

        FilteredCollection = UdxFileCollection::filter(Collection, TargetMeshCodes);
        SubFolders = FilteredCollection.getSubFolders();

        for (auto SubFolder : *SubFolders) {
            if (SubFolder.name() == "bldg") {
                ExistFeatures[0] = true;
            }
            else if (SubFolder.name() == "tran") {
                ExistFeatures[1] = true;
            }
            else if (SubFolder.name() == "frn")
            {
                ExistFeatures[2] = true;
            }
            else if (SubFolder.name() == "dem") {
                ExistFeatures[3] = true;
            }
            else if (SubFolder.name() == "veg") {
                ExistFeatures[4] = true;
            }
            else {
                ExistFeatures[5] = true;
            }
        }
    }

    for (int j = 0; j < ExistFeatures.Num(); j++) {
        if (!ExistFeatures[j]) {
            SelectFeatures[j] = false;
        }
    }

    ConvertSettingsPanelInstance.UpdateFeaturesInfo(ExistFeatures, SelectFeatures, FilteredCollection);
}

TSharedRef<SVerticalBox> FPLATEAUGmlSelectPanel::CreateSelectFeatureMesh() {
    auto VbFeatureMesh = SNew(SVerticalBox).Visibility_Lambda(
        [this]() {
            if (GmlFileSelected) {
                return EVisibility::Visible;
            }
            return EVisibility::Collapsed;
        });

    VbFeatureMesh->AddSlot()
        .Padding(FMargin(0, 20, 0, 0))[
            SNew(STextBlock).IsEnabled_Lambda(
                [this]() {
                    for (int i = 0; i < ExistFeatures.Num(); i++) {
                        if (ExistFeatures[i]) {
                            return true;
                        }
                    }
                    return false;
                })
                .Text(FText::FromString(FString(TEXT("含める地物"))))
        ];

    VbFeatureMesh->AddSlot()
        .Padding(FMargin(0, 0, 0, 8))
        .AutoHeight()[
            SNew(SHorizontalBox).Visibility_Lambda(
                [this]() {
                    for (int i = 0; i < ExistFeatures.Num(); i++) {
                        if (ExistFeatures[i]) {
                            return EVisibility::Visible;
                        }
                    }
                    return EVisibility::Collapsed;
                })
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(FMargin(0, 0, 10, 0))
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全選択"))))
                    .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnAllFeatureSelectClicked)
                    ]
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全除外"))))
                    .OnClicked_Raw(this, &FPLATEAUGmlSelectPanel::OnBtnAllFeatureRelieveClicked)
                    ]
        ];

    for (int j = 0; j < ExistFeatures.Num(); j++) {
        VbFeatureMesh->AddSlot()
            .Padding(FMargin(0, 0, 0, 3))[
                SNew(SHorizontalBox).IsEnabled_Lambda(
                    [this, j]() {
                        if (ExistFeatures[j]) return true;
                        return false;
                    })
                    + SHorizontalBox::Slot()
                        [
                            SNew(STextBlock)
                            .Text(FCityModelPlacementSettings::GetDisplayName(Features[j]))
                        ]
                    + SHorizontalBox::Slot()
                        [
                            SNew(SCheckBox)
                            .IsChecked_Lambda(
                                [this, j]() {
                                    if (SelectFeatures[j]) {
                                        return ECheckBoxState::Checked;
                                    }
                                    return ECheckBoxState::Unchecked;
                                })
                        .OnCheckStateChanged_Raw(this, &FPLATEAUGmlSelectPanel::OnToggleCbSelectFeature, j)
                        ]
            ];
    }

    return VbFeatureMesh;
}

FReply FPLATEAUGmlSelectPanel::OnBtnAllFeatureSelectClicked() {
    for (int i = 0; i < SelectFeatures.Num(); i++) {
        if (ExistFeatures[i]) {
            SelectFeatures[i] = true;
        }
    }
    ConvertSettingsPanelInstance.UpdateFeaturesInfo(ExistFeatures, SelectFeatures, FilteredCollection);
    return FReply::Handled();
}

FReply FPLATEAUGmlSelectPanel::OnBtnAllFeatureRelieveClicked() {
    for (int i = 0; i < SelectFeatures.Num(); i++) {
        if (ExistFeatures[i]) {
            SelectFeatures[i] = false;
        }
    }
    ConvertSettingsPanelInstance.UpdateFeaturesInfo(ExistFeatures, SelectFeatures, FilteredCollection);
    return FReply::Handled();
}

void FPLATEAUGmlSelectPanel::OnToggleCbSelectFeature(ECheckBoxState CheckState, int Index) {
    SelectFeatures[Index] = (CheckState == ECheckBoxState::Checked);
    ConvertSettingsPanelInstance.UpdateFeaturesInfo(ExistFeatures, SelectFeatures, FilteredCollection);
}
