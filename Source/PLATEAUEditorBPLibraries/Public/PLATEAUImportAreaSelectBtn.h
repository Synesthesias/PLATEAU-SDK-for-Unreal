// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportAreaSelectBtn.generated.h"

class UPLATEAUSDKEditorUtilityWidget;


USTRUCT(BlueprintType)
struct FPackageInfo {
    GENERATED_BODY()

    FPackageInfo() {
    }
    
    FPackageInfo(const bool InHasAppearance, const int InMinLod, const int InMaxLod) : HasAppearance(InHasAppearance), MinLod(InMinLod), MaxLod(InMaxLod) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    bool HasAppearance = true;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    int MinLod = 0;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    int MaxLod = 3;
};

USTRUCT(BlueprintType)
struct FPackageInfoSettings {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    bool bImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    bool bTextureImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    int MinLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    int MaxLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    int Granularity;
};

UCLASS()
class UPLATEAUImportAreaSelectBtn : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static void OpenAreaWindow(const int ZoneID, const FString& SourcePath, const bool bImportFromServer);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static TArray<FText> GetGranularityTexts();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static TMap<int64, FText> GetCategoryNames();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static FPackageInfo GetPackageInfo(const int64 Package);
};
