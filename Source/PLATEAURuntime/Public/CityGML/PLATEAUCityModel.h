// Fill out your copyright notice in the Description page of Project Settings.

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

private:
    friend class UPLATEAUCityModelBlueprintLibrary;

    std::shared_ptr<const citygml::CityModel> Data;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUCityModelBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static TArray<FPLATEAUCityObject>
        GetRootCityObjects(UPARAM(ref) const FPLATEAUCityModel& CityModel) {
        const auto CityObjectDataArray = CityModel.Data->getRootCityObjects();
        TArray<FPLATEAUCityObject> RootCityObjects;
        for (auto& CityObjectData : CityObjectDataArray) {
            RootCityObjects.Add(FPLATEAUCityObject(CityObjectData));
        }
        return RootCityObjects;
    }

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static int
        GetRootCityObjectCount(UPARAM(ref) const FPLATEAUCityModel& CityModel) {
        if (CityModel.Data == nullptr)
            return 0;

        return static_cast<int>(CityModel.Data->getNumRootCityObjects());
    }

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static FPLATEAUCityObject
        GetRootCityObject(UPARAM(ref) const FPLATEAUCityModel& CityModel, int Index) {
        return FPLATEAUCityObject(&CityModel.Data->getRootCityObject(Index));
    }

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static FPLATEAUCityObject
        getCityObjectByID(UPARAM(ref) const FPLATEAUCityModel& CityModel, const FString& FeatureID) {
        return FPLATEAUCityObject(CityModel.Data->getCityObjectById(TCHAR_TO_UTF8(*FeatureID)));
    }

};
