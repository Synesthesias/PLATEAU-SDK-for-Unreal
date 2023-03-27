// Copyright 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "PLATEAUExportSettings.generated.h"

UENUM(BlueprintType)
enum class EMeshTransformType : uint8 {
    //! ローカル座標
    Local,
    //! 平面直角座標系
    PlaneRect
};

UENUM(BlueprintType)
enum class ECoordinateSystem : uint8 {
    //! PLATEAUでの座標系
    ENU = 0,
    WUN = 1,
    //! Unreal Engineでの座標系
    ESU = 2,
    //! Unityでの座標系
    EUN = 3
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUFeatureExportSettings {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Export Settings")
        bool bExportTexture = true;

    UPROPERTY(EditAnywhere, Category = "Export Settings")
        EMeshTransformType ExportCoordinate = EMeshTransformType::Local;

    UPROPERTY(EditAnywhere, Category = "Export Settings")
        bool bExportHiddenModel = true;

    UPROPERTY(EditAnywhere, Category = "Export Settings")
        ECoordinateSystem CoorinateSystem = ECoordinateSystem::ENU;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUExportSettings : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Export Settings")
        FPLATEAUFeatureExportSettings ExportSetting;
};
