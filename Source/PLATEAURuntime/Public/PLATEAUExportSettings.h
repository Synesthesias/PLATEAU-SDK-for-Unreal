#pragma once

#include "CoreMinimal.h"

#include "PLATEAUExportSettings.generated.h"

UENUM(BlueprintType)
enum class EPLATEAUExportCoordinate : uint8 {
    //! 最小地物単位(LOD2, LOD3の各部品)
    Local,
    //! 主要地物単位(建築物、道路等)
    PlaneRect
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUFeatureExportSettings {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Export Settings")
        bool bExportTexture = true;

    UPROPERTY(EditAnywhere, Category = "Import Settings")
        EPLATEAUExportCoordinate ExportCoordinate = EPLATEAUExportCoordinate::Local;

    UPROPERTY(EditAnywhere, Category = "Export Settings")
        bool bExportHiddenModel = true;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUExportSettings : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureExportSettings ExportSetting;
};
