// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Reconstruct/PLATEAUMeshLoaderForLandscapeMesh.h"
#include "PLATEAUCityModelLoader.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include <plateau/height_map_generator/heightmap_generator.h>

#include "MeshDescription.h"
#include "StaticMeshOperations.h"
#include "StaticMeshAttributes.h"

#include "heightmap_mesh_generator.h"



FPLATEAUMeshLoaderForLandscapeMesh::FPLATEAUMeshLoaderForLandscapeMesh() {}

FPLATEAUMeshLoaderForLandscapeMesh::FPLATEAUMeshLoaderForLandscapeMesh(const bool InbAutomationTest){
    bAutomationTest = InbAutomationTest;
}

void FPLATEAUMeshLoaderForLandscapeMesh::CreateMeshFromHeightMap(AActor& Actor, const int32 SizeX, const int32 SizeY, 
    const TVec3d Min, const TVec3d Max, 
    const TVec2f MinUV, const TVec2f MaxUV, 
    uint16_t* HeightRawData, const FString NodeName) {
    double ActualHeight = abs(Max.z - Min.z);
    float HeightScale = ActualHeight;
    plateau::heightMapMeshGenerator::HeightmapMeshGenerator gen;
    auto mesh = gen.generateMeshFromHeightmap(SizeX, SizeY, HeightScale, HeightRawData,
        plateau::geometry::CoordinateSystem::ESU, Min, Max, MinUV, MaxUV);

    auto ParentComponent = Actor.GetRootComponent();
    const auto BaseComponents = FindComponentsByName(&Actor, NodeName);
    if (BaseComponents.Num() > 0) {
        const auto BaseComponent = BaseComponents[0];
        if (BaseComponent->IsA<UStaticMeshComponent>()) {

            const auto BaseStaticMeshComponent = (UStaticMeshComponent*)BaseComponent;
            if (BaseStaticMeshComponent->GetMaterial(0)->IsA<UMaterialInstanceDynamic>())
                ReplaceMaterial = (UMaterialInstanceDynamic*)BaseStaticMeshComponent->GetMaterial(0);
        }

        TArray<USceneComponent*> Parents;
        BaseComponent->GetParentComponents(Parents);
        if (Parents.Num() > 0)
            ParentComponent = Parents[0];
    }

    plateau::polygonMesh::MeshExtractOptions MeshExtractOptions{};
    FLoadInputData LoadInputData
    {
        MeshExtractOptions,
        std::vector<plateau::geometry::Extent>{},
        FString(),
        false,
        nullptr
    };

    FString ComponentName = FString::Format(*FString(TEXT("Mesh_{0}")), { NodeName });
    UStaticMeshComponent* Component = CreateStaticMeshComponent(Actor, *ParentComponent, mesh, LoadInputData, nullptr, TCHAR_TO_UTF8(*ComponentName));

    Component->Mobility = EComponentMobility::Movable;
    Actor.AddInstanceComponent(Component);
    Component->RegisterComponent();
    Component->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepWorldTransform);

    // メッシュをワールド内にビルド
    const auto CopiedStaticMeshes = StaticMeshes;

    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [CopiedStaticMeshes]() {
            UStaticMesh::BatchBuild(CopiedStaticMeshes, true, [](UStaticMesh* mesh) {
                return true;
                });
        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();
        StaticMeshes.Reset();
}

bool FPLATEAUMeshLoaderForLandscapeMesh::OverwriteTexture() {
    return false;
}

bool FPLATEAUMeshLoaderForLandscapeMesh::InvertMeshNormal() {
    return false;
}

bool FPLATEAUMeshLoaderForLandscapeMesh::MergeTriangles() {
    return true;
}

UStaticMeshComponent* FPLATEAUMeshLoaderForLandscapeMesh::GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
    const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
    const std::shared_ptr <const citygml::CityModel> CityModel) {

    const FString NodeName = UTF8_TO_TCHAR(InNodeName.c_str());
    const auto& PLATEAUCityObjectGroup = NewObject<UPLATEAUCityObjectGroup>(&Actor, NAME_None);

    // Originalコンポーネントの属性をそのまま利用
    const FString ReplacedName = NodeName.Replace(*FString("Mesh_"), *FString());
    const auto& OriginalComponent = GetOriginalComponent(&Actor, ReplacedName);
    if (OriginalComponent) {
        PLATEAUCityObjectGroup->SerializedCityObjects = OriginalComponent->SerializedCityObjects;
        PLATEAUCityObjectGroup->OutsideChildren = OriginalComponent->OutsideChildren;
        PLATEAUCityObjectGroup->OutsideParent = OriginalComponent->OutsideParent;
        PLATEAUCityObjectGroup->MeshGranularityIntValue = OriginalComponent->MeshGranularityIntValue;
    }
    return PLATEAUCityObjectGroup;
}

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForLandscapeMesh::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) {

    if (ReplaceMaterial)
        return ReplaceMaterial;
    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture, NodeName);
}

void FPLATEAUMeshLoaderForLandscapeMesh::ModifyMeshDescription(FMeshDescription& MeshDescription) {

    FStaticMeshOperations::DetermineEdgeHardnessesFromVertexInstanceNormals(MeshDescription);

    TEdgeAttributesRef<bool> EdgeHardness =
        MeshDescription.EdgeAttributes().GetAttributesRef<bool>(MeshAttribute::Edge::IsHard);
    for (FEdgeID EdgeID : MeshDescription.Edges().GetElementIDs()) {
        EdgeHardness.Set(EdgeID, 0, false);
    }

    FStaticMeshOperations::ComputeTriangleTangentsAndNormals(MeshDescription, FMathf::Epsilon);
    FStaticMeshOperations::RecomputeNormalsAndTangentsIfNeeded(MeshDescription, EComputeNTBsFlags::WeightedNTBs | EComputeNTBsFlags::Normals | EComputeNTBsFlags::Tangents | EComputeNTBsFlags::BlendOverlappingNormals);
}
