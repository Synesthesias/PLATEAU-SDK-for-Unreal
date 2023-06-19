// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUInstancedCityModel.h"

#include "Tasks/Task.h"

#include "Misc/DefaultValueHelper.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "CityGML/PLATEAUCityGmlProxy.h"

using namespace UE::Tasks;

namespace {
    /**
     * @brief Componentのユニーク化されていない元の名前を取得します。
     * コンポーネント名の末尾に"_{数値}"が存在する場合、ユニーク化の際に追加されたものとみなし、"_"以降を削除します。
     * 元の名前に"_{数値}"が存在する可能性もあるので、基本的に地物ID、Lod以外を取得するのには使用しないでください。
     */
    FString GetOriginalComponentName(const USceneComponent* const InComponent) {
        auto ComponentName = InComponent->GetName();
        int Index = 0;
        if (ComponentName.FindLastChar('_', Index)) {
            if (ComponentName.RightChop(Index + 1).IsNumeric()) {
                ComponentName = ComponentName.LeftChop(ComponentName.Len() - Index);
            }
        }
        return ComponentName;
    }

    /**
     * @brief Lodを名前として持つComponentの名前をパースし、Lodを数値として返します。
     */
    int ParseLodComponent(const USceneComponent* const InLodComponent) {
        auto LodString = GetOriginalComponentName(InLodComponent);
        // "Lod{数字}"から先頭3文字除外することで数字を抜き出す。
        LodString = LodString.RightChop(3);

        int Lod;
        FDefaultValueHelper::ParseInt(LodString, Lod);

        return Lod;
    }

    /**
     * @brief 3D都市モデル内のCityGMLファイルに相当するコンポーネントを入力として、CityGMLファイル名を返します。
     * @return CityGMLファイル名
     */
    FString GetGmlFileName(const USceneComponent* const InGmlComponent) {
        return InGmlComponent->GetName().Append(".gml");
    }

    /**
     * @brief Gmlコンポーネントのパッケージ情報を取得します。
     */
    plateau::dataset::PredefinedCityModelPackage GetCityModelPackage(const USceneComponent* const InGmlComponent) {
        const auto GmlFileName = GetGmlFileName(InGmlComponent);
        // udxのサブフォルダ名は地物種類名に相当するため、UdxSubFolderの関数を使用してgmlのパッケージ種を取得
        return plateau::dataset::UdxSubFolder::getPackage(plateau::dataset::GmlFile(TCHAR_TO_UTF8(*GmlFileName)).getFeatureType());
    }
}

// Sets default values
APLATEAUInstancedCityModel::APLATEAUInstancedCityModel() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

FPLATEAUCityObjectInfo APLATEAUInstancedCityModel::GetCityObjectInfo(USceneComponent* Component) {
    FPLATEAUCityObjectInfo Result;
    Result.DatasetName = DatasetName;

    if (Component == nullptr)
        return Result;

    Result.ID = GetOriginalComponentName(Component);

    auto GmlComponent = Component;
    while (GmlComponent->GetAttachParent() != RootComponent) {
        GmlComponent = GmlComponent->GetAttachParent();

        // TODO: エラーハンドリング
        if (GmlComponent == nullptr)
            return Result;
    }

    Result.GmlName = GetGmlFileName(GmlComponent);

    return Result;
}

void APLATEAUInstancedCityModel::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUInstancedCityModel::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

