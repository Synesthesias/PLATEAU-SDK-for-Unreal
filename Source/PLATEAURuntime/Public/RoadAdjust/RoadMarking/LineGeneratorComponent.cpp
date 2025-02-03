// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/LineGeneratorComponent.h"
#include "Kismet/KismetMathLibrary.h"

ULineGeneratorComponent::ULineGeneratorComponent() :SplinePointType(ESplinePointType::Linear), CoordinateSpace(ESplineCoordinateSpace::Local){
    this->SetDrawDebug(false);
    this->bInputSplinePointsToConstructionScript = false;
}

float ULineGeneratorComponent::GetMeshLength(bool includeGap)
{
	if (StaticMesh == nullptr)
		return 0.f;

    float Length = MeshLength != 0.f ? MeshLength : StaticMesh->GetBounds().SphereRadius;
	//float Length = MeshLength != 0.f ? MeshLength : StaticMesh->GetBounds().BoxExtent.X;

	if (includeGap)
	{
		return Length + MeshGap;
	}

	return Length;
}

void ULineGeneratorComponent::CreateSplineFromVectorArray(TArray<FVector> Points)
{
	//Draw Spline
	this->ClearSplinePoints();

	for (const auto& Point : Points) {
		this->AddSplinePoint(Point, CoordinateSpace, true);
	}

    for (int32 i = 0; i < this->GetNumberOfSplinePoints(); i++) {
        this->SetSplinePointType(i, SplinePointType, true);
    }
}

void ULineGeneratorComponent::CreateSplineMesh(AActor* Actor)
{
	UE_LOG(LogTemp, Log, TEXT("CreateSplineMesh"));

	if (StaticMesh == nullptr)
		return;

	if (SplineMeshRoot == nullptr) {
		SplineMeshRoot = NewObject<USceneComponent>(this, FName(TEXT("SplineMeshRoot")));
		//SplineMeshRoot->SetMobility(EComponentMobility::Static);
		Actor->AddInstanceComponent(SplineMeshRoot);
		SplineMeshRoot->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		SplineMeshRoot->RegisterComponent();
	}

	//Delete old components
	TArray<USceneComponent*> SplineMeshChildren;
	SplineMeshRoot->GetChildrenComponents(true, SplineMeshChildren);
	for (auto Comp : SplineMeshChildren)
	{
		Comp->DestroyComponent();
	}


	float SplineLength = this->GetSplineLength();
	float Length = GetMeshLength(true);

	int64 numLoop = (int64)(SplineLength / Length);

	UE_LOG(LogTemp, Log, TEXT("num meshes : %d"), numLoop);

	//Add Spline Mesh
	for (int64 index = 0; index < numLoop; index++)
	{
		//auto SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(FName(TEXT("SplineMeshComponent")));
		auto SplineMeshComponent = NewObject<USplineMeshComponent>(this, FName(TEXT("SplineMesh_") + FString::FromInt(index)));
		Actor->AddInstanceComponent(SplineMeshComponent);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable);
		SplineMeshComponent->RegisterComponent();
		SplineMeshComponent->AttachToComponent(SplineMeshRoot, FAttachmentTransformRules::KeepRelativeTransform);

		SplineMeshComponent->SetStaticMesh(StaticMesh);
		if (MaterialInterface != nullptr)
		{
			SplineMeshComponent->SetMaterial(0, MaterialInterface);
		}

		float startDistance = Length * index;
		const auto& startLocation = this->GetLocationAtDistanceAlongSpline(startDistance, CoordinateSpace);
		const auto& startTangent = UKismetMathLibrary::Normal(this->GetTangentAtDistanceAlongSpline(startDistance, CoordinateSpace));

		float endDistance = (Length * (index + 1)) - MeshGap;
		const auto& endLocation = this->GetLocationAtDistanceAlongSpline(endDistance, CoordinateSpace);
		const auto& endTangent = UKismetMathLibrary::Normal(this->GetTangentAtDistanceAlongSpline(endDistance, CoordinateSpace));

		SplineMeshComponent->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent, true);
        SplineMeshComponent->SetStartScale(FVector2D(MeshXScale, 1.0f));
        SplineMeshComponent->SetEndScale(FVector2D(MeshXScale, 1.0f));
	}

}

void ULineGeneratorComponent::CreateSplineMeshFromAssets(AActor* Actor, UStaticMesh* Mesh, UMaterialInterface* Material, float Gap, float XScale, float Length)
{
	StaticMesh = Mesh;
	MaterialInterface = Material;
	MeshGap = Gap;
    MeshXScale = XScale;
    MeshLength = Length;
	CreateSplineMesh(Actor);
}