// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUAttributeValue.generated.h"

namespace citygml {
    class AttributeValue;
}

typedef TMap<FString, struct FPLATEAUAttributeValue> FPLATEAUAttributeMap;

UENUM(BlueprintType)
enum class EPLATEAUAttributeType : uint8 {
    String,
    Double,
    Integer,
    Date,
    Uri,
    Measure,
    AttributeSet,
    Boolean
};

/*
 * 都市オブジェクト属性値のBlueprint向けラッパーです。
 */
USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUAttributeValue {
    GENERATED_USTRUCT_BODY()

public:
    FPLATEAUAttributeValue() {}

    FPLATEAUAttributeValue(const citygml::AttributeValue* const Data)
        : Data(const_cast<citygml::AttributeValue*>(Data)) {};

private:
    friend class UPLATEAUAttributeValueBlueprintLibrary;

    citygml::AttributeValue* Data;
    TSharedPtr<FPLATEAUAttributeMap> AttributeMapCache;
};


UCLASS()
class PLATEAURUNTIME_API UPLATEAUAttributeValueBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /*
     * 属性値の型を取得します。
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static EPLATEAUAttributeType GetType(
            UPARAM(ref) const FPLATEAUAttributeValue& Value);

    /*
     * 属性値を文字列として取得します。
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static FString GetString(UPARAM(ref)
            const FPLATEAUAttributeValue& Value);

    /*
     * 複数の属性を再帰的に含む属性値を取得します。
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static TMap<FString, FPLATEAUAttributeValue>& GetAttributeMap(UPARAM(ref)
            FPLATEAUAttributeValue& Value);
};
