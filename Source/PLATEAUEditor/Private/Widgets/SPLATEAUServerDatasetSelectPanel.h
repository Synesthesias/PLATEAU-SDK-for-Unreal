#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SPLATEAUServerDatasetSelectPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUServerDatasetSelectPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()
private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    int PrefectureID = 1;
    int MunicipalityID = 1;
    bool bIsVisible = false;
public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    void SetPanelVisibility(bool bVisible) { bIsVisible = bVisible; }
};