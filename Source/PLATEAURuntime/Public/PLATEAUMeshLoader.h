// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"

namespace citygml {
    class CityModel;
}

namespace plateau::polygonMesh {
    class Model;
    class Node;
    class SubMesh;
    class Mesh;
}

struct FSubMeshValueSet {

    bool hasMaterial;
    FVector3f Diffuse;
    FVector3f Specular;
    FVector3f Emissive;
    float Shininess;
    float Transparency;
    float Ambient;
    bool isSmooth;
    FString TexturePath;

    int32 PolygonGroupID = 0;
    int PolygonGroupIndex = 0;
    FString MaterialSlot = FString("");

    FSubMeshValueSet() {}
    FSubMeshValueSet(std::shared_ptr<const citygml::Material> mat, FString texPath) {
        hasMaterial = mat != nullptr;
        if (hasMaterial) {
            auto dif = mat->getDiffuse();
            Diffuse = FVector3f(dif.x, dif.y, dif.z);
            auto spc = mat->getSpecular();
            Specular = FVector3f(spc.x, spc.y, spc.z);
            auto ems = mat->getEmissive();
            Emissive = FVector3f(ems.x, ems.y, ems.z);
            Shininess = mat->getShininess();
            Transparency = mat->getTransparency();
            Ambient = mat->getAmbientIntensity();
            isSmooth = mat->isSmooth();
        }
        TexturePath = texPath;
    }

    bool operator==(const FSubMeshValueSet& Other) const {
        return Equals(Other);
    }

    bool Equals(const FSubMeshValueSet& Other) const {
        float tl = 0.001f;
        return Diffuse.Equals(Other.Diffuse, tl) &&
            Specular.Equals(Other.Specular, tl) &&
            Emissive.Equals(Other.Emissive, tl) &&
            FMath::IsNearlyEqual(Shininess, Other.Shininess, tl) &&
            FMath::IsNearlyEqual(Transparency, Other.Transparency, tl) &&
            FMath::IsNearlyEqual(Ambient, Other.Ambient, tl) &&
            isSmooth == Other.isSmooth &&
            hasMaterial == Other.hasMaterial &&
            TexturePath.Equals(Other.TexturePath);
    }
};

FORCEINLINE uint32 GetTypeHash(const FSubMeshValueSet& Value) {
    //uint32 Hash = FCrc::MemCrc32(&Value, sizeof(FSubMeshValueSet));

    TArray<uint32> HashArray;
    HashArray.Add(FCrc::MemCrc32(&Value.Diffuse, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Specular, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Emissive, sizeof(FVector3f)));
    HashArray.Add(FCrc::MemCrc32(&Value.Shininess, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.Transparency, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.Ambient, sizeof(float)));
    HashArray.Add(FCrc::MemCrc32(&Value.isSmooth, sizeof(bool)));
    HashArray.Add(FCrc::MemCrc32(&Value.TexturePath, sizeof(FString)));

    uint32 Hash = 0;
    for (auto h : HashArray) {
        Hash = HashCombine(Hash, h);
    }

    return Hash;
}

struct FLoadInputData;
class UPLATEAUCityObjectGroup;

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
public:
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

private:
    bool bAutomationTest;
    TArray<UStaticMesh*> StaticMeshes;

    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor,
        USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        const std::string& InNodeName);
    UStaticMeshComponent* LoadNode(
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
};
