#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "PLATEAUInstancedCityModel.h"

void APLATEAURnStructureModel::CreateRnModel(APLATEAUInstancedCityModel* Actor)
{
    FRoadNetworkFactoryEx::CreateRnModel(Factory, Actor, this);
}
