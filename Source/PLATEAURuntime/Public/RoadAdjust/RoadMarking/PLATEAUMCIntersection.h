// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "PLATEAUMCIntersection.generated.h"

/**
 * 道路ネットワークから、交差点の線を収集するクラスです。
 * MCはMarkedWayComposerの略です。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUMCIntersection : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()

public:
    /**
     * 道路ネットワークから交差点の線を収集します。
     * @param Target 道路ネットワークのターゲット
     * @return 収集された交差点の線のリスト
     */
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;

private:
    /** 長すぎる交差点の線を無視するしきい値 */
    static constexpr float IntersectionLineIgnoreLength = 10000.0f;
};
