// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelClassificationAPI.generated.h"

class APLATEAUInstancedCityModel;

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUModelClassificationAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static TSet<EPLATEAUCityObjectsType> SearchTypes(const TArray<USceneComponent*> TargetComponents);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static void ClassifyByType(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, TMap<uint8, UMaterialInterface*> Materials, const uint8 ReconstructType, bool bDestroyOriginal);
};
