// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUCityObject.h"


void FPLATEAUCityObject::SetGmlID(const FString& InGmlID) {
    GmlID = InGmlID;
}

void FPLATEAUCityObject::SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex) {
    CityObjectIndex = FPLATEAUCityObjectIndex(InIndex.primary_index, InIndex.atomic_index);
}

void FPLATEAUCityObject::SetCityObjectsType(const int64 InType) {
    Type = InType;
}

void FPLATEAUCityObject::SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes) {
    Attributes = InAttributes;
}
