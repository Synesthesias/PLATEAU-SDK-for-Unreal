// Copyright 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class PLATEAURUNTIME_API FPLATEAURuntimeModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FString GetContentDir();

private:


};
