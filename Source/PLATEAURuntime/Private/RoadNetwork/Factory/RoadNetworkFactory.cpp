#include "RoadNetwork/Factory/RoadNetworkFactory.h"

#include <plateau/dataset/i_dataset_accessor.h>

#include "Editor.h"
#include "Algo/Count.h"
#include "RoadNetwork/CityObject/PLATEAUSubDividedCityObjectGroup.h"
#include "RoadNetwork/CityObject/SubDividedCityObjectFactory.h"
#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/RGraph/PLATEAURGraph.h"
#include "RoadNetwork/RGraph/RGraph.h"
#include "RoadNetwork/RGraph/RGraphEx.h"
#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLineString.h"
#include "RoadNetwork/Structure/RnWay.h"


const FString FRoadNetworkFactory::FactoryVersion = TEXT("1.0.0");

namespace
{
    class FWork;
    class FTran;

    enum ERoadType
    {
        Road,
        Intersection,
        Terminate,
        Isolated
    };

    class FTranLine {
    public:
        FTran* Neighbor;
        TArray<RGraphRef_t<UREdge>> Edges;
        TArray<RGraphRef_t<URVertex>> Vertices;
        TRnRef_T<URnWay> Way;
        TSharedPtr<FTranLine> Next;
        TSharedPtr<FTranLine> Prev;

        bool IsBorder() const {
            return Neighbor != nullptr;
        }
    };

    class FTran {
    public:
        FWork& Work;
        RGraphRef_t<URGraph> Graph;
        TSet<RGraphRef_t<URFace>> Roads;
        RGraphRef_t<URFaceGroup> FaceGroup;
        TArray<RGraphRef_t<URVertex>> Vertices;
        TArray<TSharedPtr<FTranLine>> Lines;
        TRnRef_T<URnRoadBase> Node;
        TArray<TRnRef_T<URnLane>> Lanes;

        auto&& GetFaces() const
        {
            return FaceGroup->GetFaces();
        }


        ERoadType GetRoadType() const
        {
            switch (GetNeighborCount()) {
            case 0:
                return Isolated;
            case 1:
                return Terminate;
            case 2:
                return Road;
            default:
                return Intersection;
            }
        }

        int32_t GetNeighborCount() const
        {
            return Algo::CountIf(Lines, [](const TSharedPtr<FTranLine>& L) {return L->IsBorder(); });
        }

        FTran(FWork& W, const RGraphRef_t<URGraph>& G, RGraphRef_t<URFaceGroup>& FG)
        : Work(W)
        , Graph(G)
        , FaceGroup(FG)
        {
            Vertices = FRGraphEx::ComputeOutlineVertices(FaceGroup, [](RGraphRef_t<URFace> Face)
            {
                    return FRRoadTypeMaskEx::HasAnyFlag(Face->GetRoadTypes(), ERRoadTypeMask::SideWalk) == false;
            });

            if (FGeoGraph2D::IsClockwise<RGraphRef_t<URVertex>>(Vertices, [](RGraphRef_t<URVertex> v)
            {
                    return FRnDef::To2D(v->Position);
            })) {
                Algo::Reverse(Vertices);
            }
        }

        TRnRef_T<URnRoadBase> CreateRoad();

        void Build()
        {
            Node = CreateRoad();
        }

        void BuildConnection();

        bool BuildLine();
    };

    class FWork {
    public:
        TMap<RGraphRef_t<URFaceGroup>, TSharedPtr<FTran>> TranMap;
        TMap<RGraphRef_t<URVertex>, TRnRef_T<URnPoint>> PointMap;
        TMap<TArray<RGraphRef_t<URVertex>>, TRnRef_T<URnLineString>> LineMap;
        TArray<TRnRef_T<URnLineString>> PointLineStringCache;
        TMap<uint64, TArray<TRnRef_T<URnLineString>>> RnPointList2LineStringMap;
        float TerminateAllowEdgeAngle = 20.0f;
        float TerminateSkipAngleDeg = 30.0f;


