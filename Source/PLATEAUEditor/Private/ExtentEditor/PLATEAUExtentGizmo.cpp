// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentGizmo.h"

#include "SceneManagement.h"
#include "Materials/MaterialInstanceDynamic.h"

constexpr int GDefault_Minx = -1000;
constexpr int GDefault_Miny = -1000;
constexpr int GDefault_Maxx = 1000;
constexpr int GDefault_Maxy = 1000;


FPLATEAUExtentGizmo::FPLATEAUExtentGizmo()
    : MinX(GDefault_Minx), MaxX(GDefault_Maxx), MinY(GDefault_Miny), MaxY(GDefault_Maxy) {
    SphereMaterial = DuplicateObject(GEngine->ConstraintLimitMaterialPrismatic, NULL);
    SphereMaterial.Get()->SetScalarParameterValue(FName("Desaturation"), 1.0f);
    AreaMaterial = DuplicateObject(SphereMaterial, NULL);
    AreaMaterial.Get()->SetScalarParameterValue(FName("Desaturation"), 1.0f);
}

void FPLATEAUExtentGizmo::DrawHandle(int Index, FColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI, double CameraDistance) {
    SphereMaterial.Get()->SetVectorParameterValue(FName("Color"), Color);

    const auto Radius = FMath::Min(CameraDistance / 20, 500);

    DrawSphere(
        PDI,
        GetHandlePosition(Index),
        FRotator(),
        FVector(Radius),
        24, 6, SphereMaterial->GetRenderProxy(), SDPG_Foreground
    );

    DrawSphere(
        PDI,
        GetHandlePosition(Index),
        FRotator(),
        FVector(Radius),
        24, 6, SphereMaterial->GetRenderProxy(), SDPG_Foreground
    );
}

void FPLATEAUExtentGizmo::DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const {
    const FBox Box(FVector(MinX, MinY, 0), FVector(MaxX, MaxY, 0));

    DrawWireBox(
        PDI,
        Box,
        FColor(255, 127, 80),
        SDPG_World, 2, 0, true
    );

    FMatrix ObjectToWorld = FMatrix::Identity;
    ObjectToWorld.SetAxis(0, FVector((MaxX-MinX)/2, 0, 0));
    ObjectToWorld.SetAxis(1, FVector(0, (MaxY-MinY)/2, 0));
    ObjectToWorld.SetOrigin(FVector((MinX + MaxX)/2, (MinY + MaxY) / 2, 0));
    AreaMaterial.Get()->SetVectorParameterValue(FName("Color"), FColor(255, 204, 153, 100));

    DrawPlane10x10(PDI,
        ObjectToWorld,
        1.0f,
        FVector2D::Zero(), FVector2D::One(),
        AreaMaterial->GetRenderProxy(),
        SDPG_World);
}

void FPLATEAUExtentGizmo::InitHandlePosition() {
    MinX = GDefault_Minx;
    MinY = GDefault_Miny;
    MaxX = GDefault_Maxx;
    MaxY = GDefault_Maxy;
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
