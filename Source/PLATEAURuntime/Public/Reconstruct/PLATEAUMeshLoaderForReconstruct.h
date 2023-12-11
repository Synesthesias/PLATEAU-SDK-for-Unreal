// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForReconstruct : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForReconstruct() {
        bAutomationTest = false;
    }

    FPLATEAUMeshLoaderForReconstruct(const bool InbAutomationTest) {
        bAutomationTest = InbAutomationTest;
    }

    void ReloadComponentFromNode(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        TMap<FString, FPLATEAUCityObject> cityObjMap,
        AActor& InActor);

protected:

    void ReloadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& InActor);
    USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor);

private:

};
