// Fill out your copyright notice in the Description page of Project Settings.


#include "CityGML/PLATEAUCityObject.h"

#include "citygml/cityobject.h"
#include "citygml/object.h"


TMap<FString, FPLATEAUAttributeValue>& UPLATEAUCityObjectBlueprintLibrary::GetAttributeMap(FPLATEAUCityObject& CityObject) {
    if (CityObject.AttributeMapCache != nullptr)
        return *CityObject.AttributeMapCache;

    CityObject.AttributeMapCache = MakeShared<FPLATEAUAttributeMap>();

    if (CityObject.Data == nullptr)
        return *CityObject.AttributeMapCache;

    const auto& AttributeMapData = CityObject.Data->getAttributes();
    for (const auto& [Key, Value] : AttributeMapData) {
        CityObject.AttributeMapCache->Add(UTF8_TO_TCHAR(Key.c_str()), FPLATEAUAttributeValue(&Value));
    }
    return *CityObject.AttributeMapCache;
}
