// Copyright 2023 Ministry of Land�AInfrastructure and Transport

#pragma once

#include "CoreMinimal.h"

class PLATEAURUNTIME_API FPLATEAUTextureLoader {
public:
    static UTexture2D* Load(const FString& TexturePath);
    static UTexture2D* LoadTransient(const FString& TexturePath);
};
