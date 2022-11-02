#pragma once

#include "CoreMinimal.h"

#include "CityGML/PLATEAUCityModel.h"
#include "PLATEAUGeometry.h"
#include "Engine/Texture2D.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
public:
    void CreateMesh(AActor* ModelActor, std::shared_ptr<plateau::polygonMesh::Model> ModelData);

private:
    TUniquePtr<FStaticMeshRenderData> CreateRenderData(const std::vector<unsigned>& InIndicesVector, const TArray<FVector>& VerticesArray, 
        const std::vector<TVec2f>& UV1, const std::vector<TVec2f>& UV2, const std::vector<TVec2f>& UV3);
    void LoadNodes_InModel(USceneComponent* ParentComponent, plateau::polygonMesh::Node* Node, AActor& Actor, int Index, int Count);
    void SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData);
    UStaticMeshComponent* CreateStaticMeshComponent(AActor& Actor, USceneComponent& ParentComponent, std::vector<unsigned> Indices,
        TArray<FVector> Vertices, FString Name, UTexture2D* Texture, std::vector<TVec2f> UVs[]);
    void computeFlatNormals(const TArray<uint32_t>& Indices, TArray<FStaticMeshBuildVertex>& Vertices);
    UTexture2D* LoadTextureFromPath(const FString& Path);
};
