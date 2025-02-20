// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "PLATEAUMCShoulderLine.generated.h"

/**
 * 道路ネットワークから、路側帯線(ShoulderLine)を収集するクラスです。
 * 路側帯線とは車道と歩道の間の線を指します。
 * MCはMarkedWayComposerの略です。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUMCShoulderLine : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()

public:
    /**
     * 道路ネットワークから路側帯線を収集します。
     * @param Target 道路ネットワークのターゲット
     * @return 収集された路側帯線のリスト
     */
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;
};
