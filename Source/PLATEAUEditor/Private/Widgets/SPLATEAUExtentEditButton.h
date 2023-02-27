#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "Widgets/SCompoundWidget.h"
#include <plateau/network/client.h>

/**
 *
 */
class SPLATEAUExtentEditButton : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUExtentEditButton)
        : _ZoneID(9)
        , _SourcePath("")
        , _bImportFromServer(false)
        , _ClientPtr() { }

        SLATE_ATTRIBUTE(int, ZoneID)
        SLATE_ATTRIBUTE(FString, SourcePath)
	    SLATE_ATTRIBUTE(bool, bImportFromServer)
        SLATE_ATTRIBUTE(std::shared_ptr<plateau::network::Client>, ClientPtr)
        SLATE_ATTRIBUTE(std::string, ServerDatasetID)

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
    bool bImportFromServer;
    std::shared_ptr<plateau::network::Client> ClientPtr;
    std::string ServerDatasetID;

    TSharedPtr<class FPLATEAUExtentEditor> ExtentEditor;
};
