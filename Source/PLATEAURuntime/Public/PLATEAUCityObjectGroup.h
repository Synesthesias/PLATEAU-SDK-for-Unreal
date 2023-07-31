// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <memory>
#include <plateau/polygon_mesh/city_object_list.h>
#include <plateau/polygon_mesh/node.h>
#include "CityGML/PLATEAUCityObject.h"
#include "PLATEAUCityObjectGroup.generated.h"

namespace plateau::CityObject {
    constexpr TCHAR GmlIdFieldName[]            = TEXT("gmlID");
    constexpr TCHAR CityObjectIndexFieldName[]  = TEXT("cityObjectIndex");
    constexpr TCHAR CityObjectTypeFieldName[]   = TEXT("cityObjectType");
    constexpr TCHAR AttributesFieldName[]       = TEXT("attributes");
    constexpr TCHAR KeyFieldName[]              = TEXT("key");
    constexpr TCHAR TypeFieldName[]             = TEXT("type");
    constexpr TCHAR ValueFieldName[]            = TEXT("value");
    constexpr TCHAR OutsideParentFieldName[]    = TEXT("outsideParent");
    constexpr TCHAR OutsideChildrenFieldName[]  = TEXT("outsideChildren");
    constexpr TCHAR CityObjectsFieldName[]      = TEXT("cityObjects");
    constexpr TCHAR ChildrenFieldName[]         = TEXT("children");    
}

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
class PLATEAURUNTIME_API UPLATEAUCityObjectGroup : public UStaticMeshComponent {
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

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetCityObjectByID(const FString& GmlID);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    TArray<FPLATEAUCityObject> GetAllRootCityObjects();

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU")
    FString SerializedCityObjects;
    UPROPERTY()
    FString OutsideParent;
    UPROPERTY()
    TArray<FString> OutsideChildren;
private:
    TArray<FPLATEAUCityObject> RootCityObjects;
};
