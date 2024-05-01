// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelLandscape.h>
#include <Reconstruct/PLATEAUMeshLoaderForLandscape.h>

FPLATEAUModelLandscape::FPLATEAUModelLandscape() {}

FPLATEAUModelLandscape::FPLATEAUModelLandscape(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

void FPLATEAUModelLandscape::CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param) {
    FPLATEAUMeshLoaderForLandscape HMap = FPLATEAUMeshLoaderForLandscape(false);
    HMap.CreateHeightMap(CityModelActor, Model, Param);
}