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

#include "Reconstruct/PLATEAUModelReconstruct.h"

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

namespace {
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

    /**
     * @brief UPLATEAUCityObjectGroupのリストからUPLATEAUCityObjectを取り出し、GmlIDをキーとしたMapを生成
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: GmlID, Value: UPLATEAUCityObject の Map
     */
    /*
    TMap<FString, FPLATEAUCityObject> CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
        TMap<FString, FPLATEAUCityObject> cityObjMap;
        for (auto comp : TargetCityObjects) {

            if (comp->SerializedCityObjects.IsEmpty())
                continue;

            for (auto cityObj : comp->GetAllRootCityObjects()) {
                if (!comp->OutsideParent.IsEmpty() && !cityObjMap.Contains(comp->OutsideParent)) {
                    // 親を探す
                    TArray<USceneComponent*> Parents;
                    comp->GetParentComponents(Parents);
                    for (const auto& Parent : Parents)                         {
                        if (Parent->GetName().Contains(comp->OutsideParent)) {
                            for (auto pObj : Cast<UPLATEAUCityObjectGroup>(Parent)->GetAllRootCityObjects()) {
                                cityObjMap.Add(pObj.GmlID, pObj);
                            }
                            break;
                        }
                    }
                }

                cityObjMap.Add(cityObj.GmlID, cityObj);
                for (auto child : cityObj.Children) {
                    cityObjMap.Add(child.GmlID, child);
                }
            }
        }
        return cityObjMap;
    }
    */

    /**
     * @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してリストに追加します
     */
    /*
    TArray<UPLATEAUCityObjectGroup*> GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents) {
        TSet<UPLATEAUCityObjectGroup*> UniqueComponents;
        for (auto comp : TargetComponents) {
            if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && StaticCast<UStaticMeshComponent*>(comp)->GetStaticMesh() == nullptr && comp->IsVisible()) {
                TArray<USceneComponent*> children;
                comp->GetChildrenComponents(true, children);
                for (auto child : children) {
                    if (child->IsA(UPLATEAUCityObjectGroup::StaticClass()) && child->IsVisible()) {
                        auto childCityObj = StaticCast<UPLATEAUCityObjectGroup*>(child);
                        if (childCityObj->GetStaticMesh() != nullptr) {
                            UniqueComponents.Add(childCityObj);
                        }
                    }
                }
            }
            if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible())
                UniqueComponents.Add(StaticCast<UPLATEAUCityObjectGroup*>(comp));
        }
        return UniqueComponents.Array();
    }
    */
}

FString APLATEAUInstancedCityModel::GetOriginalComponentName(const USceneComponent* const InComponent) {
    auto ComponentName = InComponent->GetName();
    int Index = 0;
    if (ComponentName.FindLastChar('_', Index)) {
        if (ComponentName.RightChop(Index + 1).IsNumeric()) {
            ComponentName = ComponentName.LeftChop(ComponentName.Len() - Index + 1);
        }
    }
    return ComponentName;
}

int APLATEAUInstancedCityModel::ParseLodComponent(const USceneComponent* const InLodComponent) {
    auto LodString = GetOriginalComponentName(InLodComponent);
    // "Lod{数字}"から先頭3文字除外することで数字を抜き出す。
    LodString = LodString.RightChop(3);

    int Lod;
    FDefaultValueHelper::ParseInt(LodString, Lod);

    return Lod;
}

// Sets default values
APLATEAUInstancedCityModel::APLATEAUInstancedCityModel() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

double APLATEAUInstancedCityModel::GetLatitude() {
    return GeoReference.GetData().unproject(TVec3d(0, 0, 0)).latitude;
}

