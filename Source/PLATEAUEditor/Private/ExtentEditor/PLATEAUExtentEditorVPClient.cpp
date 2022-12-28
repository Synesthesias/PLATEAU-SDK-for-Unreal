// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUExtentEditorVPClient.h"
#include "PLATEAUExtentEditor.h"

#include <plateau/dataset/i_dataset_accessor.h>

#include "PLATEAUExtentGizmo.h"
#include "PLATEAUMeshCodeGizmo.h"
#include "PLATEAUFeatureInfoDisplay.h"

#include "EditorModeManager.h"
#include "CanvasTypes.h"

#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"
#include "SPLATEAUExtentEditorViewport.h"
#include "MouseDeltaTracker.h"

#include "AssetViewerSettings.h"
#include "CameraController.h"
#include "EditorViewportClient.h"

#define LOCTEXT_NAMESPACE "FPLATEAUExtentEditorViewportClient"

namespace {
    /**
     * 範囲選択のつまみのためのHitProxyクラス
     */
    struct HPLATEAUExtentHandleProxy : public HHitProxy {
        DECLARE_HIT_PROXY();

        int Index;

        HPLATEAUExtentHandleProxy(int index) :
            HHitProxy(HPP_UI),
            Index(index) {}
    };
    IMPLEMENT_HIT_PROXY(HPLATEAUExtentHandleProxy, HHitProxy);

}


FPLATEAUExtentEditorViewportClient::FPLATEAUExtentEditorViewportClient(
    TWeakPtr<FPLATEAUExtentEditor> InExtentEditor,
    const TSharedRef<SPLATEAUExtentEditorViewport>& InPLATEAUExtentEditorViewport,
    const TSharedRef<FAdvancedPreviewScene>& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InPLATEAUExtentEditorViewport))
    , ExtentEditorPtr(InExtentEditor) {
    InPreviewScene->SetFloorVisibility(false);
    ExtentGizmo = MakeUnique<FPLATEAUExtentGizmo>();
}

FPLATEAUExtentEditorViewportClient::~FPLATEAUExtentEditorViewportClient() {
    UAssetViewerSettings::Get()->OnAssetViewerSettingsChanged().RemoveAll(this);
}

void FPLATEAUExtentEditorViewportClient::Initialize(std::shared_ptr<plateau::dataset::IDatasetAccessor> InDatasetAccessor) {
    DatasetAccessor = InDatasetAccessor;

    InitCamera();

    const auto ExtentEditor = ExtentEditorPtr.Pin();
    auto GeoReference = ExtentEditor->GetGeoReference();
    if (ExtentEditor->GetExtent().IsSet())
        ExtentGizmo->SetExtent(ExtentEditor->GetExtent().GetValue(), GeoReference);

    MeshCodeGizmos.Reset();
    const auto& MeshCodes = DatasetAccessor->getMeshCodes();
    for (const auto& MeshCode : MeshCodes) {
        MeshCodeGizmos.AddDefaulted();
        MeshCodeGizmos.Last().Init(MeshCode, GeoReference.GetData());
    }
}

FPLATEAUExtent FPLATEAUExtentEditorViewportClient::GetExtent() const {
    auto GeoReference = ExtentEditorPtr.Pin()->GetGeoReference();
    return ExtentGizmo->GetExtent(GeoReference);
}

void FPLATEAUExtentEditorViewportClient::InitCamera() {
    ToggleOrbitCamera(false);
    SetCameraSetup(
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector(0.0, 0, 10000.0),
        FVector::Zero(),
        FVector(0, 0, 10000.0),
        FRotator(-90, -90, 0)
    );
    CameraController->AccessConfig().bLockedPitch = true;
    CameraController->AccessConfig().MaximumAllowedPitchRotation = -90;
    CameraController->AccessConfig().MinimumAllowedPitchRotation = -90;
}

