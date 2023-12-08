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

    static FString CityObjectsTypeToString(const EPLATEAUCityObjectsType InType) {
        int32 index = static_cast<int32>(InType);
        if(EPLATEAUCityObjectsTypeNameArray.IsValidIndex(index))
            return EPLATEAUCityObjectsTypeNameArray[index];
        return FString::FromInt(index);
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

int64 UPLATEAUCityObjectBlueprintLibrary::GetTypeAsInt64(const EPLATEAUCityObjectsType& Type) {
    return 1ll << static_cast<int64>(Type);
}

FString UPLATEAUCityObjectBlueprintLibrary::GetTypeAsString(const EPLATEAUCityObjectsType& Value) {
    switch (Value) {
    case EPLATEAUCityObjectsType::COT_GenericCityObject: return TEXT("汎用都市 (Generic)");
    case EPLATEAUCityObjectsType::COT_Building: return TEXT("建築物 (Building)");
    case EPLATEAUCityObjectsType::COT_BuildingInstallation: return TEXT("建築物付属設備 (BuildingInstallation)");
    case EPLATEAUCityObjectsType::COT_Door: return TEXT("ドア (Door)");
    case EPLATEAUCityObjectsType::COT_Window: return TEXT("窓 (Window)");
    case EPLATEAUCityObjectsType::COT_BuildingPart: return TEXT("建築物パーツ (BuildingPart)");
    case EPLATEAUCityObjectsType::COT_WallSurface: return TEXT("壁面 (WallSurface)");
    case EPLATEAUCityObjectsType::COT_RoofSurface: return TEXT("屋根面 (RoofSurface)");
    case EPLATEAUCityObjectsType::COT_GroundSurface: return TEXT("接地面 (GroundSurface)");
    case EPLATEAUCityObjectsType::COT_ClosureSurface: return TEXT("開口部 (ClosureSurface)");
    case EPLATEAUCityObjectsType::COT_OuterCeilingSurface: return TEXT("外側の天井 (OuterCeilingSurface)");
    case EPLATEAUCityObjectsType::COT_OuterFloorSurface: return TEXT("屋根の通行可能部分 (OuterFloorSurface)");
    case EPLATEAUCityObjectsType::COT_CityFurniture: return TEXT("都市設備 (CityFurniture)");
    case EPLATEAUCityObjectsType::COT_Track: return TEXT("徒歩道 (Track)");
    case EPLATEAUCityObjectsType::COT_Road: return TEXT("道路 (Road)");
    case EPLATEAUCityObjectsType::COT_Railway: return TEXT("鉄道 (Railway)");
    case EPLATEAUCityObjectsType::COT_Square: return TEXT("広場 (Square)");
    case EPLATEAUCityObjectsType::COT_PlantCover: return TEXT("植生 (Vegetation)");
    case EPLATEAUCityObjectsType::COT_SolitaryVegetationObject: return TEXT("植生 (Vegetation)");
    case EPLATEAUCityObjectsType::COT_WaterBody: return TEXT("水部 (WaterBody)");
    case EPLATEAUCityObjectsType::COT_LandUse: return TEXT("土地利用 (LandUse)");
    case EPLATEAUCityObjectsType::COT_Tunnel: return TEXT("トンネル (Tunnel)");
    case EPLATEAUCityObjectsType::COT_Bridge: return TEXT("橋梁 (Bridge)");
    case EPLATEAUCityObjectsType::COT_ReliefComponent: return TEXT("土地起伏 (Relief)");
    case EPLATEAUCityObjectsType::COT_TINRelief: return TEXT("ポリゴンによる起伏表現 (TINRelief)");
    case EPLATEAUCityObjectsType::COT_MassPointRelief: return TEXT("点群による起伏表現 (MassPointRelief)");
    case EPLATEAUCityObjectsType::COT_Unknown: return TEXT("その他 (Unknown)");
    default: return UEnum::GetValueAsString(Value);
    }
}