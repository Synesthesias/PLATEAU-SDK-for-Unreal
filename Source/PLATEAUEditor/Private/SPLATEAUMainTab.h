// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/ISlateStyle.h"
/**
 *
 */
class SPLATEAUMainTab : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUMainTab) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle);
    bool IsCurrentIndex(int Id);

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    int SelectingID = 1;

    TSharedRef<SHorizontalBox> CreateTabButtons();
    TSharedRef<SHorizontalBox> CreateTabSelectBackground();
    TSharedRef<SBox> CreateTabBackground();
    void OnButtonClicked(int Id);
    const FSlateColor GetTabSelectBGColor(int Id);
};