        static bool IsEqual(const TArray<RGraphRef_t<URVertex>>& A, const TArray<RGraphRef_t<URVertex>>& B, bool& IsReverse)
        {
            if (A.Num() != B.Num()) {
                return false;
            }

            if (A.Num() == 0) {
                return true;
            }

            IsReverse = A[0] != B[0];
            for (auto i = 0; i < A.Num(); ++i) 
            {
                auto AIndex = i;
                auto BIndex = i;
                if (IsReverse)
                    BIndex = B.Num() - 1 - i;
                if (A[AIndex] != B[BIndex]) {
                    return false;
                }
            }
            return true;
        }

        static bool IsEqual(const TArray<TRnRef_T<URnPoint>>& A, const TArray<TRnRef_T<URnPoint>>& B, bool& IsReverse) {
            if (A.Num() != B.Num()) {
                return false;
            }

            if (A.Num() == 0) {
                return true;
            }

            IsReverse = A[0] != B[0];
            for (auto i = 0; i < A.Num(); ++i) {
                auto AIndex = i;
                auto BIndex = i;
                if (IsReverse)
                    BIndex = B.Num() - 1 - i;
                if (A[AIndex] != B[BIndex]) {
                    return false;
                }
            }
            return true;
        }


        TRnRef_T<URnWay> CreateWay(const TArray<TRnRef_T<URnPoint>>& Vertices, bool& IsCached)
        {
            for (auto& Ls : PointLineStringCache) {
                bool IsReverse;
                if (IsEqual(Ls->GetPoints(), Vertices, IsReverse)) 
                {
                    IsCached = true;
                    return RnNew<URnWay>(Ls, IsReverse);
                }
            }
            auto LineString = URnLineString::Create(Vertices);
            PointLineStringCache.Add(LineString);
            return RnNew<URnWay>(LineString, false);
        }


        TRnRef_T<URnWay> CreateWay(const TArray<RGraphRef_t<URVertex>>& Vertices, bool& IsCached)
        {
            for (auto& Pair : LineMap) 
            {
                bool IsReverse;
                if (IsEqual(Pair.Key, Vertices, IsReverse)) {
                    IsCached = true;
                    return RnNew<URnWay>(Pair.Value, IsReverse) ;
                }
            }

            TArray<TRnRef_T<URnPoint>> Points;
            for (const auto& Vertex : Vertices) {
                auto Point = GetOrCreatePoint(Vertex);
                Points.Add(Point);
            }

            auto Key = Vertices;
            auto LineString = URnLineString::Create(Points);
            LineMap.Add(Key, LineString);

            return RnNew<URnWay>(LineString, false);
        }


        TRnRef_T<URnWay> CreateWay(const TArray<RGraphRef_t<URVertex>>& Vertices) {
            bool IsCached;
            return CreateWay(Vertices, IsCached);
        }

        TRnRef_T<URnPoint> GetOrCreatePoint(const RGraphRef_t<URVertex>& Vertex) {
            if (auto Found = PointMap.Find(Vertex)) {
                return *Found;
            }

            auto Point = RnNew<URnPoint>(Vertex->Position);
            PointMap.Add(Vertex, Point);
            return Point;
        }


        TArray<TRnRef_T<URnSideWalk>> CreateSideWalk(float minRoadSize, float lod1SideWalkSize)
        {
            TArray<TRnRef_T<URnSideWalk>> ret;
            return ret;
        }


        FTran* FindTranOrDefault(URFace* Face)
        {
            for (auto& Pair : TranMap)
            {
                if (Pair.Key->GetFaces().Contains(Face)) {
                    return Pair.Value.Get();
                }
            }
            return nullptr;
        }

    };

