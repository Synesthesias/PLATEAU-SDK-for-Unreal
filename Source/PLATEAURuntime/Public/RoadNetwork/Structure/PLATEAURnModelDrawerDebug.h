#pragma once

#include "CoreMinimal.h"
#include "RnIntersection.h"
#include "RnWay.h"
#include "GameFramework/Actor.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "PLATEAURnModelDrawerDebug.generated.h"

class URnModel;

UENUM(Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERnPartsTypeMask : uint8 {
    None = 0,
    Point = 1 << 0,
    LineString = 1 << 1,
    Way = 1 << 2,
    Lane = 1 << 3,
    Road = 1 << 4,
    Intersection = 1 << 5,
    Neighbor = 1 << 6,
    SideWalk = 1 << 7
};
ENUM_CLASS_FLAGS(ERnPartsTypeMask);

UENUM(Meta = (Bitflags))
enum class ERnModelDrawerVisibleType : uint8 {
    Empty = 0,
    NonSelected = 1 << 0,
    SceneSelected = 1 << 1,
    GuiSelected = 1 << 2,
    Valid = 1 << 3,
    Invalid = 1 << 4,
    All = ~0
};
ENUM_CLASS_FLAGS(ERnModelDrawerVisibleType);


USTRUCT()
struct FRnModelDrawOption
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere)
    bool bVisible = true;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum = ERnModelDrawerVisibleType))
    ERnModelDrawerVisibleType VisibleType = ERnModelDrawerVisibleType::All;

};

USTRUCT()
struct FRnModelDrawWayOption : public FRnModelDrawOption
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    FLinearColor Color = FLinearColor::White;

    UPROPERTY(EditAnywhere)
    bool bShowVertexNormal = false;

    UPROPERTY(EditAnywhere)
    bool bShowEdgeNormal = false;

    UPROPERTY(EditAnywhere)
    bool bChangeArrowColor = false;

    UPROPERTY(EditAnywhere)
    FLinearColor NormalWayArrowColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere)
    FLinearColor ReverseWayArrowColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere)
    float ArrowSize = 50.f;
};

// Add these structures to the header file
USTRUCT()
struct FRnModelDrawLaneOption : public FRnModelDrawOption{
    GENERATED_BODY()
public:
    FRnModelDrawLaneOption();
    UPROPERTY(EditAnywhere)
    float BothConnectedLaneAlpha = 1.0f;

    UPROPERTY(EditAnywhere)
    float ValidWayAlpha = 0.75f;

    UPROPERTY(EditAnywhere)
    float InvalidWayAlpha = 0.3f;

    UPROPERTY(EditAnywhere)
    float ReverseWayAlpha = 1.0f;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowLeftWay;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowRightWay;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowPrevBorder;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowNextBorder;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowCenterWay;

    UPROPERTY(EditAnywhere)
    bool bShowNextRoad = false;

    UPROPERTY(EditAnywhere)
    bool bShowPrevRoad = false;

    UPROPERTY(EditAnywhere)
    bool bShowCenter2Next = false;

};


USTRUCT()
struct FRnModelDrawRoadMergeOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:
    FRnModelDrawRoadMergeOption()
    {
        bVisible = false;
    }
    UPROPERTY(EditAnywhere)
    bool bShowMergedBorderNoDir = false;

    UPROPERTY(EditAnywhere)
    EPLATEAURnDir ShowMergedBorderDir = EPLATEAURnDir::Left;

    UPROPERTY(EditAnywhere)
    int32 SplitBorderNum = 1;
};
USTRUCT()
struct FRnModelDrawRoadNormalOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    bool bShowSpline = true;

    UPROPERTY(EditAnywhere)
    int ShowLaneIndex = -1;

    UPROPERTY(EditAnywhere)
    FLinearColor GroupColor = FLinearColor::Green;
};
USTRUCT()
struct FRnModelDrawRoadGroupOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:
    FRnModelDrawRoadGroupOption() {
        bVisible = false;
    }
    UPROPERTY(EditAnywhere)
    bool bShowMergedBorderNoDir = false;

    UPROPERTY(EditAnywhere)
    EPLATEAURnDir ShowMergedBorderDir = EPLATEAURnDir::Left;

    UPROPERTY(EditAnywhere)
    int32 SplitBorderNum = 1;
};

USTRUCT()
struct FRnModelDrawRoadOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere)
    FRnModelDrawRoadMergeOption MergeDrawer;

    UPROPERTY(EditAnywhere)
    FRnModelDrawRoadNormalOption NormalDrawer;

    UPROPERTY(EditAnywhere)
    FRnModelDrawRoadGroupOption GroupDrawer;

    UPROPERTY(EditAnywhere)
    bool bSliceHorizontal = false;

};

USTRUCT()
struct FRnModelDrawIntersectionOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:
    FRnModelDrawIntersectionOption();

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowNonBorderEdge;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowBorderEdge;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowMedianBorderEdge;

    UPROPERTY(EditAnywhere)
    bool bShowEdgeIndex = false;

    UPROPERTY(EditAnywhere)
    bool bShowEdgeGroup = false;

    UPROPERTY(EditAnywhere)
    bool bShowNoTrackBorder = false;

    UPROPERTY(EditAnywhere)
    bool bShowEdgeNormal = false;

    UPROPERTY(EditAnywhere)
    bool bShowTrack = false;


    UPROPERTY(EditAnywhere)
    TMap<ERnTurnType, FLinearColor> showTrackColor;
};

USTRUCT()
struct FRnModelDrawSideWalkOption : public FRnModelDrawOption {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowOutsideWay;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowInsideWay;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowStartEdgeWay;

    UPROPERTY(EditAnywhere)
    FRnModelDrawWayOption ShowEndEdgeWay;
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAURnModelDrawerDebug {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bVisible = true;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowInsideNormalMidPoint = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowVertexIndex = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowVertexPos = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    int32 ShowVertexFontSize = 20;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    float EdgeOffset = 0.0f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    float YScale = 1.0f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRnModelDrawIntersectionOption IntersectionOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRnModelDrawRoadOption RoadOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRnModelDrawLaneOption LaneOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRnModelDrawSideWalkOption SideWalkOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum = ERnPartsTypeMask))
    int32 ShowPartsType;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowOnlyTargets;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    TArray<FString> ShowTargetNames;

    void Draw(URnModel* Model);
};
