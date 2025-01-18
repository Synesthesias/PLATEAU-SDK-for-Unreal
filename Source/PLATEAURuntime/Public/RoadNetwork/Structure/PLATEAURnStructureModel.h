#pragma once
#include "RoadNetwork/Factory/RoadNetworkFactory.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "PLATEAURnStructureModel.generated.h"

class APLATEAUInstancedCityModel;
class AActor;
class URnModel;
UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API APLATEAURnStructureModel : public AActor {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PLATEAU")
    FRoadNetworkFactory Factory;

    UPROPERTY(VisibleAnywhere, Category = "PLATEAU")
    URnModel* Model;
public:

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    void CreateRnModel(APLATEAUInstancedCityModel* Actor);
};
