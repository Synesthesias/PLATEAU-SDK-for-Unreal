#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "CityMapMetadata.generated.h"

UENUM(BlueprintType)
enum class ECityModelPackage : uint8 {
    Building,
    Road,
    Relief,
    UrbanFacility,
    Vegetation,
    Others
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUImportedCityModelInfo {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
        FString GmlFilePath;
    UPROPERTY(EditAnywhere)
        ECityModelPackage Package;
    UPROPERTY(EditAnywhere)
        TArray<UStaticMesh*> StaticMeshes;
};

USTRUCT()
struct FPLATEAUFeatureMeshConvertSettings {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
        int MinLOD;
    UPROPERTY(EditAnywhere)
        int MaxLOD;
    UPROPERTY(EditAnywhere)
        bool ExportLowerLODs;
};

USTRUCT()
struct FPLATEAUMeshConvertSettings {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
        bool IsPerCityModelArea;
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

    UPROPERTY(EditAnywhere)
        FPLATEAUMeshConvertSettings MeshConvertSettings;
};
