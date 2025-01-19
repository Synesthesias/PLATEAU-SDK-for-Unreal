#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <optional>

#include "RoadNetwork/GeoGraph/LineSegment3D.h"

class UPLATEAUCityObjectGroup;
class URnIntersection;
class URnLineString;
class URnWay;
class URnPoint;
class URnLane;


class FLineCrossPointResult {
public:
    class FTargetLineInfo {
    public:
        TRnRef_T<URnLineString> LineString;
        TArray<TTuple<float, FVector>> Intersections;
    };

    TArray<FTargetLineInfo> TargetLines;
    FLineSegment3D LineSegment;
};

struct FRnEx
{

    class Vector3Comparer {
    public:
        int32 operator()(const FVector& A, const FVector& B) const;
    };


public:

    static TArray<TRnRef_T<UPLATEAUCityObjectGroup>> GetSceneSelectedCityObjectGroups();

    template<typename T>
    static void Replace(TArray<T>& Self, T Before, T After);

    static void ReplaceLane(TArray<TRnRef_T<URnLane>>& Self, TRnRef_T<URnLane> Before, TRnRef_T<URnLane> After);

    static TRnRef_T<URnLineString> CreateInnerLerpLineString(
        const TArray<FVector>& LeftVertices,
        const TArray<FVector>& RightVertices,
        TRnRef_T<URnPoint> Start,
        TRnRef_T<URnPoint> End,
        TRnRef_T<URnWay> StartBorder,
        TRnRef_T<URnWay> EndBorder,
        float T,
        float PointSkipDistance = 1e-3f);

    static FLineCrossPointResult GetLineIntersections(
        const FLineSegment3D& LineSegment,
        const TArray<TRnRef_T<URnWay>>& Ways);


    template<typename T, typename U>
    static TArray<U> Map(const TArray<T>& Src, TFunction< U(const T&)> Selector)
    {
        TArray<U> Result;
        Result.Reserve(Src.Num());
        for (const auto& S : Src) {
            Result.Add(Selector(S));
        }
        return Result;
    }

    template<typename T, typename F>
    static bool TryFirstOrDefault(const TSet<T>& Set, F&& Predicate, T& OutV) {
        for (auto& S : Set) {
            if (Predicate(S)) {
                OutV = S;
                return true;
            }
        }
        return false;
    }

    template<typename T, typename F>
    static auto SelectWithIndex(T&& Src, F&& Selector)
    {
        using U = decltype(Selector(Src[0], 0));
        TArray<U> Result;
        auto Index = 0;
        for (auto&& E : Src) 
        {
            Result.Add(Selector(E, Index));
            Index++;
        }
        return Result;
    }

    template<typename T>
    static int32 Compare(T A, T B)
    {
        if (A < B)
            return -1;
        if (A > B)
            return 1;
        return 0;
    }




    // ParentにChildを追加する
    // ActorにAddInstanceComponentをして, ChildをParentにアタッチする
    static void AddChildInstanceComponent(AActor* Actor, USceneComponent* Parent, USceneComponent* Child, FAttachmentTransformRules TransformRule = FAttachmentTransformRules::KeepRelativeTransform);

    // SelfのT型の子コンポーネントを取得する
    // bIncludeAllDescendants : 子孫も含めるか
    // bIncludeSelf : 自分自身も含めるか
    template<typename T>
    static TArray<T*> GetChildrenComponents(USceneComponent* Self, bool bIncludeAllDescendants = true, bool bIncludeSelf = false)
    {
        TArray<T*> Children;
        if (bIncludeSelf && Cast<T>(Self)) {
            Children.Add(Cast<T>(Self));
        }
        TArray<USceneComponent*> Components;
        Self->GetChildrenComponents(bIncludeAllDescendants, Components);
        for (auto Child : Components) {
            if (auto C = Cast<T>(Child)) {
                Children.Add(C);
            }
        }
        return Children;
    }

    template<typename T>
    static T* GetOrCreateInstanceComponentWithName(AActor* Actor, USceneComponent* Root, const FName& Name)
    {
        if (!Actor) 
            return nullptr;
        auto Component = Actor->GetComponentByClass<T>();

        if (!Component) {
            auto UniqueName = MakeUniqueObjectName(Actor, T::StaticClass(), Name);
            Component = NewObject<T>(Actor, UniqueName);
            AddChildInstanceComponent(Actor, Actor->GetRootComponent(), Component);
        }
        return Cast<T>(Component);
    }
};
template<typename T>
void FRnEx::Replace(TArray<T>& Self, T Before, T After) {
    for (int32 i = 0; i < Self.Num(); i++) {
        if (Self[i] == Before) {
            Self[i] = After;
        }
    }
}

