// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUTextureLoader.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include <filesystem>

#include "EditorFramework/AssetImportData.h"
namespace fs = std::filesystem;

#if WITH_EDITOR

DECLARE_STATS_GROUP(TEXT("PLATEAUTextureLoader"), STATGROUP_PLATEAUTextureLoader, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Texture.UpdateResource"), STAT_Texture_UpdateResource, STATGROUP_PLATEAUTextureLoader);

namespace {
    bool TryLoadAndUncompressImageFile(const FString& TexturePath,
        TArray64<uint8>& OutUncompressedData, int32& OutWidth, int32& OutHeight, EPixelFormat& OutPixelFormat) {
        if (TexturePath.IsEmpty())
            return false;

        IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

        TArray64<uint8> Buffer;
        if (!FFileHelper::LoadFileToArray(Buffer, *TexturePath)) {
            return false;
        }

        const EImageFormat Format = ImageWrapperModule.DetectImageFormat(Buffer.GetData(), Buffer.Num());

        if (Format == EImageFormat::Invalid)
            return false;
        const auto ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

        if (!ImageWrapper->SetCompressed((void*)Buffer.GetData(), Buffer.Num()))
            return false;

        ERGBFormat RGBFormat;
        const int32 BitDepth = ImageWrapper->GetBitDepth();
        OutWidth = ImageWrapper->GetWidth();
        OutHeight = ImageWrapper->GetHeight();

        if (BitDepth == 16) {
            OutPixelFormat = PF_FloatRGBA;
            RGBFormat = ERGBFormat::RGBAF;
        } else if (BitDepth == 8) {
            OutPixelFormat = PF_B8G8R8A8;
            RGBFormat = ERGBFormat::BGRA;
        } else {
            //UE_LOG(LogImageUtils, Warning, TEXT("Error creating texture. Bit depth is unsupported. (%d)"), BitDepth);
            return false;
        }

        if(!ImageWrapper->GetRaw(RGBFormat, BitDepth, OutUncompressedData))
        {
            return false;
        }

        return true;
    }

    void UpdateTextureGPUResourceWithDummy(UTexture2D* const Texture, const EPixelFormat PixelFormat) {
        Texture->SetPlatformData(new FTexturePlatformData());
        Texture->GetPlatformData()->SizeX = 1;
        Texture->GetPlatformData()->SizeY = 1;
        Texture->GetPlatformData()->PixelFormat = PixelFormat;

        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        Texture->GetPlatformData()->Mips.Add(Mip);
        Mip->SizeX = 1;
        Mip->SizeY = 1;

        // GPixelFormats contains meta information for each pixel format 
        {
            const uint32 MipBytes = Mip->SizeX * Mip->SizeY * GPixelFormats[PixelFormat].BlockBytes;
            Mip->BulkData.Lock(LOCK_READ_WRITE);

            void* TextureData = Mip->BulkData.Realloc(MipBytes);

            static TArray<uint8> DummyBytes;
            DummyBytes.SetNum(MipBytes);

            FMemory::Memcpy(TextureData, DummyBytes.GetData(), MipBytes);

            Mip->BulkData.Unlock();
        }

        // GPU上でテクスチャ構築
        Texture->UpdateResource();
    }

    void SetTexturePlatformData(UTexture2D* Texture, const TArray64<uint8>& UncompressedImageData,
        const int32 Mip0Size, const int32 Width, const int32 Height, const EPixelFormat PixelFormat) {
        Texture->SetPlatformData(new FTexturePlatformData());
        Texture->GetPlatformData()->SizeX = Width;
        Texture->GetPlatformData()->SizeY = Height;
        Texture->GetPlatformData()->PixelFormat = PixelFormat;
        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        Texture->GetPlatformData()->Mips.Add(Mip);
        Mip->SizeX = Width;
        Mip->SizeY = Height;
        {
            Mip->BulkData.Lock(LOCK_READ_WRITE);

            void* TextureData = Mip->BulkData.Realloc(Mip0Size);
            FMemory::Memcpy(TextureData, UncompressedImageData.GetData(), Mip0Size);

            Mip->BulkData.Unlock();
        }
    }

    void UpdateTextureGPUResourceAsync(
        const TArray64<uint8>& UncompressedImageData, UTexture2D* const Texture,
        const int32 Mip0Size, const int32 Width, const int32 Height, const EPixelFormat PixelFormat) {

        TArray<void*, TInlineAllocator<MAX_TEXTURE_MIP_COUNT>> MipData;
        MipData.Add(FMemory::Malloc(Mip0Size));
        // TODO: 動的メモリ確保不要?
        FMemory::Memcpy(MipData[0], UncompressedImageData.GetData(), Mip0Size);

        if (!GRHISupportsAsyncTextureCreation) {
            Texture->UpdateResource();
        }

        FTexture2DRHIRef RHITexture2D = RHIAsyncCreateTexture2D(
            Width, Height,
            PixelFormat,
            1,
            TexCreate_ShaderResource,
            MipData.GetData(), 1
        );

        for (void* NewData : MipData) {
            if (NewData) {
                FMemory::Free(NewData);
            }
        }
        MipData.Empty();

        {
            FFunctionGraphTask::CreateAndDispatchWhenReady(
                [&]() {
                    // link RHI texture to UTexture2D
                    ENQUEUE_RENDER_COMMAND(UpdateTextureReference)(
                        [Texture, RHITexture2D](FRHICommandListImmediate& RHICmdList) {
                            RHIUpdateTextureReference(Texture->TextureReference.TextureReferenceRHI, RHITexture2D);
                            Texture->RefreshSamplerStates();
                        }
                    );
                }, TStatId(), nullptr, ENamedThreads::GameThread)
                ->Wait();
        }
    }
}

UTexture2D* FPLATEAUTextureLoader::Load(const FString& TexturePath_SlashOrBackSlash) {
    int32 Width, Height;
    EPixelFormat PixelFormat;
    TArray64<uint8> UncompressedData;
    if(TexturePath_SlashOrBackSlash.IsEmpty()) return nullptr;
    
    // パスに ".." が含まれる場合は、std::filesystem の機能を使って適用します。
    fs::path TexturePathCpp = fs::u8path(TCHAR_TO_UTF8(*TexturePath_SlashOrBackSlash)).lexically_normal();
    const FString TexturePath_Normalized = TexturePathCpp.c_str();
    // 引数のパスのセパレーターはOSによって "/" か "¥" なので "/" に統一します。
    const auto TexturePath = TexturePath_Normalized.Replace(*FString("\\"), *FString("/"));

    if (!TryLoadAndUncompressImageFile(TexturePath, UncompressedData, Width, Height, PixelFormat))
        return nullptr;

    // Mip0Data
    const int32 Mip0Size = Width * Height * GPixelFormats[PixelFormat].BlockBytes;

    // テクスチャ作成
    UTexture2D* NewTexture = nullptr;
    {
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&]() {

                FString PackageName = TEXT("/Game/PLATEAU/Textures/");
                PackageName += FPaths::GetBaseFilename(TexturePath).Replace(TEXT("."), TEXT("_"));
                UPackage* Package = CreatePackage(*PackageName);
                Package->FullyLoad();
                NewTexture = Cast<UTexture2D>(Package->FindAssetInPackage());
                if (NewTexture != nullptr) {
                    return;
                }

                NewTexture = NewObject<UTexture2D>(Package, NAME_None, RF_Public | RF_Standalone | RF_MarkAsRootSet);

                const auto PLATEAURootDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectContentDir() + FString("PLATEAU/")));
                // アセットからテクスチャファイルへの相対パス
                auto RelativeTextureFilePath = TexturePath.Replace(*PLATEAURootDir, *FString("../"));
                NewTexture->AssetImportData->SetSourceFiles({ RelativeTextureFilePath });

                // テクスチャのアセット名設定
                // "."はパッケージの階層とみなされるためリプレース
                FString TextureName = FPaths::GetBaseFilename(TexturePath).Replace(TEXT("."), TEXT("_"));
                if (!NewTexture->Rename(*TextureName, nullptr, REN_Test)) {
                    TextureName = MakeUniqueObjectName(Package, USceneComponent::StaticClass(), FName(TextureName)).ToString();
                }
                NewTexture->Rename(*TextureName, nullptr, REN_DontCreateRedirectors);

                NewTexture->NeverStream = false;

                if (GRHISupportsAsyncTextureCreation)
                    UpdateTextureGPUResourceWithDummy(NewTexture, PixelFormat);

                // アセットとして保存するデータで上書き
                SetTexturePlatformData(NewTexture, UncompressedData, Mip0Size, Width, Height, PixelFormat);

                // GPUがRHIに対応している場合描画自体はRHIで行うため、NewTexture->UpdateResourceは実行しない。
                if (!GRHISupportsAsyncTextureCreation)
                    NewTexture->UpdateResource();

                NewTexture->AddToRoot();
                NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedData.GetData());
                // TODO: 関数化(SaveTexturePackage)
                Package->MarkPackageDirty();

                // 3Dファイルエクスポート用にテクスチャファイルのパスを保持
                const auto ContentPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectContentDir()));
                auto TextureFilePath = TexturePath.Replace(*ContentPath, TEXT("")).Replace(*FString("\\"), *FString("/"));
                Package->SetLoadedPath(FPackagePath::FromLocalPath(TexturePath));

                FAssetRegistryModule::AssetCreated(NewTexture);
                const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
                FSavePackageArgs Args;
                Args.SaveFlags = SAVE_NoError;
                Args.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
                Args.Error = GError;
                UPackage::SavePackage(Package, NewTexture, *PackageFileName, Args);

            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }
    check(IsValid(NewTexture));

    if (GRHISupportsAsyncTextureCreation)
        UpdateTextureGPUResourceAsync(UncompressedData, NewTexture, Mip0Size, Width, Height, PixelFormat);

    return NewTexture;
}

