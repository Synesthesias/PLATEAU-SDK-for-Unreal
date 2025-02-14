#pragma once
#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "GeoGraph/AxisPlane.h"
#include "PLATEAURnDef.generated.h"
class UObject;
UENUM(BlueprintType)
enum class EPLATEAURnDir : uint8 {
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right"),
};


UENUM(BlueprintType)
enum class EPLATEAURnLaneBorderType {
    Prev UMETA(DisplayName = "Prev"),
    Next UMETA(DisplayName = "Next"),
};

UENUM(Meta = (Flags))
enum class EPLATEAURnSideWalkWayTypeMask : uint8 {
    None = 0,
    Outside = 1 << 0,
    Inside = 1 << 1,
    StartEdge = 1 << 2,
    EndEdge = 1 << 3,
};
ENUM_CLASS_FLAGS(EPLATEAURnSideWalkWayTypeMask);

// 道路専用. 手動編集で道路の左側/右側どっちに所属するかが取りたいみたいなので専用フラグを用意する.
// 交差点だと意味がない
UENUM()
enum class EPLATEAURnSideWalkLaneType : uint8 {
    // 交差点 or その他(デフォルト値)
    Undefined,
    // 左レーン
    LeftLane,
    // 右レーン
    RightLane,
};


enum class EPLATEAURnLaneBorderDir {
    // LeftWay -> RightWay
    Left2Right,
    // RightWay -> LeftWay
    Right2Left
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAURnDirEx {
    GENERATED_BODY()

public:
    /// <summary>
    /// 反対を取得
    /// </summary>
    /// <param name="dir"></param>
    /// <returns></returns>
    static EPLATEAURnDir GetOpposite(EPLATEAURnDir dir);
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAURnLaneBorderDirEx {
    GENERATED_BODY()

public:
    /// <summary>
    /// 反対を取得
    /// </summary>
    /// <param name="dir"></param>
    /// <returns></returns>
    static EPLATEAURnLaneBorderDir GetOpposite(EPLATEAURnLaneBorderDir dir);
};


USTRUCT()
struct PLATEAURUNTIME_API FPLATEAURnLaneBorderTypeEx {
    GENERATED_BODY()

public:
    /// <summary>
    /// 反対を取得
    /// </summary>
    /// <param name="dir"></param>
    /// <returns></returns>
    static EPLATEAURnLaneBorderType GetOpposite(EPLATEAURnLaneBorderType dir);
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAURnDef
{
    GENERATED_BODY()
public: 
    static constexpr EAxisPlane Plane = EAxisPlane::Xy;

    // 1[m]のUnreal上での単位
    static constexpr float Meter2Unit = 100;

    static UObject* GetNewObjectWorld();

    static void SetNewObjectWorld(UObject* World);

    static FVector2D To2D(const FVector& Vector);

private:
    static inline UObject* NewObjectWorld = nullptr;
};


template<class T>
struct TPLATEAURnRef
{
    using Type = T*;

    template<class... Args>
    static Type New(Args&&... args) {
        auto Ret = NewObject<T>(FPLATEAURnDef::GetNewObjectWorld());
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
using TRnRef_T = typename TPLATEAURnRef<T>::Type;

// もしかしたらRn~はUObjectになるかもしれないので念のためラップしておく
// Rn~のオブジェクトを生成する
template<class T, class... TArgs>
inline TRnRef_T<T> RnNew(TArgs&&... Args)
{
    return TPLATEAURnRef<T>::New(Forward<TArgs>(Args)...);
}

template<class T>
inline TRnRef_T<T> RnFrom(TWeakObjectPtr<T> Ptr) {
    return TPLATEAURnRef<T>::From(Ptr);
}

template<class T>
inline TRnRef_T<T> RnFrom(T* Ptr) {
    return TPLATEAURnRef<T>::From(Ptr);
}
template<class T>
inline TRnRef_T<T> RnFrom(TObjectPtr<T> Ptr) {
    return TPLATEAURnRef<T>::From(Ptr);
}

#define PLATEAU_RN_DETAIL_LOG
