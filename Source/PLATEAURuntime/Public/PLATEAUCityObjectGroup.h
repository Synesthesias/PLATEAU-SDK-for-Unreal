// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <plateau/polygon_mesh/city_object_list.h>

class CityObject;
class FPLATEAUCityObject;

class PLATEAUCityObjectGroup : public UStaticMeshComponent {
public:
    FPLATEAUCityObject GetCityObjectByRaycast();
    FPLATEAUCityObject GetCityObject(FVector2d<int> UV);
    FPLATEAUCityObject GetCityObject(plateau::polygonMesh::CityObjectIndex Index);
    TArray<FPLATEAUCityObject> GetALlCityObjects();

private:
    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU")
    FString SerializedCityObjects;

    TArray<FPLATEAUCityObject> DeserializedCityObjects;
};
