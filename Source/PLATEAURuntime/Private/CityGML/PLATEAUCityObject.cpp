// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUCityObject.h"


void FPLATEAUCityObject::SetGmlID(const FString& InGmlID) {
    GmlID = InGmlID;
}

void FPLATEAUCityObject::SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex) {
    InternalCityObjectIndex = InIndex;
}

void FPLATEAUCityObject::SetCityObjectsType(const double InType) {
    if (InType <= MAX_int64) {
        Type = static_cast<int64>(InType);
        return;
    }
    // int64 の最大値より大きければ int64 の最大値+1 の値を引き int64 へ変換
    const int64 Temp = InType - (MAX_int64 + 1UL);
    // 最上位ビットを反転することで引いた int64 の最大値+1 分を足したのと同じにする
    Type = Temp ^ ((int64)1UL << 63);
    IsMsbReversed = true;
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

bool UPLATEAUCityObjectBlueprintLibrary::IsMsbReversed(const FPLATEAUCityObject& Value) {
    return Value.IsMsbReversed;
}

FPLATEAUAttributeMap UPLATEAUCityObjectBlueprintLibrary::GetAttributes(const FPLATEAUCityObject& Value) {
    return Value.Attributes.AttributeMap;
}

TArray<FPLATEAUCityObject> UPLATEAUCityObjectBlueprintLibrary::GetChildren(const FPLATEAUCityObject& Value) {
    return Value.Children;
}
