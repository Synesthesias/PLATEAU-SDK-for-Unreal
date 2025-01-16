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
};

USTRUCT()
struct FRGraphDrawEdgeOption : public FRnDrawOption {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    bool bUseAnyFaceVertexColor = false;

    UPROPERTY(EditAnywhere)
    bool bShowNeighborCount = false;
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
struct FRoadTypeMaskOption {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    ERRoadTypeMask Type;

    UPROPERTY(EditAnywhere)
    FLinearColor Color;

    UPROPERTY(EditAnywhere)
    bool bEnable = true;
};

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API UPLATEAURGraph : public UPLATEAUSceneComponent {
    GENERATED_BODY()
public:

    UPLATEAURGraph();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bVisibility = true;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowAll;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bOnlySelectedCityObjectGroupVisible;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    bool bShowNormal;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    ERRoadTypeMask ShowFaceType;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    ERRoadTypeMask RemoveFaceType;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    ERPartsFlag ShowId;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawFaceOption FaceOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawEdgeOption EdgeOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    FRGraphDrawVertexOption VertexOption;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Debug")
    TArray<FRoadTypeMaskOption> RoadTypeMaskOptions;

    // --------------------
    // start:フィールド
    // --------------------
    UPROPERTY()
    URGraph* RGraph;

private:
    FLinearColor GetColor(ERRoadTypeMask RoadType);
    void Draw(const FRGraphDrawVertexOption& Op, RGraphRef_t<URVertex> Vertex, struct FDrawWork& Work);
    void Draw(const FRGraphDrawEdgeOption& Op, RGraphRef_t<UREdge> Edge, struct FDrawWork& Work);
    void Draw(const FRGraphDrawFaceOption& Op, RGraphRef_t<URFace> Face, struct FDrawWork& Work);
    void DrawSideWalk(URGraph* Graph, FDrawWork& Work);
    void DrawNormal(URGraph* Graph, FDrawWork& Work);
};
