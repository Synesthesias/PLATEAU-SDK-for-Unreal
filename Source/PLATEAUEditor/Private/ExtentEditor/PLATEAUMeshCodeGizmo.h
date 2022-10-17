// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace plateau {
    namespace udx {
        class MeshCode;
    }
    namespace geometry {
        class GeoReference;
    }
}

/**
 * @brief 各地域メッシュのメッシュコードのギズモを表します。
 */
class FPLATEAUMeshCodeGizmo {
public:
    FPLATEAUMeshCodeGizmo();

    void DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const;

    /**
     * @brief 内部状態から範囲の最小値を取得します。
     */
    FVector2D GetMin() const;

    /**
     * @brief 内部状態から範囲の最大値を取得します。
     */
    FVector2D GetMax() const;

    /**
     * @brief インスタンスを初期化します。
     */
    void Init(const plateau::udx::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference);

    /**
     * @brief 範囲が交差するかどうかを判断します。
     * @param InMin 範囲の最小値
     * @param InMax 範囲の最大値
     * @return 交差部分が存在する場合true、それ以外の場合はfalse
     */
    bool IntersectsWith(FVector2D InMin, FVector2D InMax) const;

    /**
     * @brief 選択または選択解除します。
     */
    void SetSelected(const bool Value);

private:
    double MinX;
    double MinY;
    double MaxX;
    double MaxY;
    bool IsSelected;
    float LineThickness;
};
