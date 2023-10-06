// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUAsyncLoadedFeatureInfoPanel.h"

#include "CoreMinimal.h"
#include "PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <numeric>
#include <Async/Async.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "PLATEAUFeatureInfoDisplay.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneComponent.h"
#include "Tasks/Task.h"

using namespace plateau::dataset;

namespace {
    /**
     * @brief コンポーネント追加間隔
     */
    constexpr float AddPanelComponentSleepTime = 0.03f;

    constexpr float PanelScaleMultiplier = 2.0f;

    UStaticMeshComponent* CreatePanelMeshComponent(UMaterialInstanceDynamic* Material) {
        const FName MeshName = MakeUniqueObjectName(
            GetTransientPackage(),
            UStaticMeshComponent::StaticClass(),
            TEXT("Panel"));

        UStaticMeshComponent* PanelComponent = NewObject<UStaticMeshComponent>(
            GetTransientPackage(),
            MeshName, RF_Transient);
        PanelComponent->SetMaterial(0, Material);
        const auto StaticMeshName = TEXT("/Engine/BasicShapes/Plane");
        const auto Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, StaticMeshName));
        Mesh->AddMaterial(Material);
        PanelComponent->SetStaticMesh(Mesh);
        PanelComponent->SetMobility(EComponentMobility::Static);
        PanelComponent->CastShadow = false;
        PanelComponent->SetHiddenInGame(true);

        return PanelComponent;
    }

    FTransform CalculateIconTransform(const FBox& Box, const int ColIndex, const int RowIndex, const int IconCount) {
        FTransform Transform{};
        Transform.SetIdentity();

        const auto Center = Box.GetCenter();

        // 表示するアイコン数に応じて現在の行において最大何個のアイコンを表示するか求める
        int ColCount;
        if (0 < (IconCount - RowIndex * plateau::Feature::MaxIconCol) / plateau::Feature::MaxIconCol) {
            ColCount = plateau::Feature::MaxIconCol;
        } else {
            ColCount = (IconCount - RowIndex * plateau::Feature::MaxIconCol) % plateau::Feature::MaxIconCol;
        }

        // 表示するアイコン数に応じて最大の行数を求める
        int RowCount;
        if (plateau::Feature::MaxIconCol < IconCount) {
            RowCount = plateau::Feature::MaxIconRow;
        } else {
            RowCount = 1;
        }
        
        // アイコン1つの横幅は100fなため、-100f * {アイコン数}/2が左端のx座標になる。
        // 中心座標を考慮するため50fを計算に追加している。
        float XOffset;
        if (plateau::Feature::MaxIconCol < IconCount && IconCount <= plateau::Feature::MaxIconCnt && 0 < RowIndex) {
            // 2行目のオフセット値
            XOffset = 100.0f * ColIndex - 100.0f * 1 - 50.0f;
        } else {
            // 1行目のオフセット値
            XOffset = 100.0f * ColIndex - 100.0f * ColCount * 0.5f + 50.0f;
        }
        
        FVector Offset{
            XOffset,
            100.0f * RowIndex - 100.0f * RowCount / 2.0f + 50.0f,
            0.0f
        };
        Offset *= PanelScaleMultiplier;
        Transform.SetTranslation(Center + Offset);
        Transform.SetScale3D(PanelScaleMultiplier * FVector3d::One());
        return Transform;
    }
}

FPLATEAUAsyncLoadedFeatureInfoPanel::FPLATEAUAsyncLoadedFeatureInfoPanel(
    const TWeakPtr<FPLATEAUFeatureInfoDisplay> Owner,
    const TWeakPtr<FPLATEAUExtentEditorViewportClient> ViewportClient)
    : Owner(Owner)
    , ViewportClient(ViewportClient)
    , AddedIconComponentCnt(0)
    , AddedDetailedIconComponentCnt(0)
    , DeltaTime(0)
    , MaxLodTaskStatus(EPLATEAUFeatureInfoPanelStatus::Idle)
    , CreateComponentStatus(EPLATEAUFeatureInfoPanelStatus::Idle)
    , AddComponentStatus(EPLATEAUFeatureInfoPanelStatus::Idle)
    , FeatureInfoVisibility(EPLATEAUFeatureInfoVisibility::Hidden) {}

