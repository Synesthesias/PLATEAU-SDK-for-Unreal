// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUMeshLoaderForReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelReconstruct {

public:
    FPLATEAUModelReconstruct();
    FPLATEAUModelReconstruct(APLATEAUInstancedCityModel* Actor, const EPLATEAUMeshGranularity ReconstructType);

    /**
     * @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してリストに追加します
     */
    virtual TArray<UPLATEAUCityObjectGroup*> GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents);

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

protected:
    
    /**
     * @brief UPLATEAUCityObjectGroupのリストからUPLATEAUCityObjectを取り出し、GmlIDをキーとしたMapを生成
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: GmlID, Value: UPLATEAUCityObject の Map
     */
    TMap<FString, FPLATEAUCityObject> CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    APLATEAUInstancedCityModel* CityModelActor;
    plateau::polygonMesh::MeshGranularity MeshGranularity;
    bool bDivideGrid;

    TMap<FString, FPLATEAUCityObject> CityObjMap;

    /**
     * @brief 指定したMeshLoaderで、生成されたModelからStaticMeshコンポーネントを再生成します
     * @param
     */
    TArray<USceneComponent*> ReconstructFromConvertedModelWithMeshLoader(FPLATEAUMeshLoaderForReconstruct& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model);

};