    TRnRef_T<URnRoadBase> FTran::CreateRoad()
    {
        auto CityObjectGroup = FaceGroup->CityObjectGroup;
        auto RoadType = GetRoadType();
        if(RoadType == ERoadType::Isolated)
        {
            // 孤立した中央分離帯は道路構造の生成対象から外す
            // (道路に完全内包する中央分離帯だったりと無意味なので)
            if (FRRoadTypeMaskEx::IsMedian(FaceGroup->GetRoadTypes())) {
                return nullptr;
            }
            auto way = Work.CreateWay(Vertices);
            return URnRoad::CreateIsolatedRoad(CityObjectGroup.Get(), way);
        }

        // 行き止まり
        if (RoadType == ERoadType::Terminate) 
        {
            // #TODO : RN
            auto way = Work.CreateWay(Vertices);
            return URnRoad::CreateIsolatedRoad(CityObjectGroup.Get(), way);
        }

        // 通常の道
        if (RoadType == ERoadType::Road) {
            auto lanes = Lines.FilterByPredicate([](const TSharedPtr<FTranLine>& L) {return L->IsBorder() == false; });

            using KeyType = TTuple<TSharedPtr<FTranLine>, TSharedPtr<FTranLine>>;
            auto GetKey = [](TSharedPtr<FTranLine> A, TSharedPtr<FTranLine> B)-> KeyType
            {
                if (A.Get() < B.Get())
                    Swap(A, B);

                return MakeTuple(A, B);
                };
            auto Road = RnNew<URnRoad>(CityObjectGroup.Get());

            TMap<KeyType, TArray<TSharedPtr<FTranLine>>> Groups;
            for (auto&& Lane : lanes) 
            {
                auto&& Key = GetKey(Lane->Prev, Lane->Next);
                Groups.FindOrAdd(Key).Add(Lane);
            }
            TSharedPtr<FTranLine> Left = nullptr;
            TSharedPtr<FTranLine> Right = nullptr;
            for(auto G : Groups)
            {
                auto& LaneLines = G.Value;
                auto L = LaneLines.Num() > 0 ? LaneLines[0] : nullptr;
                auto R = LaneLines.Num() > 1 ? LaneLines[1] : nullptr;
                if (L && L->Way || L->Way->IsReversed)
                    Swap(L, R);
                Left = L;
                Right = R;
                if(L && R)
                    break;
            }

            if (!Left && !Right) 
            {
                UE_LOG(LogTemp, Error, TEXT("不正なレーン構成(Wayの存在しないLane). %s"), *FaceGroup->CityObjectGroup->GetName());
                return Road;
            }

            if (!Left || !Right) {
                UE_LOG(LogTemp, Warning, TEXT("不正なレーン構成(片側Wayのみ存在). %s"), *(FaceGroup->CityObjectGroup->GetName()));
            }
            auto line = Left ? Left : Right;
            auto prevBorderLine = line->Prev; ;
            auto nextBorderLine = line->Next;
            auto prevBorderWay = prevBorderLine ? prevBorderLine->Way : nullptr;
            auto nextBorderWay = nextBorderLine ? nextBorderLine->Way : nullptr;

            auto leftWay = Left ? Left->Way : nullptr;
            auto rightWay = Right ? Right->Way : nullptr;

            // 方向そろえる
            if (Left != nullptr && Left->Prev != prevBorderLine)
                leftWay = leftWay->ReversedWay();
            if (Right != nullptr && Right->Prev != prevBorderLine)
                rightWay = rightWay->ReversedWay();
            if (rightWay != nullptr)
                rightWay->IsReverseNormal = true;

            auto lane = RnNew<URnLane>(leftWay, rightWay, prevBorderWay, nextBorderWay);
            Road->AddMainLane(lane);
            return Road;
        }

        // 交差点
        if (RoadType == ERoadType::Intersection) {
            auto intersection = URnIntersection::Create(TObjectPtr<UPLATEAUCityObjectGroup>(CityObjectGroup.Get()));
            return intersection;
        }

        UE_LOG(LogTemp, Error, TEXT("不正なレーン構成 %s"), *FaceGroup->CityObjectGroup->GetName());
        return nullptr;
    }

