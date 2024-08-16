// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUMeshLoaderForLandscape.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForLandscapeMesh : public FPLATEAUMeshLoaderForLandscape {

public:
    FPLATEAUMeshLoaderForLandscapeMesh();
    FPLATEAUMeshLoaderForLandscapeMesh(const bool InbAutomationTest);

    void CreateMeshFromHeightMap(AActor& Actor, const int32 SizeX, const int32 SizeY, 
        const TVec3d Min, const TVec3d Max, 
        const TVec2f MinUV, const TVec2f MaxUV, 
        uint16_t* HeightRawData, 
        const FString NodeName);

protected:
    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, FNodeHierarchy NodeHier,
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
        const std::shared_ptr <const citygml::CityModel> CityModel) override;
    UMaterialInterface* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FNodeHierarchy NodeHier) override;

    bool OverwriteTexture() override;
    bool InvertMeshNormal() override;
    bool MergeTriangles() override;
    void ModifyMeshDescription(FMeshDescription& MeshDescription) override;

private:
    UMaterialInterface* ReplaceMaterial;

};

