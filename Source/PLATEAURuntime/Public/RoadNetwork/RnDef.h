#pragma once
#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "GeoGraph/AxisPlane.h"
#include "RnDef.generated.h"

UENUM(BlueprintType)
enum class ERnDir : uint8 {
    Left UMETA(DisplayName = "XLeft"),
    Right UMETA(DisplayName = "Right"),
};


UENUM(BlueprintType)
enum class ERnLaneBorderType {
    Prev UMETA(DisplayName = "Prev"),
    Next UMETA(DisplayName = "Next"),
};


USTRUCT()
struct PLATEAURUNTIME_API FRnLaneBorderTypeEx {
    GENERATED_BODY()

public:
    /// <summary>
    /// 反対を取得
    /// </summary>
    /// <param name="dir"></param>
    /// <returns></returns>
    static ERnLaneBorderType GetOpposite(ERnLaneBorderType dir);
};

UENUM(Meta = (Flags))
enum class ERnSideWalkWayTypeMask : uint8 {
    None = 0,
    Outside = 1 << 0,
    Inside = 1 << 1,
    StartEdge = 1 << 2,
    EndEdge = 1 << 3,
};
ENUM_CLASS_FLAGS(ERnSideWalkWayTypeMask);

// 道路専用. 手動編集で道路の左側/右側どっちに所属するかが取りたいみたいなので専用フラグを用意する.
// 交差点だと意味がない
UENUM()
enum class ERnSideWalkLaneType : uint8 {
    // 交差点 or その他(デフォルト値)
    Undefined,
    // 左レーン
    LeftLane,
    // 右レーン
    RightLane,
};


enum class ERnLaneBorderDir {
    // LeftWay -> RightWay
    Left2Right,
    // RightWay -> LeftWay
    Right2Left
};

USTRUCT()
struct PLATEAURUNTIME_API FRnDirEx {
    GENERATED_BODY()

public:
    /// <summary>
    /// 反対を取得
    /// </summary>
    /// <param name="dir"></param>
    /// <returns></returns>
    static ERnDir GetOpposite(ERnDir dir);
};

USTRUCT()
struct PLATEAURUNTIME_API FRnDef
{
    GENERATED_BODY()
public: 
    static constexpr EAxisPlane Plane = EAxisPlane::Xy;

    // 1[m]のUnreal上での単位
    static constexpr float Meter2Unit = 100;


    static FVector2D To2D(const FVector& Vector);
};


template<class T>
struct TRnRef
{
    using Type = T*;

    template<class... Args>
    static Type New(Args&&... args) {
        auto Ret = NewObject<T>();
        Ret->Init(Forward<Args>(args)...);
        return Ret;
    }

    static Type From(TWeakObjectPtr<T> Ptr) {
        return Ptr.Get();
    }
    static Type From(T* Ptr) {
        return Ptr;
    }
    static Type From(TObjectPtr<T> Ptr) {
        return Ptr.Get();
    }
};

// もしかしたらRn~はUObjectになるかもしれないので念のためラップしておく
// Rn~のオブジェクトの参照を表す
template<class T>
using TRnRef_T = typename TRnRef<T>::Type;

// もしかしたらRn~はUObjectになるかもしれないので念のためラップしておく
// Rn~のオブジェクトを生成する
template<class T, class... TArgs>
inline TRnRef_T<T> RnNew(TArgs&&... Args)
{
    return TRnRef<T>::New(Forward<TArgs>(Args)...);
}

template<class T>
inline TRnRef_T<T> RnFrom(TWeakObjectPtr<T> Ptr) {
    return TRnRef<T>::From(Ptr);
}

template<class T>
inline TRnRef_T<T> RnFrom(T* Ptr) {
    return TRnRef<T>::From(Ptr);
}
template<class T>
inline TRnRef_T<T> RnFrom(TObjectPtr<T> Ptr) {
    return TRnRef<T>::From(Ptr);
}
struct FRnPartsBase
{
public:

    uint32 GetDebugMyId() const { return DebugId; }
protected:
    FRnPartsBase(uint32 Id) : DebugId(Id) {}
private:
    uint32 DebugId;
};

template<typename TSelf>
struct FRnParts : public FRnPartsBase
{
    inline static uint32 Counter = 0;
    FRnParts();
};
