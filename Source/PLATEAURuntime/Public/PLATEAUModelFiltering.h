// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include <PLATEAUInstancedCityModel.h>

class PLATEAURUNTIME_API FPLATEAUModelFiltering {

public:
    FPLATEAUModelFiltering();

    /**
     * @brief 複数LODの形状を持つ地物について、MinLod, MaxLodで指定される範囲の内最大LOD以外の形状を非表示化します。
     * @param InGmlComponent フィルタリング対象地物を含むコンポーネント
     * @param MinLod 可視化される最小のLOD
     * @param MaxLod 可視化される最大のLOD
     */
    void FilterLowLods(const USceneComponent* const InGmlComponent, const int MinLod = 0, const int MaxLod = 4);

    /**
     * @brief 対象コンポーネントとその子コンポーネントのコリジョン設定変更
     * @param ParentComponent コリジョン設定変更対象コンポーネント
     * @param bCollisionResponseBlock コリジョンをブロック設定に変更するか？
     * @param bPropagateToChildren 子コンポーネントのコリジョン設定を変更するか？
     */
    void ApplyCollisionResponseBlockToChannel(USceneComponent* ParentComponent, const bool bCollisionResponseBlock, const bool bPropagateToChildren = false);

    void FilterByLods(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const plateau::dataset::PredefinedCityModelPackage InPackage, const TMap<plateau::dataset::PredefinedCityModelPackage, FPLATEAUMinMaxLod>& PackageToLodRangeMap, const bool bOnlyMaxLod);

    void FilterByFeatureTypes(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const citygml::CityObject::CityObjectsType InCityObjectType);

    void FilterByFeatureTypesLegacyCacheCityGml(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const citygml::CityObject::CityObjectsType InCityObjectType, const FString DatasetName);
    void FilterByFeatureTypesLegacyMain(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const citygml::CityObject::CityObjectsType InCityObjectType, const FString DatasetName);
};