plateau::dataset::PredefinedCityModelPackage APLATEAUInstancedCityModel::GetCityModelPackages() const {
    auto Packages = plateau::dataset::PredefinedCityModelPackage::None;
    for (const auto& GmlComponent : GetGmlComponents()) {
        Packages = Packages | GetCityModelPackage(GmlComponent);
    }
    return Packages;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByLods(const plateau::dataset::PredefinedCityModelPackage InPackage, const TMap<plateau::dataset::PredefinedCityModelPackage, FPLATEAUMinMaxLod>& PackageToLodRangeMap, const bool bOnlyMaxLod) {
    bIsFiltering = true;

    for (const auto& GmlComponent : GetGmlComponents()) {
        // 一度全てのメッシュを不可視にする
        GmlComponent->SetVisibility(false, true);

        // 選択されていないパッケージを除外
        const auto Package = GetCityModelPackage(GmlComponent);
        if ((Package & InPackage) == plateau::dataset::PredefinedCityModelPackage::None)
            continue;

        TArray<USceneComponent*> LodComponents;
        GmlComponent->GetChildrenComponents(false, LodComponents);

        // 各地物について全てのLodを表示する場合の処理
        if (!bOnlyMaxLod) {
            for (const auto& LodComponent : LodComponents) {
                const auto Lod = ParseLodComponent(LodComponent);
                if (PackageToLodRangeMap[Package].MinLod <= Lod && Lod <= PackageToLodRangeMap[Package].MaxLod)
                    LodComponent->SetVisibility(true, true);
            }
            continue;
        }

        for (const auto& LodComponent : LodComponents) {
            const auto Lod = ParseLodComponent(LodComponent);
            if (Lod == PackageToLodRangeMap[Package].MaxLod) {
                TArray<USceneComponent*> FeatureComponents;
                LodComponent->GetChildrenComponents(true, FeatureComponents);
                for (const auto& FeatureComponent : FeatureComponents) {
                    FeatureComponent->SetVisibility(true, true);
                }
            }
        }
    }

    bIsFiltering = false;
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypes(const citygml::CityObject::CityObjectsType InCityObjectType) {
    bIsFiltering = true;
    Launch(
        TEXT("ParseGmlsTask"),
        [this, InCityObjectType, GmlComponents = GetGmlComponents()] {
            // 処理が重いため先にCityGMLのパースを行って内部的にキャッシュしておく。
            for (const auto& GmlComponent : GmlComponents) {
                // 起伏は重いため意図的に除外
                const auto Package = GetCityModelPackage(GmlComponent);
                if (Package == plateau::dataset::PredefinedCityModelPackage::Relief)
                    continue;

                FPLATEAUCityObjectInfo GmlInfo;
                GmlInfo.DatasetName = DatasetName;
                GmlInfo.GmlName = GetGmlFileName(GmlComponent);
                const auto CityModel = UPLATEAUCityGmlProxy::Load(GmlInfo);
            }

            // フィルタリング実行。スレッドセーフでない関数を使用するためメインスレッドで実行する。
            const auto GameThreadTask =
                FFunctionGraphTask::CreateAndDispatchWhenReady(
                    [this, InCityObjectType] {
                        FilterByFeatureTypesInternal(InCityObjectType);
                    }, TStatId(), nullptr, ENamedThreads::GameThread);
            GameThreadTask->Wait();

            bIsFiltering = false;
        },
        ETaskPriority::BackgroundHigh);

    return this;
}

FPLATEAUMinMaxLod APLATEAUInstancedCityModel::GetMinMaxLod(const plateau::dataset::PredefinedCityModelPackage InPackage) const {
    TArray<int> Lods;

    for (const auto& GmlComponent : GetGmlComponents()) {
        if ((GetCityModelPackage(GmlComponent) & InPackage) == plateau::dataset::PredefinedCityModelPackage::None)
            continue;

        for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
            const auto Lod = ParseLodComponent(LodComponent);
            if (Lods.Contains(Lod))
                continue;

            Lods.Add(Lod);
        }
    }

    return { FMath::Min(Lods), FMath::Max(Lods) };
}

bool APLATEAUInstancedCityModel::IsFiltering() {
    return bIsFiltering;
}

const TArray<TObjectPtr<USceneComponent>>& APLATEAUInstancedCityModel::GetGmlComponents() const {
    return GetRootComponent()->GetAttachChildren();
}

void APLATEAUInstancedCityModel::FilterByFeatureTypesInternal(const citygml::CityObject::CityObjectsType InCityObjectType) {
    for (const auto& GmlComponent : GetRootComponent()->GetAttachChildren()) {
        // 起伏は重いため意図的に除外
        const auto Package = GetCityModelPackage(GmlComponent);
        if (Package == plateau::dataset::PredefinedCityModelPackage::Relief)
            continue;

        for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
            TArray<USceneComponent*> FeatureComponents;
            LodComponent->GetChildrenComponents(true, FeatureComponents);
            for (const auto& FeatureComponent : FeatureComponents) {
                //この時点で不可視状態ならLodフィルタリングで不可視化されたことになるので無視
                if (!FeatureComponent->IsVisible())
                    continue;

                auto FeatureID = FeatureComponent->GetName();

                // TODO: 主要地物でない場合元の地物IDに_{数値}が入っている場合があるため、主要地物についてのみ処理する。よりロバストな方法検討必要
                if (FeatureComponent->GetAttachParent() == LodComponent) {
                    FeatureID = GetOriginalComponentName(FeatureComponent);
                }

                // BillboardComponentも混ざってるので無視
                if (FeatureID.Contains("BillboardComponent"))
                    continue;

                FPLATEAUCityObjectInfo GmlInfo;
                GmlInfo.DatasetName = DatasetName;
                GmlInfo.GmlName = GetGmlFileName(GmlComponent);
                const auto CityModel = UPLATEAUCityGmlProxy::Load(GmlInfo);

                if (CityModel == nullptr) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid Dataset or Gml : %s, %s"), *GmlInfo.DatasetName, *GmlInfo.GmlName);
                    continue;
                }

                const auto CityObject = CityModel->getCityObjectById(TCHAR_TO_UTF8(*FeatureID));
                if (CityObject == nullptr) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid ID : %s"), *FeatureID);
                    continue;
                }

                const auto CityObjectType = CityObject->getType();
                if (static_cast<uint64_t>(InCityObjectType & CityObjectType))
                    continue;

                FeatureComponent->SetVisibility(false);
            }
        }
    }
}
