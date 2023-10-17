// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUInstancedCityModel.h"

#include "Tasks/Task.h"

#include "Misc/DefaultValueHelper.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "CityGML/PLATEAUCityGmlProxy.h"
#include <PLATEAUMeshExporter.h>
#include <PLATEAUMeshLoader.h>
#include <PLATEAUExportSettings.h>

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

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

    /**
     * @brief 指定コンポーネントを基準として子階層のCityObjectを取得
     * @param SceneComponent CityObject取得の基準となるコンポーネント
     * @param RootCityObjects 取得されたCityObject配列
     */
    void GetRootCityObjectsRecursive(USceneComponent* SceneComponent, TArray<FPLATEAUCityObject>& RootCityObjects) {
        if (const auto& CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(SceneComponent)) {
            const auto& AllRootCityObjects = CityObjectGroup->GetAllRootCityObjects();
            for (const auto& CityObject : AllRootCityObjects) {
                RootCityObjects.Add(CityObject);
            }
        }

        for (const auto& AttachedComponent : SceneComponent->GetAttachChildren()) {
            GetRootCityObjectsRecursive(AttachedComponent, RootCityObjects);
        }
    }

    /**
     * @brief 対象コンポーネントとその子コンポーネントのコリジョン設定変更
     * @param ParentComponent コリジョン設定変更対象コンポーネント
     * @param bCollisionResponseBlock コリジョンをブロック設定に変更するか？
     * @param bPropagateToChildren 子コンポーネントのコリジョン設定を変更するか？
     */
    void ApplyCollisionResponseBlockToChannel(USceneComponent* ParentComponent, const bool bCollisionResponseBlock, const bool bPropagateToChildren=false) {
        if (const auto& ParentStaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent); ParentStaticMeshComponent != nullptr) {
            ParentStaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, bCollisionResponseBlock ? ECR_Block : ECR_Ignore);
        }

        if (!bPropagateToChildren)
            return;
        
        if (const TArray<USceneComponent*>& AttachedChildren = ParentComponent->GetAttachChildren(); 0 < AttachedChildren.Num()) {
            TInlineComponentArray<USceneComponent*, NumInlinedActorComponents> ComponentStack;
            ComponentStack.Append(AttachedChildren);
            while (0 < ComponentStack.Num()) {
                if (const auto& CurrentComp = ComponentStack.Pop(/*bAllowShrinking=*/ false); CurrentComp != nullptr) {
                    ComponentStack.Append(CurrentComp->GetAttachChildren());
                    if (const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(CurrentComp); StaticMeshComponent != nullptr) {
                        StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, bCollisionResponseBlock ? ECR_Block : ECR_Ignore);
                    }
                }
            }
        }
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

TArray<FPLATEAUCityObject>& APLATEAUInstancedCityModel::GetAllRootCityObjects() {
    if (0 < RootCityObjects.Num()) {
        return RootCityObjects;
    }

    GetRootCityObjectsRecursive(GetRootComponent(), RootCityObjects);
    return RootCityObjects;
}

