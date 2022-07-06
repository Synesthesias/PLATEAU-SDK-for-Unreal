#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CityMapMetadata.generated.h"

/**
 *
 */
UCLASS()
class PLATEAURUNTIME_API UCityMapMetadata : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere)
        int32 ValueA;

    UPROPERTY(EditAnywhere)
        int32 ValueB;

    UPROPERTY(EditAnywhere)
        int32 ValueC;
};