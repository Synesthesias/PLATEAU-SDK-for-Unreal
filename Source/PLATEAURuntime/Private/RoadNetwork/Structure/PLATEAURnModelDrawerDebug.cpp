
#include "RoadNetwork/Structure/PLATEAURnModelDrawerDebug.h"

#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Util/RnDebugEx.h"
#include "RoadNetwork/Util/RnEx.h"


namespace
{

    /// <summary>
    /// Drawの最初でリセットされるフレーム情報
    /// </summary>
    class RnModelDrawWork {
    public:
        TSet<UObject*> Visited;

        URnModel* Model;

        int DrawRoadGroupCount = 0;

        ERnModelDrawerVisibleType visibleType = ERnModelDrawerVisibleType::Empty;

        FPLATEAURnModelDrawerDebug* Self = nullptr;

        RnModelDrawWork(FPLATEAURnModelDrawerDebug* self, URnModel* model) {
            Self = self;
            Model = model;
        }

        bool IsVisited(UObject* obj) {
            auto ret = Visited.Contains(obj);
            if (ret == false)
                Visited.Add(obj);
            return ret;
        }

        ERnModelDrawerVisibleType GetVisibleType(UObject& Obj) {
            auto ret = ERnModelDrawerVisibleType::Empty;

            if (ret == ERnModelDrawerVisibleType::Empty)
                ret = ERnModelDrawerVisibleType::NonSelected;
            return ret;
        }

        void DrawLine(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f) {
            FRnDebugEx::DrawLine(Start, End, Color, Duration, Thickness);
        }
        void DrawArrow(const FVector& Start, const FVector& End, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector, const FLinearColor& BodyColor = FLinearColor::White, const FLinearColor& ArrowColor = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f) {
            FRnDebugEx::DrawArrow(Start, End, ArrowSize, ArrowUp, BodyColor, ArrowColor, Duration, Thickness);
        }
        void DrawArrows(const TArray<FVector>& Vertices, bool bIsLoop = false, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector, const FLinearColor& Color = FLinearColor::White, const FLinearColor& ArrowColor = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f) {
            FRnDebugEx::DrawArrows(Vertices, bIsLoop, ArrowSize, ArrowUp, Color, ArrowColor, Duration, Thickness);
        }
        void DrawLines(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f, float Thickness = 0.0f) {
            FRnDebugEx::DrawLines(Vertices, bIsLoop, Color, Duration, Thickness);
        }
        void DrawSphere(const FVector& Center, float Radius, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f) {
            FRnDebugEx::DrawSphere(Center, Radius, Color, Duration);
        }
        void DrawRegularPolygon(const FVector& Center, float Radius, int32 Sides = 5, const FVector& Up = FVector::UpVector, const FLinearColor& Color = FLinearColor::White, float Duration = 0.0f) {
            FRnDebugEx::DrawRegularPolygon(Center, Radius, Sides, Up, Color, Duration);
        }
        void DrawDashedLine(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float LineLength = 1.0f, float SpaceLength = 0.2f, float Duration = 0.0f) {
            FRnDebugEx::DrawDashedLine(Start, End, Color, LineLength, SpaceLength, Duration);
        }

        void DrawDashedArrow(const FVector& Start, const FVector& End, const FLinearColor& Color = FLinearColor::White, float LineLength = 1.0f, float SpaceLength = 0.2f, float Duration = 0.0f, float ArrowSize = 0.5f, const FVector& ArrowUp = FVector::UpVector) {
            FRnDebugEx::DrawDashedArrow(Start, End, Color, LineLength, SpaceLength, Duration, ArrowSize, ArrowUp);
        }
        void DrawDashedLines(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float LineLength = 3.0f, float SpaceLength = 1.0f, float Duration = 0.0f) {
            FRnDebugEx::DrawDashedLines(Vertices, bIsLoop, Color, LineLength, SpaceLength, Duration);
        }
        void DrawDashedArrows(const TArray<FVector>& Vertices, bool bIsLoop = false, const FLinearColor& Color = FLinearColor::White, float LineLength = 3.0f, float SpaceLength = 1.0f, float Duration = 0.0f) {
            FRnDebugEx::DrawDashedArrows(Vertices, bIsLoop, Color, LineLength, SpaceLength, Duration);
        }

    };

    template<class T>
    struct FRnModelDrawerOf
    {
    public:
        bool Draw(RnModelDrawWork& Work, T* Self, ERnModelDrawerVisibleType visibleType);

    private:
        bool Exec(FRnModelDrawerOf<T>* drawer, RnModelDrawWork& Work, T* Self);
    protected:
        FRnModelDrawerOf(const FRnModelDrawOption& InOp)
            :Op(InOp)
        {       }
        bool IsVisible() const {
            return Op.bVisible;
        }

