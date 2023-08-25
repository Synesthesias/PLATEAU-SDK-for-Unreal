// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportModelBtn.generated.h"

class APLATEAUCityModelLoader;


USTRUCT(BlueprintType)
struct FPackageInfoSettings {
    GENERATED_BODY()

    FPackageInfoSettings(): bImport(false), bTextureImport(false), bIncludeAttrInfo(true), MinLod(0), MaxLod(0), Granularity(0) {
    }

    FPackageInfoSettings(const bool InbImport, const bool InbTextureImport, const bool InbIncludeAttrInfo, const int InMinLod, const int InMaxLod, const int InGranularity):
        bImport(InbImport), bTextureImport(InbTextureImport), bIncludeAttrInfo(InbIncludeAttrInfo), MinLod(InMinLod), MaxLod(InMaxLod), Granularity(InGranularity) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bTextureImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bIncludeAttrInfo;    

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int MinLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int MaxLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int Granularity;
};

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUImportModelBtn : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static APLATEAUCityModelLoader* GetCityModelLoader(const int ZoneID, const FVector& ReferencePoint, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData, const bool bImportFromServer);
};