double APLATEAUInstancedCityModel::GetLongitude() {
    return GeoReference.GetData().unproject(TVec3d(0, 0, 0)).longitude;
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
    if (!HasAttributeInfo())
        return FilterByFeatureTypesLegacy(InCityObjectType);
    bIsFiltering = true;
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
                // BillboardComponentも混ざってるので無視
                if (FeatureID.Contains("BillboardComponent"))
                    continue;

                if (!FeatureComponent->IsA(UPLATEAUCityObjectGroup::StaticClass()))
                    continue;

                const auto CityObjGrp = StaticCast<UPLATEAUCityObjectGroup*>(FeatureComponent);
                const auto ObjList = CityObjGrp->GetAllRootCityObjects();
                if (ObjList.Num() != 1)
                    continue;

                const int64 CityObjectType = UPLATEAUCityObjectBlueprintLibrary::GetTypeAsInt64(ObjList[0].Type);
                if (static_cast<int64>(InCityObjectType) & CityObjectType) 
                    continue;

                ApplyCollisionResponseBlockToChannel(FeatureComponent, false);
                FeatureComponent->SetVisibility(false);
            }
        }
    }  
    bIsFiltering = false;
    return this;
}

APLATEAUInstancedCityModel* APLATEAUInstancedCityModel::FilterByFeatureTypesLegacy(const citygml::CityObject::CityObjectsType InCityObjectType) {
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

bool APLATEAUInstancedCityModel::HasAttributeInfo() {
    TArray<USceneComponent*> Components;
    GetRootComponent()->GetChildrenComponents(true, Components);
    return Components.ContainsByPredicate([](const auto& Comp) {
        return Comp->IsA(UPLATEAUCityObjectGroup::StaticClass());
        });
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

                // TODO: 最小地物の場合元の地物IDに_{数値}が入っている場合があるため、最小地物についてのみ処理する。よりロバストな方法検討必要
                if (FeatureComponent->GetAttachParent() != LodComponent) {
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

TTask<TArray<USceneComponent*>> APLATEAUInstancedCityModel::ReconstructModel(const TArray<USceneComponent*> TargetComponents, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal)  {

    UE_LOG(LogTemp, Log, TEXT("ReconstructModel: %d %d %s"), TargetComponents.Num(), static_cast<int>(ReconstructType), bDestroyOriginal ? TEXT("True") : TEXT("False"));

    TTask<TArray<USceneComponent*>> ConvertTask = Launch(TEXT("ConvertTask"), [this, TargetComponents, ReconstructType, bDestroyOriginal] {

        FPLATEAUModelReconstruct ModelReconstruct(this, ReconstructType);
        const auto& TargetCityObjects = ModelReconstruct.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);
        std::shared_ptr<plateau::polygonMesh::Model> converted = ModelReconstruct.ConvertModelForReconstruct(TargetCityObjects);

        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //コンポーネント削除
            for (auto comp : TargetCityObjects) {
                if (bDestroyOriginal)
                    comp->DestroyComponent();
                else
                    comp->SetVisibility(false);
            }
            }, TStatId(), NULL, ENamedThreads::GameThread)
            ->Wait();

        const auto components = ModelReconstruct.ReconstructFromConvertedModel(converted);

        FFunctionGraphTask::CreateAndDispatchWhenReady([&, TargetCityObjects]() {
            //終了イベント通知
            OnReconstructFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);

        return components;

    });
    return ConvertTask;

    /*
    plateau::polygonMesh::MeshGranularity MeshGranularity = static_cast<plateau::polygonMesh::MeshGranularity>(ReconstructType);

    UE_LOG(LogTemp, Log, TEXT("ReconstructModel: %d %d %s"), TargetComponents.Num(), static_cast<int>(MeshGranularity), bDivideGrid ? TEXT("True") : TEXT("False"));

    GranularityConvertOption ConvOption(MeshGranularity, bDivideGrid ? 1 : 0);

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;

    TTask<TArray<USceneComponent*>> ConvertTask = Launch(TEXT("ConvertTask"), [this, TargetComponents, ExtOptions, ConvOption, bDestroyOriginal] {

        FPLATEAUMeshExporter MeshExporter;
        GranularityConverter Converter;

        const auto& TargetCityObjects = GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);

        //属性情報を覚えておきます。
        TMap<FString, FPLATEAUCityObject> cityObjMap = CreateMapFromCityObjectGroups(TargetCityObjects);

        std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(this, TargetCityObjects, ExtOptions);

        std::shared_ptr<plateau::polygonMesh::Model> converted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*smodel, ConvOption));

        FFunctionGraphTask::CreateAndDispatchWhenReady([&, TargetCityObjects]() {
            //コンポーネント削除
            for (auto comp : TargetCityObjects) {
                if (bDestroyOriginal)
                    comp->DestroyComponent();
                else
                    comp->SetVisibility(false);
            }
            }, TStatId(), NULL, ENamedThreads::GameThread)
            ->Wait();

        const auto components = ReconstructFromConvertedModel(converted, ConvOption.granularity_, cityObjMap);

        FFunctionGraphTask::CreateAndDispatchWhenReady([&, TargetCityObjects]() {
            //終了イベント通知
            OnReconstructFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);

         return components;
        });
    return ConvertTask;
    */
}
/*
TArray<USceneComponent*> APLATEAUInstancedCityModel::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model, plateau::polygonMesh::MeshGranularity Granularity, const TMap<FString, FPLATEAUCityObject> cityObjMap)     {
    FPipe LoadComponentPipe{ TEXT("LoadComponentPipe") };
    FPLATEAUMeshLoader MeshLoader(false);

    // マテリアル分けのマテリアルが設定されている場合
    if (!ClassificationMaterials.IsEmpty()) 
        MeshLoader.SetClassificationMaterials(ClassificationMaterials);

    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        FTask LoadComponentTask = LoadComponentPipe.Launch(TEXT("LoadComponentTask"), [this, &MeshLoader, Model, Granularity, cityObjMap, i] {
            MeshLoader.ReloadComponentFromNode(this->GetRootComponent(), Model->getRootNodeAt(i), Granularity, cityObjMap, *this );
            });  
        AddNested(LoadComponentTask);
        LoadComponentTask.Wait();
    }
    LoadComponentPipe.WaitUntilEmpty();
    return MeshLoader.GetLastCreatedComponents();
}
*/

FTask APLATEAUInstancedCityModel::ClassifyModel(const TArray<USceneComponent*> TargetComponents, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDivideGrid, bool bDestroyOriginal) {


    FTask ClassifyTask = Launch(TEXT("ClassifyTask"), [&, this, TargetComponents, bDestroyOriginal, Materials, ReconstructType] {

        FPLATEAUModelReconstruct ModelReconstruct_Pre(this, EPLATEAUMeshGranularity::PerAtomicFeatureObject);
        const auto& TargetCityObjects_Pre = ModelReconstruct_Pre.GetUPLATEAUCityObjectGroupsFromSceneComponents(TargetComponents);
        std::shared_ptr<plateau::polygonMesh::Model> Converted_Pre = ModelReconstruct_Pre.ConvertModelForReconstruct(TargetCityObjects_Pre);

        TArray<EPLATEAUCityObjectsType> Types;
        for (auto kv : Materials) {
            if (kv.Value != nullptr) {
                Types.Add(kv.Key);

                UE_LOG(LogTemp, Error, TEXT("Type added %s"), *UEnum::GetValueAsString(kv.Key));
            }         
        }

        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //コンポーネント削除
            for (auto comp : TargetCityObjects_Pre) {
                if (bDestroyOriginal)
                    comp->DestroyComponent();
                else
                    comp->SetVisibility(false);
            }
            }, TStatId(), NULL, ENamedThreads::GameThread)
            ->Wait();
        
        const auto CreatedComponents = ModelReconstruct_Pre.ReconstructFromConvertedModelForClassificationSet(Converted_Pre, Types);

        /*  */
        //FPlatformProcess::Sleep(4);
        for (auto comp : CreatedComponents) {
            if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible()) {
                auto CityObjGrp = StaticCast<UPLATEAUCityObjectGroup*>(comp);
                FGenericPlatformProcess::ConditionalSleep(
                    [&]() {
                        return CityObjGrp->GetStaticMesh() != nullptr;
                    }, 3);
            }
        }

        UE_LOG(LogTemp, Error, TEXT("ModelReconstruct_Post Start type %d"), ReconstructType);

        FPLATEAUModelReconstruct ModelReconstruct_Post(this, ReconstructType);
        const auto& TargetCityObjects_Post = ModelReconstruct_Post.GetUPLATEAUCityObjectGroupsFromSceneComponents(CreatedComponents);
        std::shared_ptr<plateau::polygonMesh::Model> Converted_Post = ModelReconstruct_Post.ConvertModelForReconstruct(TargetCityObjects_Post);

        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //コンポーネント削除
            for (auto comp : TargetCityObjects_Post) {
                    comp->DestroyComponent();
            }
            }, TStatId(), NULL, ENamedThreads::GameThread)
            ->Wait();

        const auto ResultComponents = ModelReconstruct_Post.ReconstructFromConvertedModelForClassificationGet(Converted_Post, Materials);
        
        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //終了イベント通知
            OnClassifyFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);


        });

    return ClassifyTask;

    /*
    FTask ClassifyTask = Launch(TEXT("ClassifyTask"), [&, this, TargetComponents, bDestroyOriginal, Materials, ReconstructType] {
        
        TTask<TArray<USceneComponent*>> ConvertToAtomicTask = ReconstructModel(TargetComponents, EPLATEAUMeshGranularity::PerAtomicFeatureObject, bDestroyOriginal);
        AddNested(ConvertToAtomicTask);
        ConvertToAtomicTask.Wait();    

        //　Convert したTargetComponentsの取得
        const auto& CreatedComponents = ConvertToAtomicTask.GetResult();

        // Typeに応じてカスタムシェーダーMaterialを設定
        for (auto comp : CreatedComponents) {
            if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible()) {
                auto CityObjGrp = StaticCast<UPLATEAUCityObjectGroup*>(comp);

                FGenericPlatformProcess::ConditionalSleep(
                    [&]() {
                        return CityObjGrp->GetStaticMesh() != nullptr;
                    }, 3);
                const auto& StaticMesh = CityObjGrp->GetStaticMesh();
                if (StaticMesh != nullptr) {   
                    const auto& CityObjs = CityObjGrp->GetAllRootCityObjects();
                    if (CityObjs.Num() == 1) {

                        const auto& CityObj = CityObjs[0];
                        const auto& CityObjType = StaticCast<uint8>(CityObj.Type);
                        if (Materials.Contains(CityObjType)) {
                            UMaterialInterface* Material = Materials[CityObjType];
                            if (Material != nullptr) {
                                FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {     
                                        comp->SetMobility(EComponentMobility::Stationary);
                                        const auto SourceMaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/Materials/ClassificationMaterial");
                                        UMaterial* Mat = Cast<UMaterial>(
                                            StaticLoadObject(UMaterial::StaticClass(), nullptr, SourceMaterialPath));
                                        auto ClassificationMaterial = UMaterialInstanceDynamic::Create(Mat, comp);
                                        float MaterialID = static_cast<float>(CityObj.Type);
                                        ClassificationMaterial->SetScalarParameterValue(FName("GameMaterialID"), MaterialID);
                                        StaticMesh->SetMaterial(0, ClassificationMaterial);
                                    }, TStatId(), NULL, ENamedThreads::GameThread)->Wait();
                            }
                        }
                    }
                }
                else {
                    UE_LOG(LogTemp, Error, TEXT("StaticMesh Null : %s"), *comp->GetName());
                }
            }
        }

        UE_LOG(LogTemp, Error, TEXT("Reconvert Start type %d") , ReconstructType);

        //ClassificationMaterials = Materials;

        TTask<TArray<USceneComponent*>> ReconvertTask = ReconstructModel(CreatedComponents, ReconstructType, false, true);
        AddNested(ReconvertTask);
        ReconvertTask.Wait();

        //ClassificationMaterials.Reset();
       
        FFunctionGraphTask::CreateAndDispatchWhenReady([&]() {
            //終了イベント通知
            OnClassifyFinished.Broadcast();
            }, TStatId(), NULL, ENamedThreads::GameThread);
        
        UE_LOG(LogTemp, Error, TEXT("ClassifyModel Finished"));
        });
        
    return ClassifyTask;
    */
}


void APLATEAUInstancedCityModel::CallOnReconstructFinished() {
    OnReconstructFinished.Broadcast();
}
