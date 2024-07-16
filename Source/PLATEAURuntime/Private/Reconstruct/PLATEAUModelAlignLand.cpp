// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <Reconstruct/PLATEAUMeshLoaderCloneComponent.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>
#include <plateau/height_map_generator/heightmap_generator.h>

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand() {}

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::FilterLod3RoadComponents(APLATEAUInstancedCityModel* Actor, TArray<UPLATEAUCityObjectGroup*> TargetComponents) {
    TArray<UPLATEAUCityObjectGroup*> FilterdList;
        for (auto Comp : TargetComponents) {
        auto PackageComps = Actor->GetComponentsByPackage(EPLATEAUCityModelPackage::Road);
        for (auto PComp : PackageComps) {
            TArray<USceneComponent*> Children;
            ((USceneComponent*)PComp)->GetChildrenComponents(true, Children);
            if (Children.Contains(Comp)) {
                auto Parent = Comp->GetAttachParent();
                int Lod = APLATEAUInstancedCityModel::ParseLodComponent(Parent);
                if (Lod >= 3) {
                    FilterdList.Add(Comp);
                }
            }
        }
    }
    return FilterdList;
}

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::GetTargetCityObjectsForAlignLand() {
    TSet<USceneComponent*> AlignComponents;
    for (const auto Pkg : IncludePacakges) {
        auto Comps = CityModelActor->GetComponentsByPackage(Pkg);
        for (const auto Comp : Comps) {
            if (Comp->IsA<USceneComponent>())
                AlignComponents.Add((USceneComponent*)Comp);
        }
    }
    return GetUPLATEAUCityObjectGroupsFromSceneComponents(AlignComponents.Array());
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelAlignLand::CreateModelFromTargets(TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;
    FPLATEAUMeshExporter MeshExporter;
    return MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);
}

plateau::heightMapAligner::HeightMapFrame FPLATEAUModelAlignLand::CreateAlignData(const TSharedPtr<std::vector<uint16_t>> HeightData, 
    const TVec3d Min, const TVec3d Max, 
    const FString NodeName, FPLATEAULandscapeParam Param) {

    plateau::heightMapAligner::HeightMapFrame frame(*HeightData.Get(),
        (int)Param.TextureWidth, (int)Param.TextureHeight,
        (float)Min.x, (float)Max.x, (float)Min.y, (float)Max.y, (float)Min.z, (float)Max.z, plateau::geometry::CoordinateSystem::ESU);

    return frame;
}

TArray<USceneComponent*> FPLATEAUModelAlignLand::SetAlignData(const TArray<plateau::heightMapAligner::HeightMapFrame> Frames, TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, FPLATEAULandscapeParam Param) {

    plateau::heightMapAligner::HeightMapAligner heightmapAligner(HeightOffset, plateau::geometry::CoordinateSystem::ESU);

    for (const auto& Frame : Frames) {
        heightmapAligner.addHeightmapFrame(Frame);
    }
    std::shared_ptr<plateau::polygonMesh::Model> Model = CreateModelFromTargets(TargetCityObjects);

    // 高さ合わせをします。
    heightmapAligner.align(*Model, MaxEdgeLength);

    // 元コンポーネントを覚えておきます。
    const auto& ComponentsMap = FPLATEAUMeshLoaderCloneComponent::CreateComponentsMap(TargetCityObjects);

    FPLATEAUMeshLoaderCloneComponent MeshLoader(false);
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(Model->getRootNodeAt(i), ComponentsMap, *CityModelActor);
    }
    return MeshLoader.GetLastCreatedComponents();
}

void FPLATEAUModelAlignLand::UpdateHeightMapForLod3Road(TArray<HeightmapCreationResult>& Results, TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects, FPLATEAULandscapeParam Param) {
    
    TArray<UPLATEAUCityObjectGroup*> InvertedTargetCityObjects;
    InvertedTargetCityObjects = FilterLod3RoadComponents(CityModelActor, TargetCityObjects);
    if (InvertedTargetCityObjects.Num() <= 0)
        return;

    // LOD3の道路は、TargetCityObjectsから除外
    for (auto Comp : TargetCityObjects) {
        if (InvertedTargetCityObjects.Contains(Comp)) {
            TargetCityObjects.Remove(Comp);
        }
    }
    std::shared_ptr<plateau::polygonMesh::Model> Model = CreateModelFromTargets(InvertedTargetCityObjects);
    // ResultsのHeightmap書き換え
    for (auto& Result : Results) {
            plateau::heightMapAligner::HeightMapAligner heightmapAligner(HeightOffset, plateau::geometry::CoordinateSystem::ESU);
            auto Frame = CreateAlignData(Result.Data, Result.Min, Result.Max, Result.NodeName, Param);
            heightmapAligner.addHeightmapFrame(Frame);

            heightmapAligner.alignInvert(*Model, AlphaExpandWidthCartesian, AlphaAveragingWidthCartesian, InvertedHeightOffset, SkipThresholdOfMapLandDistance);
            auto HMFrame = heightmapAligner.getHeightMapFrameAt(0);
            Result.Data = MakeShared<std::vector<uint16_t>>(HMFrame.heightmap);
            Result.Min = TVec3d(HMFrame.min_x, HMFrame.min_y, HMFrame.min_height);
            Result.Max = TVec3d(HMFrame.max_x, HMFrame.max_y, HMFrame.max_height);
    }
}
