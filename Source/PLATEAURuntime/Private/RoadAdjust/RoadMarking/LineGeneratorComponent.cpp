// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/LineGeneratorComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

ULineGeneratorComponent::ULineGeneratorComponent() :
    SplineMeshType(ESplineMeshType::LengthBased), 
    SplinePointType(ESplinePointType::Linear), 
    CoordinateSpace(ESplineCoordinateSpace::Local),  
    FillEnd(false),
    EnableShadow(false),  
    SplineMeshRoot(nullptr) {
    this->SetDrawDebug(false);
    this->bInputSplinePointsToConstructionScript = false;
    this->SetMobility(EComponentMobility::Static);
}

float ULineGeneratorComponent::GetMeshLength(bool includeGap) {
	if (StaticMesh == nullptr)
		return 0.f;

    float Length = MeshLength != 0.f ? MeshLength : StaticMesh->GetBounds().SphereRadius;
    //UE_LOG(LogTemp, Log, TEXT("Mesh BoxExtent : ( %f , %f , %f )"), StaticMesh->GetBounds().BoxExtent.X, StaticMesh->GetBounds().BoxExtent.Y, StaticMesh->GetBounds().BoxExtent.Z);

	if (includeGap)
	{
		return Length + MeshGap;
	}

	return Length;
}

void ULineGeneratorComponent::CreateSplineFromVectorArray(TArray<FVector> Points) {
	this->ClearSplinePoints();

	for (const auto& Point : Points) {
		this->AddSplinePoint(Point, CoordinateSpace, true);
	}

    RefreshSplinePoints();
}

void ULineGeneratorComponent::RefreshSplinePoints() {
    for (int32 i = 0; i < this->GetNumberOfSplinePoints(); i++) {
        this->SetSplinePointType(i, SplinePointType, true);
    }
}

void ULineGeneratorComponent::CreateSplineMesh(AActor* Actor) {

	if (StaticMesh == nullptr)
		return;

	if (SplineMeshRoot == nullptr) {
		SplineMeshRoot = NewObject<USceneComponent>(this, FName(TEXT("SplineMeshRoot")));
        SplineMeshRoot->SetMobility(EComponentMobility::Static);
		SplineMeshRoot->RegisterComponent();
        SplineMeshRoot->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(SplineMeshRoot);
	}

	//Delete old components
	TArray<USceneComponent*> SplineMeshChildren;
	SplineMeshRoot->GetChildrenComponents(true, SplineMeshChildren);
	for (auto Comp : SplineMeshChildren)
	{
		Comp->DestroyComponent();
	}

    if (SplineMeshType == ESplineMeshType::LengthBased)
        CreateSplineMeshLengthBased(Actor);
    else if (SplineMeshType == ESplineMeshType::SegmentBased)
        CreateSplineMeshSegmentBased(Actor);
}

void ULineGeneratorComponent::CreateSplineMeshLengthBased(AActor* Actor) {
    float SplineLength = this->GetSplineLength();
    float Length = GetMeshLength(true);

    int64 numLoop = (int64)(SplineLength / Length);
    //Add Spline Mesh
    for (int64 index = 0; index < numLoop; index++) {
        float startDistance = Length * index;
        const auto& startLocation = this->GetLocationAtDistanceAlongSpline(startDistance, CoordinateSpace);
        const auto& startTangent = UKismetMathLibrary::Normal(this->GetTangentAtDistanceAlongSpline(startDistance, CoordinateSpace));

        float endDistance = (Length * (index + 1)) - MeshGap;

        //Last Mesh & FillEnd 
        if (index == numLoop - 1 && FillEnd) {
            endDistance = this->GetSplineLength();

            if (SplinePointType == ESplinePointType::Linear)
                endDistance -= 0.01f;
        }

        const auto& endLocation = this->GetLocationAtDistanceAlongSpline(endDistance, CoordinateSpace);
        const auto& endTangent = UKismetMathLibrary::Normal(this->GetTangentAtDistanceAlongSpline(endDistance, CoordinateSpace));

        CreateSplineMeshComponent(FName(TEXT("SplineMesh_") + FString::FromInt(index)), Actor, startLocation, startTangent, endLocation, endTangent);
    }
}

void ULineGeneratorComponent::CreateSplineMeshSegmentBased(AActor* Actor) {

    int64 numLoop = this->GetNumberOfSplinePoints() - 1;
    //Add Spline Mesh
    for (int64 index = 0; index < numLoop; index++) {
        const auto& startLocation = this->GetLocationAtSplinePoint(index, CoordinateSpace);
        const auto& startTangent = this->GetTangentAtSplinePoint(index, CoordinateSpace);
        const auto& endLocation = this->GetLocationAtSplinePoint(index + 1, CoordinateSpace);
        const auto& endTangent = this->GetTangentAtSplinePoint(index + 1, CoordinateSpace);
        CreateSplineMeshComponent(FName(TEXT("SplineMesh_") + FString::FromInt(index)), Actor, startLocation, startTangent, endLocation, endTangent);
    }
}

USplineMeshComponent* ULineGeneratorComponent::CreateSplineMeshComponent(FName Name, AActor* Actor, FVector StartLocation, FVector StartTangent, FVector EndLocation, FVector EndTangent) {
    auto SplineMeshComponent = NewObject<USplineMeshComponent>(this, Name);
    SplineMeshComponent->SetMobility(EComponentMobility::Static);
    SplineMeshComponent->RegisterComponent();
    SplineMeshComponent->AttachToComponent(SplineMeshRoot, FAttachmentTransformRules::KeepWorldTransform);
    Actor->AddInstanceComponent(SplineMeshComponent);

    SplineMeshComponent->SetStaticMesh(StaticMesh);
    if (MaterialInterface != nullptr) {
        SplineMeshComponent->SetMaterial(0, MaterialInterface);
    }

    SplineMeshComponent->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
    SplineMeshComponent->SetStartScale(FVector2D(MeshXScale, 1.0f));
    SplineMeshComponent->SetEndScale(FVector2D(MeshXScale, 1.0f));
    SplineMeshComponent->SetStartOffset(Offset);
    SplineMeshComponent->SetEndOffset(Offset);
    SplineMeshComponent->CastShadow = EnableShadow;
    return SplineMeshComponent;
}

void ULineGeneratorComponent::CreateSplineMeshFromAssets(AActor* Actor, UStaticMesh* Mesh, UMaterialInterface* Material, float Gap, float XScale, float Length) {
	StaticMesh = Mesh;
	MaterialInterface = Material;
	MeshGap = Gap;
    MeshXScale = XScale;
    MeshLength = Length;
	CreateSplineMesh(Actor);
}