#include "RoadNetwork/RGraph/PLATEAURGraph.h"

#include "RoadNetwork/RGraph/RGraphEx.h"
#include "RoadNetwork/Util/RnDebugEx.h"


struct FDrawWork {
    URGraph* Graph;
    TSet<RGraphRef_t<URVertex>> VisitedVertices;
    TSet<RGraphRef_t<UREdge>>VisitedEdges;
    TSet<RGraphRef_t<URFace>>VisitedFaces;
};

UPLATEAURGraph::UPLATEAURGraph() {
    bShowAll = true;
    bShowNormal = true;
    bOnlySelectedCityObjectGroupVisible = true;
    ShowFaceType = ERRoadTypeMask::All;
    RemoveFaceType = ERRoadTypeMask::Empty;
    ShowId = ERPartsFlag::None;

    // Initialize default road type mask options
    int32 Index = 0;
   /* for (const auto Value : TEnumRange<ERRoadTypeMask>()) 
    {
        if (Value == ERRoadTypeMask::Empty || Value == ERRoadTypeMask::All)
            continue;

        FRoadTypeMaskOption Option;
        Option.Type = Value;
        Option.Color = FRnDebugEx::GetDebugColor(Index++);
        Option.bEnable = true;
        RoadTypeMaskOptions.Add(Option);
    }*/
}

FLinearColor UPLATEAURGraph::GetColor(ERRoadTypeMask RoadType) {
    FLinearColor Result = FLinearColor::Black;
    int32 Count = 0;

    for (const auto& Option : RoadTypeMaskOptions) {
        if (Option.bEnable && EnumHasAnyFlags(RoadType, Option.Type)) {
            Result += Option.Color;
            Count++;
        }
    }

    if (Count > 0)
        Result /= Count;

    return Result;
}

void UPLATEAURGraph::Draw(const FRGraphDrawVertexOption& Op, RGraphRef_t<URVertex> Vertex, FDrawWork& Work) {
    if (!Op.bVisible || Work.VisitedVertices.Contains(Vertex))
        return;

    Work.VisitedVertices.Add(Vertex);

    FString Text = TEXT("●");
    if (Op.bShowEdgeCount)
        Text += FString::Printf(TEXT("%d"), Vertex->GetEdges().Num());

    //if (EnumHasAnyFlags(ShowId, ERPartsFlag::Vertex))
    //    Text += FString::Printf(TEXT("[%d]"), Vertex->GetDebugMyId());

    if (Op.bShowPos)
        Text += FString::Printf(TEXT("(%.2f,%.2f)"), Vertex->GetPosition().X, Vertex->GetPosition().Z);

    // Draw vertex visualization
    FRnDebugEx::DrawString(Text, Vertex->GetPosition(),   GetColor(Vertex->GetTypeMaskOrDefault(Op.bUseAnyFaceVertexColor)));

    //if (TargetVertices.Contains(Vertex) && Op.NeighborOption.bVisible) {
    //    for (const auto& Edge : Vertex->GetEdges()) {
    //        FRnDebugEx::DrawLine(Edge->GetV0()->GetPosition(),
    //            Edge->GetV1()->GetPosition(), Op.NeighborOption.Color);
    //    }
    //}
}

void UPLATEAURGraph::Draw(const FRGraphDrawEdgeOption& Op, RGraphRef_t<UREdge> Edge, FDrawWork& Work)
{
    if (Work.VisitedEdges.Contains(Edge))
        return;

    Work.VisitedEdges.Add(Edge);

    if (Op.bVisible) {
        const FLinearColor Color = GetColor(Edge->GetTypeMaskOrDefault(Op.bUseAnyFaceVertexColor));
        FRnDebugEx::DrawLine(Edge->GetV0()->GetPosition(), Edge->GetV1()->GetPosition(), Color);

        if (EnumHasAnyFlags(ShowId, ERPartsFlag::Edge)) {
            const FVector MidPoint = (Edge->GetV0()->GetPosition() + Edge->GetV1()->GetPosition()) * 0.5f;
           // FRnDebugEx::DrawString(FString::Printf(TEXT("[%d]"), Edge->GetDebugMyId()), MidPoint);
        }

        if (Op.bShowNeighborCount) {
            const FVector MidPoint = (Edge->GetV0()->GetPosition() + Edge->GetV1()->GetPosition()) * 0.5f;
            FRnDebugEx::DrawString(FString::Printf(TEXT("%d"), Edge->GetFaces().Num()), MidPoint);
        }
    }

    Draw(VertexOption, Edge->GetV0(), Work);
    Draw(VertexOption, Edge->GetV1(), Work);
}

