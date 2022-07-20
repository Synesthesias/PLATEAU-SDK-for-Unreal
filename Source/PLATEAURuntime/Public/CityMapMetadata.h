#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CityMapMetadata.generated.h"

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUImportedCityModelInfo {
    GENERATED_BODY()
        UPROPERTY(EditAnywhere)
        FString GmlFilePath;
        UPROPERTY(EditAnywhere)
        TArray<UStaticMesh*> StaticMeshes;
};

/**
 *
 */
UCLASS()
class PLATEAURUNTIME_API UCityMapMetadata : public UObject {
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(EditAnywhere)
        TArray<FPLATEAUImportedCityModelInfo> ImportedCityModelInfoArray;
};