// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include <citygml/material.h>
#include "MeshTypes.h"
#include "CoreMinimal.h"

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

    //前回のロードで作成されたComponentのリストを返します
    TArray<USceneComponent*> GetLastCreatedComponents();

protected:
    USceneComponent* FindChildComponentWithOriginalName(USceneComponent* ParentComponent, const FString& OriginalName);
    FString MakeUniqueGmlObjectName(AActor* Actor, UClass* Class, const FString& BaseName);

    // SubMesh情報等に応じてMaterialを作成
    virtual UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName);
    // Loaderのタイプに応じて異なるStaticMeshComponentを作成
    virtual UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName, 
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData, 
        const std::shared_ptr <const citygml::CityModel> CityModel);

    virtual bool UseCachedMaterial();
    virtual bool InvertMeshNormal();

protected:
    bool bAutomationTest;
    TArray<UStaticMesh*> StaticMeshes;
    TMap<FSubMeshMaterialSet, UMaterialInstanceDynamic*> CachedMaterials;

    /// 何度も同じテクスチャをロードすると重いので使い回せるように覚えておきます
     FPathToTexture PathToTexture;

    // 前回のLoadModel, ReloadComponentFromNode実行時に作成されたComponentを保持しておきます
    TArray<USceneComponent*> LastCreatedComponents;

    virtual UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor,
        USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        const std::string& InNodeName);
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

    virtual bool OverwriteTexture();
};
