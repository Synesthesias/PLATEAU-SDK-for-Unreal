// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUMeshCodeGizmo.h"

#include "SceneManagement.h"

#include <plateau/dataset/mesh_code.h>
#include <plateau/geometry/geo_reference.h>

FPLATEAUMeshCodeGizmo::FPLATEAUMeshCodeGizmo()
    : MinX(-500), MinY(-500), MaxX(500), MaxY(500), IsSelected(false), LineThickness(1.0f) {}

void FPLATEAUMeshCodeGizmo::DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const {
    const FBox Box(FVector(MinX, MinY, 0), FVector(MaxX, MaxY, 0));

    const auto Color = MeshCodeLevel == 2
        ? FColor(10, 10, 10)
        : FColor(10, 10, 130);
    const auto DepthPriority = IsSelected
        ? 1
        : 0;

    DrawWireBox(
        PDI,
        Box,
        Color,
        DepthPriority, LineThickness, 0, true
    );

    if (!bShowLevel5Mesh)
        return;

    if (MeshCodeLevel == 2)
        return;

    for (int i = 1; i < 4; ++i) {
        FVector	P, Q;

        P.X = (Box.Min.X * i + Box.Max.X * (4 - i)) / 4;
        Q.X = P.X;
        P.Y = Box.Min.Y; Q.Y = Box.Max.Y;
        PDI->DrawLine(P, Q, Color, DepthPriority, 1, 0, true);

        P.Y = (Box.Min.Y * i + Box.Max.Y * (4 - i)) / 4;
        Q.Y = P.Y;
        P.X = Box.Min.X; Q.X = Box.Max.X;
        PDI->DrawLine(P, Q, Color, DepthPriority, 1, 0, true);
    }
}

FVector2D FPLATEAUMeshCodeGizmo::GetMin() const {
    return FVector2D(MinX, MinY);
}

FVector2D FPLATEAUMeshCodeGizmo::GetMax() const {
    return FVector2D(MaxX, MaxY);
}

void FPLATEAUMeshCodeGizmo::Init(const plateau::dataset::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference) {
    const auto Extent = InMeshCode.getExtent();
    const auto RawMin = InGeoReference.project(Extent.min);
    const auto RawMax = InGeoReference.project(Extent.max);
    MinX = FGenericPlatformMath::Min(RawMin.x, RawMax.x);
    MinY = FGenericPlatformMath::Min(RawMin.y, RawMax.y);
    MaxX = FGenericPlatformMath::Max(RawMin.x, RawMax.x);
    MaxY = FGenericPlatformMath::Max(RawMin.y, RawMax.y);
    if (InMeshCode.get().size() == 6) {
        LineThickness = 3.0f;
        MeshCodeLevel = 2;
    } else {
        LineThickness = 2.0f;
        MeshCodeLevel = 3;
    }
}

bool FPLATEAUMeshCodeGizmo::IntersectsWith(FVector2D InMin, FVector2D InMax) const {
    return !(MaxX < InMin.X || MinX > InMax.X
        || MaxY < InMin.Y || MinY > InMax.Y);
}

void FPLATEAUMeshCodeGizmo::SetSelected(const bool Value) {
    IsSelected = Value;
}

void FPLATEAUMeshCodeGizmo::SetShowLevel5Mesh(const bool bValue) {
    bShowLevel5Mesh = bValue;
}
