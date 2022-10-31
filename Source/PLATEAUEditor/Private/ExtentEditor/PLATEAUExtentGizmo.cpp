// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUExtentGizmo.h"

#include "SceneManagement.h"

FPLATEAUExtentGizmo::FPLATEAUExtentGizmo()
    : MinX(-500), MaxX(500), MinY(-500), MaxY(500) {}

void FPLATEAUExtentGizmo::DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    DrawWireSphere(
        PDI,
        GetHandlePosition(Index),
        Color,
        10, 10, 10, 5, 0, true
    );
}

void FPLATEAUExtentGizmo::DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const {
    const FBox Box(FVector(MinX, MinY, 0), FVector(MaxX, MaxY, 0));

    DrawWireBox(
        PDI,
        Box,
        FColor::White,
        9, 2, 0, true
    );
}

FVector FPLATEAUExtentGizmo::GetHandlePosition(int Index) {
    if (Index == 0)
        return { MinX, MinY, 0 };
    if (Index == 1)
        return { MinX, MaxY, 0 };
    if (Index == 2)
        return { MaxX, MinY, 0 };
    if (Index == 3)
        return { MaxX, MaxY, 0 };

    return {};
}

void FPLATEAUExtentGizmo::SetHandlePosition(int Index, FVector Position) {
    if (Index == 0) {
        MinX = Position.X;
        MinY = Position.Y;
    }
    if (Index == 1) {
        MinX = Position.X;
        MaxY = Position.Y;
    }
    if (Index == 2) {
        MaxX = Position.X;
        MinY = Position.Y;
    }
    if (Index == 3) {
        MaxX = Position.X;
        MaxY = Position.Y;
    }
}

void FPLATEAUExtentGizmo::SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference) {
    auto RawMin = GeoReference.GetData().project(Extent.Min.GetNativeData());
    auto RawMax = GeoReference.GetData().project(Extent.Max.GetNativeData());

    // 座標系変換時にxの大小が逆転するので再設定を行う。
    const auto Tmp = RawMin.x;
    RawMin.x = FMath::Min(RawMin.x, RawMax.x);
    RawMax.x = FMath::Max(Tmp, RawMax.x);

    MinX = RawMin.x;
    MinY = RawMin.y;
    MaxX = RawMax.x;
    MaxY = RawMax.y;
}

FPLATEAUExtent FPLATEAUExtentGizmo::GetExtent(FPLATEAUGeoReference& GeoReference) const {
    const TVec3d Min(MinX, MinY, 0);
    const TVec3d Max(MaxX, MaxY, 0);

    auto RawMin = GeoReference.GetData().unproject(Min);
    auto RawMax = GeoReference.GetData().unproject(Max);

    // 座標系変換時に経度の大小が逆転するので再設定を行う。
    const auto Tmp = RawMin.longitude;
    RawMin.longitude = FMath::Min(RawMin.longitude, RawMax.longitude);
    RawMax.longitude = FMath::Max(Tmp, RawMax.longitude);

    return FPLATEAUExtent(plateau::geometry::Extent(RawMin, RawMax));
}

FVector2D FPLATEAUExtentGizmo::GetMin() const {
    return FVector2D(MinX, MinY);
}

FVector2D FPLATEAUExtentGizmo::GetMax() const {
    return FVector2D(MaxX, MaxY);
}
