// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUSceneComponent.h"

UPLATEAUSceneComponent::UPLATEAUSceneComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPLATEAUSceneComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPLATEAUSceneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

