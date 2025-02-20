// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "RoadAdjust/PLATEAUCrosswalkPlacementRule.h"
#include "RoadAdjust/PLATEAUReproducedRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "PLATEAUCrosswalkComposer.generated.h"

UCLASS()
class PLATEAURUNTIME_API UPLATEAUCrosswalkComposer : public UObject {
    GENERATED_BODY()
public:
    UPLATEAUCrosswalkComposer();
    virtual ~UPLATEAUCrosswalkComposer() override;

    FPLATEAUMarkedWayList Compose(const IPLATEAURrTarget& Target, const EPLATEAUCrosswalkFrequency& CrosswalkFrequency);

private:
    FPLATEAUMarkedWayList GenerateCrosswalk(const FPLATEAUMWLine& Border,
                                            const TRnRef_T<URnIntersection>& Intersection,
                                            const TRnRef_T<URnRoadBase>& SrcRoad,
                                            EPLATEAUReproducedRoadDirection Direction);

    /**
     * 引数である道路のBorderを停止線とし、そこからPositionOffset分だけ移動した線を返します。
     */
    TArray<FVector> ShiftStopLine(const FPLATEAUMWLine& Border,
                                  const TRnRef_T<URnIntersection>& Intersection,
                                  float PositionOffsetArg);

    static constexpr float PositionOffset = 350.0f;      // 3.5m * 100 (Unreal units)
    static constexpr float CrosslineWidth = 400.0f;      // 4.0m * 100
    static constexpr float CrosslineDashInterval = 45.0f; // 0.45m * 100
    static constexpr float LineOffset = 10.0f;           // 0.1m * 100
    static constexpr float HeightOffset = 7.0f;          // 0.07m * 100
};
