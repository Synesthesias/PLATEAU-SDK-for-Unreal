// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "GameFramework/Actor.h"

#include <plateau/dataset/city_model_package.h>

#include "PLATEAUInstancedCityModel.generated.h"

struct PLATEAUPackageLOD;


USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUCityObjectInfo {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
        FString DatasetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
        FString GmlName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
        FString ID;
};



UCLASS()
class PLATEAURUNTIME_API APLATEAUInstancedCityModel : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLATEAUInstancedCityModel();

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FPLATEAUGeoReference GeoReference;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FString DatasetName;

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|CityGML"))
        FPLATEAUCityObjectInfo GetCityObjectInfo(USceneComponent* Component);

    plateau::dataset::PredefinedCityModelPackage GetExistPackage() const;


    APLATEAUInstancedCityModel* FilterByLODs(const plateau::dataset::PredefinedCityModelPackage InPackage, const int MinLOD, const int MaxLOD, const bool bSingleLOD);
    APLATEAUInstancedCityModel* FilterByFeatureTypes(const citygml::CityObject::CityObjectsType InCityObjectType);
    TArray<PLATEAUPackageLOD> GetPackageLODs() const;

    bool IsFiltering();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void SetIsFiltering(bool InValue);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

private:
    FCriticalSection FilterSection;
    bool bIsFiltering;
};
