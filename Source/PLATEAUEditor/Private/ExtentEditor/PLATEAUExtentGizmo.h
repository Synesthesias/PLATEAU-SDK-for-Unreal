// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"

/**
 * @brief 範囲選択ギズモを表します。
 */
class FPLATEAUExtentGizmo {
public:
    FPLATEAUExtentGizmo();

    void DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI);
    void DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const;

    FVector GetHandlePosition(int Index);
    void SetHandlePosition(int Index, FVector Position);

    /**
     * @brief Extentを入力として内部状態を更新します。
     */
    void SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference);

    /**
     * @brief 内部状態からExtentを取得します。
     */
    FPLATEAUExtent GetExtent(FPLATEAUGeoReference& GeoReference) const;

private:
    double XMin;
    double XMax;
    double YMin;
    double YMax;
};
