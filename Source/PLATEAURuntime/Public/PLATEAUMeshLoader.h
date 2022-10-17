#pragma once

#include "CoreMinimal.h"

#include "CityGML/PLATEAUCityModel.h"
#include "PLATEAUGeometry.h"
#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "Engine/Texture2D.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoader {
public:
    void CreateMesh(AActor* ModelActor, std::shared_ptr<plateau::polygonMesh::Model> ModelData);

private:
    TUniquePtr<FStaticMeshRenderData> CreateRenderData(std::vector<int> InIndicesVector, TArray<FVector> VerticesArray, std::vector<TVec2f> UVs[]);
    void LoadNodes_InModel(USceneComponent* ParentComponent, plateau::polygonMesh::Node* Node, AActor& Actor, int Index, int Count);
    void SetRenderData(UStaticMesh* StaticMesh, TUniquePtr<FStaticMeshRenderData>& RenderData);
    UStaticMeshComponent* CreateStaticMeshComponent(AActor& Actor, USceneComponent& ParentComponent, std::vector<int> Indices,
        TArray<FVector> Vertices, FString Name, UTexture2D* Texture, std::vector<TVec2f> UVs[]);
    void computeFlatNormals(const TArray<uint32_t>& Indices, TArray<FStaticMeshBuildVertex>& Vertices);
    UTexture2D* LoadTextureFromPath(const FString& Path);


};