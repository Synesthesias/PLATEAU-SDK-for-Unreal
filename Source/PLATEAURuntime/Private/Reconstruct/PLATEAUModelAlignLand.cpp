// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <Reconstruct/PLATEAUMeshLoaderForAlignLand.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand() {}

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

TMap<FString, UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::CreateComponentsMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    TMap<FString, UPLATEAUCityObjectGroup*> Map;
    for (auto Comp : TargetCityObjects) {
        Map.Add(APLATEAUInstancedCityModel::GetOriginalComponentName(Comp), Comp);
    }
    return Map;
}

plateau::heightMapAligner::HeightMapFrame FPLATEAUModelAlignLand::CreateAlignData(const TSharedPtr<std::vector<uint16_t>> HeightData, 
    const TVec3d Min, const TVec3d Max, 
    const FString NodeName, FPLATEAULandscapeParam Param) {

    UE_LOG(LogTemp, Error, TEXT("HeightData %d TextureWidth %d TextureHeight %d Min x %f Max x %f Min z %f Max z %f Min y %f Max y %f "), HeightData->size(), Param.TextureWidth, Param.TextureHeight, Min.x, Max.x, Min.y, Max.y, Min.z, Max.z);

    const auto ENUMin = plateau::geometry::GeoReference::convertAxisToENU(plateau::geometry::CoordinateSystem::ESU, Min);
    const auto ENUMax = plateau::geometry::GeoReference::convertAxisToENU(plateau::geometry::CoordinateSystem::ESU, Max);

    plateau::heightMapAligner::HeightMapFrame frame(*HeightData.Get(),
        (int)Param.TextureWidth, (int)Param.TextureHeight,
        //(float)ENUMin.x, (float)ENUMax.x, (float)ENUMin.y, (float)ENUMax.y, (float)ENUMin.z, (float)ENUMax.z, plateau::geometry::CoordinateSystem::ESU);
        (float)Min.x, (float)Max.x, (float)Min.y, (float)Max.y, (float)Min.z, (float)Max.z, plateau::geometry::CoordinateSystem::ESU);

    UE_LOG(LogTemp, Error, TEXT("Frame HeightData %d TextureWidth %d TextureHeight %d Min x %f Max x %f Min y %f Max y %f Min H %f Max H %f "), frame.heightmap.size(), frame.map_width, frame.map_height, frame.min_x, frame.max_x, frame.min_y, frame.max_y, frame.min_height, frame.max_height);

    return frame;
}

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::SetAlignData(const TArray<plateau::heightMapAligner::HeightMapFrame> Frames) {

    const float HeightOffset = 0.3f;
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

    const auto& TargetCityObjects = GetUPLATEAUCityObjectGroupsFromSceneComponents(AlignComponents.Array());

    UE_LOG(LogTemp, Error, TEXT("AlignComponents %d %d"), AlignComponents.Num(), TargetCityObjects.Num());

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU; 
    FPLATEAUMeshExporter MeshExporter;
    std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    // 高さ合わせをします。
    heightmapAligner.align(*smodel, 400.f );

    //属性情報を覚えておきます。
    CityObjMap = CreateMapFromCityObjectGroups(TargetCityObjects);

    // 元コンポーネントを覚えておきます。
    ComponentsMap = CreateComponentsMap(TargetCityObjects);

    FPLATEAUMeshLoaderForAlignLand MeshLoader(false);
    for (int i = 0; i < smodel->getRootNodeCount(); i++) {

        //元コンポーネントのParent,MeshGranularity取得 
        USceneComponent* ParentComponent = CityModelActor->GetRootComponent();
        MeshGranularity = plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
        const auto OriginalComponentPtr = ComponentsMap.Find(FString(smodel->getRootNodeAt(i).getName().c_str()));
        if (OriginalComponentPtr) {
            const auto OriginalComponent = *OriginalComponentPtr;
            MeshGranularity = StaticCast<plateau::polygonMesh::MeshGranularity>(OriginalComponent->MeshGranularityIntValue);
        }
        MeshLoader.ReloadComponentFromNode( smodel->getRootNodeAt(i), MeshGranularity, ComponentsMap, *CityModelActor);
    }

    return TargetCityObjects;
}
