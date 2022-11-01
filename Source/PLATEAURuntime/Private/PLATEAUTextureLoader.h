#pragma once

#include "CoreMinimal.h"

class PLATEAURUNTIME_API FPLATEAUTextureLoader {
public:
    static UTexture2D* Load(const FString& TexturePath);
};
