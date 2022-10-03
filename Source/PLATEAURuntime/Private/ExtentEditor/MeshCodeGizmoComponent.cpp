// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/MeshCodeGizmoComponent.h"

#include "WorldCollision.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "SceneManagement.h"

UMeshCodeGizmoComponent::UMeshCodeGizmoComponent(const FObjectInitializer& ObjectInitializer)
    : UPrimitiveComponent(ObjectInitializer) {
    Min = FVector2D(0.0f, 0.0f);
    Max = FVector2D(1000.0f, 1000.0f);

    bUseEditorCompositing = true;
}

bool UMeshCodeGizmoComponent::IsZeroExtent() const {
    return Min == Max;
}

FBoxSphereBounds UMeshCodeGizmoComponent::CalcBounds(const FTransform& LocalToWorld) const {
    const auto Box = FBox(FVector(Min.X, Min.Y, -0.1f), FVector(Max.X, Max.Y, 0.1f));
    return FBoxSphereBounds(Box).TransformBy(LocalToWorld);
}

void UMeshCodeGizmoComponent::Init(const plateau::udx::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference) {
    const auto Extent = InMeshCode.getExtent();
    const auto RawMin = InGeoReference.project(Extent.min);
    Min.X = RawMin.x;
    Min.Y = RawMin.y;
    const auto RawMax = InGeoReference.project(Extent.max);
    Max.X = RawMax.x;
    Max.Y = RawMax.y;
    Color = FColor::Green;
    if (InMeshCode.get().size() == 6)
        LineThickness = 8.0f;
    else
        LineThickness = 3.0f;
}

FPrimitiveSceneProxy* UMeshCodeGizmoComponent::CreateSceneProxy() {
    /* Represents a UMeshCodeGizmoComponent to the scene manager. */
    class FMeshCodeGizmoSceneProxy final : public FPrimitiveSceneProxy {
    public:
        SIZE_T GetTypeHash() const override {
            static size_t UniquePointer;
            return reinterpret_cast<size_t>(&UniquePointer);
        }

        FMeshCodeGizmoSceneProxy(const UMeshCodeGizmoComponent* InComponent)
            : FPrimitiveSceneProxy(InComponent)
            , Min(InComponent->Min)
            , Max(InComponent->Max)
            , Color(InComponent->Color)
            , LineThickness(InComponent->LineThickness) {
            bWillEverBeLit = false;
        }

        virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override {
            QUICK_SCOPE_CYCLE_COUNTER(STAT_BoxSceneProxy_GetDynamicMeshElements);

            const FMatrix& LocalToWorld = GetLocalToWorld();

            for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++) {
                if (VisibilityMap & (1 << ViewIndex)) {
                    const FSceneView* View = Views[ViewIndex];

                    const FLinearColor DrawColor = GetViewSelectionColor(Color, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

                    FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
                    FVector BoxMin(Min.X, Min.Y, -0.1f);
                    FVector BoxMax(Max.X, Max.Y, 0.1f);
                    FVector Center = (BoxMin + BoxMax) / 2;
                    FVector BoxExtent = (BoxMax - BoxMin) / 2;
                    DrawOrientedWireBox(PDI,
                        Center,
                        LocalToWorld.GetScaledAxis(EAxis::X),
                        LocalToWorld.GetScaledAxis(EAxis::Y),
                        LocalToWorld.GetScaledAxis(EAxis::Z),
                        BoxExtent, DrawColor, SDPG_World, LineThickness);
                }
            }
        }

        virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override {
            // Should we draw this because collision drawing is enabled, and we have collision
            const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

            FPrimitiveViewRelevance Result;
            Result.bDrawRelevance = IsShown(View) || bShowForCollision;
            Result.bDynamicRelevance = true;
            Result.bShadowRelevance = IsShadowCast(View);
            Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
            return Result;
        }
        virtual uint32 GetMemoryFootprint(void) const override {
            return(sizeof(*this) + GetAllocatedSize());
        }
        uint32 GetAllocatedSize(void) const {
            return(FPrimitiveSceneProxy::GetAllocatedSize());
        }

    private:
        const FVector2D Min;
        const FVector2D Max;

        const FColor Color;
        const float LineThickness;
    };

    return new FMeshCodeGizmoSceneProxy(this);
}

FCollisionShape UMeshCodeGizmoComponent::GetCollisionShape(float Inflation) const {
    FVector Extent = FVector(1000, 1000, 1000) + Inflation;
    if (Inflation < 0.0f) {
        // Don't shrink below zero size.
        Extent = Extent.ComponentMax(FVector::ZeroVector);
    }
    return FCollisionShape::MakeBox(Extent);
}
