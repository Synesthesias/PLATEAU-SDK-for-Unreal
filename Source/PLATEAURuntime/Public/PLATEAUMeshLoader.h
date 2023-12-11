// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include <citygml/material.h>

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"

struct FPLATEAUCityObject;

namespace citygml {
    class CityModel;
}

namespace plateau::polygonMesh {
    enum class MeshGranularity;
    class Model;
    class Node;
    class SubMesh;
    class Mesh;
}

struct FSubMeshMaterialSet {
public:
    bool hasMaterial;
    FVector3f Diffuse;
    FVector3f Specular;
    FVector3f Emissive;
    float Shininess;
    float Transparency;
    float Ambient;
    bool isSmooth;
    FString TexturePath;
    FPolygonGroupID PolygonGroupID = 0;
    FString MaterialSlot = FString("");
    int GameMaterialID = 0;

    FSubMeshMaterialSet();
    FSubMeshMaterialSet(std::shared_ptr<const citygml::Material> mat, FString texPath, int matId);
    bool operator==(const FSubMeshMaterialSet& Other) const;
    bool Equals(const FSubMeshMaterialSet& Other) const;
private:
};

FORCEINLINE uint32 GetTypeHash(const FSubMeshMaterialSet& Value);

struct FLoadInputData;
class UPLATEAUCityObjectGroup;

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
    using FPathToTexture = TMap<FString, UTexture2D*>;
public:
    FPLATEAUMeshLoader() {
        bAutomationTest = false;
    }
    FPLATEAUMeshLoader(const bool InbAutomationTest) {
        bAutomationTest = InbAutomationTest;
    }

    void LoadModel(
        AActor* ModelActor,
        USceneComponent* ParentComponent,
        std::shared_ptr<plateau::polygonMesh::Model> Model,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        TAtomic<bool>* bCanceled);

    //分割・結合時
    void ReloadComponentFromNode(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        TMap<FString, FPLATEAUCityObject> cityObjMap,     
        AActor& InActor);


    //Material分け時のタイプリストをセットします
    //void SetClassificationTypes(TArray<uint8> &Types);

    //Material分け時のマテリアルリストをセットします
    //void SetClassificationMaterials(TMap<uint8, UMaterialInterface*> &Materials);

    //前回のロードで作成されたComponentのリストを返します
    TArray<USceneComponent*> GetLastCreatedComponents();

protected:
    virtual bool CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component);
    virtual UMaterialInstanceDynamic* GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component);
    virtual bool UseCachedMaterial();

protected:
    bool bAutomationTest;
    TArray<UStaticMesh*> StaticMeshes;
    TMap<FSubMeshMaterialSet, UMaterialInstanceDynamic*> CachedMaterials;

    /// 何度も同じテクスチャをロードすると重いので使い回せるように覚えておきます
     FPathToTexture PathToTexture;

    //分割・結合時に属性情報を保持
    TMap<FString, FPLATEAUCityObject> CityObjMap;


    //Material分け時のタイプリスト
    //TArray<uint8>  ClassificationTypes;

    //Material分け時のマテリアルリスト
    //TMap<uint8, UMaterialInterface*> ClassificationMaterials;

    // 前回のLoadModel, ReloadComponentFromNode実行時に作成されたComponentを保持しておきます
    TArray<USceneComponent*> LastCreatedComponents;

    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor,
        USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        const std::string& InNodeName,
        bool IsReconstruct = false);
    USceneComponent* LoadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        AActor& Actor);
    void LoadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        const FLoadInputData& InLoadInputData,
        const std::shared_ptr<const citygml::CityModel> InCityModel,
        AActor& InActor);

    //分割・結合時
    void ReloadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& InActor);
    USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor);
};
