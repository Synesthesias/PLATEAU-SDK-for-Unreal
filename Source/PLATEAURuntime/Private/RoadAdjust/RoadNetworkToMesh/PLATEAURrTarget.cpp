// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "RoadNetwork/Structure/RnIntersection.h"


void UPLATEAURrTargetModel::Initialize(TRnRef_T<URnModel> InNetwork)
{
    Model = InNetwork;
}


TArray<TRnRef_T<URnRoadBase>> UPLATEAURrTargetModel::GetRoadBases() const {
    TArray<TRnRef_T<URnRoadBase>> Result;
    Result.Append(GetRoads());
    Result.Append(GetIntersections());
    return Result;
}

TArray<TRnRef_T<URnRoad>> UPLATEAURrTargetModel::GetRoads() const {
    return Model->GetRoads();
}

TArray<TRnRef_T<URnIntersection>> UPLATEAURrTargetModel::GetIntersections() const {
    return Model->GetIntersections();
}

const URnModel* UPLATEAURrTargetModel::GetNetwork() const {
    return Model;
}



void UPLATEAURrTargetRoadBases::Initialize(const TRnRef_T<URnModel>& InNetwork, const TArray<TRnRef_T<URnRoadBase>>& InRoadBases) {
    Network = InNetwork;
    RoadBases = InRoadBases;
}

TArray<TRnRef_T<URnRoadBase>> UPLATEAURrTargetRoadBases::GetRoadBases() const {
    return RoadBases;
}

TArray<TRnRef_T<URnRoad>> UPLATEAURrTargetRoadBases::GetRoads() const {
    TArray<TRnRef_T<URnRoad>> Result;
    for (const auto& RoadBase : RoadBases) {
            
        if (auto Road = Cast<URnRoad>(RoadBase)) {
            Result.Add(Road);
        }
    }
    return Result;
}

TArray<TRnRef_T<URnIntersection>> UPLATEAURrTargetRoadBases::GetIntersections() const {
    TArray<TRnRef_T<URnIntersection>> Result;
    for (const auto& RoadBase : RoadBases) {
            
        if (auto Intersection = Cast<URnIntersection>(RoadBase)) {
            Result.Add(Intersection);
        }
    }
    return Result;
}


const URnModel* UPLATEAURrTargetRoadBases::GetNetwork() const {
    return Network;
}
