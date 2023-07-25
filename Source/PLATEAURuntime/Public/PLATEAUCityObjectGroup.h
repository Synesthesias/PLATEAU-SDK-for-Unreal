// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <memory>
#include <plateau/polygon_mesh/city_object_list.h>
#include <plateau/polygon_mesh/node.h>
#include "PLATEAUCityObjectGroup.generated.h"

namespace plateau::polygonMesh {
    class Mesh;
}

namespace citygml {
    class CityModel;
    class CityObject;
}

class FPLATEAUCityObject;
struct FLoadInputData;


UCLASS()
class UPLATEAUCityObjectGroup : public UStaticMeshComponent {
    GENERATED_BODY()
public:
    /**
     * @brief メッシュを持たないがCityObjectを持つノードをシリアライズ
     * @param InNode シリアライズ対象ノード
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     */
    void SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject);

    /**
     * @brief メッシュを持つノードをシリアライズ
     * @param InNodeName ノード名
     * @param InMesh メッシュ情報
     * @param InLoadInputData メッシュの結合単位を確認するために用いる
     * @param InCityModel GMLをパースして得られたモデル
     */
    void SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& InLoadInputData, std::shared_ptr<const citygml::CityModel> InCityModel);

    TSharedPtr<FPLATEAUCityObject> GetPrimaryCityObjectByRaycast();
    TSharedPtr<FPLATEAUCityObject> GetAtomicCityObjectByRaycast();
    TSharedPtr<FPLATEAUCityObject> GetCityObjectByUV(FVector2d UV);
    TSharedPtr<FPLATEAUCityObject> GetCityObjectByIndex(plateau::polygonMesh::CityObjectIndex Index);
    TSharedPtr<FPLATEAUCityObject> GetCityObjectByID(FString GmlId);
    TArray<TSharedPtr<FPLATEAUCityObject>> GetAllRootCityObjects();

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU")
    FString SerializedCityObjects;
    UPROPERTY()
    FString OutsideParent;
    UPROPERTY()
    TArray<FString> OutsideChildren;
private:
    std::string NodeName;
    TArray<FPLATEAUCityObject> DeserializedCityObjects;
    TArray<TSharedPtr<FPLATEAUCityObject>> RootCityObjects;
};
