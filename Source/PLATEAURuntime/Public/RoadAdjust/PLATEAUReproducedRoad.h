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

/**
* @brief Road Line 生成用パラメータ
*
*/
USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAURoadLineParam {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    EPLATEAURoadLineType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    UStaticMesh* LineMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    UMaterialInterface* LineMaterial;

    //Mesh間の隙間
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    float LineGap;

    //元メッシュの幅に対する倍率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|RoadAdjust")
    float LineXScale;

    //この値が0の場合アセットMeshのBoundsのサイズを利用
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

public:
    virtual void Tick(float DeltaTime) override;

private:

    int32 NumComponents;
    TMap<EPLATEAURoadLineType, FPLATEAURoadLineParam> LineTypeMap;

    void CreateLineTypeMap();
    void CreateLineComponentByType(EPLATEAURoadLineType Type, TArray<FVector> LinePoints);
};
