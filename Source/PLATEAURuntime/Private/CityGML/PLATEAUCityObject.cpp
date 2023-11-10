// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUCityObject.h"


namespace {
    TArray<FString> EPLATEAUCityObjectsTypeNameArray = {
        "GenericCityObject",
        "Building",
        "Room",
        "BuildingInstallation",
        "BuildingFurniture",
        "Door",
        "Window",
        "CityFurniture",
        "Track",
        "Road",
        "Railway",
        "Square",
        "PlantCover",
        "SolitaryVegetationObject",
        "WaterBody",
        "ReliefFeature",
        "LandUse",
        "Tunnel",
        "Bridge",
        "BridgeConstructionElement",
        "BridgeInstallation",
        "BridgePart",
        "BuildingPart",
        "WallSurface",
        "RoofSurface",
        "GroundSurface",
        "ClosureSurface",
        "FloorSurface",
        "InteriorWallSurface",
        "CeilingSurface",
        "CityObjectGroup",
        "OuterCeilingSurface",
        "OuterFloorSurface",
        "TransportationObject",
        "IntBuildingInstallation",
        "WaterSurface",
        "ReliefComponent",
        "TINRelief",
        "MassPointRelief",
        "BreaklineRelief",
        "RasterRelief",
        "Unknown",
    };
    
    /**
     * @brief ビット数取得
     * @param Value 対象値
     * @return ビット数
     */
    int32 Count64Bit(const uint64_t Value) {
        uint64_t Count = (Value & 0x5555555555555555) + (Value >> 1 & 0x5555555555555555);
        Count = (Count & 0x3333333333333333) + (Count >> 2 & 0x3333333333333333);
        Count = (Count & 0x0f0f0f0f0f0f0f0f) + (Count >> 4 & 0x0f0f0f0f0f0f0f0f);
        Count = (Count & 0x00ff00ff00ff00ff) + (Count >> 8 & 0x00ff00ff00ff00ff);
        Count = (Count & 0x0000ffff0000ffff) + (Count >> 16 & 0x0000ffff0000ffff);
        return static_cast<int32>((Count & 0x00000000ffffffff) + (Count >> 32 & 0x00000000ffffffff));
    }

    /**
     * @brief 最大ビット取得
     * @param Value 対象値
     * @param Out 最大ビット
     */
    void MSB64Bit(uint64_t Value, int32& Out) {
        if (Value == 0) {
            Out = 0;
            return;
        }
        Value |= Value >> 1;
        Value |= Value >> 2;
        Value |= Value >> 4;
        Value |= Value >> 8;
        Value |= Value >> 16;
        Value |= Value >> 32;
        Out = Count64Bit(Value) - 1;
    }
}

namespace plateau::CityObject {
    static FString CityObjectsTypeToString(const citygml::CityObject::CityObjectsType InType) {
        int32 TypeMsbBit;
        MSB64Bit(static_cast<uint64_t>(InType), TypeMsbBit);
        return EPLATEAUCityObjectsTypeNameArray[TypeMsbBit];
    }
}

void FPLATEAUCityObject::SetGmlID(const FString& InGmlID) {
    GmlID = InGmlID;
}

void FPLATEAUCityObject::SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex) {
    CityObjectIndex = FPLATEAUCityObjectIndex(InIndex.primary_index, InIndex.atomic_index);
}

void FPLATEAUCityObject::SetCityObjectsType(const FString& InType) {
    int32 TypeMsbBit = EPLATEAUCityObjectsTypeNameArray.IndexOfByKey(InType);
    Type = static_cast<EPLATEAUCityObjectsType>(TypeMsbBit);
}

void FPLATEAUCityObject::SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes) {
    Attributes = InAttributes;
}

FString UPLATEAUCityObjectBlueprintLibrary::GetGmlID(const FPLATEAUCityObject& Value) {
    return Value.GmlID;
}

FPLATEAUCityObjectIndex UPLATEAUCityObjectBlueprintLibrary::GetCityObjectIndex(const FPLATEAUCityObject& Value) {
    return Value.CityObjectIndex;
}

EPLATEAUCityObjectsType UPLATEAUCityObjectBlueprintLibrary::GetType(const FPLATEAUCityObject& Value) {
    return Value.Type;
}

FPLATEAUAttributeMap UPLATEAUCityObjectBlueprintLibrary::GetAttributes(const FPLATEAUCityObject& Value) {
    return Value.Attributes.AttributeMap;
}

TArray<FPLATEAUCityObject> UPLATEAUCityObjectBlueprintLibrary::GetChildren(const FPLATEAUCityObject& Value) {
    return Value.Children;
}

uint64_t UPLATEAUCityObjectBlueprintLibrary::GetTypeAsUint64(const FPLATEAUCityObject& Value) {
    return 1ll << static_cast<uint64_t>(Value.Type);
}