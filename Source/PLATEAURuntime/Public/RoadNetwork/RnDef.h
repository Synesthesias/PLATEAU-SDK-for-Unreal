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

    static FVector2D To2D(const FVector& Vector);
};


template<class T>
struct RnRef
{
    using Type = TSharedPtr<T>;
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
    return MakeShared<T>(std::forward<TArgs>(Args)...);
}

//// DynamicCastのラッパー
//// #NOTE : RnRef_t<U>だとinfer substitutionに失敗するのでTSharedPtr<U>にしておく
//template<class T, class U>
//auto RnCast(TSharedPtr<U> In) -> RnRef_t<T> {
//    return RnRef_t<T>(dynamic_cast<T*>(In.Get()));
//}