    void FTran::BuildConnection()
    {
        if (!Node)
            return;
        if (auto road = Node->CastToRoad()) 
        {
            auto lane = road->MainLanes.Num() > 0 ? road->MainLanes[0] : nullptr;
            if (lane == nullptr)
                return;
            auto Prev =Lines.FindByPredicate([lane](TSharedPtr<FTranLine> L) {
                if (!L->IsBorder() || !L->Way)
                    return false;
                return lane->PrevBorder && lane->PrevBorder->IsSameLineReference(L->Way);
                });

            auto Next = Lines.FindByPredicate([lane](TSharedPtr<FTranLine> L) {
                if (!L->IsBorder() || !L->Way)
                    return false;
                return lane->NextBorder && lane->NextBorder->IsSameLineReference(L->Way);
                });

            auto GetNode = [](TSharedPtr<FTranLine> L)-> TRnRef_T<URnRoadBase>
            {
                if (L == nullptr)
                    return nullptr;
                return L->Neighbor ? L->Neighbor->Node : nullptr;
                };
            auto prevRoad = GetNode(Prev ? *Prev : nullptr);
            auto nextRoad = GetNode(Next ? *Next : nullptr);
            road->SetPrevNext(prevRoad, nextRoad);
        }
        else if (auto intersection = Node->CastToIntersection() ) 
        {
            // 境界線情報
            for(auto B : Lines)
            {
                intersection->AddEdge(B->Neighbor ? B->Neighbor->Node : nullptr, B->Way);
            }
        }
    }

    bool FTran::BuildLine()
    {
        auto Edges = TArray<RGraphRef_t<UREdge>>();
        Edges.SetNum(Vertices.Num());
        auto Neighbors = TArray<FTran*>();
        Neighbors.SetNum(Vertices.Num());
        auto Success = true;
        for (auto i = 0; i < Vertices.Num(); i++) {
            auto V0 = Vertices[i];
            auto V1 = Vertices[(i + 1) % Vertices.Num()];

            TObjectPtr<UREdge> E;                
            if(FRnEx::TryFirstOrDefault(
                V0->GetEdges()
                , [V0, V1](TObjectPtr<UREdge> E)-> bool {
                    return E->IsSameVertex(V0, V1);
                }, E) == false)
            {
                UE_LOG(LogTemp,Error, TEXT("ループしていないメッシュ. %s"), *FaceGroup->CityObjectGroup->GetName());
                Success = false;
                continue;        
            }

            Edges[i] = E;
            for (auto F : E->GetFaces()) 
            {
                auto Tran = Work.FindTranOrDefault(F);
                if (Tran != nullptr && Tran != this) {
                    Neighbors[i] = Tran;
                    break;
                }
            }
        }
        auto startIndex = 0;
        for (auto i = 1; i < Edges.Num(); i++) {
            if (Neighbors[i] != Neighbors[0]) {
                startIndex = i;
                break;
            }
        }

        TSharedPtr<FTranLine> line = nullptr;
        for (auto x = 0; x < Edges.Num(); x++)
        {
            auto i = (x + startIndex) % Edges.Num();
            auto v = Vertices[i];
            auto e = Edges[i];
            auto n = Neighbors[i];
            // 切り替わり発生したら新しいLineを作成
            if (line == nullptr || line->Neighbor != n) {
                if (line != nullptr)
                    line->Vertices.Add(v);
                auto next = MakeShared<FTranLine>();
                next->Neighbor = n;
                next->Prev = line;
                if (line != nullptr)
                    line->Next = next;
                line = next;
                Lines.Add(line);
            }

            line->Edges.Add(e);
            line->Vertices.Add(v);
        }

        if (line != nullptr && Lines.Num() > 1) {
            line->Vertices.Add(Vertices[startIndex]);
            line->Next = Lines[0];
            Lines[0]->Prev = line;
        }

        // Wayを先に作っておく
        for(auto l : Lines)
            l->Way = Work.CreateWay(l->Vertices);
        return Success;
    }
}

void FRoadNetworkFactoryEx::CreateRnModel(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, APLATEAURnStructureModel* DestActor)
{
    TArray<UPLATEAUCityObjectGroup*> CityObjectGroups;
    Actor->GetComponents(CityObjectGroups);
    auto res = CreateRoadNetwork(Self, Actor, DestActor, CityObjectGroups);
}

