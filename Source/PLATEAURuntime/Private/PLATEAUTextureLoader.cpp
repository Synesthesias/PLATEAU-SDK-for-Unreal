#include "PLATEAUTextureLoader.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

DECLARE_STATS_GROUP(TEXT("PLATEAUTextureLoader"), STATGROUP_PLATEAUTextureLoader, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Texture.UpdateResource"), STAT_Texture_UpdateResource, STATGROUP_PLATEAUTextureLoader);

UTexture2D* FPLATEAUTextureLoader::Load(const FString& TexturePath) {
    if (TexturePath.IsEmpty())
        return nullptr;

    IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

    TArray64<uint8> Buffer;
    if (!FFileHelper::LoadFileToArray(Buffer, *TexturePath)) {
        return nullptr;
    }

    const EImageFormat Format = ImageWrapperModule.DetectImageFormat(Buffer.GetData(), Buffer.Num());

    if (Format == EImageFormat::Invalid)
        return nullptr;
    const auto ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

    if (!ImageWrapper->SetCompressed((void*)Buffer.GetData(), Buffer.Num()))
        return nullptr;

    EPixelFormat PixelFormat;
    ERGBFormat RGBFormat;
    const int32 BitDepth = ImageWrapper->GetBitDepth();
    const int32 Width = ImageWrapper->GetWidth();
    const int32 Height = ImageWrapper->GetHeight();

    if (BitDepth == 16) {
        PixelFormat = PF_FloatRGBA;
        RGBFormat = ERGBFormat::RGBAF;
    } else if (BitDepth == 8) {
        PixelFormat = PF_B8G8R8A8;
        RGBFormat = ERGBFormat::BGRA;
    } else {
        //UE_LOG(LogImageUtils, Warning, TEXT("Error creating texture. Bit depth is unsupported. (%d)"), BitDepth);
        return nullptr;
    }

    TArray64<uint8> UncompressedData;
    ImageWrapper->GetRaw(RGBFormat, BitDepth, UncompressedData);

    // Mip0Data
    const int32 Mip0Size = Width * Height * GPixelFormats[PixelFormat].BlockBytes;

    // テクスチャ作成
    UTexture2D* NewTexture = nullptr;
    {
        FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&]() {
                auto DesiredTextureName = FPaths::GetBaseFilename(TexturePath);

                FString PackageName = TEXT("/Game/PLATEAU/Textures/");
                PackageName += FPaths::GetBaseFilename(TexturePath);
                UPackage* Package = CreatePackage(*PackageName);
                Package->FullyLoad();
                NewTexture = Cast<UTexture2D>(Package->FindAssetInPackage());
                if (NewTexture != nullptr) {
                    return;
                }

                NewTexture = NewObject<UTexture2D>(Package, NAME_None, RF_Public | RF_Standalone | RF_MarkAsRootSet);

                FString NewUniqueName = DesiredTextureName;
                if (!NewTexture->Rename(*NewUniqueName, nullptr, REN_Test)) {
                    NewUniqueName = MakeUniqueObjectName(Package, USceneComponent::StaticClass(), FName(DesiredTextureName)).ToString();
                }
                NewTexture->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

                // TODO: Streaming有効化
                NewTexture->NeverStream = true;



                // ダミーテクスチャ作成
                {
                    NewTexture->SetPlatformData(new FTexturePlatformData());
                    NewTexture->GetPlatformData()->SizeX = 1;
                    NewTexture->GetPlatformData()->SizeY = 1;
                    NewTexture->GetPlatformData()->PixelFormat = PixelFormat;

                    FTexture2DMipMap* Mip = new FTexture2DMipMap();
                    NewTexture->GetPlatformData()->Mips.Add(Mip);
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
                    NewTexture->UpdateResource();
                }

                // アセットとして保存するデータで上書き
                NewTexture->SetPlatformData(new FTexturePlatformData());
                NewTexture->GetPlatformData()->SizeX = Width;
                NewTexture->GetPlatformData()->SizeY = Height;
                NewTexture->GetPlatformData()->PixelFormat = PixelFormat;
                FTexture2DMipMap* Mip = new FTexture2DMipMap();
                NewTexture->GetPlatformData()->Mips.Add(Mip);
                Mip->SizeX = Width;
                Mip->SizeY = Height;
                {
                    Mip->BulkData.Lock(LOCK_READ_WRITE);

                    void* TextureData = Mip->BulkData.Realloc(Mip0Size);
                    FMemory::Memcpy(TextureData, UncompressedData.GetData(), Mip0Size);

                    Mip->BulkData.Unlock();
                }

                // 描画自体はRHIで行うため、NewTexture->UpdateResourceは実行しない。

                NewTexture->AddToRoot();
                NewTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_BGRA8, UncompressedData.GetData());
                
                // TODO: 関数化(SaveTexturePackage)
                //Package->MarkPackageDirty();
                //FAssetRegistryModule::AssetCreated(NewTexture);

                //const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
                //FSavePackageArgs Args;
                //Args.SaveFlags = SAVE_NoError;
                //Args.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
                //Args.Error = GError;
                //UPackage::SavePackage(Package, NewTexture, *PackageFileName, Args);

            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }
    check(IsValid(NewTexture));

    TArray<void*, TInlineAllocator<MAX_TEXTURE_MIP_COUNT>> MipData;
    MipData.Add(FMemory::Malloc(Mip0Size));
    // TODO: 動的メモリ確保不要?
    FMemory::Memcpy(MipData[0], UncompressedData.GetData(), Mip0Size);

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
                    [NewTexture, RHITexture2D](FRHICommandListImmediate& RHICmdList) {
                        RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
                        NewTexture->RefreshSamplerStates();
                    }
                );
            }, TStatId(), nullptr, ENamedThreads::GameThread)
            ->Wait();
    }

    return NewTexture;
}
