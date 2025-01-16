#include "RoadNetwork/RGraph/RGraphFactory.h"
#include "RoadNetwork/RnDef.h"
#include "RoadNetwork/RGraph/RGraph.h"
#include "RoadNetwork/RGraph/RGraphEx.h"

RnRef_t<FRGraph> FRGraphFactory::CreateGraph(const TArray<TSharedPtr<FSubDividedCityObject>>& CityObjects) {
    auto Graph = MakeShared<FRGraph>();

    if (bReductionOnCreate) 
    {
        FRGraphHelper::Optimize(Graph, MergeCellSize, MergeCellLength,
            RemoveMidPointTolerance, Lod1HeightTolerance);
    }
    return Graph;
}