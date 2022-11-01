#pragma once

#include "CoreMinimal.h"

#include "PLATEAUExportSettings.generated.h"

UENUM(BlueprintType)
enum class EPLATEAUExportCoordinate : uint8 {
	//! ローカル座標
	Local,
	//! 平面直角座標系
	PlaneRect
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUFeatureExportSettings {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Export Settings")
		bool bExportTexture = true;

	UPROPERTY(EditAnywhere, Category = "Export Settings")
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
