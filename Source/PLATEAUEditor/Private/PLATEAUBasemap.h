// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

struct FPLATEAUExtent;

// TODO: 名前空間
struct TileCoordinate;

struct FPLATEAUTileCoordinate {
    int Column;
    int Row;
    int ZoomLevel;

    static FPLATEAUTileCoordinate FromNativeData(const TileCoordinate& Data);
    TileCoordinate ToNativeData() const;

    bool operator==(const FPLATEAUTileCoordinate& Other) const;
    bool operator!=(const FPLATEAUTileCoordinate& Other) const;
};

struct FPLATEAUAsyncLoadedVectorTile {
public:
    //FPLATEAUAsyncLoadedVectorTile()
    //    : IsFullyLoaded(false)
    //    , TileComponent(nullptr) {}

    bool GetFullyLoaded() {
        FScopeLock Lock(&CriticalSection);
        return IsFullyLoaded;
    }

    UStaticMeshComponent* GetComponent() {
        FScopeLock Lock(&CriticalSection);
        return TileComponent;
    }

    void LoadAsync(const FPLATEAUTileCoordinate& InTileCoordinate);
    
private:
    FCriticalSection CriticalSection;
    bool IsFullyLoaded;
    UStaticMeshComponent* TileComponent;
};

uint32 GetTypeHash(const FPLATEAUTileCoordinate& Value);

/**
 *
 */
class FPLATEAUBasemap {
public:
    FPLATEAUBasemap(const FPLATEAUGeoReference& InGeoReference, const TSharedPtr<class FPLATEAUExtentEditorViewportClient> InViewportClient);
    ~FPLATEAUBasemap();

    void UpdateAsync(const FPLATEAUExtent& InExtent);

private:
    FPLATEAUGeoReference GeoReference;
    TWeakPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    TMap<FPLATEAUTileCoordinate, TSharedPtr<FPLATEAUAsyncLoadedVectorTile>> AsyncLoadedTiles;
    TSet<UStaticMeshComponent*> TilesInScene;
};
