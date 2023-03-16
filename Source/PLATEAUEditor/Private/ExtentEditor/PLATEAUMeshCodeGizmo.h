// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

namespace plateau {
    namespace dataset {
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
    void Init(const plateau::dataset::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference);

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

    static void SetShowLevel5Mesh(const bool bValue);

private:
    int MeshCodeLevel;
    inline static bool bShowLevel5Mesh = false;

    double MinX;
    double MinY;
    double MaxX;
    double MaxY;
    bool IsSelected;
    float LineThickness;
};
