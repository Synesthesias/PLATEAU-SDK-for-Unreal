// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUAttributeValue.h"
#include <citygml/cityobject.h>
#include <plateau/polygon_mesh/city_object_list.h>
#include "PLATEAUCityObject.generated.h"

/*
 * 都市オブジェクトのBlueprint向けラッパーです。
 */
USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUCityObject {
    GENERATED_USTRUCT_BODY()

    FPLATEAUCityObject()
        : Data(nullptr) {}
    FPLATEAUCityObject(const citygml::CityObject* const Data)
        : Data(const_cast<citygml::CityObject*>(Data)) {}
    TMap<FString, FPLATEAUAttributeValue> GetAttributes();
    void SetAttribute(TMap<FString, FPLATEAUAttributeValue> InAttributes);
    FString GetGmlID();
    void SetGmlID(FString InGmlID);
    citygml::CityObject::CityObjectsType GetType();
    void SetCityObjectsType(citygml::CityObject::CityObjectsType InType);
    plateau::polygonMesh::CityObjectIndex GetCityObjectIndex();
    void SetCityObjectIndex(plateau::polygonMesh::CityObjectIndex InIndex);
    TArray<TSharedPtr<FPLATEAUCityObject>> GetChildren();
    void GetChildren(TArray<TSharedPtr<FPLATEAUCityObject>> InCityObjectObjects);
private:
    friend class UPLATEAUCityObjectBlueprintLibrary;

    citygml::CityObject* Data;
    TSharedPtr<FPLATEAUAttributeMap> AttributeMapCache;
    FString GmlID;
    TMap<FString, FPLATEAUAttributeValue> Attributes;
    citygml::CityObject::CityObjectsType Type;
    plateau::polygonMesh::CityObjectIndex CityObjectIndex;
    TArray<TSharedPtr<FPLATEAUCityObject>> Children;
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
