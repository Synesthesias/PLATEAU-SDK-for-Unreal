// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportModelBtn.generated.h"

class APLATEAUCityModelLoader;


USTRUCT(BlueprintType)
struct FPackageInfoSettings {
    GENERATED_BODY()

    FPackageInfoSettings(): bImport(false), bTextureImport(false), MinLod(0), MaxLod(0), Granularity(0) {
    }

    FPackageInfoSettings(const bool InbImport, const bool InbTextureImport, const int InMinLod, const int InMaxLod, const int InGranularity):
        bImport(InbImport), bTextureImport(InbTextureImport), MinLod(InMinLod), MaxLod(InMaxLod), Granularity(InGranularity) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    bool bImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    bool bTextureImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    int MinLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    int MaxLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    int Granularity;
};

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUImportModelBtn : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static APLATEAUCityModelLoader* GetCityModelLoader(const int ZoneID, const FVector& ReferencePoint, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData, const bool bImportFromServer);
};