TRnRef_T<URnModel> FRoadNetworkFactoryEx::CreateRoadNetwork(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, APLATEAURnStructureModel* DestActor,
                                                        TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups)
{
    if(DestActor->GetRootComponent() == nullptr)
    {
        auto DefaultSceneRoot = NewObject<USceneComponent>(DestActor, TEXT("DefaultSceneRoot"));
        DestActor->AddOwnedComponent(DefaultSceneRoot);
        DestActor->SetRootComponent(DefaultSceneRoot);
        DefaultSceneRoot->RegisterComponent();
    }

    const auto Root = DestActor->GetRootComponent();
    TArray<FSubDividedCityObject> SubDividedCityObjects;
    CreateSubDividedCityObjects(Self, Actor, DestActor, Root, CityObjectGroups, SubDividedCityObjects);

    RGraphRef_t<URGraph> Graph;
    CreateRGraph(Self, Actor, DestActor, Root, SubDividedCityObjects, Graph);

    DestActor->Model = CreateRnModel(Self, Graph);
    return nullptr;
}

TRnRef_T<URnModel> FRoadNetworkFactoryEx::CreateRnModel(
    const FRoadNetworkFactory& Self
    , RGraphRef_t<URGraph> Graph)
{
    auto Model = RnNew<URnModel>();
    try {
        // 道路/中央分離帯は一つのfaceGroupとしてまとめる
        auto&& mask = ~(ERRoadTypeMask::Road | ERRoadTypeMask::Median);
        auto&& faceGroups = FRGraphEx::GroupBy(Graph, [mask](const RGraphRef_t<URFace>& F0, const RGraphRef_t<URFace>& F1) {
            auto&& M0 = F0->GetRoadTypes() & mask;
            auto&& M1 = F1->GetRoadTypes() & mask;
            return M0 == M1;
            });

        Model->FactoryVersion = Self.FactoryVersion;

        FWork work;
        work.TerminateAllowEdgeAngle = Self.TerminateAllowEdgeAngle;
        work.TerminateSkipAngleDeg = Self.TerminateSkipAngle;
    
        
        for(auto&& faceGroup : faceGroups) {
            auto&& roadType = faceGroup->GetRoadTypes();

            // 道路を全く含まない時は無視
            if (FRRoadTypeMaskEx::IsRoad(roadType) == false)
                continue;
            if (FRRoadTypeMaskEx::IsSideWalk(roadType))
                continue;
            // ignoreHighway=trueの時は高速道路も無視
            if (FRRoadTypeMaskEx::IsHighWay(roadType) && Self.bIgnoreHighway)
                continue;
            work.TranMap.Add(faceGroup, MakeShared<FTran>(work, Graph, faceGroup));
        }

        // 作成したFTranを元にRoadを作成
        for(auto&& pair : work.TranMap)
            pair.Value->BuildLine();

        for(auto&& pair : work.TranMap) {
            auto&& tran = pair.Value;
            tran->Build();
            if (tran->Node != nullptr)
                Model->AddRoadBase(tran->Node);
        }

        for(auto&& pair : work.TranMap) {
            auto&& tran = pair.Value;
            tran->BuildConnection();
        }

        if (Self.bAddSideWalk) 
        {
            // 歩道を作成する
            auto&& sideWalks = work.CreateSideWalk(Self.Lod1SideWalkThresholdRoadWidth, Self.Lod1SideWalkSize);
            for (auto&& sideWalk : sideWalks)
                Model->AddSideWalk(sideWalk);

            for(auto&& fg : faceGroups) 
            {
                for(auto&& sideWalkFace : fg->GetFaces()) 
                {
                    if (FRRoadTypeMaskEx::IsSideWalk(sideWalkFace->GetRoadTypes()) == false)
                        continue;
                    TArray<RGraphRef_t<UREdge>> OutsideEdges;
                    TArray<RGraphRef_t<UREdge>> InsideEdges;
                    TArray<RGraphRef_t<UREdge>> StartEdges;
                    TArray<RGraphRef_t<UREdge>> EndEdges;

                    if (FRGraphEx::CreateSideWalk(sideWalkFace, OutsideEdges, InsideEdges, StartEdges, EndEdges) == false)
                        continue;


                    auto AsWay = [&](const TArray<RGraphRef_t<UREdge>>& edges) -> TRnRef_T<URnWay>
                    {
                        if (edges.IsEmpty())
                            return nullptr;
                        TArray<RGraphRef_t<URVertex>> Vertices;
                        bool IsLoop;
                        bool isCached = false;
                        if (FRGraphEx::SegmentEdge2VertexArray(edges, Vertices, IsLoop))
                            return work.CreateWay(Vertices, isCached);
                        return nullptr;
                    };
                    auto&& outsideWay = AsWay(OutsideEdges);
                    auto&& insideWay = AsWay(InsideEdges);
                    auto&& startWay = AsWay(StartEdges);
                    auto&& endWay = AsWay(EndEdges);
                    auto ParentPair = Algo::FindByPredicate(work.TranMap, [&](const TTuple<RGraphRef_t<URFaceGroup>, TSharedPtr<FTran>>& X) {
                        return X.Value->FaceGroup->CityObjectGroup == sideWalkFace->GetCityObjectGroup() && X.Value->Node != nullptr;
                        });
                    ERnSideWalkLaneType laneType = ERnSideWalkLaneType::Undefined;

                    TRnRef_T<URnRoadBase> Parent = nullptr;
                    if (ParentPair != nullptr && ParentPair->Value && ParentPair->Value->Node)
                    {
                        Parent = ParentPair->Value->Node;
                    }

                    if (auto road = Parent->CastToRoad()) {
                        auto&& way = road->GetMergedSideWay(ERnDir::Left);
                        if (insideWay != nullptr) {
                            // #NOTE : 自動生成の段階だと線分共通なので同一判定でチェックする
                            // #TODO : 自動生成の段階で分かれているケースが存在するならは点や法線方向で判定するように変える
                            if (way == nullptr)
                                laneType = ERnSideWalkLaneType::Undefined;
                            else if (insideWay->IsSameLineReference(way))
                                laneType = ERnSideWalkLaneType::LeftLane;
                            else
                                laneType = ERnSideWalkLaneType::RightLane;
                        }
                    }

                    auto&& sideWalk = URnSideWalk::Create(Parent, outsideWay, insideWay, startWay, endWay, laneType);
                    Model->AddSideWalk(sideWalk);
                }
            }
        }

        // 交差点の
        //if (bSeparateContinuousBorder)
        //    ret->SeparateContinuousBorder();

        // 中央分離帯の幅で道路を分割する
        //{
        //    auto&& visited = new HashSet<RnRoad>();
        //    for(auto&& n : work->TranMap.Values) {
        //        if (n.Node is RnRoad road) {
        //            if (visited.Contains(road))
        //                continue;
        //            auto&& medianWidth = n.GetMedianWidth();
        //            if (medianWidth <= 0f)
        //                continue;
        //            auto&& linkGroup = road.CreateRoadGroupOrDefault();
        //            if (linkGroup == nullptr)
        //                continue;

        //            // 中央分離帯の幅が道路の幅を超えている場合は分割
        //            if (road.MainLanes.Count > 0) {
        //                auto&& borderWidth = road.MainLanes[0].CalcWidth();
        //                if (CheckMedian && borderWidth > medianWidth) {
        //                    linkGroup.SetLaneCountWithMedian(1, 1, medianWidth / borderWidth);
        //                }
        //            }

        //            for(auto&& r : linkGroup.Roads)
        //                visited.Add(r);
        //        }
        //    }
        //}

        //// 連続した道路を一つにまとめる
        //if (MergeRoadGroup)
        //    ret.MergeRoadGroup();


        //// 交差点との境界線が垂直になるようにする
        //if (CalibrateIntersection && CalibrateIntersectionOption != nullptr) {
        //    ret.CalibrateIntersectionBorderForAllRoad(CalibrateIntersectionOption);
        //}

        //// 道路を分割する
        //ret->SplitLaneByWidth(RoadSize, false, out auto&& failedLinks);
        //ret->ReBuildIntersectionTracks();

        //// 信号制御器をデフォ値で配置する
        //if (AddTrafficSignalLights)
        //    ret.AddDefaultTrafficSignalLights();
        
    }
    catch(std::exception e)
    {
        
    }


    return Model;
}

