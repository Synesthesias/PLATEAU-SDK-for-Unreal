// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUExtentGizmo.h"

#include "SceneManagement.h"

FPLATEAUExtentGizmo::FPLATEAUExtentGizmo()
    : MinX(-500), MaxX(500), MinY(-500), MaxY(500) {}

void FPLATEAUExtentGizmo::DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    
    //auto MaterialInstance = new FColoredMaterialRenderProxy(
    //    GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),
    //    FColor(255, 127, 80, 100)
    //);

    //DrawPlane10x10(PDI,
    //    FMatrix()
    //    0,
    //    FVector2D::Zero(), FVector2D::One(),
    //    MaterialInstance, 0);

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
        FColor(255, 127, 80),
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
    if (Index == 4)
        return { MinX + (MaxX - MinX) / 2, MinY + (MaxY - MinY) / 2, 0 };

    return {};
}

void FPLATEAUExtentGizmo::SetHandlePosition(const int Index, const FVector& Position) {
    if (Index == 0) {
        SetMinX(Position.X);
        SetMinY(Position.Y);
    }
    if (Index == 1) {
        SetMinX(Position.X);
        SetMaxY(Position.Y);
    }
    if (Index == 2) {
        SetMaxX(Position.X);
        SetMinY(Position.Y);
    }
    if (Index == 3) {
        SetMaxX(Position.X);
        SetMaxY(Position.Y);
    }
    if (Index == 4) {
        double DiffX = MaxX - MinX;
        double DiffY = MaxY - MinY;
        SetMinX(Position.X - DiffX / 2);
        SetMinY(Position.Y - DiffY / 2);
        SetMaxX(Position.X + DiffX / 2);
        SetMaxY(Position.Y + DiffY / 2);
    }
}

void FPLATEAUExtentGizmo::SetExtent(const FPLATEAUExtent& Extent, FPLATEAUGeoReference& GeoReference) {
    auto RawMin = GeoReference.GetData().project(Extent.Min.GetNativeData());
    auto RawMax = GeoReference.GetData().project(Extent.Max.GetNativeData());

    // 座標系変換時にyの大小が逆転するので再設定を行う。
    const auto Tmp = RawMin.y;
    RawMin.y = FMath::Min(RawMin.y, RawMax.y);
    RawMax.y = FMath::Max(Tmp, RawMax.y);

    MinX = RawMin.x;
    MinY = RawMin.y;
    MaxX = RawMax.x;
    MaxY = RawMax.y;
}

void FPLATEAUExtentGizmo::SetMinX(double Value) {
    MinX = FMath::Min(Value, MaxX);
}

void FPLATEAUExtentGizmo::SetMinY(const double Value) {
    MinY = FMath::Min(Value, MaxY);
}

void FPLATEAUExtentGizmo::SetMaxX(const double Value) {
    MaxX = FMath::Max(Value, MinX);
}

void FPLATEAUExtentGizmo::SetMaxY(const double Value) {
    MaxY = FMath::Max(Value, MinY);
}

FPLATEAUExtent FPLATEAUExtentGizmo::GetExtent(FPLATEAUGeoReference& GeoReference) const {
    const TVec3d Min(MinX, MinY, 0);
    const TVec3d Max(MaxX, MaxY, 0);

    auto RawMin = GeoReference.GetData().unproject(Min);
    auto RawMax = GeoReference.GetData().unproject(Max);

    // 座標系変換時に緯度の大小が逆転するので再設定を行う。
    const auto Tmp = RawMin.latitude;
    RawMin.latitude = FMath::Min(RawMin.latitude, RawMax.latitude);
    RawMax.latitude = FMath::Max(Tmp, RawMax.latitude);

    return FPLATEAUExtent(plateau::geometry::Extent(RawMin, RawMax));
}

FVector2D FPLATEAUExtentGizmo::GetMin() const {
    return FVector2D(MinX, MinY);
}

FVector2D FPLATEAUExtentGizmo::GetMax() const {
    return FVector2D(MaxX, MaxY);
}
