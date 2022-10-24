// Fill out your copyright notice in the Description page of Project Settings.

#include "ExtentEditor/SPLATEAUExtentEditorViewport.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#include "AdvancedPreviewScene.h"
#include "SSubobjectEditor.h"
#include "Slate/SceneViewport.h"
#include "BlueprintEditorSettings.h"
#include "SlateOptMacros.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/basemap/TileProjection.h"
#include "plateau/basemap/VectorTileDownloader.h"

#include <filesystem>
#include <fstream>
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "AssetRegistryModule.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AssetSelection.h"


#define LOCTEXT_NAMESPACE "SPLATEAUExtentEditorViewport"

SPLATEAUExtentEditorViewport::SPLATEAUExtentEditorViewport()
    : PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()))) {}

SPLATEAUExtentEditorViewport::~SPLATEAUExtentEditorViewport() {
    if (ViewportClient.IsValid()) {
        ViewportClient->Viewport = nullptr;
    }
}

void SPLATEAUExtentEditorViewport::Construct(const FArguments& InArgs) {
    ExtentEditorPtr = InArgs._ExtentEditor;

    SEditorViewport::Construct(SEditorViewport::FArguments());

    if (ViewportClient.IsValid()) {
        UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
        if (World != nullptr) {
            World->ChangeFeatureLevel(GWorld->FeatureLevel);
        }
        std::shared_ptr<plateau::udx::UdxFileCollection> FileCollection;
        const auto& SourcePath = ExtentEditorPtr.Pin()->GetSourcePath();
        try {
            FileCollection = plateau::udx::UdxFileCollection::find(TCHAR_TO_UTF8(*SourcePath));
        }
        catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Failed to open udx source path: %s"), *SourcePath);
        }

        if (FileCollection == nullptr)
            return;

        if (FileCollection->getMeshCodes().size() == 0)
            return;

        auto GeoReference = ExtentEditorPtr.Pin()->GetGeoReference();
        const auto RawCenterPoint = FileCollection->calculateCenterPoint(GeoReference.GetData());
        GeoReference.ReferencePoint.X = RawCenterPoint.x;
        GeoReference.ReferencePoint.Y = RawCenterPoint.y;
        GeoReference.ReferencePoint.Z = RawCenterPoint.z;
        ExtentEditorPtr.Pin()->SetGeoReference(GeoReference);

        ViewportClient->Initialize(*FileCollection);
    }
}

bool SPLATEAUExtentEditorViewport::IsVisible() const {
    // We consider the viewport to be visible if the reference is valid
    return ViewportWidget.IsValid() && SEditorViewport::IsVisible();
}

TSharedRef<FEditorViewportClient> SPLATEAUExtentEditorViewport::MakeEditorViewportClient() {
    // Construct a new viewport client instance.
    ViewportClient = MakeShareable(
        new FPLATEAUExtentEditorViewportClient(
            ExtentEditorPtr,
            SharedThis(this),
            PreviewScene.ToSharedRef()));
    ViewportClient->SetRealtime(true);
    ViewportClient->bSetListenerPosition = false;
    ViewportClient->VisibilityDelegate.BindSP(this, &SPLATEAUExtentEditorViewport::IsVisible);

    return ViewportClient.ToSharedRef();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUExtentEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) {
    SEditorViewport::PopulateViewportOverlays(Overlay);

    // add the feature level display widget
    Overlay->AddSlot()
        .VAlign(VAlign_Top)
        .HAlign(HAlign_Left)
        .Padding(5.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("FloatingBorder"))
        .Padding(4.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                if (GetOwnerTab())
                    GetOwnerTab()->RequestCloseTab();
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Cancel Button", "キャンセル"))
                ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                const auto Extent = ViewportClient->GetExtent();
                ExtentEditorPtr.Pin()->SetExtent(Extent);
                ExtentEditorPtr.Pin()->HandleClickOK();
                AttachVectorTile(Extent);

                //if (GetOwnerTab())
                //    GetOwnerTab()->RequestCloseTab();
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("OK Button", "決定"))
                ]
        ]
        ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUExtentEditorViewport::BindCommands() {}

