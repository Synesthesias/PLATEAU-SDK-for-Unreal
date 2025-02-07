// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "PLATEAUMCLaneLine.generated.h"

/**
 * 道路ネットワークから、LaneLineを収集するクラスです。
 * LaneLineとは車線間の線のうち、同じ方向を通行する線を区切るもの（センターラインでないもの）を指します。
 * MCはMarkedWayComposerの略です。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUMCLaneLine : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()

public:
    /**
     * 道路ネットワークから車線境界線を収集します。
     * @param Target 道路ネットワークのターゲット
     * @return 収集された車線境界線のリスト
     */
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;
};
