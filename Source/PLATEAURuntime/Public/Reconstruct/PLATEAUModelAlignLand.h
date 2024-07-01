// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelAlignLand : public FPLATEAUModelReconstruct {

public:
    FPLATEAUModelAlignLand();
    FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor);

    /**
     * @brief 元のComponentを記憶します
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: Component Name(GmlID), Value: Component の Map
     */
    TMap<FString, UPLATEAUCityObjectGroup*> CreateComponentsMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    void SetHeightData(const std::vector<uint16_t> HeightData, const TVec3d Min, const TVec3d Max, const FString NodeName, FPLATEAULandscapeParam Param);

protected:

    TMap<FString, UPLATEAUCityObjectGroup*> ComponentsMap;

private:

    TSet<EPLATEAUCityModelPackage> IncludePacakges{ EPLATEAUCityModelPackage::Area,
    EPLATEAUCityModelPackage::Road,
    EPLATEAUCityModelPackage::Square,
    EPLATEAUCityModelPackage::Track,
    EPLATEAUCityModelPackage::Waterway,
    EPLATEAUCityModelPackage::DisasterRisk,
    EPLATEAUCityModelPackage::LandUse,
    EPLATEAUCityModelPackage::WaterBody,
    EPLATEAUCityModelPackage::UrbanPlanningDecision,
    };

};

