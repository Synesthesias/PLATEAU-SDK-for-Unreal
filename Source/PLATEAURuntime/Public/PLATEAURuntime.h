// Copyright 2023 Ministry of Land�AInfrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FPLATEAURuntimeModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:


};
