// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

namespace plateau::polygonMesh {
    class Model;
    class Node;
    class SubMesh;
    class Mesh;
}

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
public:
    FPLATEAUMeshLoader(const bool InbAutomationTest) {
        bAutomationTest = InbAutomationTest;
    }
    void LoadModel(AActor* ModelActor, USceneComponent* ParentComponent, std::shared_ptr<plateau::polygonMesh::Model> InModel, TAtomic<bool>* bCanceled);

private:
    bool bAutomationTest;
    TArray<UStaticMesh*> StaticMeshes;

    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor, USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        FString Name);
    USceneComponent* LoadNode(USceneComponent* ParentComponent, const plateau::polygonMesh::Node& Node, AActor& Actor);
    void LoadNodeRecursive(USceneComponent* ParentComponent, const plateau::polygonMesh::Node& Node, AActor& Actor);
};
