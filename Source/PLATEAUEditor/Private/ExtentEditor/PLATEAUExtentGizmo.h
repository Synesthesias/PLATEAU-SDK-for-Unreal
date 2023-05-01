// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"

/**
 * @brief 範囲選択ギズモを表します。
 */
class FPLATEAUExtentGizmo {
public:
    FPLATEAUExtentGizmo();

    void DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI, double CameraDistance);
    void DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const;

    FVector GetHandlePosition(int Index);
    void SetHandlePosition(const int Index, const FVector& Position);

    /**
     * @brief Extentを入力として内部状態を更新します。
     */
    void SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference);

    void SetMinX(const double Value);
    void SetMinY(const double Value);
    void SetMaxX(const double Value);
    void SetMaxY(const double Value);

    /**
     * @brief 内部状態からExtentを取得します。
     */
    FPLATEAUExtent GetExtent(FPLATEAUGeoReference& GeoReference) const;

    /**
     * @brief 内部状態から範囲の最小値を取得します。
     */
    FVector2D GetMin() const;

    /**
     * @brief 内部状態から範囲の最大値を取得します。
     */
    FVector2D GetMax() const;


private:
    double MinX;
    double MaxX;
    double MinY;
    double MaxY;

    TObjectPtr<class UMaterialInstanceDynamic> SphereMaterial;
    TObjectPtr<class UMaterialInstanceDynamic> AreaMaterial;
};
