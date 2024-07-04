// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUMeshLoaderForReconstruct.h"

/**
* @brief 元のコンポーネントから属性、マテリアル, 分割をコピーして利用
* 元のコンポーネントと同一階層にコピーを生成(親階層は既存のものをそのまま利用）
*/
class PLATEAURUNTIME_API FPLATEAUMeshLoaderCloneComponent : public FPLATEAUMeshLoaderForReconstruct {

public:
    FPLATEAUMeshLoaderCloneComponent();
    FPLATEAUMeshLoaderCloneComponent(const bool InbAutomationTest);

    /**
     * @brief 元のComponentを記憶します
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: Component Name(GmlID), Value: Component の Map
     */
    static TMap<FString, UPLATEAUCityObjectGroup*> CreateComponentsMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    void ReloadComponentFromNode(
        const plateau::polygonMesh::Node& InNode,
        TMap<FString, UPLATEAUCityObjectGroup*> Components,
        AActor& InActor);     

    USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor) override;

protected:

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
        const std::shared_ptr <const citygml::CityModel> CityModel) override;

    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) override;

    UPLATEAUCityObjectGroup* GetOriginalComponent(FString Name);

private:

    //元のコンポーネント情報を保持　
    TMap<FString, UPLATEAUCityObjectGroup*> ComponentsMap;

};

