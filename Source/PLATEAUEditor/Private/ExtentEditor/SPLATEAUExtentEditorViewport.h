#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SEditorViewport.h"
#include "PLATEAUExtentEditor.h"

class FAdvancedPreviewScene;

class SPLATEAUExtentEditorViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SPLATEAUExtentEditorViewport) {}
        SLATE_ARGUMENT(TWeakPtr<FPLATEAUExtentEditor>, ExtentEditor)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    SPLATEAUExtentEditorViewport();
    ~SPLATEAUExtentEditorViewport();

    void Invalidate();
    void EnablePreview(bool bEnable);
    void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);

    // SWidget interface
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    // End of SWidget interface

    void OnComponentSelectionChanged();
    virtual void OnFocusViewportToSelection() override;
    bool GetIsSimulateEnabled();
    void SetOwnerTab(TSharedRef<SDockTab> Tab);

    TSharedPtr<SDockTab> GetOwnerTab() const;

protected:
    /**
     * Determines if the viewport widget is visible.
     *
     * @return true if the viewport is visible; false otherwise.
     */
    bool IsVisible() const override;

    /**
     * Returns true if the viewport commands should be shown
     */
    bool ShouldShowViewportCommands() const;

    /** Called when the simulation toggle command is fired */
    void ToggleIsSimulateEnabled();

    /** SEditorViewport interface */
    virtual TSharedRef<class FEditorViewportClient> MakeEditorViewportClient() override;
    virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
    virtual void BindCommands() override;

private:
    /** One-off active timer to update the preview */
    EActiveTimerReturnType DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera);

private:
    /** Pointer back to editor tool (owner) */
    TWeakPtr<class FPLATEAUExtentEditor> ExtentEditorPtr;

    /** The scene for this viewport. */
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;

    /** Viewport client */
    TSharedPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    /** Whether the active timer (for updating the preview) is registered */
    bool bIsActiveTimerRegistered;

    /** The owner dock tab for this viewport. */
    TWeakPtr<SDockTab> OwnerTab;

    /** Handle to the registered OnPreviewFeatureLevelChanged delegate. */
    FDelegateHandle PreviewFeatureLevelChangedHandle;
};
