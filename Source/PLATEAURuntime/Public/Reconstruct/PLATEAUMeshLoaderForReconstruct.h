// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForReconstruct : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForReconstruct();
    FPLATEAUMeshLoaderForReconstruct(const bool InbAutomationTest);

    /**
     * @brief UPLATEAUCityObjectGroupのリストからUPLATEAUCityObjectを取り出し、GmlIDをキーとしたMapを生成
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: GmlID, Value: UPLATEAUCityObject の Map
     */
    static TMap<FString, FPLATEAUCityObject> CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    void ReloadComponentFromNode(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        TMap<FString, FPLATEAUCityObject> CityObj,
        AActor& InActor);

protected:

    virtual void ReloadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& InActor);
    virtual USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor);

    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) override;

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName, 
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData, 
        const std::shared_ptr <const citygml::CityModel> CityModel) override;

    bool InvertMeshNormal() override;
    bool OverwriteTexture() override;

    //分割・結合時に属性情報を保持　
    TMap<FString, FPLATEAUCityObject> CityObjMap;

private:
};
