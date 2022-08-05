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
 * �s�s�I�u�W�F�N�g�����l��Blueprint�������b�p�[�ł��B
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
     * �����l�̌^���擾���܂��B
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static EPLATEAUAttributeType GetType(
            UPARAM(ref) const FPLATEAUAttributeValue& Value);

    /*
     * �����l�𕶎���Ƃ��Ď擾���܂��B
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static FString GetString(UPARAM(ref)
            const FPLATEAUAttributeValue& Value);

    /*
     * �����̑������ċA�I�Ɋ܂ޑ����l���擾���܂��B
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "PLATEAU|CityGML")
        static TMap<FString, FPLATEAUAttributeValue>& GetAttributeMap(UPARAM(ref)
            FPLATEAUAttributeValue& Value);
};
