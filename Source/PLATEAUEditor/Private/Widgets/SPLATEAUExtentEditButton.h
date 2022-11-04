#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class SPLATEAUExtentEditButton : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUExtentEditButton)
        : _ZoneID(9)
        , _SourcePath("") {}

        SLATE_ATTRIBUTE(int, ZoneID)
        SLATE_ATTRIBUTE(FString, SourcePath)

        SLATE_END_ARGS()

public:
    SPLATEAUExtentEditButton();

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

    bool IsExtentSet() const;
    TOptional<FPLATEAUExtent> GetExtent() const;

private:
    int ZoneIDCache;
    FString SourcePathCache;

    TSharedPtr<class FPLATEAUExtentEditor> ExtentEditor;
};
