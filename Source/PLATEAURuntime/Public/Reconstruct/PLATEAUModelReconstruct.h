// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUMeshLoaderForReconstruct.h"

using ConvertGranularity = plateau::granularityConvert::ConvertGranularity;

class PLATEAURUNTIME_API FPLATEAUModelReconstruct {

public:
    FPLATEAUModelReconstruct();
    FPLATEAUModelReconstruct(APLATEAUInstancedCityModel* Actor, const ConvertGranularity Granularity);

    virtual ~FPLATEAUModelReconstruct() = default;
    /**
     * @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してリストに追加します
     */
    virtual TArray<UPLATEAUCityObjectGroup*> GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents);

    /**
     * @brief 粒度単位でコンポーネントをフィルタリングします
     */
    virtual TArray<UPLATEAUCityObjectGroup*> FilterComponentsByConvertGranularity(TArray<UPLATEAUCityObjectGroup*> TargetComponents, const ConvertGranularity Granularity);

    /**
     * @brief 選択されたComponentの結合・分割処理用のModelを生成します
     * @param
     */
    virtual std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    /**
     * @brief 生成されたModelからStaticMeshコンポーネントを再生成します
     * @param
     */
    virtual TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model);

    /**
     * @brief EPLATEAUMeshGranularityをplateau::granularityConvert::ConvertGranularityに変換します
     */
    static ConvertGranularity GetConvertGranularityFromReconstructType(const EPLATEAUMeshGranularity ReconstructType);

protected:
    
    APLATEAUInstancedCityModel* CityModelActor;
    ConvertGranularity ConvGranularity;
    bool bDivideGrid;

    TMap<FString, FPLATEAUCityObject> CityObjMap;

    /**
     * @brief CityObjectのChildrenのidリストを返します
     * @param
     */
    void GetChildrenGmlIds(const FPLATEAUCityObject CityObj, TSet<FString>& IdList);

    /**
     * @brief 指定したMeshLoaderで、生成されたModelからStaticMeshコンポーネントを再生成します
     * @param
     */
    TArray<USceneComponent*> ReconstructFromConvertedModelWithMeshLoader(FPLATEAUMeshLoaderForReconstruct& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model);

    virtual std::shared_ptr<plateau::polygonMesh::Model> ConvertModelWithGranularity(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const ConvertGranularity Granularity);

};
