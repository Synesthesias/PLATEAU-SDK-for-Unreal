// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PLATEAUGeometry.h"

namespace plateau::udx {
    class UdxFileCollection;
}

/** Viewport Client for the preview viewport */
class FPLATEAUExtentEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FPLATEAUExtentEditorViewportClient> {
public:
    FPLATEAUExtentEditorViewportClient(
        TWeakPtr<class FPLATEAUExtentEditor> InExtentEditor,
        const TSharedRef<class SPLATEAUExtentEditorViewport>& InPLATEAUExtentEditorViewport,
        const TSharedRef<class FAdvancedPreviewScene>& InPreviewScene);
    virtual ~FPLATEAUExtentEditorViewportClient() override;

    /**
     * @brief ViewportのConstructから呼び出される初期化処理です。
     */
    void Initialize(std::shared_ptr<plateau::udx::UdxFileCollection> InFileCollection);
    
    FPLATEAUExtent GetExtent() const;

    // FEditorViewportClient interface
    virtual void Tick(float DeltaSeconds) override;
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
    virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override;
    virtual void TrackingStopped() override;
    virtual bool ShouldScaleCameraSpeedByDistance() const override;

private:
    // このインスタンスを保持しているExtentEditorへのポインタ
    TWeakPtr<class FPLATEAUExtentEditor> ExtentEditorPtr;
    FAdvancedPreviewScene* AdvancedPreviewScene;

    TUniquePtr<class FPLATEAUExtentGizmo> ExtentGizmo;
    TArray<class FPLATEAUMeshCodeGizmo> MeshCodeGizmos;
    TUniquePtr<class FPLATEAUBasemap> Basemap;
    TUniquePtr<class FPLATEAUFeatureInfoDisplay> FeatureInfoDisplay;
    std::shared_ptr<plateau::udx::UdxFileCollection> FileCollection;

    // 内部状態
    int SelectedHandleIndex = -1;
    bool IsCameraMoving = false;
    FVector TrackingStartedPosition;
    FVector TrackingStartedGizmoPosition;
    FVector TrackingStartedCameraPosition;

    FVector GetWorldPosition(uint32 X, uint32 Y);
    bool TryGetWorldPositionOfCursor(FVector& Position);
    void InitCamera();
};
