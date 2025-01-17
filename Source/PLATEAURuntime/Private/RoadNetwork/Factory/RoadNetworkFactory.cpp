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
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLineString.h"


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
        TArray<TSharedPtr<UREdge>> Edges;
        TArray<RGraphRef_t<URVertex>> Vertices;
        TRnRef_T<URnWay> Way;
        TSharedPtr<FTranLine> Next;
        TSharedPtr<FTranLine> Prev;

        bool IsBorder() const {
            return Neighbor == nullptr;
        }
    };

    class FTran {
    public:
        TSharedPtr<FWork> Work;
        RGraphRef_t<URGraph> Graph;
        TSet<RGraphRef_t<URFace>> Roads;
        RGraphRef_t<URFaceGroup> FaceGroup;
        TArray<RGraphRef_t<URVertex>> Vertices;
        TArray<TSharedPtr<FTranLine>> Lines;
        TRnRef_T<URnRoadBase> Node;
        TArray<TRnRef_T<URnLane>> Lanes;

        auto GetFaces() const
        {
            return FaceGroup->Faces;
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

        FTran(const TSharedPtr<FWork>& W, const RGraphRef_t<URGraph>& G, RGraphRef_t<URFaceGroup>& FG)
        : Work(W)
        , Graph(G)
        , FaceGroup(FG)
        {
            Vertices = FRGraphEx::ComputeOutlineVertices(FaceGroup, [](RGraphRef_t<URFace> Face)
            {
                    return FRRoadTypeEx::HasAnyFlag(Face->GetRoadTypes(), ERRoadTypeMask::SideWalk) == false;
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

        void BuildConnection()
        {
            // #TODO : RN
        }

        void BuildLine()
        {
            // #TODO : RN
        }

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
                if (IsEqual(*Ls->Points, Vertices, IsReverse)) 
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
    };

    TRnRef_T<URnRoadBase> FTran::CreateRoad()
    {
        auto CityObjectGroup = FaceGroup->CityObjectGroup;
        auto RoadType = GetRoadType();
        if(RoadType == ERoadType::Isolated)
        {
            // 孤立した中央分離帯は道路構造の生成対象から外す
            // (道路に完全内包する中央分離帯だったりと無意味なので)
            if (FRRoadTypeEx::IsMedian(FaceGroup->GetRoadTypes())) {
                return nullptr;
            }
            auto way = Work->CreateWay(Vertices);
            return URnRoad::CreateIsolatedRoad(CityObjectGroup.Get(), way);
        }

        // 行き止まり
        if (RoadType == ERoadType::Terminate) 
        {
            // #TODO : RN
            auto way = Work->CreateWay(Vertices);
            return URnRoad::CreateIsolatedRoad(CityObjectGroup.Get(), way);
        }
        // #TODO : RN
        return nullptr;
    }

}

void FRoadNetworkFactoryEx::CreateRnModel(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor)
{
    TArray<UPLATEAUCityObjectGroup*> CityObjectGroups;
    Actor->GetComponents(CityObjectGroups);
    auto res = CreateRoadNetwork(Self, Actor, DestActor, CityObjectGroups);
}

TRnRef_T<URnModel> FRoadNetworkFactoryEx::CreateRoadNetwork(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor,
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
    CreateRGraph(Self, Actor, DestActor, Root, CityObjectGroups, SubDividedCityObjects, Graph);
    return nullptr;
}

TRnRef_T<URnModel> FRoadNetworkFactoryEx::CreateRoadNetwork(const FRoadNetworkFactory& Self, RGraphRef_t<URGraph> Graph)
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

        auto&& ret = TRnRef_T<URnModel>();
        ret->FactoryVersion = Self.FactoryVersion;

        auto&& work = MakeShared<FWork>();
        work->TerminateAllowEdgeAngle;
        work->TerminateSkipAngleDeg = Self.TerminateSkipAngle;
    
        
        for(auto&& faceGroup : faceGroups) {
            auto&& roadType = faceGroup->GetRoadTypes();

            // 道路を全く含まない時は無視
            if (FRRoadTypeEx::IsRoad(roadType) == false)
                continue;
            if (FRRoadTypeEx::IsSideWalk(roadType))
                continue;
            // ignoreHighway=trueの時は高速道路も無視
            if (FRRoadTypeEx::IsHighWay(roadType) && Self.bIgnoreHighway)
                continue;
            work->TranMap[faceGroup] = MakeShared<FTran>(work, Graph, faceGroup);
        }

        // 作成したFTranを元にRoadを作成
        for(auto&& pair : work->TranMap)
            pair.Value->BuildLine();
        for(auto&& pair : work->TranMap) {
            auto&& tran = pair.Value;
            tran->Build();
            if (tran->Node != nullptr)
                ret->AddRoadBase(tran->Node);
        }

        for(auto&& pair : work->TranMap) {
            auto&& tran = pair.Value;
            tran->BuildConnection();
        }

        if (Self.bAddSideWalk) 
        {
            // 歩道を作成する
            auto&& sideWalks = work->CreateSideWalk(Self.Lod1SideWalkThresholdRoadWidth, Self.Lod1SideWalkSize);
            for (auto&& sideWalk : sideWalks)
                ret->AddSideWalk(sideWalk);

            //for(auto&& fg : faceGroups) {
            //    for(auto&& sideWalkFace : fg.Faces.Where(f = > f.RoadTypes.IsSideWalk())) {
            //        if (sideWalkFace.CreateSideWalk(out auto&& outsideEdges, out auto&& insideEdges,
            //            out auto&& startEdges, out auto&& endEdges) == false)
            //            continue;

            //        RnWay AsWay(IReadOnlyList<REdge> edges, out bool isCached) {
            //            isCached = false;
            //            if (edges.Any() == false)
            //                return nullptr;
            //            if (RGraphEx.SegmentEdge2Vertex(edges, out auto&& vertices, out auto&& isLoop))
            //                return work->CreateWay(vertices, out isCached);
            //            return nullptr;
            //        }
            //        auto&& outsideWay = AsWay(outsideEdges, out auto&& outsideCached);
            //        auto&& insideWay = AsWay(insideEdges, out auto&& insideCached);
            //        auto&& startWay = AsWay(startEdges, out auto&& startCached);
            //        auto&& endWay = AsWay(endEdges, out auto&& endCached);
            //        auto&& parent = work->TranMap.Values.FirstOrDefault(t = >
            //            t.FaceGroup.CityObjectGroup == sideWalkFace.CityObjectGroup && t.Node != nullptr);

            //        RnSideWalkLaneType laneType = RnSideWalkLaneType.Undefined;
            //        if (parent ? .Node is RnRoad road) {
            //            auto&& way = road.GetMergedSideWay(RnDir.Left);
            //            if (insideWay != nullptr) {
            //                // #NOTE : 自動生成の段階だと線分共通なので同一判定でチェックする
            //                // #TODO : 自動生成の段階で分かれているケースが存在するならは点や法線方向で判定するように変える
            //                if (way == nullptr)
            //                    laneType = RnSideWalkLaneType.Undefined;
            //                else if (insideWay.IsSameLineReference(way))
            //                    laneType = RnSideWalkLaneType.LeftLane;
            //                else
            //                    laneType = RnSideWalkLaneType.RightLane;
            //            }
            //        }

            //        auto&& sideWalk = RnSideWalk.Create(parent ? .Node, outsideWay, insideWay, startWay, endWay, laneType);
            //        ret.AddSideWalk(sideWalk);
            //    }
            //}
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

void FRoadNetworkFactoryEx::CreateRGraph(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor, USceneComponent* Root,
    TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups, TArray<FSubDividedCityObject>& SubDividedCityObjects,
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
