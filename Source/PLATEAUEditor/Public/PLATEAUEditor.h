// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IPLATEAUEditorModule : public IModuleInterface
{
public:
    static IPLATEAUEditorModule& Get();
    static bool IsAvailable();

    virtual TSharedRef<class FPLATEAUExtentEditor> GetExtentEditor() = 0;
};
