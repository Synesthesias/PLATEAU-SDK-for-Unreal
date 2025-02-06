// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAURoadAdjustmentAPI.generated.h"

class APLATEAURnStructureModel;
class APLATEAUInstancedCityModel;
class UPLATEAUCityObjectGroup;

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAURoadAdjustmentAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|RoadAdjustmentAPI")
    static void CreateRnModel(APLATEAUInstancedCityModel* TargetCityModel, APLATEAURnStructureModel* DestActor);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|RoadAdjustmentAPI")
    static void GenerateRoadMarking(APLATEAURnStructureModel* DestActor);

};
