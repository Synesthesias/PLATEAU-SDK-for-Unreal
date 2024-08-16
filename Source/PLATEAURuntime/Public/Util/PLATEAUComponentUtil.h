// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include <plateau/dataset/city_model_package.h>
#include <PLATEAUInstancedCityModel.h>

class PLATEAURUNTIME_API FPLATEAUComponentUtil {
public:
    // 元の名前の末尾に_{数値}がある場合元の名前が復元不可能になるため毎回ユニーク化
    // ユニーク化後は{元の名前}__{数値}
    static FString MakeUniqueGmlObjectName(AActor* Actor, UClass* Class, const FString& BaseName);

    /**
     * @brief Componentのユニーク化されていない元の名前を取得します。
     * コンポーネント名の末尾に"__{数値}"が存在する場合、ユニーク化の際に追加されたものとみなし、"__"以降を削除します。
     * 元の名前に"__{数値}"が存在する可能性もあるので、基本的に地物ID、Lod以外を取得するのには使用しないでください。
     */
    static FString GetOriginalComponentName(const USceneComponent* InComponent);

    static USceneComponent* FindChildComponentWithOriginalName(USceneComponent* ParentComponent, const FString& OriginalName);

    /**
     * @brief 元のComponentを記憶します
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: Component Name(GmlID), Value: Component の Map
     */
    static TMap<FString, UPLATEAUCityObjectGroup*> CreateComponentsMapWithGmlId(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    // ComponentのパスをキーとしたMapを生成(/区切り）
    static TMap<FString, UPLATEAUCityObjectGroup*> CreateComponentsMapWithNodePath(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);

    /**
     * @brief 指定コンポーネントを基準として子階層のCityObjectを取得
     * @param SceneComponent CityObject取得の基準となるコンポーネント
     * @param RootCityObjects 取得されたCityObject配列
     */
    static TArray<FPLATEAUCityObject> GetRootCityObjects(USceneComponent* SceneComponent);

    static TArray<UActorComponent*> GetComponentsByPackage(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, EPLATEAUCityModelPackage Pkg);

    /**
     * @brief ComponentをDestroy又は、Visible:falseにします
     * @param bDestroy true:Destoryします, false: Visibleをfalseにします
     */
    static void DestroyOrHideComponents(TArray<UPLATEAUCityObjectGroup*> Components, bool bDestroy);

    /**
     * @brief Lodを名前として持つComponentの名前をパースし、Lodを数値として返します。
     */
    static int ParseLodComponent(const USceneComponent* InLodComponent);

    /**
     * @brief 3D都市モデル内に含まれるLodを取得します。
     * @param InPackage 検索対象のパッケージ。フラグによって複数指定可能です。
     * @return 存在するLodの最小最大値
     */
    static FPLATEAUMinMaxLod GetMinMaxLod(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const plateau::dataset::PredefinedCityModelPackage InPackage);

    /**
     * @brief TArray<UActorComponent*>をTArray<USceneComponent*>に変換します
     * @param InComponents UActorComponentの配列
     * @return Typeのみ変換された配列
     */
    static TArray<USceneComponent*> ConvertArrayToSceneComponentArray(TArray<UActorComponent*> InComponents);

};
