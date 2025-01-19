#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "PLATEAUInstancedCityModel.h"

APLATEAURnStructureModel::APLATEAURnStructureModel()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APLATEAURnStructureModel::CreateRnModel(APLATEAUInstancedCityModel* Actor)
{
    FRoadNetworkFactoryEx::CreateRnModel(Factory, Actor, this);
}

void APLATEAURnStructureModel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (Debug.bVisible) {
        Debug.Draw(Model);
    }
}
