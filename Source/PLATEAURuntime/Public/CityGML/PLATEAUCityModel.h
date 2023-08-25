// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "citygml/citymodel.h"
#include "PLATEAUCityObject.h"

#include "PLATEAUCityModel.generated.h"

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUCityModel {
    GENERATED_USTRUCT_BODY()

public:
    FPLATEAUCityModel()
        : Data(nullptr) {}

    FPLATEAUCityModel(const std::shared_ptr<const citygml::CityModel> Data)
        : Data(Data) {}

    std::shared_ptr<const citygml::CityModel> GetData() {
        return Data;
    }

private:
    friend class UPLATEAUCityModelBlueprintLibrary;

    std::shared_ptr<const citygml::CityModel> Data;
};