void FPLATEAUExtentEditorViewportClient::Tick(float DeltaSeconds) {
    const auto ExtentMin = ExtentGizmo->GetMin();
    const auto ExtentMax = ExtentGizmo->GetMax();
    for (auto& Gizmo : MeshCodeGizmos) {
        Gizmo.SetSelected(Gizmo.IntersectsWith(ExtentMin, ExtentMax));
    }

    FPLATEAUMeshCodeGizmo::SetShowLevel5Mesh(GetViewTransform().GetLocation().Z < 10000);

    // ベースマップ
    const auto ExtentEditor = ExtentEditorPtr.Pin();
    auto GeoReference = ExtentEditor->GetGeoReference();
    if (Basemap == nullptr) {
        Basemap = MakeUnique<FPLATEAUBasemap>(GeoReference, SharedThis(this));
    }

    // 地物表示
    if (FeatureInfoDisplay == nullptr) {
        FeatureInfoDisplay = MakeUnique<FPLATEAUFeatureInfoDisplay>(GeoReference, SharedThis(this));
    }

    TArray<FVector> CornerWorldPositions;
    CornerWorldPositions.Add(GetWorldPosition(0, 0));
    CornerWorldPositions.Add(GetWorldPosition(0, Viewport->GetSizeXY().Y));
    CornerWorldPositions.Add(GetWorldPosition(Viewport->GetSizeXY().X, 0));
    CornerWorldPositions.Add(GetWorldPosition(Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y));

    FVector MinPosition = CornerWorldPositions[0];
    FVector MaxPosition = CornerWorldPositions[1];
    for (int i = 1; i < 4; ++i) {
        MinPosition.X = FMath::Min(MinPosition.X, CornerWorldPositions[i].X);
        MinPosition.Y = FMath::Min(MinPosition.Y, CornerWorldPositions[i].Y);
        MaxPosition.X = FMath::Max(MaxPosition.X, CornerWorldPositions[i].X);
        MaxPosition.Y = FMath::Max(MaxPosition.Y, CornerWorldPositions[i].Y);
    }

    const TVec3d RawMinPosition(MinPosition.X, MinPosition.Y, MinPosition.Z);
    const TVec3d RawMaxPosition(MaxPosition.X, MaxPosition.Y, MaxPosition.Z);

    auto MinCoordinate = GeoReference.GetData().unproject(RawMinPosition);
    auto MaxCoordinate = GeoReference.GetData().unproject(RawMaxPosition);

    // Unproject後の最小最大を再計算
    const auto TempMinCoordinate = MinCoordinate;
    MinCoordinate.latitude = FMath::Min(TempMinCoordinate.latitude, MaxCoordinate.latitude);
    MinCoordinate.longitude = FMath::Min(TempMinCoordinate.longitude, MaxCoordinate.longitude);
    MaxCoordinate.latitude = FMath::Max(TempMinCoordinate.latitude, MaxCoordinate.latitude);
    MaxCoordinate.longitude = FMath::Max(TempMinCoordinate.longitude, MaxCoordinate.longitude);

    FPLATEAUExtent Extent(plateau::geometry::Extent(MinCoordinate, MaxCoordinate));

    Basemap->UpdateAsync(Extent);
    FeatureInfoDisplay->UpdateAsync(Extent, *DatasetAccessor, GetViewTransform().GetLocation().Z < 4000, GetViewTransform().GetLocation().Z < 2000);

    // 何も選択されていない場合は既定の動作(視点移動等)
    if (SelectedHandleIndex == -1) {
        if (IsCameraMoving) {
            SetViewLocation(TrackingStartedCameraPosition);
            FVector CurrentCursorPosition;
            if (!TryGetWorldPositionOfCursor(CurrentCursorPosition))
                return;

            const auto Offset = CurrentCursorPosition - TrackingStartedPosition;

            SetViewLocation(TrackingStartedCameraPosition - Offset);
        }
        return;
    }

    FVector CurrentCursorPosition;
    if (!TryGetWorldPositionOfCursor(CurrentCursorPosition))
        return;

    const auto Offset = CurrentCursorPosition - TrackingStartedPosition;

    ExtentGizmo->SetHandlePosition(SelectedHandleIndex, TrackingStartedGizmoPosition + Offset);
}

void FPLATEAUExtentEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorViewportClient::Draw(View, PDI);

    constexpr FColor SelectedColor(225, 225, 110);
    constexpr FColor UnselectedColor(255, 127, 80);

    double CameraDistance = GetViewTransform().GetLocation().Z;
    for (int i = 0; i < 5; ++i) {
        const auto HitProxy = new HPLATEAUExtentHandleProxy(i);
        PDI->SetHitProxy(HitProxy);
        const FColor Color = i == SelectedHandleIndex ? SelectedColor : UnselectedColor;
        ExtentGizmo->DrawHandle(i, Color, View, PDI, CameraDistance);
        PDI->SetHitProxy(nullptr);
    }
    ExtentGizmo->DrawExtent(View, PDI);

    for (const auto& Gizmo : MeshCodeGizmos) {
        Gizmo.DrawExtent(View, PDI);
    }
}

void FPLATEAUExtentEditorViewportClient::TrackingStarted(const FInputEventState& InInputState, bool bIsDragging,
    bool bNudge) {
    if (!TryGetWorldPositionOfCursor(TrackingStartedPosition))
        return;

    const auto HitProxy = static_cast<HPLATEAUExtentHandleProxy*>(Viewport->GetHitProxy(CachedMouseX, CachedMouseY));
    if (!HitProxy) {
        TrackingStartedCameraPosition = GetViewLocation();
        IsCameraMoving = true;
        return;
    }

    TrackingStartedGizmoPosition = ExtentGizmo->GetHandlePosition(HitProxy->Index);
    SelectedHandleIndex = HitProxy->Index;
}

void FPLATEAUExtentEditorViewportClient::TrackingStopped() {
    SelectedHandleIndex = -1;
    IsCameraMoving = false;
}

bool FPLATEAUExtentEditorViewportClient::ShouldScaleCameraSpeedByDistance() const {
    return true;
}

FVector FPLATEAUExtentEditorViewportClient::GetWorldPosition(uint32 X, uint32 Y) {
    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        Viewport,
        GetScene(),
        EngineShowFlags)
        .SetRealtimeUpdate(IsRealtime()));

    const FSceneView* View = CalcSceneView(&ViewFamily);

    const auto Location = FViewportCursorLocation(View,
        this,
        X,
        Y
    );

    const FPlane Plane(FVector::ZeroVector, FVector::UpVector);
    const auto StartPoint = Location.GetOrigin();
    const auto EndPoint = Location.GetOrigin() + Location.GetDirection() * 100000.0;
    FVector Position;
    FMath::SegmentPlaneIntersection(
        StartPoint,
        EndPoint,
        Plane, Position);
    return Position;
}

bool FPLATEAUExtentEditorViewportClient::TryGetWorldPositionOfCursor(FVector& Position) {
    const auto CursorLocation = GetCursorWorldLocationFromMousePos();
    const FPlane Plane(FVector::ZeroVector, FVector::UpVector);
    const auto StartPoint = CursorLocation.GetOrigin();
    const auto EndPoint = CursorLocation.GetOrigin() + CursorLocation.GetDirection() * 100000.0;
    return FMath::SegmentPlaneIntersection(
        StartPoint,
        EndPoint,
        Plane, Position);
}

#undef LOCTEXT_NAMESPACE
