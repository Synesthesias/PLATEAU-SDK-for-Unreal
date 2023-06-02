// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "citygml/attributesmap.h"
#include "PLATEAUAttributeValue.generated.h"


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
    TSharedPtr<struct FPLATEAUAttributeMap> AttributeMapCache;
};


USTRUCT(BlueprintType,
Category = "PLATEAU|CityGML")
struct FPLATEAUAttributeMap
{
    GENERATED_USTRUCT_BODY();
    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    TMap<FString, FPLATEAUAttributeValue> value;
};

UENUM(BlueprintType,
Category = "PLATEAU|CityGML")
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
        static FPLATEAUAttributeMap& GetAttributeMap(UPARAM(ref)
            FPLATEAUAttributeValue& Value);
};
