// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"
#include <plateau/height_map_alighner/height_map_aligner.h>

class PLATEAURUNTIME_API FPLATEAUModelAlignLand : public FPLATEAUModelReconstruct {

public:

    //HeightMapAligner Parameters
    const double HeightOffset = 30;
    const float MaxEdgeLength = 400.f; // 高さ合わせのためメッシュを細分化するときの、最大の辺の長さ。だいたい4mくらいが良さそう。
    const int AlphaExpandWidthCartesian = 200; // 逆高さ合わせのアルファの平滑化処理において、不透明部分を広げる幅（直交座標系）
    const int AlphaAveragingWidthCartesian = 200; // 逆高さ合わせのアルファの平滑化処理において、周りの平均を取る幅（直交座標系）
    const double InvertedHeightOffset = -20.0; // 逆高さ合わせで、土地を対象と比べてどの高さに合わせるか（直交座標系）
    const float SkipThresholdOfMapLandDistance = 80.f; // 逆高さ合わせで、土地との距離がこの値以上の箇所は高さ合わせしない（直交座標系）

    // 地形にAlignするパッケージリスト
    const TSet<EPLATEAUCityModelPackage> IncludePacakges{ EPLATEAUCityModelPackage::Area,
    EPLATEAUCityModelPackage::Road,
    EPLATEAUCityModelPackage::Square,
    EPLATEAUCityModelPackage::Track,
    EPLATEAUCityModelPackage::Waterway,
    EPLATEAUCityModelPackage::DisasterRisk,
    EPLATEAUCityModelPackage::LandUse,
    EPLATEAUCityModelPackage::WaterBody,
    EPLATEAUCityModelPackage::UrbanPlanningDecision,
    };

    /**
     * @brief ComponentリストからLOD3のRoadを探してリストに追加します
     */
    static TArray<UPLATEAUCityObjectGroup*> FilterLod3RoadComponents(APLATEAUInstancedCityModel* Actor, TArray<UPLATEAUCityObjectGroup*> TargetComponents);

    FPLATEAUModelAlignLand();
    FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor);

    plateau::heightMapAligner::HeightMapFrame CreateAlignData(const TSharedPtr<std::vector<uint16_t>> HeightData, const TVec3d Min, const TVec3d Max, const FString NodeName, FPLATEAULandscapeParam Param);

    TArray<UPLATEAUCityObjectGroup*> SetAlignData(const TArray<plateau::heightMapAligner::HeightMapFrame> Frames, FPLATEAULandscapeParam Param);

protected:

private:

};

