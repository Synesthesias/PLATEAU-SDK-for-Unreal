// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#pragma once

#include "PLATEAUAttributeValue.h"
#include <plateau/polygon_mesh/city_object_list.h>
#include <citygml/cityobject.h>
#include "PLATEAUCityObject.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EPLATEAUCityObjectsType : uint8 {
    COT_GenericCityObject           = 0,
    COT_Building                    = 1,
    COT_Room                        = 2,
    COT_BuildingInstallation        = 3,
    COT_BuildingFurniture           = 4,
    COT_Door                        = 5,
    COT_Window                      = 6,
    COT_CityFurniture               = 7,
    COT_Track                       = 8,
    COT_Road                        = 9,
    COT_Railway                     = 10,
    COT_Square                      = 11,
    COT_PlantCover                  = 12,
    COT_SolitaryVegetationObject    = 13,
    COT_WaterBody                   = 14,
    COT_ReliefFeature               = 15,
    COT_LandUse                     = 16,
    COT_Tunnel                      = 17,
    COT_Bridge                      = 18,
    COT_BridgeConstructionElement   = 19,
    COT_BridgeInstallation          = 20,
    COT_BridgePart                  = 21,
    COT_BuildingPart                = 22,
    COT_WallSurface                 = 23,
    COT_RoofSurface                 = 24,
    COT_GroundSurface               = 25,
    COT_ClosureSurface              = 26,
    COT_FloorSurface                = 27,
    COT_InteriorWallSurface         = 28,
    COT_CeilingSurface              = 29,
    COT_CityObjectGroup             = 30,
    COT_OuterCeilingSurface         = 31,
    COT_OuterFloorSurface           = 32,
    COT_TransportationObject        = 33,
    COT_IntBuildingInstallation     = 34,
    COT_WaterSurface                = 35,
    COT_ReliefComponent             = 36,
    COT_TINRelief                   = 37,
    COT_MassPointRelief             = 38,
    COT_BreaklineRelief             = 39,
    COT_RasterRelief                = 40,
    COT_Unknown                     = 41,
    COT_All                         = 63
};

USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct FPLATEAUCityObjectIndex {
    GENERATED_USTRUCT_BODY()

    FPLATEAUCityObjectIndex() : PrimaryIndex(0), AtomicIndex(0) {
    }

    FPLATEAUCityObjectIndex(const int32 InPrimaryIndex, const int32 InAtomicIndex) : PrimaryIndex(InPrimaryIndex), AtomicIndex(InAtomicIndex) {
    }

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    int32 PrimaryIndex;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU|CityGML")
    int32 AtomicIndex;

    bool operator==(const FPLATEAUCityObjectIndex& Other) const {
        return PrimaryIndex == Other.PrimaryIndex && AtomicIndex == Other.AtomicIndex;
    }
};

USTRUCT(BlueprintType, Category = "PLATEAU|CityGML")
struct PLATEAURUNTIME_API FPLATEAUCityObject {
    GENERATED_USTRUCT_BODY()

    FString GmlID;
    FPLATEAUCityObjectIndex CityObjectIndex;
    EPLATEAUCityObjectsType Type;
    FPLATEAUAttributeMap Attributes;
    TArray<FPLATEAUCityObject> Children;

    void SetGmlID(const FString& InGmlID);
    void SetCityObjectIndex(const plateau::polygonMesh::CityObjectIndex& InIndex);
    void SetCityObjectsType(const FString& InType);
    void SetAttribute(const TMap<FString, FPLATEAUAttributeValue>& InAttributes);

    static FString CityObjectsTypeToString(const citygml::CityObject::CityObjectsType InType);
    static FString CityObjectsTypeToString(const EPLATEAUCityObjectsType InType);
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
    static EPLATEAUCityObjectsType GetType(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FPLATEAUAttributeMap GetAttributes(UPARAM(ref) const FPLATEAUCityObject& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static TArray<FPLATEAUCityObject> GetChildren(UPARAM(ref) const FPLATEAUCityObject& Value);
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static FString GetTypeAsString(UPARAM(ref) const EPLATEAUCityObjectsType& Value);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|CityGML")
    static int64 GetTypeAsInt64(UPARAM(ref) const EPLATEAUCityObjectsType& Type);
};
