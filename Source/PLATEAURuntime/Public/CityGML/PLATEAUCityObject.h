// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUAttributeValue.h"

#include "PLATEAUCityObject.generated.h"

namespace citygml {
    class CityObject;
}

/*
 * �s�s�I�u�W�F�N�g��Blueprint�������b�p�[�ł��B
 */
USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUCityObject {
    GENERATED_USTRUCT_BODY()

public:
    FPLATEAUCityObject()
        : Data(nullptr) {}

    FPLATEAUCityObject(const citygml::CityObject* const Data)
        : Data(const_cast<citygml::CityObject*>(Data)) {}

private:
    friend class UPLATEAUCityObjectBlueprintLibrary;

    citygml::CityObject* Data;
    TSharedPtr<FPLATEAUAttributeMap> AttributeMapCache;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUCityObjectBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /*
     * �s�s�I�u�W�F�N�g���ێ����鑮�������擾���܂��B
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static TMap<FString, FPLATEAUAttributeValue>& GetAttributeMap(
            UPARAM(ref) FPLATEAUCityObject& CityObject);
};
