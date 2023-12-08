// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUExportModelAPI.generated.h"

class APLATEAUInstancedCityModel;

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUExportModelAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ExportAPI")
    static void ExportModel(APLATEAUInstancedCityModel* TargetCityModel, const FString& ExportPath, const FPLATEAUMeshExportOptions& Options);

};
