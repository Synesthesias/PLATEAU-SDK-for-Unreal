// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include "Dom/JsonObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUAttributeValue.generated.h"


UENUM(BlueprintType, Category = "PLATEAU|CityGML")
enum class EPLATEAUAttributeType : uint8 {
    String,
    Double,
    Integer,
    Date,
    Uri,
    Measure,
    AttributeSets,
    Boolean
};

USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct PLATEAURUNTIME_API FPLATEAUAttributeValue {
    GENERATED_USTRUCT_BODY()

    EPLATEAUAttributeType Type = EPLATEAUAttributeType::String;
    int IntValue = 0;
    double DoubleValue = 0;
    FString StringValue;
    TSharedPtr<struct FPLATEAUAttributeMap> Attributes;

    void SetType(const FString& InType);
    void SetValue(const EPLATEAUAttributeType&, const TSharedPtr<FJsonObject>& InValue);
    void SetValue(const EPLATEAUAttributeType&, const FString& InValue);
    void SetValue(const TArray<TSharedPtr<FJsonValue>>& InValue);
};

USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct FPLATEAUAttributeMap {
    GENERATED_USTRUCT_BODY()

    FPLATEAUAttributeMap() {
    }

    FPLATEAUAttributeMap(const TMap<FString, FPLATEAUAttributeValue>& InAttributeMap) {
        AttributeMap.Append(InAttributeMap);
    }

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    TMap<FString, FPLATEAUAttributeValue> AttributeMap;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUAttributeValueBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static EPLATEAUAttributeType GetType(UPARAM(ref) const FPLATEAUAttributeValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static int GetInt(UPARAM(ref) const FPLATEAUAttributeValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static double GetDouble(UPARAM(ref) const FPLATEAUAttributeValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FString GetString(UPARAM(ref) const FPLATEAUAttributeValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FPLATEAUAttributeMap GetAttributes(UPARAM(ref) const FPLATEAUAttributeValue& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static TArray<FPLATEAUAttributeValue> GetAttributesByKey(UPARAM(ref) const FString& Key, UPARAM(ref) const FPLATEAUAttributeMap& AttributeMap);
};