void FRoadNetworkFactoryEx::CreateSubDividedCityObjects(
    const FRoadNetworkFactory& Self
    , APLATEAUInstancedCityModel* Actor
    , AActor* DestActor
    , USceneComponent* Root
    , TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups
    , TArray<FSubDividedCityObject>& OutSubDividedCityObjects)
{
    // 一番子のオブジェクトだけが必要なのでそれを抽出する

    TArray<FSubDividedCityObject> Result;
    struct FSubDividedObjectVisitor {
        static void Visit(FSubDividedCityObject& So, TArray<FSubDividedCityObject>& Result) {
            if (So.Children.Num() == 0) {
                if (So.SelfRoadType != ERRoadTypeMask::Undefined) {
                    Result.Add(So);                   
                }
                return;
            }

            for (auto& Child : So.Children) {
                Visit( Child, Result);
            }
        }
    };
   
    FSubDividedCityObjectFactory Factory;
    auto SubDividedObjectResult = Factory.ConvertCityObjectsAsync(Actor, CityObjectGroups, true);
    for (auto C : SubDividedObjectResult->ConvertedCityObjects) {
        FSubDividedObjectVisitor::Visit(*C, OutSubDividedCityObjects);
    }

    const auto SubDividedObjectName = TEXT("SubDivided");
    
    if(Self.bSaveTmpData)
    {
        auto SubDividedCityObjectGroup = FRnEx::GetOrCreateInstanceComponentWithName<UPLATEAUSubDividedCityObjectGroup>(DestActor, Root, SubDividedObjectName);
        if (SubDividedCityObjectGroup == nullptr) {
            SubDividedCityObjectGroup = NewObject<UPLATEAUSubDividedCityObjectGroup>(DestActor, SubDividedObjectName);
            FRnEx::AddChildInstanceComponent(DestActor, Root, SubDividedCityObjectGroup);
        }

        // 現在の子は削除する
        auto SubDividedCityObjects = SubDividedCityObjectGroup->GetCityObjects();
        for (auto& C : SubDividedCityObjects) {
            C->DestroyComponent(false);
            DestActor->RemoveInstanceComponent(C);
        }

        for (auto& So : OutSubDividedCityObjects) 
        {
            auto NewCityObject = NewObject<UPLATEAUSubDividedCityObject>(DestActor, FName(So.Name));
            NewCityObject->CityObject = So;
            FRnEx::AddChildInstanceComponent(DestActor, SubDividedCityObjectGroup, NewCityObject);
        }
    }
    else
    {
        auto SubDividedCityObjectGroup = Cast<UPLATEAUSubDividedCityObjectGroup>(DestActor->GetDefaultSubobjectByName(SubDividedObjectName));
        if(SubDividedCityObjectGroup)
            SubDividedCityObjectGroup->DestroyComponent(false);
    }
    

}

void FRoadNetworkFactoryEx::CreateRGraph(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor, USceneComponent* Root, TArray<FSubDividedCityObject>& SubDividedCityObjects,
    RGraphRef_t<URGraph>& OutGraph)
{
    OutGraph = FRGraphFactoryEx::CreateGraph(Self.GraphFactory, SubDividedCityObjects);

    const auto RGraphName = TEXT("RGaph");
    if(Self.bSaveTmpData)
    {
        auto RGraphObject = FRnEx::GetOrCreateInstanceComponentWithName<UPLATEAURGraph>(DestActor, Root, RGraphName);
        if (RGraphObject == nullptr) {
            RGraphObject = NewObject<UPLATEAURGraph>(DestActor, RGraphName);
            FRnEx::AddChildInstanceComponent(DestActor, Root, RGraphObject);
        }
        RGraphObject->RGraph = OutGraph;
    }
    else
    {
        auto RGraphObject = Cast<UPLATEAURGraph>(DestActor->GetDefaultSubobjectByName(RGraphName));
        if (RGraphObject)
            RGraphObject->DestroyComponent(false);
    }
}
