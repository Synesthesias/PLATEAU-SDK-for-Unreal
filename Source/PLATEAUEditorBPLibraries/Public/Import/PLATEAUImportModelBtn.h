// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportModelBtn.generated.h"

class APLATEAUCityModelLoader;
enum class EPLATEAUTexturePackingResolution : uint8;

USTRUCT(BlueprintType)
struct FPackageInfoSettings {
    GENERATED_BODY()

    FPackageInfoSettings()
        : bImport(false)
        , bTextureImport(false)
        , bIncludeAttrInfo(false)
        , bEnableTexturePacking(false)
        , TexturePackingResolution(static_cast<EPLATEAUTexturePackingResolution>(1))
        , MinLod(0)
        , MaxLod(0)
        , Granularity(0)
        , FallbackMaterial(nullptr)
        , bAttachMapTile(false)
        , MapTileUrl("")
        , ZoomLevel(7) {
    }

    FPackageInfoSettings(
        const bool InbImport,
        const bool InbTextureImport,
        const bool InbIncludeAttrInfo,
        const bool InbEnableTexturePacking,
        const EPLATEAUTexturePackingResolution InTexturePackingResolution,
        const int InMinLod,
        const int InMaxLod,
        const int InGranularity,
        UMaterialInterface* InFallbackMaterial,
        const bool InbAttachMapTile,
        const FString InMapTileUrl,
        const int InZoomLevel)
        : bImport(InbImport)
        , bTextureImport(InbTextureImport)
        , bIncludeAttrInfo(InbIncludeAttrInfo)
        , bEnableTexturePacking(InbEnableTexturePacking)
        , TexturePackingResolution(static_cast<EPLATEAUTexturePackingResolution>(InTexturePackingResolution))
        , MinLod(InMinLod)
        , MaxLod(InMaxLod)
        , Granularity(InGranularity)
        , FallbackMaterial(InFallbackMaterial)
        , bAttachMapTile(InbAttachMapTile)
        , MapTileUrl(InMapTileUrl)
        , ZoomLevel(InZoomLevel) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bTextureImport;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bIncludeAttrInfo;    

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bEnableTexturePacking;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    EPLATEAUTexturePackingResolution TexturePackingResolution;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int MinLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int MaxLod;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int Granularity;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    UMaterialInterface* FallbackMaterial;

    /*
    * @brief 地図タイルを付与するかどうかを指定します。地形パッケージでのみ使用されます。
    */
    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    bool bAttachMapTile;

    /*
    * @brief 地図タイルのURLを指定します。地形パッケージでのみ使用されます。
    */
    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    FString MapTileUrl;

    /*
    * @brief 地図タイルのズームレベルを指定します。地形パッケージでのみ使用されます。
    */
    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|ImportPanel")
    int ZoomLevel;
};

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUImportModelBtn : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static APLATEAUCityModelLoader* GetCityModelLoader(const int ZoneID, const FVector& ReferencePoint, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData, const bool bImportFromServer);
};