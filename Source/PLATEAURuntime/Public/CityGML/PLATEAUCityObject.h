// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUAttributeValue.h"
#include "PLATEAUCityObject.generated.h"

namespace citygml {
    class CityObject;
}

/*
 * 都市オブジェクトのBlueprint向けラッパーです。
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
     * 都市オブジェクトが保持する属性情報を取得します。
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static FPLATEAUAttributeMap& GetAttributeMap(
            UPARAM(ref) FPLATEAUCityObject& CityObject);
};
