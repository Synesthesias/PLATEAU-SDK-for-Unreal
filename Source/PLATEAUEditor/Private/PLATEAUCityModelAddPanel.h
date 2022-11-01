// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUCityModelAddPanel {
public:
    FPLATEAUCityModelAddPanel();
    ~FPLATEAUCityModelAddPanel();

    void UpdateWindow(TWeakPtr<SWindow> MyWindow);

private:
    FString SourcePath;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel(TWeakPtr<SWindow> MyWindow);
    FReply OnBtnSelectGmlFileClicked(TWeakPtr<SWindow> MyWindow);
};
