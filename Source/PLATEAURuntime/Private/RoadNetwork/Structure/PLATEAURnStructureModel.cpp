#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "PLATEAUInstancedCityModel.h"

APLATEAURnStructureModel::APLATEAURnStructureModel()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APLATEAURnStructureModel::CreateRnModel(APLATEAUInstancedCityModel* Actor)
{
    FRoadNetworkFactoryEx::CreateRnModel(Factory, Actor, this);
    FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
        //終了イベント通知
        OnCreateRnModelFinished.Broadcast();
    }, TStatId(), NULL, ENamedThreads::GameThread);
}

void APLATEAURnStructureModel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (Debug.bVisible) {
        Debug.Draw(Model);
    }
}

UE::Tasks::TTask<APLATEAURnStructureModel*> APLATEAURnStructureModel::CreateRnModelAsync(APLATEAUInstancedCityModel* TargetActor)
{
    UE::Tasks::TTask<APLATEAURnStructureModel*> CreateRnModelTask = UE::Tasks::Launch(
        TEXT("CreateRnModelTask")
        , [this, TargetActor]() {
            this->CreateRnModel(TargetActor);           
            return this;
        });
    return CreateRnModelTask;
}

