// Copyright © 2023 Ministry of Land、Infrastructure and Transport

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
    bool IsCurrentIndex(const int ID);

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    int SelectingID = 1;

    TSharedRef<SHorizontalBox> CreateTabButtons();
    TSharedRef<SHorizontalBox> CreateTabSelectBackground();
    TSharedRef<SBox> CreateTabBackground();
    void OnButtonClicked(const int ID);
    const FSlateColor GetTabSelectBGColor(const int ID);
};
