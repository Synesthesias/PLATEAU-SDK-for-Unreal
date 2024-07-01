// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelAlignLand.h>
#include <Reconstruct/PLATEAUMeshLoaderForAlignLand.h>
#include <plateau/height_map_alighner/height_map_aligner.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand() {}

FPLATEAUModelAlignLand::FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

TMap<FString, plateau::polygonMesh::MeshGranularity> FPLATEAUModelAlignLand::CreateMeshGranularityMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    return TMap<FString, plateau::polygonMesh::MeshGranularity>();
}

void FPLATEAUModelAlignLand::SetHeightData(const std::vector<uint16_t> HeightData, 
    const TVec3d Min, const TVec3d Max, 
    const FString NodeName, FPLATEAULandscapeParam Param) {

    const float HeightOffset = 0.3f;

    plateau::heightMapAligner::HeightMapFrame frame(HeightData, Param.TextureWidth, Param.TextureHeight, (float)Min.x, (float)Max.x, (float)Min.y, (float)Max.y, (float)Min.z, (float)Max.z);
    plateau::heightMapAligner::HeightMapAligner heightmapAligner(HeightOffset);
    heightmapAligner.addHeightmapFrame(frame);

    TSet<EPLATEAUCityModelPackage> IncludePacakges{ EPLATEAUCityModelPackage::Area,
    EPLATEAUCityModelPackage::Road,
    EPLATEAUCityModelPackage::Square,
    EPLATEAUCityModelPackage::Track,
    EPLATEAUCityModelPackage::Waterway,
    EPLATEAUCityModelPackage::DisasterRisk,
    EPLATEAUCityModelPackage::LandUse,
    EPLATEAUCityModelPackage::WaterBody,
    EPLATEAUCityModelPackage::UrbanPlanningDecision,
    };

    TSet<UActorComponent*> BaseAlignComponents;
    for (const auto Pkg : IncludePacakges) {
        auto Comps = CityModelActor->GetComponentsByPackage(EPLATEAUCityModelPackage::Area);
        BaseAlignComponents.Append(Comps);
    }

    TArray<USceneComponent*> AlignComponents;
    for (const auto Comp : BaseAlignComponents) {
        if (Comp->IsA<USceneComponent>())
            AlignComponents.Add((USceneComponent*)Comp);
    }

    const auto& TargetCityObjects = GetUPLATEAUCityObjectGroupsFromSceneComponents(AlignComponents);

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;
    FPLATEAUMeshExporter MeshExporter;
    std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    // 高さ合わせをします。
    heightmapAligner.align(*smodel);

    //属性情報を覚えておきます。
    CityObjMap = CreateMapFromCityObjectGroups(TargetCityObjects);

    // TODO: MeshGranularity取得
    MeshGranularityMap = CreateMeshGranularityMap(TargetCityObjects);

    FPLATEAUMeshLoaderForAlignLand MeshLoader(false);
    for (int i = 0; i < smodel->getRootNodeCount(); i++) {
        MeshGranularity = MeshGranularityMap[FString(smodel->getRootNodeAt(i).getName().c_str())];
        MeshLoader.ReloadComponentFromNode(CityModelActor->GetRootComponent(), smodel->getRootNodeAt(i), MeshGranularity, CityObjMap, *CityModelActor);
    }

}