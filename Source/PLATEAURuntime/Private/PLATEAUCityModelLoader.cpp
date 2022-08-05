// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "citygml/citygml.h"

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

FPLATEAUCityModel APLATEAUCityModelLoader::LoadCityModel(int GmlIndex) {
    if (CityModelCache.Contains(GmlIndex)) {
        return CityModelCache[GmlIndex];
    }

    citygml::ParserParams params;
    params.tesselate = false;
    const auto RelativeGmlPath = ImportData->ImportedCityModelInfoArray[GmlIndex].GmlFilePath;
    const auto FullGmlPath = FPaths::ProjectContentDir() + "PLATEAU/" + RelativeGmlPath;
    const auto CityModelData = citygml::load(TCHAR_TO_UTF8(*FullGmlPath), params);
    CityModelCache.Add(GmlIndex, FPLATEAUCityModel(CityModelData));
    return CityModelCache[GmlIndex];
}
