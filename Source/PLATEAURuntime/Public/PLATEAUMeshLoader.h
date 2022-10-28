#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

namespace plateau::polygonMesh {
    class Model;
    class Node;
    class SubMesh;
}

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
public:
    void CreateMesh(AActor* ModelActor, USceneComponent* ParentComponent, std::shared_ptr<plateau::polygonMesh::Model> ModelData);

private:
    TUniquePtr<FStaticMeshRenderData> CreateRenderData(
        const std::vector<unsigned>& InIndicesVector, const TArray<FVector>& VerticesArray, 
        const std::vector<TVec2f>& UV1, const std::vector<TVec2f>& UV2, const std::vector<TVec2f>& UV3,
        const std::vector<plateau::polygonMesh::SubMesh>& SubMeshes
        );
    void LoadNodes_InModel(USceneComponent* ParentComponent, const plateau::polygonMesh::Node* Node, AActor& Actor);
    void SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData);
    UStaticMeshComponent* CreateStaticMeshComponent(
        AActor& Actor, USceneComponent& ParentComponent,
        const std::vector<unsigned>& Indices,
        const TArray<FVector>& Vertices,
        FString Name,
        const TArray<UTexture2D*>& SubmeshTextures,
        const std::vector<plateau::polygonMesh::SubMesh>& SubMeshes,
        const std::vector<std::vector<TVec2f>>& UVs);
    UTexture2D* LoadTextureFromPath(const FString& Path);
};
