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

void FPLATEAUMeshLoaderForLandscapeMesh::LoadNodeRecursiveForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results) {
    LoadNodeForHeightMap(InNode, InActor, Param, Results);
    const size_t ChildNodeCount = InNode.getChildCount();
    for (int i = 0; i < ChildNodeCount; i++) {
        const auto& TargetNode = InNode.getChildAt(i);
        LoadNodeRecursiveForHeightMap( TargetNode, InActor, Param, Results);
    }
}

void FPLATEAUMeshLoaderForLandscapeMesh::LoadNodeForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results) {
    if (InNode.getMesh() == nullptr || InNode.getMesh()->getVertices().size() == 0) {
        const FString DesiredName = FString(UTF8_TO_TCHAR(InNode.getName().c_str()));      
    }
    else {
        auto Result = CreateHeightMapFromMesh(*InNode.getMesh(), FString(UTF8_TO_TCHAR(InNode.getName().c_str())), InActor, Param);
        Results.Add(Result);
    }
}

HeightmapCreationResult FPLATEAUMeshLoaderForLandscapeMesh::CreateHeightMapFromMesh(
    const plateau::polygonMesh::Mesh& InMesh, const FString NodeName, AActor& Actor, FPLATEAULandscapeParam Param) {

    plateau::heightMapGenerator::HeightmapGenerator generator;
    TVec3d ExtMin, ExtMax;
    TVec2f UVMin, UVMax;
    TVec2d Offset(Param.Offset.X, Param.Offset.Y);
    std::vector<uint16_t> heightMapData = generator.generateFromMesh(InMesh, Param.TextureWidth, Param.TextureHeight, Offset, 
        plateau::geometry::CoordinateSystem::ESU, Param.FillEdges, Param.ApplyBlurFilter, ExtMin, ExtMax, UVMin, UVMax);
    
    // Heightmap Image Output 
    SaveHeightmapImage(Param.HeightmapImageOutput, "HM_" + NodeName , Param.TextureWidth, Param.TextureHeight, heightMapData.data());

    //Texture
    FString TexturePath;
    const auto& subMeshes = InMesh.getSubMeshes();
    if (subMeshes.size() > 0) {
        const auto& subMesh = subMeshes.at(0);
        TexturePath = FString(subMesh.getTexturePath().c_str());
    }

    TSharedPtr<std::vector<uint16_t>> sharedData = MakeShared<std::vector<uint16_t>>(heightMapData);
    HeightmapCreationResult Result{ NodeName, sharedData ,ExtMin, ExtMax , UVMin, UVMax, TexturePath };
    return Result;
}

void FPLATEAUMeshLoaderForLandscapeMesh::CreateMeshFromHeightMap(AActor& Actor, const int32 SizeX, const int32 SizeY, const TVec3d Min, const TVec3d Max, const TVec2f MinUV, const TVec2f MaxUV, uint16_t* HeightRawData, const FString NodeName) {
    double ActualHeight = abs(Max.z - Min.z);
    float HeightScale = ActualHeight;
    plateau::heightMapMeshGenerator::HeightmapMeshGenerator gen;
    auto mesh = gen.generateMeshFromHeightmap(SizeX, SizeY, HeightScale, HeightRawData,
        plateau::geometry::CoordinateSystem::ESU, Min, Max, MinUV, MaxUV);

    UE_LOG(LogTemp, Error, TEXT("Created Mesh : %d %d"), mesh.getVertices().size(), mesh.getIndices().size());

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
    const auto& OriginalComponent = GetOriginalComponent(Actor, NodeName);
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

TArray<USceneComponent*> FPLATEAUMeshLoaderForLandscapeMesh::FindComponentsByName(AActor* ModelActor, FString Name) {

    UE_LOG(LogTemp, Warning, TEXT("FindComponentsByName: %s"), *Name);

    const FString ReplacedName = Name.Replace(*FString("Mesh_"), *FString());

    UE_LOG(LogTemp, Warning, TEXT("FindComponentsByName NEW : %s"), *ReplacedName);

    const FRegexPattern pattern = FRegexPattern(FString::Format(*FString(TEXT("^{0}__([0-9]+)")), { ReplacedName }));
    TArray<USceneComponent*> Result;
    const auto Components = ModelActor->GetComponents();
    for (auto Component : Components) {
        if (Component->IsA<USceneComponent>()) {
            FRegexMatcher matcher(pattern, Component->GetName());
            if (matcher.FindNext()) {
                Result.Add((USceneComponent*)Component);
                UE_LOG(LogTemp, Warning, TEXT("Found Component Name: %s"), *Component->GetName());
            }
        }
    }
    return Result;
}

UPLATEAUCityObjectGroup* FPLATEAUMeshLoaderForLandscapeMesh::GetOriginalComponent(AActor& Actor, FString Name) {

    UE_LOG(LogTemp, Warning, TEXT("GetOriginalComponent: %s"), *Name);

    const auto BaseComponents = FindComponentsByName(&Actor, Name);
    if (BaseComponents.Num() > 0) {
        
        UPLATEAUCityObjectGroup* FoundItem;
        int32 ItemIndex;
        if (BaseComponents.FindItemByClass<UPLATEAUCityObjectGroup>(&FoundItem, &ItemIndex)) {
            UE_LOG(LogTemp, Warning, TEXT("GetOriginalComponent: Found %s "), *Name);
            return FoundItem;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("GetOriginalComponent: Not Found %s "), *Name);
    return nullptr;
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
