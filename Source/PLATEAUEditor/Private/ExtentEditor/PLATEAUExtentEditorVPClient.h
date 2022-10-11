// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PLATEAUGeometry.h"

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
    void Initialize();

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

    // 内部状態
    int SelectedHandleIndex = -1;
    FVector TrackingStartedPosition;
    FVector TrackingStartedGizmoPosition;

    bool TryGetWorldPositionOfCursor(FVector& Position);
    void InitCamera();
};
