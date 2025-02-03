// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUInstancedCityModel.h"
#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "RoadMarking/LineGeneratorComponent.h"
#include "PLATEAUReproducedRoad.generated.h"


UENUM(BlueprintType)
enum class EPLATEAURoadLineType : uint8 {
    WhiteLine = 0 UMETA(DisplayName = "WhiteLine"),
    YellowLine = 1 UMETA(DisplayName = "YellowLine"),
    DashedWhilteLine = 2 UMETA(DisplayName = "DashedWhilteLine"),
    StopLine = 3 UMETA(DisplayName = "StopLine"),
    Crossing = 4 UMETA(DisplayName = "Crossing"),
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAURoadLineParam {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    EPLATEAURoadLineType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    UStaticMesh* LineMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    UMaterialInterface* LineMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    float LineGap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    float LineXScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    float LineLength;
};


UCLASS()
class PLATEAURUNTIME_API APLATEAUReproducedRoad : public AActor {
    GENERATED_BODY()
public:
    APLATEAUReproducedRoad();

    UFUNCTION(BlueprintCallable, meta = (Category = "PLATEAU|RoadAdjust"))
    void CreateRoadMarks(APLATEAURnStructureModel* Model);


protected:
    // Called when the game starts or when spawneds
    virtual void BeginPlay() override;

    void GetVectors(TArray<FVector>& Vectors, URnWay* Way) const;

    void CreateLineComponentByType(EPLATEAURoadLineType type, TArray<FVector> LinePoints, int32 index);

public:
    virtual void Tick(float DeltaTime) override;

private:

    TMap<EPLATEAURoadLineType, FPLATEAURoadLineParam> LineTypeMap;

};
