#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CityMapMetadata.generated.h"

USTRUCT()
struct PLATEAURUNTIME_API FStaticMeshCollection {
    GENERATED_BODY()

        UPROPERTY(EditAnywhere)
        TArray<UStaticMesh*> Value;
};

/**
 *
 */
UCLASS()
class PLATEAURUNTIME_API UCityMapMetadata : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere)
        TMap<int32, FStaticMeshCollection> StaticMeshes;

    UPROPERTY(EditAnywhere)
        TArray<FString> SourceGmlFiles;

    UPROPERTY(EditAnywhere)
        int32 ValueC;

    //UFUNCTION(CallInEditor)
    //    void CreateActor();
};