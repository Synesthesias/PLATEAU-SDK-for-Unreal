// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Reconstruct/PLATEAULandscape.h"
#include <Reconstruct/PLATEAUHeightMapCreator.h>

FPLATEAULandscape::FPLATEAULandscape() {}

FPLATEAULandscape::FPLATEAULandscape(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

void FPLATEAULandscape::CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model) {


    FPLATEAUHeightMapCreator HMap = FPLATEAUHeightMapCreator(false);

    //HMap.CalculateExtent(InputData.ExtractOptions, InputData.Extents);

    HMap.CreateHeightMap(CityModelActor, Model);



}