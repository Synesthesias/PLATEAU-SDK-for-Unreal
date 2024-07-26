// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Component/PLATEAULandscapeRefComponent.h"

UPLATEAULandscapeRefComponent::UPLATEAULandscapeRefComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPLATEAULandscapeRefComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPLATEAULandscapeRefComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

