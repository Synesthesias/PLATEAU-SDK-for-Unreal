// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelReconstructBtn.generated.h"

class APLATEAUInstancedCityModel;
class UPLATEAUCityObjectGroup;

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUModelReconstructBtn : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructPanel")
    static TArray<UActorComponent*> GetSelectedComponents(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructPanel")
    static TArray<UActorComponent*> GetSelectedComponentsByClass(AActor* Actor, UClass* Class);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ReconstructPanel")
    static void ReconstructModel(APLATEAUInstancedCityModel* TargetCityModel, TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const uint8 ReconstructType, bool bDivideGrid, bool bDestoroyOriginal);
};