void UPLATEAURGraph::Draw(const FRGraphDrawFaceOption& Op, RGraphRef_t<URFace> Face, FDrawWork& Work)
{
    if (!Op.bVisible)
        return;

    if (!EnumHasAnyFlags(Face->GetRoadTypes(), ShowFaceType))
        return;

    if (EnumHasAnyFlags(Face->GetRoadTypes(), RemoveFaceType))
        return;

    if (Work.VisitedFaces.Contains(Face))
        return;

    Work.VisitedFaces.Add(Face);

    if (bOnlySelectedCityObjectGroupVisible) {
        // Implement selected city object group check
        bool bShow = false;
        if (!bShow)
            return;
    }

  /*  TArray<URVertex*> Vertices;
    if (Op.bShowCityObjectOutline) {
        Vertices = Work.Graph->ComputeOutlineVerticesByCityObjectGroup(Face->CityObjectGroup,
            Op.ShowOutlineMask, Op.ShowOutlineRemoveMask);
    }
    else {
        Vertices = Face->ComputeOutlineVertices();
    }

    TArray<UREdge*> Edges;
    FRGraphEx::OutlineVertex2Edge(Vertices, Edges);

    if (EnumHasAnyFlags(ShowId, ERPartsFlag::Face)) {
        FVector Center = FVector::ZeroVector;
        for (const auto& Vertex : Vertices) {
            Center += Vertex->GetPosition();
        }
        Center /= Vertices.Num();
        UPLATEAUDebugEx::DrawString(FString::Printf(TEXT("F[%d]"), Face->GetDebugMyId()), Center);
    }

    if (Op.bShowConvexVolume) {
        TArray<FVector> ConvexPoints;
        for (const auto& Vertex : Face->ComputeConvexHullVertices()) {
            ConvexPoints.Add(Vertex->GetPosition());
        }
        UPLATEAUDebugEx::DrawArrows(ConvexPoints, true, 0.5f, FVector::UpVector, EdgeOption.Color);
    }
    else if (Op.bShowOutline) {
        for (const auto& Edge : Edges) {
            Draw(EdgeOption, Edge, Work);
        }
    }
    else {
        for (const auto& Edge : Face->GetEdges()) {
            Draw(EdgeOption, Edge, Work);
        }
    }*/
}
void UPLATEAURGraph::DrawSideWalk(URGraph* Graph, FDrawWork& Work) {
    /*auto DrawEdges = [this](const TArray<UREdge*>& Edges, const FDrawOption& Option) {
        if (!Option.bVisible)
            return;

        for (const auto& Edge : Edges) {
            UPLATEAUDebugEx::DrawLine(
                Edge->GetV0()->GetPosition(),
                Edge->GetV1()->GetPosition(),
                Option.Color
            );
        }
        };

    for (const auto& Face : Graph->GetFaces()) {
        if (!Face->GetRoadTypes().IsSideWalk())
            continue;

        TArray<UREdge*> OutsideEdges, InsideEdges, StartEdges, EndEdges;
        if (!Face->CreateSideWalk(OutsideEdges, InsideEdges, StartEdges, EndEdges))
            continue;

        DrawEdges(OutsideEdges, DrawSideWalkOption.OutsideColor);
        DrawEdges(InsideEdges, DrawSideWalkOption.InsideColor);
        DrawEdges(StartEdges, DrawSideWalkOption.StartColor);
        DrawEdges(EndEdges, DrawSideWalkOption.EndColor);
    }*/
}

void UPLATEAURGraph::DrawNormal(URGraph* Graph, FDrawWork& Work)
{
    for (const auto& Face : Graph->GetFaces()) {
        Draw(FaceOption, Face, Work);
    }
}

void UPLATEAURGraph::TickComponent(float DeltaTime, ELevelTick TickType,
                                   FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (IsVisible() == false)
        return;
    if (bVisibility == false)
        return;
    if (!RGraph)
        return;

    FDrawWork Work;
    Work.Graph = RGraph;

    if (bShowNormal) 
    {
        DrawNormal(RGraph, Work);
    }
    else
    {
        DrawSideWalk(RGraph, Work);
    }
}
