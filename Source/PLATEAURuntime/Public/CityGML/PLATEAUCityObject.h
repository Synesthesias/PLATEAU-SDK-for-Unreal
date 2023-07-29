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

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    FString GmlID;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    FPLATEAUCityObjectIndex CityObjectIndex;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    int64 Type;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    TMap<FString, FPLATEAUAttributeValue> Attributes;

    // UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    // TArray<FPLATEAUCityObject> Children;

    void SetGmlID(const FString& InGmlID);
    void SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex);
    void SetCityObjectsType(const int64 InType);
    void SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes);
    // void GetChildren(TArray<TSharedPtr<FPLATEAUCityObject>> InCityObjectObjects);
private:
    plateau::polygonMesh::CityObjectIndex InternalCityObjectIndex;
};
