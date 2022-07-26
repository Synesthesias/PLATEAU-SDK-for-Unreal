// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
    CityModelPlacementSettings.BuildingPlacementSettings.TargetLOD = 3;
    CityModelPlacementSettings.RoadPlacementSettings.TargetLOD = 3;
    CityModelPlacementSettings.UrbanFacilityPlacementSettings.TargetLOD = 3;
    CityModelPlacementSettings.ReliefPlacementSettings.TargetLOD = 3;
    CityModelPlacementSettings.VegetationPlacementSettings.TargetLOD = 3;
    CityModelPlacementSettings.OtherPlacementSettings.TargetLOD = 3;
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();

}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
