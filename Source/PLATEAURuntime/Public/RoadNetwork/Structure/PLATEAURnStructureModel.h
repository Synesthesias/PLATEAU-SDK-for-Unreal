#pragma once
#include "RoadNetwork/Factory/RoadNetworkFactory.h"

#include "PLATEAURnStructureModel.generated.h"

class APLATEAUInstancedCityModel;
class AActor;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API APLATEAURnStructureModel : public AActor {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PLATEAU")
    FRoadNetworkFactory Factory;

public:

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    void CreateRnModel(APLATEAUInstancedCityModel* Actor);
};
