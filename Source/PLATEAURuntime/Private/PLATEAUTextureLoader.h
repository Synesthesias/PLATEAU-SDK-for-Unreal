//#pragma once
//
//namespace {
//    void Load() {
//        const int32 TextureSizeX = 1024;
//        const int32 TextureSizeY = 1024;
//        EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8;
//        const int32 NumMips = 1;
//
//        // Mip0Data
//        const int32 Mip0Size = TextureSizeX * TextureSizeY * GPixelFormats[PixelFormat].BlockBytes;
//
//        TArray<uint8> Mip0Data;
//        Mip0Data.SetNum(Mip0Size);
//
//        // Fill Mip0Data;
//        // below "green" mip0 is constructed in a very bruteforce way
//        for (int32 Index = 0; Index < Mip0Size; Index += 4) {
//            Mip0Data[Index] = 0;
//            Mip0Data[Index + 1] = 255;
//            Mip0Data[Index + 2] = 0;
//            Mip0Data[Index + 3] = 255;
//        }
//
//        // make sure UTexture2D created on the game thread 
//        // game thread task should be spawned in case this code is being executed on the separated thread
//        check(IsInGameThread());
//
//        // Create transient texture
//        UTexture2D* NewTexture = NewObject<UTexture2D>(
//            GetTransientPackage(),
//            MakeUniqueObjectName(GetTransientPackage(), UTexture2D::StaticClass(), *BaseFilename),
//            RF_Transient
//            );
//        check(IsValid(NewTexture));
//
//        // never link the texture to Unreal streaming system
//        NewTexture->NeverStream = true;
//
//        {
//            // allocate dummy mipmap of 1x1 size
//            NewTexture->PlatformData = new FTexturePlatformData();
//            NewTexture->PlatformData->SizeX = 1;
//            NewTexture->PlatformData->SizeY = 1;
//            NewTexture->PlatformData->PixelFormat = PixelFormat;
//
//            FTexture2DMipMap* Mip = new FTexture2DMipMap();
//            NewTexture->PlatformData->Mips.Add(Mip);
//            Mip->SizeX = 1;
//            Mip->SizeY = 1;
//
//            // GPixelFormats contains meta information for each pixel format 
//            const uint32 MipBytes = Mip->SizeX * Mip->SizeY * GPixelFormats[PixelFormat].BlockBytes;
//            {
//                Mip->BulkData.Lock(LOCK_READ_WRITE);
//
//                void* TextureData = Mip->BulkData.Realloc(MipBytes);
//
//                static TArray<uint8> DummyBytes;
//                DummyBytes.SetNum(MipBytes);
//
//                FMemory::Memcpy(TextureData, DummyBytes.GetData(), MipBytes);
//
//                Mip->BulkData.Unlock();
//            }
//
//            // construct texture
//            NewTexture->UpdateResource();
//        }
//
//        // async create texture on the separate thread
//        FTexture2DRHIRef RHITexture2D;
//
//        Async(
//            EAsyncExecution::Thread,
//            [&RHITexture2D, TextureSizeX, TextureSizeY, PixelFormat, NumMips, Mip0Data] {
//                RHITexture2D = RHIAsyncCreateTexture2D(
//                    TextureSizeX, TextureSizeY,
//                    PixelFormat,
//                    NumMips,
//                    TexCreate_ShaderResource, Mip0Data.GetData(), NumMips
//                );
//            }
//        ).Wait();
//
//        // link RHI texture to UTexture2D
//        ENQUEUE_RENDER_COMMAND(UpdateTextureReference)(
//            [NewTexture, RHITexture2D](FRHICommandListImmediate& RHICmdList) {
//                RHIUpdateTextureReference(NewTexture->TextureReference.TextureReferenceRHI, RHITexture2D);
//                NewTexture->RefreshSamplerStates();
//            }
//        );
//    }
//}
