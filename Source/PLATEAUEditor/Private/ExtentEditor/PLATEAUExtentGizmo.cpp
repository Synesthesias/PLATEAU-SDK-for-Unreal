// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUExtentGizmo.h"

#include "SceneManagement.h"

FPLATEAUExtentGizmo::FPLATEAUExtentGizmo()
    : XMin(-500), XMax(500), YMin(-500), YMax(500) {}

void FPLATEAUExtentGizmo::DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    DrawWireSphere(
        PDI,
        GetHandlePosition(Index),
        Color,
        10, 10, 1, 5, 0, true
    );
}

void FPLATEAUExtentGizmo::DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const {
    const FBox Box(FVector(XMin, YMin, 0), FVector(XMax, YMax, 0));

    DrawWireBox(
        PDI,
        Box,
        FColor::White,
        0, 2, 0, true
    );
}

FVector FPLATEAUExtentGizmo::GetHandlePosition(int Index) {
    if (Index == 0)
        return { XMin, YMin, 0 };
    if (Index == 1)
        return { XMin, YMax, 0 };
    if (Index == 2)
        return { XMax, YMin, 0 };
    if (Index == 3)
        return { XMax, YMax, 0 };

    return {};
}

void FPLATEAUExtentGizmo::SetHandlePosition(int Index, FVector Position) {
    if (Index == 0) {
        XMin = Position.X;
        YMin = Position.Y;
    }
    if (Index == 1) {
        XMin = Position.X;
        YMax = Position.Y;
    }
    if (Index == 2) {
        XMax = Position.X;
        YMin = Position.Y;
    }
    if (Index == 3) {
        XMax = Position.X;
        YMax = Position.Y;
    }
}

void FPLATEAUExtentGizmo::SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference) {
    const auto RawMin = GeoReference.GetData().project(Extent.Min.GetNativeData());
    const auto RawMax = GeoReference.GetData().project(Extent.Max.GetNativeData());

    XMin = RawMin.x;
    YMin = RawMin.y;
    XMax = RawMax.x;
    YMax = RawMax.y;
}

FPLATEAUExtent FPLATEAUExtentGizmo::GetExtent(FPLATEAUGeoReference& GeoReference) const {
    const TVec3d Min(XMin, YMin, 0);
    const TVec3d Max(XMax, YMax, 0);

    const auto RawMin = GeoReference.GetData().unproject(Min);
    const auto RawMax = GeoReference.GetData().unproject(Max);

    return FPLATEAUExtent(plateau::geometry::Extent(RawMin, RawMax));
}
