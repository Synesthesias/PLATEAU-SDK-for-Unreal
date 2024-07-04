// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <Reconstruct/PLATEAUMeshLoaderCloneComponent.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

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

plateau::heightMapAligner::HeightMapFrame FPLATEAUModelAlignLand::CreateAlignData(const TSharedPtr<std::vector<uint16_t>> HeightData, 
    const TVec3d Min, const TVec3d Max, 
    const FString NodeName, FPLATEAULandscapeParam Param) {

    plateau::heightMapAligner::HeightMapFrame frame(*HeightData.Get(),
        (int)Param.TextureWidth, (int)Param.TextureHeight,
        (float)Min.x, (float)Max.x, (float)Min.y, (float)Max.y, (float)Min.z, (float)Max.z, plateau::geometry::CoordinateSystem::ESU);

    return frame;
}

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::SetAlignData(const TArray<plateau::heightMapAligner::HeightMapFrame> Frames, FPLATEAULandscapeParam Param) {

    plateau::heightMapAligner::HeightMapAligner heightmapAligner(HeightOffset, plateau::geometry::CoordinateSystem::ESU);

    for (const auto& Frame : Frames) {
        heightmapAligner.addHeightmapFrame(Frame);
    }

    TSet<USceneComponent*> AlignComponents;
    for (const auto Pkg : IncludePacakges) {
        auto Comps = CityModelActor->GetComponentsByPackage(Pkg);
        for (const auto Comp : Comps) {
            if (Comp->IsA<USceneComponent>())
                AlignComponents.Add((USceneComponent*)Comp);
        }
    }

    auto TargetCityObjects = GetUPLATEAUCityObjectGroupsFromSceneComponents(AlignComponents.Array());

    // LOD3の道路は、道路の高さの正確性を尊重するため、土地のほうを道路に合わせます。
    /*   
    TArray<UPLATEAUCityObjectGroup*> InvertedTargetCityObjects;
    if (Param.InvertRoadLod3) {
        InvertedTargetCityObjects = FilterLod3RoadComponents(CityModelActor, TargetCityObjects);
        for (auto Comp : TargetCityObjects) {
            if (InvertedTargetCityObjects.Contains(Comp)) {
                TargetCityObjects.Remove(Comp);
            }
        }
    }
    */

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU; 
    FPLATEAUMeshExporter MeshExporter;
    std::shared_ptr<plateau::polygonMesh::Model> Model = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    // 高さ合わせをします。
    heightmapAligner.align(*Model, MaxEdgeLength);

    // 元コンポーネントを覚えておきます。
    const auto& ComponentsMap = FPLATEAUMeshLoaderCloneComponent::CreateComponentsMap(TargetCityObjects);

    FPLATEAUMeshLoaderCloneComponent MeshLoader(false);
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(Model->getRootNodeAt(i), ComponentsMap, *CityModelActor);
    }

    // LOD3道路の処理
    /*
    if (InvertedTargetCityObjects.Num() > 0) {
        std::shared_ptr<plateau::polygonMesh::Model> InvModel = MeshExporter.CreateModelFromComponents(CityModelActor, InvertedTargetCityObjects, ExtOptions);
        heightmapAligner.alignInvert(*InvModel, AlphaExpandWidthCartesian, AlphaAveragingWidthCartesian, InvertedHeightOffset, SkipThresholdOfMapLandDistance);

        // TODO: Landscapeのハイトマップを再生成(別枠で処理）
    }    
    TargetCityObjects.Append(InvertedTargetCityObjects);
    */

    return TargetCityObjects;
}
