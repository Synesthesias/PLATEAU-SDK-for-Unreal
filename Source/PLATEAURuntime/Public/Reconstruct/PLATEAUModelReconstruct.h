// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "GameFramework/Actor.h"
#include "PLATEAUCityObjectGroup.h"
#include <plateau/polygon_mesh/model.h>
#include <plateau/dataset/city_model_package.h>
#include <PLATEAUImportSettings.h>


class PLATEAURUNTIME_API FPLATEAUModelReconstruct {

public:
    FPLATEAUModelReconstruct() {}
    FPLATEAUModelReconstruct(APLATEAUInstancedCityModel* Actor, const EPLATEAUMeshGranularity ReconstructType) {
        CityModelActor = Actor;
        MeshGranularity = static_cast<plateau::polygonMesh::MeshGranularity>(ReconstructType);
        bDivideGrid = false;
    }

    /**
     * @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してリストに追加します
     */
    virtual TArray<UPLATEAUCityObjectGroup*> GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents);

    /**
     * @brief 選択されたComponentの結合・分割処理を行います。
     * @param
     */
    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model);

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

    TArray<USceneComponent*> ReconstructFromConvertedModel(FPLATEAUMeshLoader& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model);

};
