// Copyright 2023 Ministry of Land, Infrastructure and Transport

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

UENUM(BlueprintType)
enum class EMeshFileFormat : uint8 {
    OBJ = 0,
    FBX,
    GLTF,
    EMeshFileFormat_MAX,
};
