// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <memory>
#include <plateau/polygon_mesh/node.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include "Components/StaticMeshComponent.h"
#include "CityGML/PLATEAUCityObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "PLATEAUComponentInterface.h"
#include "CityGML/Serialization/PLATEAUNativeCityObjectSerialization.h"
#include "CityGML/Serialization/PLATEAUCityObjectSerialization.h"
#include "CityGML/Serialization/PLATEAUCityObjectDeserialization.h"
#include "PLATEAUCityObjectGroup.generated.h"

namespace plateau::CityObjectGroup {
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

struct FPLATEAUCityObject;
struct FLoadInputData;


UCLASS()
class PLATEAURUNTIME_API UPLATEAUCityObjectGroup : public UStaticMeshComponent , public IPLATEAUComponentInterface{
    GENERATED_BODY()
public:
    /**
     * @brief レイキャストヒットした位置のUVを取得
     * @param HitResult レイキャストヒット結果
     * @param UV 取得したい位置のUV
     * @param UVChannel 対象のUVチャンネル
     */
    static void FindCollisionUV(const FHitResult& HitResult, FVector2D& UV, const int32 UVChannel = 3);
    
    /**
     * @brief メッシュを持たないがCityObjectを持つノードをシリアライズ
     * @param InNode シリアライズ対象ノード
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     */
    void SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity);

    /**
     * @brief メッシュを持つノードをシリアライズ
     * @param InNodeName ノード名
     * @param InMesh メッシュ情報
     * @param InLoadInputData メッシュの結合単位を確認するために用いる
     * @param InCityModel GMLをパースして得られたモデル
     */
    void SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& InLoadInputData, std::shared_ptr<const citygml::CityModel> InCityModel);

    /**
     * @brief 結合・分割時のメッシュを持たないノードをシリアライズ
     * @param InNode シリアライズ対象ノード
     * @param InCityObject 結合・分割前に保存したFPLATEAUCityObject
     */
    void SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject, const plateau::granularityConvert::ConvertGranularity& Granularity);
    void SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity);
    void SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject);

    /** 
     * @brief 結合・分割時のメッシュを持つノードをシリアライズ
     * @param InNodeName ノード名
     * @param InMesh メッシュ情報
     * @param InLoadInputData メッシュの結合単位を確認するために用いる
     * @param CityObjMap 結合・分割前に保存したFPLATEAUCityObjectのMap
     */
    void SerializeCityObject(const FString& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const plateau::granularityConvert::ConvertGranularity& Granularity, TMap<FString, FPLATEAUCityObject> CityObjMap);
    void SerializeCityObject(const FString& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const plateau::polygonMesh::MeshGranularity& Granularity, TMap<FString, FPLATEAUCityObject> CityObjMap);

    /**
     * @brief FPLATEAUCityObjectのシンプルなシリアライズ
     * @param InCityObject FPLATEAUCityObject
     */
    void SerializeCityObject(const FPLATEAUCityObject& InCityObject, const FString InOutsideParent = "", const TArray<FString> InOutsideChildren = {});

    /**
     * @brief MeshGranularity取得Getter
     */
    const plateau::granularityConvert::ConvertGranularity GetConvertGranularity();
    void SetConvertGranularity(const plateau::granularityConvert::ConvertGranularity Granularity);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetPrimaryCityObjectByRaycast(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetAtomicCityObjectByRaycast(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetCityObjectByUV(const FVector2D& UV);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetCityObjectByIndex(const FPLATEAUCityObjectIndex Index);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    FPLATEAUCityObject GetCityObjectByID(const FString& GmlID);

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
    TArray<FPLATEAUCityObject> GetAllRootCityObjects();

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU")
    FString SerializedCityObjects;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU")
    FString OutsideParent;

    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU")
    TArray<FString> OutsideChildren;

    /**
     * @brief MeshGranularity/ConvertGranularityのintの値
     */
    UPROPERTY(BlueprintReadOnly, Category = "PLATEAU")
    int MeshGranularityIntValue;

private:
    TArray<FPLATEAUCityObject> RootCityObjects;
    void SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity);

    FPLATEAUNativeCityObjectSerialization CityModelSerializer;
    FPLATEAUCityObjectSerialization PlateauSerializer;
    FPLATEAUCityObjectDeserialization Deserializer;
};
