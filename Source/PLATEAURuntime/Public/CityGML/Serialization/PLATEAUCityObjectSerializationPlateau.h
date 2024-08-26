// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUCityObjectSerializationBase.h"

class PLATEAURUNTIME_API FPLATEAUCityObjectSerializationPlateau : public IPLATEAUCityObjectSerializationBase {

public:

    /**
     * @brief 結合・分割時のメッシュを持つノードをシリアライズ
     * @param InNodeName ノード名
     * @param InMesh メッシュ情報
     * @param Granularity メッシュの結合単位を確認するために用いる
     * @param CityObjMap 結合・分割前に保存したFPLATEAUCityObjectのMap
     */
    FString SerializeCityObject(const FString& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const plateau::polygonMesh::MeshGranularity& Granularity, TMap<FString, FPLATEAUCityObject> CityObjMap);

    /**
     * @brief 結合・分割時のメッシュを持たないノードをシリアライズ
     * @param InNode シリアライズ対象ノード
     * @param InCityObject 結合・分割前に保存したFPLATEAUCityObject
     */
    FString SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject);

    /**
     * @brief FPLATEAUCityObjectのシンプルなシリアライズ
     * @param InCityObject FPLATEAUCityObject
     */
    FString SerializeCityObject(const FPLATEAUCityObject& InCityObject, const FString InOutsideParent, const TArray<FString> InOutsideChildren);

protected:

    void GetAttributesJsonObjectRecursive(const FPLATEAUAttributeMap& InAttributesMap, TArray<TSharedPtr<FJsonValue>>& InAttributesJsonObjectArray);
    TSharedRef<FJsonObject> GetCityJsonObject(const FPLATEAUCityObject& InCityObject);
    TSharedRef<FJsonObject> GetCityJsonObject(const FPLATEAUCityObject& InCityObject, const plateau::polygonMesh::CityObjectIndex& CityObjectIndex);
    TSharedRef<FJsonObject> GetCityJsonObjectWithChildren(const FPLATEAUCityObject& InCityObject);
};


