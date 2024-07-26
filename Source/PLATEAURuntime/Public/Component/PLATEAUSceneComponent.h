// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PLATEAUComponentInterface.h"
#include "PLATEAUSceneComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLATEAURUNTIME_API UPLATEAUSceneComponent : public USceneComponent, public IPLATEAUComponentInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPLATEAUSceneComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