        ERnModelDrawerVisibleType GetShowVisibleType() const {
            return Op.VisibleType;
        }

        virtual bool DrawImpl(RnModelDrawWork& Work, T& Self) {
            return true;
        }

        virtual TArray<FRnModelDrawerOf<T>*> GetChildDrawers() {
            return TArray<FRnModelDrawerOf<T>*>();
        }

    private:
        const FRnModelDrawOption& Op;
    };

    template<class TOp>
    struct FRnModelWayDrawer : public FRnModelDrawerOf<URnWay> {
    public:
        FRnModelWayDrawer(const TOp& InOp)
            : FRnModelDrawerOf(InOp)
            , Option(InOp) {
        }

        const TOp& Option;

    };

    template<class TOp>
    struct FRnModelLaneDrawer : public FRnModelDrawerOf<URnLane> {
    public:
        FRnModelLaneDrawer(const TOp& InOp)
            : FRnModelDrawerOf(InOp)
            , Option(InOp) {
        }

        const TOp& Option;

    };
    template<class TOp>
    struct FRnModelRoadDrawer : public FRnModelDrawerOf<URnRoad> {
    public:
        FRnModelRoadDrawer(const TOp& InOp)
            : FRnModelDrawerOf(InOp)
        , Option(InOp)
        {
        }

        const TOp& Option;

    };
    template<class TOp>
    struct FRnModelIntersectionDrawer : public FRnModelDrawerOf<URnIntersection> {
    public:
        FRnModelIntersectionDrawer(const TOp& InOp)
            : FRnModelDrawerOf(InOp)
            , Option(InOp) {
        }

        const TOp& Option;
    };

    template<class TOp>
    struct FRnModelSideWalkDrawer : public FRnModelDrawerOf<URnSideWalk> {
    public:
        FRnModelSideWalkDrawer(const TOp& InOp)
            : FRnModelDrawerOf(InOp)
            , Option(InOp) {
        }

        const TOp& Option;
    };
}

template <class T>
bool FRnModelDrawerOf<T>::Draw(RnModelDrawWork& Work, T* Self, ERnModelDrawerVisibleType visibleType) {
    if (IsVisible() == false)
        return false;

    if (Self == nullptr)
        return false;

    auto lastVisibleType = visibleType;
    if (Work.IsVisited(Self))
        return false;
    Work.visibleType = visibleType;
    return Exec(this, Work, Self);
}

template <class T>
bool FRnModelDrawerOf<T>::Exec(FRnModelDrawerOf<T>* drawer, RnModelDrawWork& Work, T* Self) {
    if (drawer->IsVisible() == false)
        return false;

    if (Self == nullptr)
        return false;

    auto visibleType = Work.visibleType;

    if ((int)(visibleType & drawer->GetShowVisibleType()) == 0)
        return false;

    if (drawer->DrawImpl(Work, *Self) == false)
        return false;

    for (auto child : drawer->GetChildDrawers()) {
        Exec(child, Work, Self);
    }

    return true;
}

FRnModelDrawLaneOption::FRnModelDrawLaneOption()
{
    ShowLeftWay.Color = FLinearColor::Green;
    ShowRightWay.Color = FLinearColor::Red;
    ShowPrevBorder.Color = FLinearColor::Blue;
    ShowNextBorder.Color = FLinearColor::Yellow;
    ShowCenterWay.Color = FLinearColor::White;
}

FRnModelDrawIntersectionOption::FRnModelDrawIntersectionOption()
{
    ShowNonBorderEdge.Color = FLinearColor::White;
    ShowBorderEdge.Color = FLinearColor::Blue;
}

