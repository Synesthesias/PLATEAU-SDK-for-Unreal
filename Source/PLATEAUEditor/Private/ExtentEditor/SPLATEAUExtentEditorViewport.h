#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "SEditorViewport.h"

class SPLATEAUExtentEditorViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SPLATEAUExtentEditorViewport) {}
    SLATE_ARGUMENT(TWeakPtr<class FPLATEAUExtentEditor>, ExtentEditor)
        SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    SPLATEAUExtentEditorViewport();
    virtual ~SPLATEAUExtentEditorViewport() override;

    void SetOwnerTab(TSharedRef<class SDockTab> Tab);
    TSharedPtr<class SDockTab> GetOwnerTab() const;

protected:
    /** SEditorViewport interface */
    virtual bool IsVisible() const override;
    virtual TSharedRef<class FEditorViewportClient> MakeEditorViewportClient() override;
    virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
    virtual void BindCommands() override;

private:
    void AttachVectorTile(FPLATEAUExtent Extent);

    // このインスタンスを保持しているExtentEditorへのポインタ
    TWeakPtr<class FPLATEAUExtentEditor> ExtentEditorPtr;

    // ViewportClientに渡すScene
    TSharedPtr<class FAdvancedPreviewScene> PreviewScene;

    TSharedPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    // このビューポートを含むDockTab
    TWeakPtr<class SDockTab> OwnerTab;
};

