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

    UPLATEAUCityObjectGroup* CreateStaticMeshComponent(
        AActor& Actor,
        USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        const std::string& InNodeName);
    UPLATEAUCityObjectGroup* LoadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        AActor& Actor);
    void LoadNodeRecursive(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel,
        AActor& Actor);
};
