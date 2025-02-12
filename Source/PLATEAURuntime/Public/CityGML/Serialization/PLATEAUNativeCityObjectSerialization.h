// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUCityObjectSerializationBase.h"
#include "CityGML/PLATEAUAttributeValue.h"

namespace plateau::polygonMesh {
    class Mesh;
    class Node;
    enum class MeshGranularity;
    struct CityObjectIndex;
}
namespace citygml
{
    class CityModel;
    class CityObject;
}

/**
* @brief citygml::CityModelをJsonにシリアライズ
*
*/
class PLATEAURUNTIME_API FPLATEAUNativeCityObjectSerialization : public IPLATEAUCityObjectSerializationBase {

public:

    FString SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const plateau::polygonMesh::MeshGranularity& Granularity, std::shared_ptr<const citygml::CityModel> InCityModel);
    FString SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity);

protected:

    void GetAttributesJsonObjectRecursive(const citygml::AttributesMap& InAttributesMap, TArray<TSharedPtr<FJsonValue>>& InAttributesJsonObjectArray);
    TSharedRef<FJsonObject> GetCityJsonObject(const citygml::CityObject* InCityObject);
    TSharedRef<FJsonObject> GetCityJsonObject(const citygml::CityObject* InCityObject, const plateau::polygonMesh::CityObjectIndex& CityObjectIndex);

};
