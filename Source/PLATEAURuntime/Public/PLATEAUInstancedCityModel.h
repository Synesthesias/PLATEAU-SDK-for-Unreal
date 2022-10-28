// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "GameFramework/Actor.h"
#include "PLATEAUInstancedCityModel.generated.h"


UCLASS()
class PLATEAURUNTIME_API APLATEAUInstancedCityModel : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLATEAUInstancedCityModel();

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FPLATEAUGeoReference GeoReference;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
