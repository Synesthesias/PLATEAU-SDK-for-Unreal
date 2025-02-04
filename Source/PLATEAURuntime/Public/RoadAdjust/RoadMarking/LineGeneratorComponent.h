// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "LineGeneratorComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API ULineGeneratorComponent : public USplineComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
	UMaterialInterface* MaterialInterface;

	UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
	float MeshGap;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
    float MeshXScale;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
    float MeshLength;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|RoadAdjust")
    FVector2D Offset;

	UFUNCTION(BlueprintCallable, Category = "PLATEAU|RoadAdjust")
	void CreateSplineFromVectorArray(TArray<FVector> Points);

	UFUNCTION(BlueprintCallable, Category = "PLATEAU|RoadAdjust")
	void CreateSplineMesh(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "PLATEAU|RoadAdjust")
	void CreateSplineMeshFromAssets(AActor* Actor, UStaticMesh* Mesh, UMaterialInterface* Material, float Gap, float XScale, float Length = 0.f);

    ULineGeneratorComponent();

private:

	float GetMeshLength(bool includeGap);

	USceneComponent* SplineMeshRoot;

    ESplinePointType::Type SplinePointType;
	ESplineCoordinateSpace::Type CoordinateSpace;
	
};
