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

TMap<FString, UPLATEAUCityObjectGroup*> FPLATEAUModelAlignLand::CreateComponentsMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    TMap<FString, UPLATEAUCityObjectGroup*> Map;
    for (auto Comp : TargetCityObjects) {
        Map.Add(APLATEAUInstancedCityModel::GetOriginalComponentName(Comp), Comp);
    }
    return Map;
}

void FPLATEAUModelAlignLand::SetHeightData(const std::vector<uint16_t> HeightData, 
    const TVec3d Min, const TVec3d Max, 
    const FString NodeName, FPLATEAULandscapeParam Param) {

    UE_LOG(LogTemp, Error, TEXT("AlignLand :: SetHeightData"));

    const float HeightOffset = 0.3f;


    UE_LOG(LogTemp, Error, TEXT("TextureWidth %d TextureHeight %d Min x %f Max x %f Min z %f Max z %f Min y %f Max y %f "), Param.TextureWidth, Param.TextureHeight, Min.x, Max.x, Min.z, Max.z, Min.y, Max.y);


    plateau::heightMapAligner::HeightMapFrame frame(HeightData, 
        (int)Param.TextureWidth, (int)Param.TextureHeight,
        (float)Min.x, (float)Max.x,(float)Min.z, (float)Max.z, (float)Min.y, (float)Max.y);

    UE_LOG(LogTemp, Error, TEXT("Frame TextureWidth %d TextureHeight %d Min x %f Max x %f Min y %f Max y %f Min H %f Max H %f "), frame.map_width, frame.map_height, frame.min_x, frame.max_x, frame.min_y, frame.max_y, frame.min_height, frame.max_height);


    plateau::heightMapAligner::HeightMapAligner heightmapAligner(HeightOffset);
    heightmapAligner.addHeightmapFrame(frame);

    TSet<UActorComponent*> BaseAlignComponents;
    for (const auto Pkg : IncludePacakges) {
        auto Comps = CityModelActor->GetComponentsByPackage(Pkg);
        BaseAlignComponents.Append(Comps);
    }

    TArray<USceneComponent*> AlignComponents;
    for (const auto Comp : BaseAlignComponents) {
        if (Comp->IsA<USceneComponent>())
            AlignComponents.Add((USceneComponent*)Comp);
    }

    const auto& TargetCityObjects = GetUPLATEAUCityObjectGroupsFromSceneComponents(AlignComponents);

    UE_LOG(LogTemp, Error, TEXT("AlignComponents %d %d"), AlignComponents.Num(), TargetCityObjects.Num());

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
            ParentComponent = OriginalComponent->GetAttachParent();
        }
        MeshLoader.ReloadComponentFromNode(ParentComponent, smodel->getRootNodeAt(i), MeshGranularity, CityObjMap, *CityModelActor);
    }



}