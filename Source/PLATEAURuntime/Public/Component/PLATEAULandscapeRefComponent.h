// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "PLATEAUComponentInterface.h"
#include "PLATEAULandscapeRefComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLATEAURUNTIME_API UPLATEAULandscapeRefComponent : public UPLATEAUCityObjectGroup, public IPLATEAUComponentInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
    UPLATEAULandscapeRefComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
