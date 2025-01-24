#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"

APLATEAURnStructureModel::APLATEAURnStructureModel()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<UPLATEAUSceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
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
            FRoadNetworkFactoryEx::CreateRnModel(Factory, TargetActor, this);
            FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
                //終了イベント通知
                OnCreateRnModelFinished.Broadcast();
                }, TStatId(), NULL, ENamedThreads::GameThread);
            return this;
        });
    return CreateRnModelTask;
}

void APLATEAURnStructureModel::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    Super::EndPlay(EndPlayReason);
}

void APLATEAURnStructureModel::PostInitProperties()
{
    Super::PostInitProperties();
}

void APLATEAURnStructureModel::PostActorCreated()
{
    Super::PostActorCreated();
}

void APLATEAURnStructureModel::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}
