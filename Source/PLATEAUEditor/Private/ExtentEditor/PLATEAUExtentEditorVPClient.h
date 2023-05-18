// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PLATEAUGeometry.h"

namespace plateau::dataset {
    class IDatasetAccessor;
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
    void Initialize(std::shared_ptr<plateau::dataset::IDatasetAccessor> InFileCollection);
    
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
    TSharedPtr<class FPLATEAUFeatureInfoDisplay> FeatureInfoDisplay;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;

    // 内部状態
    int SelectedHandleIndex = -1;
    bool IsCameraMoving = false;
    FVector TrackingStartedPosition;
    FVector TrackingStartedGizmoPosition;
    FVector TrackingStartedCameraPosition;
    FVector2D DefaultGizmoHandlePosMin;
    FVector2D DefaultGizmoHandlePosMax;

    FVector GetWorldPosition(uint32 X, uint32 Y);
    bool TryGetWorldPositionOfCursor(FVector& Position);
    void InitCamera();
};
