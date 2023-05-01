// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUBasemap.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUTextureLoader.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>

namespace {
    UStaticMeshComponent* CreateTileComponentInGameThread(UTexture* Texture) {
        UStaticMeshComponent* TileComponent;
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&] {
                //mesh component作成，テクスチャを適用
                const FName MeshName = MakeUniqueObjectName(
                    GetTransientPackage(),
                    UStaticMeshComponent::StaticClass(),
                    TEXT("Tile"));

                TileComponent =
                    NewObject<UStaticMeshComponent>(
                        GetTransientPackage(),
                        MeshName, RF_Transient);

                const auto Mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PLATEAU-SDK-for-Unreal/DefaultMaterial")));
                const auto DynMat = UMaterialInstanceDynamic::Create(Mat, GetTransientPackage());
                DynMat->SetTextureParameterValue(TEXT("Texture"), Texture);
                TileComponent->SetMaterial(0, DynMat);
                const auto StaticMeshName = TEXT("/Engine/BasicShapes/Plane");
                const auto Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, StaticMeshName));
                Mesh->AddMaterial(DynMat);
                TileComponent->SetStaticMesh(Mesh);
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();

            return TileComponent;
    }

}

FPLATEAUBasemap::FPLATEAUBasemap(
    const FPLATEAUGeoReference& InGeoReference,
    const TSharedPtr<FPLATEAUExtentEditorViewportClient> InViewportClient)
    : GeoReference(InGeoReference)
    , ViewportClient(InViewportClient) {
}

FPLATEAUBasemap::~FPLATEAUBasemap() {

}


void FPLATEAUBasemap::UpdateAsync(const FPLATEAUExtent& InExtent) {
    int ZoomLevel = 18;
    std::shared_ptr<std::vector<TileCoordinate>> TileCoordinates;
    while (true) {
        TileCoordinates = TileProjection::getTileCoordinates(InExtent.GetNativeData(), ZoomLevel);
        // 画面内のタイル数が一定数よりも低くなるようにズームレベルを設定
        if (TileCoordinates->size() <= 16 || ZoomLevel == 1)
            break;
        --ZoomLevel;
    }

    const auto Destination = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("\\PLATEAU\\Basemap"));

    for (auto& Entry : AsyncLoadedTiles) {
        if (!Entry.Value->GetFullyLoaded())
            continue;

        const auto TileComponent = Entry.Value->GetComponent();
        if (TileComponent == nullptr)
            continue;

        // 全てのタイルを非表示化する。使用するタイルだけ後で再表示
        TileComponent->SetVisibility(false);

        if (TilesInScene.Contains(TileComponent))
            continue;
        if (!ViewportClient.IsValid())
            continue;

        //タイルの座標を取得
        auto TileExtent = TileProjection::unproject(Entry.Key.ToNativeData());
        const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
        const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
        FBox Box(FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
            FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z));

        auto Extent = Box.GetExtent();
        Extent.X = FMath::Abs(Extent.X);
        Extent.Y = FMath::Abs(Extent.Y);
        Extent.Z = FMath::Abs(Extent.Z);

        ViewportClient.Pin()->GetPreviewScene()->AddComponent(
            TileComponent,
            FTransform(FRotator(0, 0, 0),
                Box.GetCenter() - FVector::UpVector,
                Extent / 50
            ));
        TilesInScene.Add(TileComponent);
    }

    for (const auto& RawTileCoordinate : *TileCoordinates) {
        auto TileCoordinate = FPLATEAUTileCoordinate::FromNativeData(RawTileCoordinate);
        if (!AsyncLoadedTiles.Find(TileCoordinate)) {
            const auto& AsyncLoadedTile = AsyncLoadedTiles.Add(TileCoordinate, MakeShared<FPLATEAUAsyncLoadedVectorTile>());
            AsyncLoadedTile->StartLoading(TileCoordinate);
            continue;
        }

        const auto& AsyncLoadedTile = AsyncLoadedTiles[TileCoordinate];
        if (AsyncLoadedTile->GetFullyLoaded()) {
            const auto TileComponent = AsyncLoadedTile->GetComponent();
            TileComponent->SetVisibility(true);
        }
    }
}

FPLATEAUTileCoordinate FPLATEAUTileCoordinate::FromNativeData(const TileCoordinate& Data) {
    FPLATEAUTileCoordinate Result{};
    Result.Column = Data.column;
    Result.Row = Data.row;
    Result.ZoomLevel = Data.zoom_level;
    return Result;
}

TileCoordinate FPLATEAUTileCoordinate::ToNativeData() const {
    TileCoordinate Result;
    Result.column = Column;
    Result.row = Row;
    Result.zoom_level = ZoomLevel;
    return Result;
}

bool FPLATEAUTileCoordinate::operator==(const FPLATEAUTileCoordinate& Other) const {
    return Column == Other.Column
        && Row == Other.Row
        && ZoomLevel == Other.ZoomLevel;
}

bool FPLATEAUTileCoordinate::operator!=(const FPLATEAUTileCoordinate& Other) const {
    return !(*this == Other);
}

uint32 GetTypeHash(const FPLATEAUTileCoordinate& Value) {
    return Value.ZoomLevel * 100000000 + Value.Row * 10000 + Value.Column;
}

void FPLATEAUAsyncLoadedVectorTile::StartLoading(const FPLATEAUTileCoordinate& InTileCoordinate) {
    IsLoading = true;
    Task = Async(EAsyncExecution::Thread,
        [this, InTileCoordinate]() {

            const auto Destination = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("\\PLATEAU\\Basemap"));

            const auto Tile = VectorTileDownloader::download(
                VectorTileDownloader::getDefaultUrl(),
                TCHAR_TO_UTF8(*Destination),
                InTileCoordinate.ToNativeData());

            const FString TexturePath = UTF8_TO_TCHAR(Tile->image_path.c_str());

            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            if (!PlatformFile.FileExists(*TexturePath)) {
                UE_LOG(LogTemp, Error, TEXT("File not exists : %s"), *TexturePath);
                IsLoading = false;
                return;
            }
            else if (PlatformFile.FileSize(*TexturePath) <= 0) {
                UE_LOG(LogTemp, Error, TEXT("File size 0 : %s"), *TexturePath, PlatformFile.FileSize(*TexturePath));
                IsLoading = false;
                return;
            }

            // テクスチャ読み込み
            const auto Texture = FPLATEAUTextureLoader::LoadTransient(*TexturePath);

            if (Texture == nullptr)
                UE_LOG(LogTemp, Error, TEXT("Load Error :  %s"), *TexturePath);

            const auto TempComponent = CreateTileComponentInGameThread(Texture);
            {
                FScopeLock Lock(&CriticalSection);
                TileComponent = TempComponent;
                IsFullyLoaded = true;
            }
            IsLoading = false;
        });
}