UTexture2D* FPLATEAUTextureLoader::LoadTransient(const FString& TexturePath) {
    int32 Width, Height;
    EPixelFormat PixelFormat;
    TArray64<uint8> UncompressedData;
    if (!TryLoadAndUncompressImageFile(TexturePath, UncompressedData, Width, Height, PixelFormat))
        return nullptr;

    // Mip0Data
    const int32 Mip0Size = Width * Height * GPixelFormats[PixelFormat].BlockBytes;

    // テクスチャ作成
    UTexture2D* NewTexture = nullptr;
    {
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&]() {
                auto DesiredTextureName = FPaths::GetBaseFilename(TexturePath);

                NewTexture = NewObject<UTexture2D>(
                    GetTransientPackage(),
                    MakeUniqueObjectName(GetTransientPackage(), UTexture2D::StaticClass(), *FPaths::GetBaseFilename(TexturePath)),
                    RF_Transient
                    );

                NewTexture->NeverStream = false;

                if (GRHISupportsAsyncTextureCreation)
                    UpdateTextureGPUResourceWithDummy(NewTexture, PixelFormat);
                else {
                    SetTexturePlatformData(NewTexture, UncompressedData, Mip0Size, Width, Height, PixelFormat);
                    NewTexture->UpdateResource();
                }

            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }
    check(IsValid(NewTexture));

    if (GRHISupportsAsyncTextureCreation)
        UpdateTextureGPUResourceAsync(UncompressedData, NewTexture, Mip0Size, Width, Height, PixelFormat);

    return NewTexture;
}

#endif
