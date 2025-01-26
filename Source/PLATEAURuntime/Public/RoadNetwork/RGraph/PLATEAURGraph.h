#pragma once
#include <memory>
#include "../../Component/PLATEAUSceneComponent.h"
#include "RGraph.h"
#include "RoadNetwork/Util/RnDebugEx.h"
#include "PLATEAURGraph.generated.h"

struct FActorComponentTickFunction;

UENUM(Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ERPartsFlag : uint8 {
    None = 0,
    Vertex = 1 << 0,
    Edge = 1 << 1,
    Face = 1 << 2,
    All = Vertex | Edge | Face
};
ENUM_CLASS_FLAGS(ERPartsFlag);

USTRUCT()
struct FRGraphDrawFaceOption : public FRnDrawOption {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bShowOutline = true;

    UPROPERTY(EditAnywhere)
    bool bShowConvexVolume = false;

    UPROPERTY(EditAnywhere)
    ERRoadTypeMask ShowOutlineMask = ERRoadTypeMask::Road;

    UPROPERTY(EditAnywhere)
    ERRoadTypeMask ShowOutlineRemoveMask = ERRoadTypeMask::Empty;

    UPROPERTY(EditAnywhere)
    bool bShowCityObjectOutline = false;

    UPROPERTY(EditAnywhere)
    bool bShowOutlineLoop = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowOnlyTargets;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    TArray<FString> ShowTargetNames;
};

USTRUCT()
struct FRGraphDrawEdgeOption : public FRnDrawOption {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bUseAnyFaceVertexColor = false;

    UPROPERTY(EditAnywhere)
    bool bShowNeighborCount = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum = ERRoadTypeMask))
    int32 ShowTypeMask = 0;
};

USTRUCT()
struct FRGraphDrawVertexOption : public FRnDrawOption {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 Size = 10;

    UPROPERTY(EditAnywhere)
    FRnDrawOption NeighborOption;

    UPROPERTY(EditAnywhere)
    bool bShowPos = false;

    UPROPERTY(EditAnywhere)
    bool bShowEdgeCount = false;

    UPROPERTY(EditAnywhere)
    bool bUseAnyFaceVertexColor = false;
};
USTRUCT()
struct FRGraphDrawSideWalkOption : public FRnDrawOption {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    FRnDrawOption OutsideWay;

    UPROPERTY(EditAnywhere)
    FRnDrawOption InsideWay;

    UPROPERTY(EditAnywhere)
    FRnDrawOption StartEdgeWay;

    UPROPERTY(EditAnywhere)
    FRnDrawOption EndEdgeWay;

};
UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API UPLATEAURGraph : public UPLATEAUSceneComponent {
    GENERATED_BODY()
public:

    UPLATEAURGraph();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

    // --------------------
    // start:フィールド
    // --------------------
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bVisibility = false;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowNormal;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum = ERRoadTypeMask))
    int32 ShowFaceType;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum= ERRoadTypeMask))
    int32 RemoveFaceType;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug", Meta = (Bitmask, BitmaskEnum = ERPartsFlag))
    ERPartsFlag ShowId;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawFaceOption FaceOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawEdgeOption EdgeOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawVertexOption VertexOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FLinearColor RoadColor;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FLinearColor SideWalkColor;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FLinearColor HighWayColor;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    float FontScale = 1.f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FLinearColor UndefinedColor;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawSideWalkOption SideWalkOption;

    UPROPERTY(VisibleAnywhere, Category="PLATEAU")
    URGraph* RGraph;

private:
    FLinearColor GetColor(ERRoadTypeMask RoadType);
    void Draw(const FRGraphDrawVertexOption& Op, RGraphRef_t<URVertex> Vertex, struct FDrawWork& Work);
    void Draw(const FRGraphDrawEdgeOption& Op, RGraphRef_t<UREdge> Edge, struct FDrawWork& Work);
    void Draw(const FRGraphDrawFaceOption& Op, RGraphRef_t<URFace> Face, struct FDrawWork& Work);
    void DrawSideWalk(URGraph* Graph, FDrawWork& Work);
    void DrawNormal(URGraph* Graph, FDrawWork& Work);
};
