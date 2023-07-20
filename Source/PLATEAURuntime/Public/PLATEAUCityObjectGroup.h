// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <memory>
#include <plateau/polygon_mesh/city_object_list.h>
#include "PLATEAUCityObjectGroup.generated.h"

namespace plateau::polygonMesh {
    class Mesh;
}

namespace citygml {
    class CityModel;
}

class CityObject;
class FPLATEAUCityObject;
struct FLoadInputData;

// USTRUCT()
// struct FPLATEAUAttribute {
//     GENERATED_USTRUCT_BODY()
//
//     FPLATEAUAttribute() {}
//     FString Key;
//     FString Type;
//     FString Value;
// };

USTRUCT()
struct FPLATEAUCityJsonObject {
    GENERATED_USTRUCT_BODY()

    FPLATEAUCityJsonObject() {}

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
    FString GmlId;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
    TArray<int> CityObjectIndex;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
    int64 CityObjectType;
    // TArray<FPLATEAUAttribute> Attributes;
};

UCLASS()
class UPLATEAUCityObjectGroup : public UStaticMeshComponent {
    GENERATED_BODY()
public:
    void SetNodeName(const std::string& InNodeName);
    void InitializeSerializedCityObjects(
        const plateau::polygonMesh::Mesh& Mesh,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel);
    FPLATEAUCityObject GetCityObjectByRaycast();
    FPLATEAUCityObject GetCityObject(FVector2d UV);
    FPLATEAUCityObject GetCityObject(plateau::polygonMesh::CityObjectIndex Index);
    TArray<FPLATEAUCityObject> GetALlCityObjects();

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU")
    FString SerializedCityObjects;
private:
    std::string NodeName;
    TArray<FPLATEAUCityObject> DeserializedCityObjects;
};
