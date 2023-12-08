// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelClassificationAPI.generated.h"

class APLATEAUInstancedCityModel;

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUModelClassificationAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static TSet<EPLATEAUCityObjectsType> SearchTypes(const TArray<USceneComponent*> TargetComponents);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static void ClassifyByType(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal);
};
