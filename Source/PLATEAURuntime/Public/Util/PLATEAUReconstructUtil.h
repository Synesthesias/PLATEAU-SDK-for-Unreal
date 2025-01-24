// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "PLATEAUInstancedCityModel.h"
#include "plateau/height_map_generator/heightmap_generator.h"
#include "plateau/granularity_convert/granularity_converter.h"

using ConvertGranularity = plateau::granularityConvert::ConvertGranularity;

class PLATEAURUNTIME_API FPLATEAUReconstructUtil {
public:
    /**
     * @brief UPLATEAUCityObjectGroupのリストからUPLATEAUCityObjectを取り出し、GmlIDをキーとしたMapを生成
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: GmlID, Value: UPLATEAUCityObject の Map
     */
    static TMap<FString, FPLATEAUCityObject> CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    /**
     * @brief 16bitグレイスケールの画像を保存します。Debug用
     */
    static void SaveHeightmapImage(EPLATEAULandscapeHeightmapImageOutput OutputParam, FString FileName, int32 Width, int32 Height, uint16_t* Data);

    /**
     * @brief EPLATEAUMeshGranularityをplateau::polygonMesh::MeshGranularityに変換します
     */
    static plateau::polygonMesh::MeshGranularity ConvertGranularityToMeshGranularity(const ConvertGranularity ConvertGranularity);

    /**
     * @brief EPLATEAUMeshGranularityをplateau::granularityConvert::ConvertGranularityに変換します
     */
    static ConvertGranularity GetConvertGranularityFromReconstructType(const EPLATEAUMeshGranularity ReconstructType);

    /**
     * @brief Componentリストから指定したパッケージとLODのものを探してリストに追加します
     */
    static TArray<UPLATEAUCityObjectGroup*> FilterComponentsByPackageAndLod(APLATEAUInstancedCityModel* Actor, TArray<UPLATEAUCityObjectGroup*> TargetComponents,
        EPLATEAUCityModelPackage Pkg, int Lod, bool includeHigherLods);
};
