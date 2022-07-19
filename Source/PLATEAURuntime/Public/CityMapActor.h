// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityMapMetadata.h"
#include "GameFramework/Actor.h"
#include "CityMapActor.generated.h"

UCLASS()
class PLATEAURUNTIME_API ACityMapActor : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACityMapActor();

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        UCityMapMetadata* Metadata;

    UFUNCTION(CallInEditor)
        void PlaceCityModels();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;


public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
