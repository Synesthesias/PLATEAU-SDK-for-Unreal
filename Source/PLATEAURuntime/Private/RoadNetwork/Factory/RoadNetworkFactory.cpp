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
#include "RoadNetwork/Util/PLATEAURnLinq.h"


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

    // 輪郭線分構造
    class FTranLine {
    public:
        // 隣接している道路
        FTran* Neighbor;
        // 線分リスト
        TArray<RGraphRef_t<UREdge>> Edges;
        // 頂点リスト
        TArray<RGraphRef_t<URVertex>> Vertices;
        // 線分リストをRnWayに変換したもの
        TRnRef_T<URnWay> Way;
        // 次の輪郭線分
        TSharedPtr<FTranLine> Next;
        // ひとつ前の輪郭線分
        TSharedPtr<FTranLine> Prev;

        bool IsBorder() const {
            return Neighbor != nullptr;
        }
    };

    // 道路構造に対応
    class FTran {
    public:
        FWork& Work;
        RGraphRef_t<URGraph> Graph;
        TSet<RGraphRef_t<URFace>> Roads;
        RGraphRef_t<URFaceGroup> FaceGroup;
        // 輪郭の頂点配列
        TArray<RGraphRef_t<URVertex>> Vertices;
        // 輪郭の線分配列
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

        // 最大LODレベルを取得
        int32_t GetLodLevel() const
        {
            auto Ret = 0;
            for(const auto Face : GetFaces())
            {
                Ret = FMath::Max(Ret, Face->GetLodLevel());
            }
            return Ret;
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
                    return FPLATEAURnDef::To2D(v->Position);
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



        TRnRef_T<URnWay> CreateWay(const TArray<RGraphRef_t<URnPoint>>& Points) {
            bool IsCached;
            return CreateWay(Points, IsCached);
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


        TArray<TRnRef_T<URnSideWalk>> CreateSideWalk(float MinRoadSize, float Lod1SideWalkSize)
        {
            if (Lod1SideWalkSize < 0) {
                return TArray<URnSideWalk*>();
            }

            struct FPointMoveInfo
            {
                TRnRef_T<URnPoint> Point;
                TArray<FVector> Normal;
            };
            TMap<TRnRef_T<URnPoint>, TArray<TRnRef_T<URnSideWalk>>> WaySideWalks;
            TMap<TRnRef_T<URnPoint>, FPointMoveInfo> Visited;
            TMap<TRnRef_T<URnRoad>, float> AddedRoads;

            TArray<TRnRef_T<URnSideWalk>> ReturnSideWalks;
            auto MoveWay = [&](URnWay* Way, URnRoadBase* Parent, EPLATEAURnSideWalkLaneType LaneType, float SideWalkSize)
            {
                if (!Way || SideWalkSize <= 0.0f) {
                    return false;
                }

                TArray<FVector> Normals;
                for (int32 i = 0; i < Way->Count(); ++i) {
                    Normals.Add(Way->GetVertexNormal(i));
                }

                TArray<URnPoint*> OutsidePoints;
                for (int32 i = 0; i < Way->Count(); ++i) {
                    URnPoint* P = Way->GetPoint(i);
                    FVector N = Normals[i];

                    if (!Visited.Contains(P)) {
                        URnPoint* Before = RnNew<URnPoint>(P->Vertex);
                        P->Vertex -= N * SideWalkSize;

                        FPointMoveInfo VertexInfo;
                        VertexInfo.Point = Before;
                        VertexInfo.Normal.Add(N);
                        Visited.Add(P, VertexInfo);
                        OutsidePoints.Add(Before);
                    }
                    else {
                        auto& Last = Visited[P];
                        for (const FVector& NN : Last.Normal) {
                            N -= FVector::DotProduct(N, NN) * NN;
                        }
                        P->Vertex -= N * SideWalkSize;
                        Last.Normal.Add(N);
                        OutsidePoints.Add(Last.Point);
                    }
                }
                ;
                URnWay* StartWay = CreateWay(TArray{ OutsidePoints[0], Way->GetPoint(0) });
                URnWay* EndWay = CreateWay(TArray{OutsidePoints.Last(), Way->GetPoint(-1)});
                URnWay* OutsideWay = CreateWay(OutsidePoints);

                URnSideWalk* SideWalk = URnSideWalk::Create(Parent, OutsideWay, Way, StartWay, EndWay, LaneType);
                ReturnSideWalks.Add(SideWalk);

                return true;
                };
            // 道路の幅によってはLOD1でも歩道を作らない場合もある

            // そのため先に道路に対して歩道を作成する
            // そのあと交差点に対して歩道を作成する
            // その際, 各輪郭線に対してそれに接続する道路が
            // 　1. 歩道を作成した(十分な道幅があった)
            //   2. LOD2以上だった
            // のどれかの場合のみ作成するようにする.
            // 交差点は広く見えても接続する道路が狭い場合.

            for (const auto& TranPair : TranMap) {
                const auto& Tran = TranPair.Value;

                // Only process LODLevel 1
                if (Tran->GetRoadType() != 1) {
                    if (URnRoad* Road = Cast<URnRoad>(Tran->Node)) {
                        AddedRoads.Add(Road, Lod1SideWalkSize);
                    }
                    continue;
                }

                if (URnRoad* Road = Cast<URnRoad>(Tran->Node)) {
                    URnLane* LeftLane = Road->MainLanes.Num() > 0 ? Road->MainLanes[0] : nullptr;
                    URnLane* RightLane = Road->MainLanes.Num() > 0 ? Road->MainLanes.Last() : nullptr;

                    float SideWalkSize;
                    if (LeftLane == RightLane) {
                        float LeftWidth = FMath::Min(LeftLane->CalcMinWidth(), RightLane->CalcMinWidth());
                        SideWalkSize = FMath::Min(LeftWidth - MinRoadSize * 2, Lod1SideWalkSize);
                    }
                    else {
                        float LeftWidth = LeftLane->CalcMinWidth();
                        float RightWidth = RightLane->CalcMinWidth();
                        SideWalkSize = FMath::Min3(LeftWidth - MinRoadSize, RightWidth - MinRoadSize, Lod1SideWalkSize);
                    }

                    if (SideWalkSize < Lod1SideWalkSize) {
                        continue;
                    }

                    AddedRoads.Add(Road, SideWalkSize);
                }
            }

            // Process roads and intersections
            for (const auto& TranPair : TranMap) {
                const auto& Tran = TranPair.Value;

                if (Tran->GetLodLevel() != 1) {
                    continue;
                }

                if (URnRoad* Road = Cast<URnRoad>(Tran->Node)) {
                    float SideWalkSize;
                    if (!AddedRoads.Contains(Road)) {
                        continue;
                    }
                    SideWalkSize = AddedRoads[Road];

                    URnLane* LeftLane = Road->GetMainLanes().Num() > 0 ? Road->GetMainLanes()[0] : nullptr;
                    URnLane* RightLane = Road->GetMainLanes().Num() > 0 ? Road->GetMainLanes().Last() : nullptr;

                    MoveWay(LeftLane ? LeftLane->GetLeftWay() : nullptr, Road, EPLATEAURnSideWalkLaneType::LeftLane, SideWalkSize);
                    MoveWay(RightLane ? RightLane->GetRightWay() : nullptr, Road, EPLATEAURnSideWalkLaneType::RightLane, SideWalkSize);
                }
                else if (URnIntersection* Intersection = Cast<URnIntersection>(Tran->Node)) 
                {
                    auto EdgeGroups = FRnIntersectionEx::CreateEdgeGroup(Intersection);
                    for (const auto& Eg : EdgeGroups) 
                    {
                        // 不正なエッジや境界線はスキップ
                        if (Eg.IsBorder() || !Eg.IsValid()) {
                            continue;
                        }

                        auto IsTarget = [&](const FRnIntersectionEx::FEdgeGroup* E, float& OutSwSize) -> bool
                        {
                            OutSwSize = 0.0f;
                            if (E == &Eg) {
                                return false;
                            }
                            if (URnRoad* R = Cast<URnRoad>(E->Key)) {
                                float* FoundSize = AddedRoads.Find(R);
                                if (FoundSize) {
                                    OutSwSize = *FoundSize;
                                    return true;
                                }
                                return false;
                            }
                            OutSwSize = Lod1SideWalkSize;
                            return true;
                        };

                        float LeftSwSize, RightSwSize;
                        // このEdgeGroupと隣接する道路のどっちかが歩道対象じゃない場合は無視する
                        if (!IsTarget(Eg.LeftSide, LeftSwSize) || !IsTarget(Eg.RightSide, RightSwSize)) {
                            continue;
                        }

                        float SideWalkSize = FMath::Min3(LeftSwSize, RightSwSize, Lod1SideWalkSize);
                        // lod1SizeWalkSize以下になる場合は作らない
                        if (SideWalkSize < Lod1SideWalkSize) {
                            continue;
                        }

                        for (const auto& Edge : Eg.Edges) {
                            MoveWay(Edge->GetBorder(), Intersection, EPLATEAURnSideWalkLaneType::Undefined, SideWalkSize);
                        }
                    }
                }
            }

            return ReturnSideWalks;
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
                return lane->GetPrevBorder() && lane->GetPrevBorder()->IsSameLineReference(L->Way);
                });

            auto Next = Lines.FindByPredicate([lane](TSharedPtr<FTranLine> L) {
                if (!L->IsBorder() || !L->Way)
                    return false;
                return lane->GetNextBorder() && lane->GetNextBorder()->IsSameLineReference(L->Way);
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
            if(FPLATEAURnLinq::TryFirstOrDefault(
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

TRnRef_T<URnModel> FRoadNetworkFactoryEx::CreateRoadNetwork(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* TargetCityModel, APLATEAURnStructureModel* Actor,
                                                        TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups)
{
#if WITH_EDITOR
    const auto Root = Actor->GetRootComponent();
    TArray<FSubDividedCityObject> SubDividedCityObjects;
    CreateSubDividedCityObjects(Self, TargetCityModel, Actor, Root, CityObjectGroups, SubDividedCityObjects);

    RGraphRef_t<URGraph> Graph;
    CreateRGraph(Self, TargetCityModel, Actor, Root, SubDividedCityObjects, Graph);
    Actor->Model = CreateRnModel(Self, Graph);
    return Actor->Model;
#else
    return nullptr;
#endif
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

        Model->SetFactoryVersion(Self.FactoryVersion);

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
                    EPLATEAURnSideWalkLaneType laneType = EPLATEAURnSideWalkLaneType::Undefined;

                    TRnRef_T<URnRoadBase> Parent = nullptr;
                    if (ParentPair != nullptr && ParentPair->Value && ParentPair->Value->Node)
                    {
                        Parent = ParentPair->Value->Node;
                    }
                    if(Parent)
                    {
                        if (auto road = Parent->CastToRoad()) {
                            auto&& way = road->GetMergedSideWay(EPLATEAURnDir::Left);
                            if (insideWay != nullptr) {
                                // #NOTE : 自動生成の段階だと線分共通なので同一判定でチェックする
                                // #TODO : 自動生成の段階で分かれているケースが存在するならは点や法線方向で判定するように変える
                                if (way == nullptr)
                                    laneType = EPLATEAURnSideWalkLaneType::Undefined;
                                else if (insideWay->IsSameLineReference(way))
                                    laneType = EPLATEAURnSideWalkLaneType::LeftLane;
                                else
                                    laneType = EPLATEAURnSideWalkLaneType::RightLane;
                            }
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

        // 連続した道路を一つにまとめる
        if (Self.bMergeRoadGroup) {
            Model->MergeRoadGroup();
        }


        //// 交差点との境界線が垂直になるようにする
        //if (CalibrateIntersection && CalibrateIntersectionOption != nullptr) {
        //    ret.CalibrateIntersectionBorderForAllRoad(CalibrateIntersectionOption);
        //}

        // 道路を分割する
        TArray<FString> FailedRoads;
        Model->SplitLaneByWidth(Self.RoadSize, false, FailedRoads);        
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
        auto SubDividedCityObjectGroup = FPLATEAURnEx::GetOrCreateInstanceComponentWithName<UPLATEAUSubDividedCityObjectGroup>(DestActor, Root, SubDividedObjectName);

        // 現在の子は削除する
        auto SubDividedCityObjects = SubDividedCityObjectGroup->GetCityObjects();
        for (auto& C : SubDividedCityObjects) {
            C->DestroyComponent(false);
            DestActor->RemoveInstanceComponent(C);
        }

        for (auto& So : OutSubDividedCityObjects) 
        {
            auto UniqueName = MakeUniqueObjectName(Actor, UPLATEAUSubDividedCityObject::StaticClass(), FName(So.Name));
            auto NewCityObject = NewObject<UPLATEAUSubDividedCityObject>(DestActor, UniqueName);
            NewCityObject->CityObject = So;
            FPLATEAURnEx::AddChildInstanceComponent(DestActor, SubDividedCityObjectGroup, NewCityObject);
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
        auto RGraphObject = FPLATEAURnEx::GetOrCreateInstanceComponentWithName<UPLATEAURGraph>(DestActor, Root, RGraphName);
        if (RGraphObject == nullptr) {
            RGraphObject = NewObject<UPLATEAURGraph>(DestActor, RGraphName);
            FPLATEAURnEx::AddChildInstanceComponent(DestActor, Root, RGraphObject);
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
