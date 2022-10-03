// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/ShapeComponent.h"
#include "plateau/geometry/geo_reference.h"

#include "plateau/udx/mesh_code.h"

#include "MeshCodeGizmoComponent.generated.h"

class FPrimitiveSceneProxy;

/**
 * A box generally used for simple collision. Bounds are rendered as lines in the editor.
 */
UCLASS(hidecategories = (Object, LOD, Lighting, TextureStreaming), editinlinenew)
class PLATEAURUNTIME_API UMeshCodeGizmoComponent : public UPrimitiveComponent {
    GENERATED_UCLASS_BODY()

protected:
    /** The extents (radii dimensions) of the box **/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape)
        FVector2D Min;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape)
        FVector2D Max;

    /** Used to control the line thickness when rendering */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape)
        FColor Color;

    /** Used to control the line thickness when rendering */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = Shape)
        float LineThickness;

public:
    //~ Begin UPrimitiveComponent Interface.
    virtual bool IsZeroExtent() const override;
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual struct FCollisionShape GetCollisionShape(float Inflation = 0.0f) const override;
    //~ End UPrimitiveComponent Interface.

    //~ Begin USceneComponent Interface
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    //~ End USceneComponent Interface

    void Init(const plateau::udx::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference);
};
