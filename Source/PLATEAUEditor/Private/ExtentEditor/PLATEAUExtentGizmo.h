// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"

/**
 * @brief �͈͑I���M�Y����\���܂��B
 */
class FPLATEAUExtentGizmo {
public:
    FPLATEAUExtentGizmo();

    void DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI);
    void DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const;

    FVector GetHandlePosition(int Index);
    void SetHandlePosition(int Index, FVector Position);

    /**
     * @brief Extent����͂Ƃ��ē�����Ԃ��X�V���܂��B
     */
    void SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference);

    /**
     * @brief ������Ԃ���Extent���擾���܂��B
     */
    FPLATEAUExtent GetExtent(FPLATEAUGeoReference& GeoReference) const;

private:
    double XMin;
    double XMax;
    double YMin;
    double YMax;
};
