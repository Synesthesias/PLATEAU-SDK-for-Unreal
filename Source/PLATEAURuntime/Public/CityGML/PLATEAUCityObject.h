// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "PLATEAUAttributeValue.h"
#include <plateau/polygon_mesh/city_object_list.h>
#include "PLATEAUCityObject.generated.h"


USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct FPLATEAUCityObjectIndex {
    GENERATED_USTRUCT_BODY()

    FPLATEAUCityObjectIndex() : PrimaryIndex(0), AtomicIndex(0) {
    }

    FPLATEAUCityObjectIndex(const int InPrimaryIndex, const int InAtomicIndex) : PrimaryIndex(InPrimaryIndex), AtomicIndex(InAtomicIndex) {
    }

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    int PrimaryIndex;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    int AtomicIndex;
};

USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct PLATEAURUNTIME_API FPLATEAUCityObject {
    GENERATED_USTRUCT_BODY()

    FString GmlID;
    plateau::polygonMesh::CityObjectIndex InternalCityObjectIndex;
    int64 Type = 0;
    bool IsMsbReversed = false;
    FPLATEAUAttributeMap Attributes;
    TArray<FPLATEAUCityObject> Children;

    void SetGmlID(const FString& InGmlID);
    void SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex);
    void SetCityObjectsType(const double InType);
    void SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes);
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUCityObjectBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FString GetGmlID(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FPLATEAUCityObjectIndex GetCityObjectIndex(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static int64 GetType(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static bool IsMsbReversed(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FPLATEAUAttributeMap GetAttributes(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static TArray<FPLATEAUCityObject> GetChildren(UPARAM(ref) const FPLATEAUCityObject& Value);
};
