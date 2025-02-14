// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "PLATEAUMarkedWayListComposerMain.generated.h"

/// インターフェイス宣言のためのダミークラス。 IPLATEAUMarkedWayListComposer を使ってください
UINTERFACE()
class UPLATEAUMarkedWayListComposer : public UInterface
{
    GENERATED_BODY()
};

/**
 * 道路ネットワークから車線を引く対象となるMarkedWayのリストを生成するためのインターフェース
 */
class PLATEAURUNTIME_API IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()
public:
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) = 0;
};

/**
 * 道路ネットワークをもとに、車線を引く対象となるMarkedWayのリストを生成します。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUMarkedWayListComposerMain : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()

public:
    virtual ~UPLATEAUMarkedWayListComposerMain() override = default;

    /**
     * 道路ネットワークから、車線を引く対象となるMarkedWayListを収集します。
     */
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;
    
    // 経験的にこのくらいの高さなら道路にめりこまないという値
    static constexpr float HeightOffset = 9.0f;

private:
    
};