EPLATEAUFeatureInfoVisibility FPLATEAUAsyncLoadedFeatureInfoPanel::GetFeatureInfoVisibility() const {
    return FeatureInfoVisibility;
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::SetFeatureInfoVisibility(const TArray<int>& ShowLods, const EPLATEAUFeatureInfoVisibility Value, const bool bForce) {
    if (CreateComponentStatus != EPLATEAUFeatureInfoPanelStatus::FullyLoaded)
        return;

    if (FeatureInfoVisibility == Value && !bForce)
        return;

    FeatureInfoVisibility = Value;
    ApplyFeatureInfoVisibility(ShowLods);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::ApplyFeatureInfoVisibility(const TArray<int>& ShowLods) const {
    const auto NumIcon = FeatureInfoMaterialMaps.Num();
    for (int i = 0; i < NumIcon; i++) {
        if (const auto& IconMaterialInstanceDynamic = IconMaterialInstanceDynamics[i]; IconMaterialInstanceDynamic != nullptr) {
            if (const auto& MaterialKey = FeatureInfoMaterialMaps[i]; ShowLods.Contains(MaterialKey.Lod)) {
                IconMaterialInstanceDynamic->SetScalarParameterValue(FName("Opacity"), FeatureInfoVisibility == EPLATEAUFeatureInfoVisibility::Visible ? 1.0 : 0);
            } else {
                IconMaterialInstanceDynamic->SetScalarParameterValue(FName("Opacity"), 0);
            }
        }

        if (const auto& DetailedIconMaterialInstanceDynamic = DetailedIconMaterialInstanceDynamics[i]; DetailedIconMaterialInstanceDynamic != nullptr) {
            if (const auto& MaterialKey = FeatureInfoMaterialMaps[i]; ShowLods.Contains(MaterialKey.Lod)) {
                DetailedIconMaterialInstanceDynamic->SetScalarParameterValue(FName("Opacity"), FeatureInfoVisibility == EPLATEAUFeatureInfoVisibility::Detailed ? 1.0 : 0);
            } else {
                DetailedIconMaterialInstanceDynamic->SetScalarParameterValue(FName("Opacity"), 0);
            }
        }
    }
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::CreatePanelComponents(const TMap<PredefinedCityModelPackage, int>& MaxLods) {
    if (MaxLods.IsEmpty())
        return;

    TMap<FString, FPLATEAUFeatureInfoMaterialKey> FeatureInfoMaterialMap;
    const auto OwnerStrongPtr = Owner.Pin();
    for (const auto& Package : FPLATEAUFeatureInfoDisplay::GetDisplayedPackages()) {
        if (!MaxLods.Find(Package))
            continue;

        FPLATEAUFeatureInfoMaterialKey Key;
        Key.bDetailed = false;
        Key.Package = Package;
        Key.Lod = MaxLods[Package];

        // アイコンはパッケージ毎に割当てられているが、同名のアイコンファイルが利用されるのは防ぐ
        const auto IconFileName = FPLATEAUFeatureInfoDisplay::GetIconFileName(Package);
        if (FeatureInfoMaterialMap.Contains(IconFileName)) {
            if (FeatureInfoMaterialMap[IconFileName].Lod < Key.Lod) {
                FeatureInfoMaterialMap[IconFileName] = Key;
            }
        } else {
            FeatureInfoMaterialMap.Emplace(IconFileName, Key);
        }
    }

    // 範囲選択画面に表示する順番に並び替える
    FeatureInfoMaterialMaps.Reset();
    const auto IconFileNameList = FPLATEAUFeatureInfoDisplay::GetIconFileNameList();
    for (const auto& IconFileName : IconFileNameList) {
        if (FeatureInfoMaterialMap.Contains(IconFileName)) {
            FeatureInfoMaterialMaps.Add(FeatureInfoMaterialMap[IconFileName]);
        }
    }

    for (const auto& FeatureInfoMaterial : FeatureInfoMaterialMaps) {
        const auto IconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(FeatureInfoMaterial);
        IconMaterial->SetScalarParameterValue(FName("Opacity"), 0);
        const auto IconComponent = CreatePanelMeshComponent(IconMaterial);
        IconMaterialInstanceDynamics.Emplace(IconMaterial);
        IconComponent->SetTranslucentSortPriority(SortPriority_IconComponent);
        IconComponents.Emplace(IconComponent);

        auto Key = FeatureInfoMaterial;
        Key.bDetailed = true;
        const auto DetailedIconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        DetailedIconMaterial->SetScalarParameterValue(FName("Opacity"), 0);
        const auto DetailedIconComponent = CreatePanelMeshComponent(DetailedIconMaterial);
        DetailedIconMaterialInstanceDynamics.Emplace(DetailedIconMaterial);
        DetailedIconComponent->SetTranslucentSortPriority(SortPriority_IconComponent);
        DetailedIconComponents.Emplace(DetailedIconComponent);
    }
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadMaxLodAsync(const FPLATEAUFeatureInfoPanelInput& Input, const FBox& InBox) {
    Box = InBox;
    MaxLodTaskStatus = EPLATEAUFeatureInfoPanelStatus::Loading;

    GetMaxLodTask = UE::Tasks::Launch(TEXT("GetMaxLODTask"), [Input]() mutable {
        TMap<PredefinedCityModelPackage, int> MaxLods;

        for (const auto& Entry : Input) {
            if (Entry.Value->empty())
                continue;

            int MaxLod = 0;
            for (auto& GmlFile : *Entry.Value) {
                // GMLファイル内を検索して最大LODを取得
                MaxLod = FMath::Max(GmlFile.getMaxLod(), MaxLod);
            }

            MaxLods.Add(Entry.Key, MaxLod);
        }

        return MaxLods;
    }, LowLevelTasks::ETaskPriority::BackgroundHigh);
}

bool FPLATEAUAsyncLoadedFeatureInfoPanel::AddIconComponent(const float DeltaSeconds) {
    if (AddComponentStatus == EPLATEAUFeatureInfoPanelStatus::FullyLoaded)
        return false;

    if (!GetMaxLodTask.IsValid() || !GetMaxLodTask.IsCompleted())
        return false;

    MaxLodTaskStatus = EPLATEAUFeatureInfoPanelStatus::FullyLoaded;

    const auto PreviewScene = ViewportClient.Pin()->GetPreviewScene();
    if (PreviewScene == nullptr)
        return false;

    if (CreateComponentStatus != EPLATEAUFeatureInfoPanelStatus::FullyLoaded) {
        CreatePanelComponents(GetMaxLodTask.GetResult());
        CreateComponentStatus = EPLATEAUFeatureInfoPanelStatus::FullyLoaded;
    }

    AddComponentStatus = EPLATEAUFeatureInfoPanelStatus::Loading;
    DeltaTime = DeltaSeconds + DeltaTime;
    if (AddPanelComponentSleepTime < DeltaTime && AddedIconComponentCnt < IconComponents.Num()) {
        DeltaTime = 0;

        check(IconComponents.Num() == DetailedIconComponents.Num());
        const auto Transform = CalculateIconTransform(Box, AddedIconComponentCnt % plateau::Feature::MaxIconCol, AddedIconComponentCnt / plateau::Feature::MaxIconCol, IconComponents.Num());
        PreviewScene->AddComponent(IconComponents[AddedIconComponentCnt], Transform);
        AddedIconComponentCnt++;

        return true;
    }

    if (AddPanelComponentSleepTime < DeltaTime && AddedDetailedIconComponentCnt < DetailedIconComponents.Num()) {
        DeltaTime = 0;

        const auto Transform = CalculateIconTransform(Box, AddedDetailedIconComponentCnt % plateau::Feature::MaxIconCol, AddedDetailedIconComponentCnt / plateau::Feature::MaxIconCol, IconComponents.Num());
        PreviewScene->AddComponent(DetailedIconComponents[AddedDetailedIconComponentCnt], Transform);
        const auto IconCnt = FMath::Min(DetailedIconComponents.Num(), plateau::Feature::MaxIconCnt);
        if (IconCnt - 1 <= AddedDetailedIconComponentCnt++)
            AddComponentStatus = EPLATEAUFeatureInfoPanelStatus::FullyLoaded;

        return true;
    }

    return false;
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::RecalculateIconTransform(const TArray<int>& ShowLods) {
    if (CreateComponentStatus != EPLATEAUFeatureInfoPanelStatus::FullyLoaded)
        return;

    const int NumFilteredIconComponents = std::accumulate(FeatureInfoMaterialMaps.begin(), FeatureInfoMaterialMaps.end(), 0, [ShowLods](const int Sum, const FPLATEAUFeatureInfoMaterialKey MaterialKey) {
        return Sum + (ShowLods.Contains(MaterialKey.Lod) ? 1 : 0);
    });
    
    int ColIndex = 0;
    const auto NumIcon = FeatureInfoMaterialMaps.Num();
    for (int i = 0; i < NumIcon; i++) {
        if (const auto& MaterialKey = FeatureInfoMaterialMaps[i]; ShowLods.Contains(MaterialKey.Lod)) {
            const auto Transform = CalculateIconTransform(Box, ColIndex % plateau::Feature::MaxIconCol, ColIndex / plateau::Feature::MaxIconCol, NumFilteredIconComponents);
            const auto& IconComponent = IconComponents[i];
            USceneComponent* SceneComp = Cast<USceneComponent>(IconComponent);
            if (SceneComp && SceneComp->GetAttachParent() == nullptr) {
                SceneComp->SetRelativeTransform(Transform);
            }

            const auto& DetailedIconComponent = DetailedIconComponents[i];
            SceneComp = Cast<USceneComponent>(DetailedIconComponent);
            if (SceneComp && SceneComp->GetAttachParent() == nullptr) {
                SceneComp->SetRelativeTransform(Transform);
            }

            ColIndex++;
        }
    }
}
