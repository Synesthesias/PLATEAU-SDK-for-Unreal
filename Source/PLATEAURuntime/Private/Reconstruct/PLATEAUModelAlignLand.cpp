// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <Reconstruct/PLATEAUMeshLoaderCloneComponent.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>
#include <plateau/height_map_generator/heightmap_generator.h>

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand():heightmapAligner(HeightOffset, plateau::geometry::CoordinateSystem::ESU) {}
FPLATEAUModelAlignLand::FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor) :heightmapAligner(HeightOffset, plateau::geometry::CoordinateSystem::ESU) {
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

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelAlignLand::CreateModelFromTargets(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
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
    const FString NodeName, const FPLATEAULandscapeParam Param) {

    plateau::heightMapAligner::HeightMapFrame frame(*HeightData.Get(),
        (int)Param.TextureWidth, (int)Param.TextureHeight,
        (float)Min.x, (float)Max.x, (float)Min.y, (float)Max.y, (float)Min.z, (float)Max.z, plateau::geometry::CoordinateSystem::ESU);
    return frame;
}

void FPLATEAUModelAlignLand::SetResults(const TArray<HeightmapCreationResult> Results, const FPLATEAULandscapeParam Param) {
    HeightmapCreationResults = Results;
    LandscapeParam = Param;
    for (auto& Result : Results) {
        auto Frame = CreateAlignData(Result.Data, Result.Min, Result.Max, Result.NodeName, Param);
        heightmapAligner.addHeightmapFrame(Frame);
    }
}

TArray<USceneComponent*> FPLATEAUModelAlignLand::Align(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    std::shared_ptr<plateau::polygonMesh::Model> Model = CreateModelFromTargets(TargetCityObjects);

    // 高さ合わせをします。
    heightmapAligner.align(*Model, MaxEdgeLength);

    // 元コンポーネントを覚えておきます。
    const auto& ComponentsMap = FPLATEAUMeshLoaderCloneComponent::CreateComponentsMap(TargetCityObjects);

    FPLATEAUMeshLoaderCloneComponent MeshLoader(false);
    MeshLoader.SetSmoothing(true);
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(Model->getRootNodeAt(i), ComponentsMap, *CityModelActor);
    }
    return MeshLoader.GetLastCreatedComponents();
}

TArray<HeightmapCreationResult> FPLATEAUModelAlignLand::UpdateHeightMapForLod3Road(TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) {

    TArray<UPLATEAUCityObjectGroup*> InvertedTargetCityObjects;
    InvertedTargetCityObjects = FilterLod3RoadComponents(CityModelActor, TargetCityObjects);
    if (InvertedTargetCityObjects.Num() <= 0)
        return HeightmapCreationResults;

    // LOD3の道路は、TargetCityObjectsから除外
    TArray<UPLATEAUCityObjectGroup*> NewTargetCityObjects;
    for (auto& Comp : TargetCityObjects) {
        if (!InvertedTargetCityObjects.Contains(Comp)) 
            NewTargetCityObjects.Add(Comp);
    }
    TargetCityObjects = NewTargetCityObjects;

    std::shared_ptr<plateau::polygonMesh::Model> Model = CreateModelFromTargets(InvertedTargetCityObjects);
    heightmapAligner.alignInvert(*Model, AlphaExpandWidthCartesian, AlphaAveragingWidthCartesian, InvertedHeightOffset, SkipThresholdOfMapLandDistance);
    TArray<HeightmapCreationResult> NewResults;

    check(heightmapAligner.heightmapCount() == HeightmapCreationResults.Num())

    // ResultsのHeightmap書き換え(ResultをSetした順番でindex取得)
    int32 index = 0;
    for (auto& Result : HeightmapCreationResults) {   
        HeightmapCreationResult NewResult = Result;
        auto HMFrame = heightmapAligner.getHeightMapFrameAt(index++);
        NewResult.Data = MakeShared<std::vector<uint16_t>>(HMFrame.heightmap);
        NewResult.Min = TVec3d(HMFrame.min_x, HMFrame.min_y, HMFrame.min_height);
        NewResult.Max = TVec3d(HMFrame.max_x, HMFrame.max_y, HMFrame.max_height);
        NewResults.Add(NewResult);

        // Heightmap Image Output 
        FPLATEAUMeshLoaderForLandscape::SaveHeightmapImage(LandscapeParam.HeightmapImageOutput, 
            "HM_ALN_" + NewResult.NodeName , 
            LandscapeParam.TextureWidth, LandscapeParam.TextureHeight, NewResult.Data->data());
    }
    HeightmapCreationResults = NewResults;
    return HeightmapCreationResults;
}
