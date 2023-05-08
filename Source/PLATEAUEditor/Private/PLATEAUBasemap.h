// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"
#include "PLATEAUGeometry.h"

using namespace UE::Tasks;

UENUM(BlueprintType)
enum class EVectorTileLoadingPhase : uint8 {
    Idle = 0,
    Loading = 1,
    FullyLoaded = 2,
    Failed = 3
};

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
        : LoadPhase(EVectorTileLoadingPhase::Idle)
        , TileComponent(nullptr) {}

    ~FPLATEAUAsyncLoadedVectorTile()
    {
        if(LoadPhase == EVectorTileLoadingPhase::Loading)
            Task.Wait();
    }

    bool GetFullyLoaded() {
        return (LoadPhase == EVectorTileLoadingPhase::FullyLoaded);
    }

    bool GetFailed() {
        return (LoadPhase == EVectorTileLoadingPhase::Failed);
    }

    UStaticMeshComponent* GetComponent() {
        FScopeLock Lock(&CriticalSection);
        return TileComponent;
    }

    void StartLoading(const FPLATEAUTileCoordinate& InTileCoordinate, FPipe& Pipe);

private:
    FCriticalSection CriticalSection;
    TAtomic<EVectorTileLoadingPhase> LoadPhase;
    UStaticMeshComponent* TileComponent;
    FTask Task;
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

    FPipe VectorTilePipe{ TEXT("VectorTilePipe") };
};
