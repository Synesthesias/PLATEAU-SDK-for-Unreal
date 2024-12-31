#pragma once
#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "GeoGraph/AxisPlane.h"
#include "RnDef.generated.h"

USTRUCT()
struct PLATEAURUNTIME_API FRnDef
{
    GENERATED_BODY()
public: 
    static constexpr EAxisPlane Plane = EAxisPlane::Xy;
};


template<class T>
struct RnRef
{
    using Type = std::shared_ptr<T>;
};

// もしかしたらRn~はUObjectになるかもしれないので念のためラップしておく
// Rn~のオブジェクトの参照を表す
template<class T>
using RnRef_t = typename RnRef<T>::Type;


// もしかしたらRn~はUObjectになるかもしれないので念のためラップしておく
// Rn~のオブジェクトを生成する
template<class T, class... TArgs>
RnRef_t<T> RnNew(TArgs&&... Args)
{
    return std::make_shared<T>(std::forward<TArgs>(Args)...);
}