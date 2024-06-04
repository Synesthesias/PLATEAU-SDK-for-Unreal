// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PLATEAUComponentInterface.h"
#include "PLATEAUStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUStaticMeshComponent : public UStaticMeshComponent, public IPLATEAUComponentInterface
{
	GENERATED_BODY()
	
};
