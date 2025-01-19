#pragma once
#include "PLATEAURnModelDrawerDebug.h"
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
    APLATEAURnStructureModel();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PLATEAU")
    FRoadNetworkFactory Factory;

    UPROPERTY(VisibleAnywhere, Category = "PLATEAU")
    TObjectPtr<URnModel> Model;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FPLATEAURnModelDrawerDebug Debug;
public:

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    void CreateRnModel(APLATEAUInstancedCityModel* Actor);


    virtual void Tick(float DeltaTime) override;
};
