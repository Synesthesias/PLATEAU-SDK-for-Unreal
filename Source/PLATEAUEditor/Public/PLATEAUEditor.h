// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

class IPLATEAUEditorModule : public IModuleInterface
{
public:
    static IPLATEAUEditorModule& Get();
    static bool IsAvailable();

    virtual TSharedRef<class FPLATEAUExtentEditor> GetExtentEditor() = 0;
};