//
//bool FRnModelDrawWayOption::DrawImpl(RnModelDrawWork& work, URnWay& way)
//{
//    if (way.Count() <= 1)
//        return false;
//
//    // 矢印色は設定されていない場合は反転しているかどうかで返る
//    auto arrowColor = Color;
//    if (bChangeArrowColor)
//        arrowColor = way.IsReversed ? ReverseWayArrowColor : NormalWayArrowColor;
//
//    auto Vertices = FRnEx::SelectWithIndex(
//        way.GetVertices()
//        , [&](FVector v, int32 i) {
//            return v - work.Self->EdgeOffset * way.GetVertexNormal(i);
//        });
//    work.DrawArrows(
//        Vertices
//        , false
//        , Color
//        , arrowColor
//        , ArrowSize
//    );
//
//    if ((int)(work.Self->ShowPartsType & ERnPartsTypeMask::Way) != 0)
//    {
//        FVector p;
//        way.GetLerpPoint(0.5f, p);
//        FRnDebugEx::DrawString(FString::Printf(TEXT("%s"), *way.GetName()), p);
//    }
//
//    return true;
//}
void FPLATEAURnModelDrawerDebug::Draw(URnModel* Model)
{
    if (bVisible == false)
        return;

    RnModelDrawWork Work(this, Model);


    struct Way : public FRnModelWayDrawer<FRnModelDrawWayOption>
    {
        Way(const FRnModelDrawWayOption& InOption)
            : FRnModelWayDrawer(InOption)
        {}

    private:
        virtual bool DrawImpl(RnModelDrawWork& Work, URnWay& Self) override
        {
            const auto Vertices = Self.GetVertices().ToArray();
            Work.DrawArrows(Vertices, false, Option.ArrowSize, FVector::UnitZ(), Option.Color);
            return true;
        }
    };

    struct Lane : public FRnModelLaneDrawer<FRnModelDrawLaneOption>
    {
        Lane(const FRnModelDrawLaneOption& InOption)
            :FRnModelLaneDrawer(InOption) {

        }
    private:
        virtual bool DrawImpl(RnModelDrawWork& Work, URnLane& Self) override
        {
            Way(Option.ShowLeftWay).Draw(Work, Self.LeftWay, Work.visibleType);
            Way(Option.ShowRightWay).Draw(Work, Self.RightWay, Work.visibleType);
            Way(Option.ShowNextBorder).Draw(Work, Self.NextBorder, Work.visibleType);
            Way(Option.ShowPrevBorder).Draw(Work, Self.PrevBorder, Work.visibleType);

            return true;
        }
    };

    struct Road : public FRnModelRoadDrawer<FRnModelDrawRoadOption>
    {
        Road(const FRnModelDrawRoadOption& InOption)
            :FRnModelRoadDrawer(InOption)
        {

        }
    private:
        virtual bool DrawImpl(RnModelDrawWork& Work, URnRoad& Self) override
        {
            Lane L(Work.Self->LaneOption);
            for(auto&& Lane : Self.MainLanes)
            {
                L.Draw(Work, Lane, Work.visibleType);
            }
            return true;
        }
    };

    struct Intersection : public FRnModelIntersectionDrawer<FRnModelDrawIntersectionOption>
    {
        Intersection(const FRnModelDrawIntersectionOption& InOption)
            :FRnModelIntersectionDrawer(InOption) {

        }
    private:
        virtual bool DrawImpl(RnModelDrawWork& Work, URnIntersection& Self) override
        {
            Lane L(Work.Self->LaneOption);
            Way Border(Option.ShowBorderEdge);
            Way NonBorder(Option.ShowNonBorderEdge);

            for (auto&& Edge : Self.GetEdges()) 
            {
                if(Edge->IsBorder())
                {
                    Border.Draw(Work, Edge->GetBorder(), Work.visibleType);
                }
                else
                {
                    NonBorder.Draw(Work, Edge->GetBorder(), Work.visibleType);
                }
            }
            return true;
        }
    };

    struct SideWalk : public FRnModelSideWalkDrawer<FRnModelDrawSideWalkOption> {
        SideWalk(const FRnModelDrawSideWalkOption& InOption)
            :FRnModelSideWalkDrawer(InOption) {

        }
    private:
        virtual bool DrawImpl(RnModelDrawWork& Work, URnSideWalk& Self) override
        {
            Way(Option.ShowInsideWay).Draw(Work,Self.GetInsideWay(), Work.visibleType);
            Way(Option.ShowOutsideWay).Draw(Work, Self.GetOutsideWay(), Work.visibleType);
            Way(Option.ShowStartEdgeWay).Draw(Work, Self.GetStartEdgeWay(), Work.visibleType);
            Way(Option.ShowEndEdgeWay).Draw(Work, Self.GetEndEdgeWay(), Work.visibleType);
          
            return true;
        }
    };

    Road RoadDrawer(RoadOption);
    Intersection IntersectionDrawer(IntersectionOption);
    SideWalk SideWalkDrawer(SideWalkOption);

    for (auto road : Model->GetRoads()) {
        RoadDrawer.Draw(Work, road, ERnModelDrawerVisibleType::NonSelected);
    }

    for (auto intersection : Model->GetIntersections()) {
        IntersectionDrawer.Draw(Work, intersection, ERnModelDrawerVisibleType::NonSelected);
    }

    for (auto sideWalk : Model->GetSideWalks()) {
        SideWalkDrawer.Draw(Work, sideWalk, ERnModelDrawerVisibleType::NonSelected);
    }
}