void APLATEAUInstancedCityModel::FilterLowLods(const USceneComponent* const InGmlComponent, const int MinLod, const int MaxLod) {
    const TArray<USceneComponent*>& AttachedLodChildren = InGmlComponent->GetAttachChildren();

    // 各LODに対して形状データ(コンポーネント)が存在するコンポーネント名を検索
    TMap<int, TSet<FString>> NameMap;
    for (const auto& LodComponent : AttachedLodChildren) {
        const auto Lod = ParseLodComponent(LodComponent);
        auto& Value = NameMap.Add(Lod);

        if (Lod < MinLod || Lod > MaxLod)
            continue;

        TArray<USceneComponent*> FeatureComponents;
        LodComponent->GetChildrenComponents(false, FeatureComponents);
        for (const auto FeatureComponent: FeatureComponents) {
            Value.Add(GetOriginalComponentName(FeatureComponent));
        }
    }

    // フィルタリング実行
    for (const auto& LodComponent : AttachedLodChildren) {
        const TArray<USceneComponent*>& AttachedFeatureChildren = LodComponent->GetAttachChildren();
        const auto Lod = ParseLodComponent(LodComponent);

        if (Lod < MinLod || Lod > MaxLod) {
            // LOD範囲外のLOD形状は非表示化
            for (const auto& FeatureComponent : AttachedFeatureChildren) {
                ApplyCollisionResponseBlockToChannel(FeatureComponent, false, true);
                FeatureComponent->SetVisibility(false, true);
            }
            continue;
        }

        for (const auto& FeatureComponent : AttachedFeatureChildren) {
            auto ComponentName = GetOriginalComponentName(FeatureComponent);
            TArray<int> Keys;
            NameMap.GetKeys(Keys);
            auto bIsMaxLod = true;
            for (const auto Key : Keys) {
                if (Key <= Lod)
                    continue;

                // コンポーネントのLODよりも大きいLODが存在する場合非表示
                if (NameMap[Key].Contains(ComponentName))
                    bIsMaxLod = false;
            }

            ApplyCollisionResponseBlockToChannel(FeatureComponent, bIsMaxLod, true);
            FeatureComponent->SetVisibility(bIsMaxLod, true);
        }
    }
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
        const TArray<USceneComponent*>& AttachedLodChildren = GmlComponent->GetAttachChildren();

        // 一度全ての地物メッシュを不可視にする
        for (const auto& LodComponent : AttachedLodChildren) {
            const TArray<USceneComponent*>& AttachedFeatureChildren = LodComponent->GetAttachChildren();
            for (const auto& FeatureComponent : AttachedFeatureChildren) {
                ApplyCollisionResponseBlockToChannel(FeatureComponent, false, true);
                FeatureComponent->SetVisibility(false, true);
            }
        }

        // 選択されていないパッケージを除外
        const auto Package = GetCityModelPackage(GmlComponent);
        if ((Package & InPackage) == plateau::dataset::PredefinedCityModelPackage::None)
            continue;

        const auto MinLod = PackageToLodRangeMap[Package].MinLod;
        const auto MaxLod = PackageToLodRangeMap[Package].MaxLod;

        // 各地物について全てのLODを表示する場合の処理
        if (!bOnlyMaxLod) {
            for (const auto& LodComponent : AttachedLodChildren) {
                const auto Lod = ParseLodComponent(LodComponent);
                if (MinLod <= Lod && Lod <= MaxLod) {
                    for (const auto& FeatureComponent : LodComponent->GetAttachChildren()) {
                        ApplyCollisionResponseBlockToChannel(FeatureComponent, true, true);
                        FeatureComponent->SetVisibility(true, true);
                    }
                } else {
                    for (const auto& FeatureComponent : LodComponent->GetAttachChildren()) {
                        ApplyCollisionResponseBlockToChannel(FeatureComponent, false, true);
                        FeatureComponent->SetVisibility(false, true);
                    }
                }
            }
            continue;
        }

        // 各地物について最大LODのみを表示する場合の処理
        FilterLowLods(GmlComponent, MinLod, MaxLod);
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
                // BillboardComponentを無視
                if (GmlComponent.GetName().Contains("BillboardComponent"))
                    continue;

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
        // BillboardComponentを無視
        if (GmlComponent.GetName().Contains("BillboardComponent"))
            continue;

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

                ApplyCollisionResponseBlockToChannel(FeatureComponent, false);
                FeatureComponent->SetVisibility(false);
            }
        }
    }
}

FTask APLATEAUInstancedCityModel::ReconstructModel(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const uint8 ReconstructType, bool bDivideGrid) {

    UE_LOG(LogTemp, Log, TEXT("PLATEAUInstancedCityModel ReconstructModel: %d %d %s"), TargetCityObjects.Num(), ReconstructType, bDivideGrid ? TEXT("True"): TEXT("False"));

    plateau::polygonMesh::MeshGranularity MeshGranularity = plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
    switch (ReconstructType)         {
    case 0: MeshGranularity = plateau::polygonMesh::MeshGranularity::PerCityModelArea; break;
    case 1: MeshGranularity = plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject; break;
    case 2: MeshGranularity = plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject; break;
    }

    UE_LOG(LogTemp, Log, TEXT("ReconstructType: %d"), static_cast<int>(MeshGranularity));

    GranularityConvertOption ConvOption(MeshGranularity, bDivideGrid ? 1 : 0);

    MeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;

    FTask ConvertTask = Launch(TEXT("ConvertTask"), [this, TargetCityObjects, ExtOptions, ConvOption] {

        FPLATEAUMeshExporter MeshExporter;
        GranularityConverter Converter;

        std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(this, StaticCast<const TArray<USceneComponent*>>(TargetCityObjects), ExtOptions);

        UE_LOG(LogTemp, Log, TEXT("model: %s %d"), *FString(smodel->debugString().c_str()), smodel->getAllMeshes().size());

        std::shared_ptr<plateau::polygonMesh::Model> converted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*smodel, ConvOption));
        UE_LOG(LogTemp, Log, TEXT("converted: %s %d"), *FString(converted->debugString().c_str()), converted->getAllMeshes().size());

        for (auto comp : TargetCityObjects) {
            //comp->DestroyComponent();
            comp->SetVisibility(false);
        }

        ReconstructFromConvertedModel(converted);

        UE_LOG(LogTemp, Log, TEXT("ReconstructModel Task Finished!"));

        });
    return ConvertTask;
}

void APLATEAUInstancedCityModel::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model)     {

    UE_LOG(LogTemp, Log, TEXT("GML Name: %s "), *this->GetActorNameOrLabel());
    FPipe LoadComponentPipe{ TEXT("LoadComponentPipe") };
    FPLATEAUMeshLoader MeshLoader(false);
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        FTask LoadComponentTask = LoadComponentPipe.Launch(TEXT("LoadComponentTask"), [&MeshLoader, this, Model, i] {
            MeshLoader.ReloadComponentFromNode(this->GetRootComponent(), Model->getRootNodeAt(i), *this);
            });  
        AddNested(LoadComponentTask);
        LoadComponentTask.Wait();
    }
}
