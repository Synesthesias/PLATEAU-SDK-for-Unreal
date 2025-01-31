#pragma once

#include "CoreMinimal.h"
#include "AxisPlane.h"
#include "LineSegment3D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"

class PLATEAURUNTIME_API FGeoGraphEx
{
public:

    template<class T>
    struct Edge
    {
    public:
        Edge(const T& P0, const T& P1)
            : P0(P0)
            , P1(P1) {
        }
        const T& P0;
        const T& P1;    
    };

    template<class T>
    class EdgeEnumerator
    {
        using InnerValueType = T;
    public:
        
        class Iterator
        {
        public:
            Iterator(const TArray<T>& vertices, bool isLoop, int it, int nextIt)
            : Vertices(vertices)
            , IsLoop(isLoop)
            , It(it)
            , NextIt(nextIt)
            { }

            Iterator& operator++()
            {
                // 最後まで行ったらループする
                if (It == Vertices.Num())
                    It = 0;
                else
                    ++It;

                if (NextIt == Vertices.Num())
                    NextIt = 0;
                else
                    ++NextIt;
                return *this;
            }

            Iterator& operator++(int) {
                Iterator Temp = *this;
                ++It;
                return Temp;
            }

            Edge<InnerValueType> operator*() const {
                auto Get = [&](int I) -> InnerValueType
                {
                    if (I == Vertices.Num())
                        return Vertices[0];
                    return Vertices[I];
                };
                return Edge<InnerValueType>(Get(It), Get(NextIt));
            }

            bool operator==(const Iterator& Other) const {
                return It == Other.It && NextIt == Other.NextIt;
            }

            bool operator!=(const Iterator& Other) const {
                return !(*this == Other);
            }
        private:
            const TArray<T>& Vertices;
            bool IsLoop;
            int It;
            int NextIt;
        };

    public:
        EdgeEnumerator(const TArray<T>& vertices, bool isLoop)
        : IsLoop(isLoop)
        , Vertices(vertices)
        {}

        Iterator begin() const
        {
            // 要素が0の場合空のイテレータを返す
            if (Vertices.Num() == 0 || Vertices.Num() == 1)
                return Iterator(Vertices, IsLoop, Vertices.Num(), Vertices.Num());

            return Iterator(Vertices, IsLoop, 0, 1);
        }

        Iterator end() const
        {
            // 要素が0の場合空のイテレータを返す
            if (Vertices.Num() == 0 || Vertices.Num() == 1)
                return Iterator(Vertices, IsLoop, Vertices.Num(), Vertices.Num());

            if(IsLoop)
                return Iterator(Vertices, IsLoop, Vertices.Num(), 0);

            return Iterator(Vertices, IsLoop, Vertices.Num() - 1, Vertices.Num());
        }

        const bool IsLoop;
        const TArray<T>& Vertices;
    };
public:
    // T型の頂点配列Verticesで構成される線分リストに対し
    // (Vertices[0], Vertices[1]), (Vertices[1], Vertices[2]), ... (Vertices[N-1], Vertices[0])となる線分リストを取得
    // bIsLoopがtrueの場合、最後の線分は(Vertices[N-1], Vertices[0])となる
    template<typename T>
    static EdgeEnumerator<T> GetEdges(const TArray<T>& Vertices, bool bIsLoop)
    {
        return EdgeEnumerator<T>(Vertices, bIsLoop);
    }

    static TArray<FLineSegment3D> GetEdgeSegments(const TArray<FVector>& Vertices, bool bIsLoop)
    {
        TArray<FLineSegment3D> Segments;
        for (auto& Edge : GetEdges(Vertices, bIsLoop)) {
            Segments.Add(FLineSegment3D(Edge.P0, Edge.P1));
        }
        return Segments;    
    }

    static TArray<FLineSegment2D> GetEdgeSegments(const TArray<FVector2D>& Vertices, bool bIsLoop) {
        TArray<FLineSegment2D> Segments;
        for (auto& Edge : GetEdges(Vertices, bIsLoop)) {
            Segments.Add(FLineSegment2D(Edge.P0, Edge.P1));
        }
        return Segments;
    }

   /* template<typename T>
    static TArray<TTuple<T, T>> GetEdges(const TArray<T>& Vertices, bool bIsLoop);*/

    static TArray<FVector> GetInnerLerpSegments(
        const TArray<FVector>& LeftVertices,
        const TArray<FVector>& RightVertices,
        EAxisPlane Plane,
        float P,
        float CheckMeter = 3.f);


    static TArray<FIntVector> GetNeighborDistance3D(int32 D);
    static TArray<FIntPoint> GetNeighborDistance2D(int32 D);

    static  bool IsCollinear(const FVector& A, const FVector& B, const FVector& C, float DegEpsilon, float MidPointTolerance);
    static TMap<FVector, FVector> MergeVertices(const TArray<FVector>& Vertices, float CellSize, int32 MergeCellLength);


private:
    static bool IsInInnerSide(const TOptional<FLineSegment3D>& Edge, const FVector& Direction, bool bReverse, bool bIsPrev);
    static bool CheckCollision(const FVector& A, const FVector& B, const TArray<FLineSegment3D>& Edges, float IndexF);
};

