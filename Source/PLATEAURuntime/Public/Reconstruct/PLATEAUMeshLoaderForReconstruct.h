// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include <plateau/granularity_convert/granularity_converter.h>
#include "CityGML/PLATEAUCityObject.h"

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

using ConvertGranularity = plateau::granularityConvert::ConvertGranularity;

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

    /**
     * @brief EPLATEAUMeshGranularityをplateau::polygonMesh::MeshGranularityに変換します
     */
    static plateau::polygonMesh::MeshGranularity ConvertGranularityToMeshGranularity(const ConvertGranularity ConvertGranularity);

    void ReloadComponentFromNode(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        ConvertGranularity Granularity,
        TMap<FString, FPLATEAUCityObject> CityObj,
        AActor& InActor);

protected:

    virtual void ReloadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        ConvertGranularity Granularity,
        AActor& InActor);
    virtual USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        ConvertGranularity Granularity,
        AActor& Actor);

    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) override;

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName, 
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData, 
        const std::shared_ptr <const citygml::CityModel> CityModel) override;

    bool InvertMeshNormal() override;
    bool OverwriteTexture() override;

    //分割・結合時に属性情報を保持　
    TMap<FString, FPLATEAUCityObject> CityObjMap;

    ConvertGranularity ConvGranularity;

private:
};
