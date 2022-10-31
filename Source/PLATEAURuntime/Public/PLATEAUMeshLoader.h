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
    void LoadNodeRecursive(USceneComponent* ParentComponent, const plateau::polygonMesh::Node* Node, AActor& Actor);
    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor, USceneComponent& ParentComponent,
        const plateau::polygonMesh::Mesh& InMesh,
        FString Name,
        const TArray<UTexture2D*>& SubMeshTextures) const;
    UTexture2D* LoadTextureFromPath(const FString& Path);
};
