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
    void LoadModel(AActor* ModelActor, USceneComponent* ParentComponent, std::shared_ptr<plateau::polygonMesh::Model> InModel);

private:
    TArray<UStaticMesh*> StaticMeshes;

    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor, USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        FString Name);
    USceneComponent* LoadNode(USceneComponent* ParentComponent, const plateau::polygonMesh::Node* Node, AActor& Actor);
    void LoadNodeRecursive(USceneComponent* ParentComponent, const plateau::polygonMesh::Node* Node, AActor& Actor);
};
