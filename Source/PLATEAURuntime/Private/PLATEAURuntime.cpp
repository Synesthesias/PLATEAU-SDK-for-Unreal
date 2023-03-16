// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAURuntime.h"

#define LOCTEXT_NAMESPACE "FPLATEAURuntimeModule"

void FPLATEAURuntimeModule::StartupModule() {
    // TODO: キャッシュディレクトリ作成
    // IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
}

void FPLATEAURuntimeModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAURuntimeModule, PLATEAURuntime)