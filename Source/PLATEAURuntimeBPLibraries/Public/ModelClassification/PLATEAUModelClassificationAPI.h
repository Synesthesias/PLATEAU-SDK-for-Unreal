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
    static TSet<FString> SearchAttributeKeys(const TArray<USceneComponent*> TargetComponents);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static TSet<FString> SearchAttributeStringValuesFromKey(const TArray<USceneComponent*> TargetComponents, FString Key);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static TArray<FString> SortAttributeStringValues(const TArray<FString> InStrings);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static TSet<EPLATEAUCityObjectsType> SearchTypes(const TArray<USceneComponent*> TargetComponents);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static void ClassifyByType(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelClassificationAPI")
    static void ClassifyByAttribute(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, FString AttributeKey, TMap<FString, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal);
};
