// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUCityObject.h"


void FPLATEAUCityObject::SetGmlID(const FString& InGmlID) {
    GmlID = InGmlID;
}

void FPLATEAUCityObject::SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex) {
    InternalCityObjectIndex = InIndex;
}

void FPLATEAUCityObject::SetCityObjectsType(const int64 InType) {
    Type = InType;
}

void FPLATEAUCityObject::SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes) {
    Attributes = InAttributes;
}

FString UPLATEAUCityObjectBlueprintLibrary::GetGmlID(const FPLATEAUCityObject& Value) {
    return Value.GmlID;
}

FPLATEAUCityObjectIndex UPLATEAUCityObjectBlueprintLibrary::GetCityObjectIndex(const FPLATEAUCityObject& Value) {
    return FPLATEAUCityObjectIndex(Value.InternalCityObjectIndex.primary_index, Value.InternalCityObjectIndex.atomic_index);
}

int64 UPLATEAUCityObjectBlueprintLibrary::GetType(const FPLATEAUCityObject& Value) {
    return Value.Type;
}

FPLATEAUAttributeMap UPLATEAUCityObjectBlueprintLibrary::GetAttributes(const FPLATEAUCityObject& Value) {
    return Value.Attributes.AttributeMap;
}

TArray<FPLATEAUCityObject> UPLATEAUCityObjectBlueprintLibrary::GetChildren(const FPLATEAUCityObject& Value) {
    return Value.Children;
}
