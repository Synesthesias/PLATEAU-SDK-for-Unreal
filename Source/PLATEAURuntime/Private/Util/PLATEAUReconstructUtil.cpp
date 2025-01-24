// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Util/PLATEAUReconstructUtil.h"
#include "Util/PLATEAUComponentUtil.h"

TMap<FString, FPLATEAUCityObject> FPLATEAUReconstructUtil::CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjectGroups) {
    TMap<FString, FPLATEAUCityObject> OutCityObjMap;
    for (auto Comp : TargetCityObjectGroups) {

        if (Comp->SerializedCityObjects.IsEmpty())
            continue;

        for (auto CityObj : Comp->GetAllRootCityObjects()) {
            if (!Comp->OutsideParent.IsEmpty() && !OutCityObjMap.Contains(Comp->OutsideParent)) {
                // 親を探す
                TArray<USceneComponent*> Parents;
                Comp->GetParentComponents(Parents);
                for (const auto& Parent : Parents) {
                    if (Parent->GetName().Contains(Comp->OutsideParent)) {
                        for (auto Pobj : Cast<UPLATEAUCityObjectGroup>(Parent)->GetAllRootCityObjects()) {
                            OutCityObjMap.Add(Pobj.GmlID, Pobj);
                        }
                        break;
                    }
                }
            }

            OutCityObjMap.Add(CityObj.GmlID, CityObj);
            for (auto Child : CityObj.Children) {
                OutCityObjMap.Add(Child.GmlID, Child);
            }
        }
    }
    return OutCityObjMap;
}

void FPLATEAUReconstructUtil::SaveHeightmapImage(EPLATEAULandscapeHeightmapImageOutput OutputParam, FString FileName, int32 Width, int32 Height, uint16_t* Data) {
    // Heightmap Image Output 
    if (OutputParam == EPLATEAULandscapeHeightmapImageOutput::PNG || OutputParam == EPLATEAULandscapeHeightmapImageOutput::PNG_RAW) {
        FString PngSavePath = FString::Format(*FString(TEXT("{0}PLATEAU/{1}_{2}_{3}.png")), { FPaths::ProjectContentDir(), FileName, Width, Height });
        plateau::heightMapGenerator::HeightmapGenerator::savePngFile(TCHAR_TO_ANSI(*PngSavePath), Width, Height, Data);
        UE_LOG(LogTemp, Log, TEXT("height map png saved: %s"), *PngSavePath);
    }
    if (OutputParam == EPLATEAULandscapeHeightmapImageOutput::RAW || OutputParam == EPLATEAULandscapeHeightmapImageOutput::PNG_RAW) {
        FString RawSavePath = FString::Format(*FString(TEXT("{0}PLATEAU/{1}_{2}_{3}.raw")), { FPaths::ProjectContentDir(), FileName, Width, Height });
        plateau::heightMapGenerator::HeightmapGenerator::saveRawFile(TCHAR_TO_ANSI(*RawSavePath), Width, Height, Data);
        UE_LOG(LogTemp, Log, TEXT("height map raw saved: %s"), *RawSavePath);
    }
}

plateau::polygonMesh::MeshGranularity FPLATEAUReconstructUtil::ConvertGranularityToMeshGranularity(const ConvertGranularity ConvertGranularity) {
    if (ConvertGranularity == plateau::granularityConvert::ConvertGranularity::MaterialInPrimary)
        return plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
    return (plateau::polygonMesh::MeshGranularity)ConvertGranularity;
}

ConvertGranularity FPLATEAUReconstructUtil::GetConvertGranularityFromReconstructType(const EPLATEAUMeshGranularity ReconstructType) {
    switch (ReconstructType) {
    case EPLATEAUMeshGranularity::PerAtomicFeatureObject:
        return ConvertGranularity::PerAtomicFeatureObject;
    case EPLATEAUMeshGranularity::PerPrimaryFeatureObject:
        return ConvertGranularity::PerPrimaryFeatureObject;
    case EPLATEAUMeshGranularity::PerCityModelArea:
        return ConvertGranularity::PerCityModelArea;
    case EPLATEAUMeshGranularity::PerMaterialInPrimary:
        return ConvertGranularity::MaterialInPrimary;
    default:
        return ConvertGranularity::PerPrimaryFeatureObject;
    }
}

TArray<UPLATEAUCityObjectGroup*> FPLATEAUReconstructUtil::FilterComponentsByPackageAndLod(APLATEAUInstancedCityModel* Actor, TArray<UPLATEAUCityObjectGroup*> TargetComponents, 
    EPLATEAUCityModelPackage Pkg, int Lod, bool includeHigherLods ) {
    TArray<UPLATEAUCityObjectGroup*> FilterdList;
    for (auto Comp : TargetComponents) {
        auto PackageComps = Actor->GetComponentsByPackage(EPLATEAUCityModelPackage::Road);
        for (auto PComp : PackageComps) {
            TArray<USceneComponent*> Children;
            ((USceneComponent*)PComp)->GetChildrenComponents(true, Children);
            if (Children.Contains(Comp)) {
                auto Parent = Comp->GetAttachParent();
                int LodValue = FPLATEAUComponentUtil::ParseLodComponent(Parent);
                if (includeHigherLods) {
                    if (LodValue >= Lod) {
                        FilterdList.Add(Comp);
                    }
                }
                else {
                    if (LodValue == Lod) {
                        FilterdList.Add(Comp);
                    }
                }
            }
        }
    }
    return FilterdList;
}