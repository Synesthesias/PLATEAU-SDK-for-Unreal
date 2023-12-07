// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelReconstructAPI.generated.h"

class APLATEAUInstancedCityModel;
class UPLATEAUCityObjectGroup;

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUModelReconstructAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructAPI")
    static TArray<UActorComponent*> GetSelectedComponents(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructAPI")
    static TArray<UActorComponent*> GetSelectedComponentsByClass(AActor* Actor, UClass* Class);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|BPLibraries|ReconstructAPI")
    static EPLATEAUMeshGranularity GetMeshGranularityFromIndex(int index);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructAPI")
    static void ReconstructModel(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal);
};
