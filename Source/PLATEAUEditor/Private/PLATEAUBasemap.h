// Copyright © 2023 Ministry of Land, Infrastructure and Transport

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
    FPLATEAUAsyncLoadedVectorTile()
        : IsFullyLoaded(false)
        , TileComponent(nullptr) {}

    ~FPLATEAUAsyncLoadedVectorTile()
    {
        if (IsLoading && !IsFullyLoaded)
            Task.Wait();
    }

    bool GetFullyLoaded() {
        return IsFullyLoaded;
    }

    UStaticMeshComponent* GetComponent() {
        FScopeLock Lock(&CriticalSection);
        return TileComponent;
    }

    void StartLoading(const FPLATEAUTileCoordinate& InTileCoordinate);

private:
    FCriticalSection CriticalSection;
    std::atomic<bool> IsFullyLoaded;
    std::atomic<bool> IsLoading;
    UStaticMeshComponent* TileComponent;
    TFuture<void> Task;
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
