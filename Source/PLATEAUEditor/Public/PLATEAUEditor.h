// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

class PLATEAUEDITOR_API IPLATEAUEditorModule : public IModuleInterface
{
public:
    static IPLATEAUEditorModule& Get();
    static bool IsAvailable();

    virtual TSharedRef<class FPLATEAUWindow> GetWindow() = 0;
    virtual TSharedRef<class FPLATEAUExtentEditor> GetExtentEditor() = 0;
};