void SPLATEAUExtentEditorViewport::SetOwnerTab(TSharedRef<SDockTab> Tab) {
    OwnerTab = Tab;
}

TSharedPtr<SDockTab> SPLATEAUExtentEditorViewport::GetOwnerTab() const {
    return OwnerTab.Pin();
}

void SPLATEAUExtentEditorViewport::AttachVectorTile(FPLATEAUExtent Extent) {
    auto Fpath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("\\PLATEAU\\images"));
    auto str = std::string(TCHAR_TO_UTF8(*Fpath));

    TileProjection tileProjection;
    VectorTileDownloader Downloader("http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png");
    const auto Coordinates = tileProjection.getTileCoordinates(Extent.GetNativeData(), 15);
    for (auto Cooridinate : *Coordinates){
        auto Tile = Downloader.download(str, Cooridinate);

        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
        TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
        TArray<uint8> RawFileData;

        int i = 0;
        if (FFileHelper::LoadFileToArray(RawFileData, *Fpath))
        {
            // 非圧縮の画像データを取得
            TArray<uint8> UncompressedRawData;
            if (ImageWrapper.IsValid() &&
                ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()) &&
                ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedRawData)
                )
            {
                FString Filename = FPaths::GetBaseFilename(Fpath);
                int Width = ImageWrapper->GetWidth();
                int Height = ImageWrapper->GetHeight();

                // パッケージを作成
                FString PackagePath(TEXT("/Game/LoadedTexture/"));
                FString AbsolutePackagePath = FPaths::ProjectContentDir() + TEXT("/LoadedTexture/");

                FPackageName::RegisterMountPoint(PackagePath, AbsolutePackagePath);

                PackagePath += Filename;

                UPackage* Package = CreatePackage(nullptr, *PackagePath);
                Package->FullyLoad();

                // テクスチャを作成
                FName TextureName = MakeUniqueObjectName(Package, UTexture2D::StaticClass(), FName(*Filename));
                UTexture2D* Texture = NewObject<UTexture2D>(Package, TextureName, RF_Public | RF_Standalone);

                // テクスチャの設定
                Texture->PlatformData = new FTexturePlatformData();
                Texture->PlatformData->SizeX = Width;
                Texture->PlatformData->SizeY = Height;
                Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
                Texture->NeverStream = false;

                // ピクセルデータをテクスチャに書き込む
                FTexture2DMipMap* Mip = new FTexture2DMipMap();
                Texture->PlatformData->Mips.Add(Mip);
                Mip->SizeX = Width;
                Mip->SizeY = Height;
                Mip->BulkData.Lock(LOCK_READ_WRITE);
                uint8* TextureData = (uint8*)Mip->BulkData.Realloc(UncompressedRawData.Num());
                FMemory::Memcpy(TextureData, UncompressedRawData.GetData(), UncompressedRawData.Num());
                Mip->BulkData.Unlock();

                // テクスチャを更新
                Texture->AddToRoot();
                Texture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedRawData.GetData());
                Texture->UpdateResource();

                FName meshName = MakeUniqueObjectName(Package, UStaticMeshComponent::StaticClass(), FName(*Filename));
                UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Package, meshName, RF_Public | RF_Standalone);
                auto m = UMaterialInstanceDynamic::Create(Cast< UMaterial >(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"))), Package);
                m->SetTextureParameterValue(TEXT("sample"), Texture);
                MeshComponent->SetMaterial(0, m);
                auto mesh = Cast< UStaticMesh >(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/Plane")));
                mesh->SetMaterial(0, m);
                MeshComponent->SetStaticMesh(mesh);

                FAssetData EmptyActorAssetData = FAssetData(MeshComponent);
                UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
                auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);


                ViewportClient->GetPreviewScene()->AddComponent(MeshComponent, FTransform(FVector(i, 0, 0)));
                i += 1000;
            }
        }

    }
}


#undef LOCTEXT_NAMESPACE
