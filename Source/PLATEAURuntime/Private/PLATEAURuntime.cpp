// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAURuntime.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FPLATEAURuntimeModule"

void FPLATEAURuntimeModule::StartupModule() {
    // TODO: キャッシュディレクトリ作成
    // IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
}

void FPLATEAURuntimeModule::ShutdownModule() {}

FString FPLATEAURuntimeModule::GetContentDir() {
    return IPluginManager::Get()
        .FindPlugin(TEXT("PLATEAU-SDK-for-Unreal"))
        ->GetContentDir();
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAURuntimeModule, PLATEAURuntime)