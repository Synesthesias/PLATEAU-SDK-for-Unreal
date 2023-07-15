// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUAsyncLoadedFeatureInfoPanel.h"

#include "CoreMinimal.h"
#include "PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

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

    FTransform CalculateBackPanelTransform(const int IconCount) {
        FTransform Transform{};
        Transform.SetIdentity();
        const auto ColCount = 4 < IconCount ? 4 : IconCount;
        const auto RowCount = 4 < IconCount ? 2 : 1;
        Transform.SetScale3D(FVector3d(ColCount + 0.4, 1.5 * RowCount, 1.0) * PanelScaleMultiplier);

        return Transform;
    }
}

FPLATEAUAsyncLoadedFeatureInfoPanel::FPLATEAUAsyncLoadedFeatureInfoPanel(
    const TWeakPtr<FPLATEAUFeatureInfoDisplay> Owner,
    const TWeakPtr<FPLATEAUExtentEditorViewportClient> ViewportClient)
    : Owner(Owner)
    , ViewportClient(ViewportClient)
    , Status(EPLATEAUFeatureInfoPanelStatus::Idle)
    , Visibility(EPLATEAUFeatureInfoVisibility::Hidden)
    , BackPanelComponent(nullptr) {}

EPLATEAUFeatureInfoPanelStatus FPLATEAUAsyncLoadedFeatureInfoPanel::GetStatus() const {
    return Status;
}

EPLATEAUFeatureInfoVisibility FPLATEAUAsyncLoadedFeatureInfoPanel::GetVisibility() const {
    return Visibility;
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::SetVisibility(const EPLATEAUFeatureInfoVisibility Value) {
    if (Visibility == Value)
        return;

    Visibility = Value;
    ApplyVisibility();
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::ApplyVisibility() const {
    TArray<USceneComponent*> Components;
    Components.Append(IconComponents);
    Components.Append(DetailedIconComponents);
    Components.Add(BackPanelComponent);

    for (const auto Component : IconComponents) {
        if (Component == nullptr)
            continue;

        Component->SetVisibility(Visibility == EPLATEAUFeatureInfoVisibility::Visible);
    }

    for (const auto Component : DetailedIconComponents) {
        if (Component == nullptr)
            continue;

        Component->SetVisibility(Visibility == EPLATEAUFeatureInfoVisibility::Detailed);
    }

    if (BackPanelComponent == nullptr)
        return;

    BackPanelComponent->SetVisibility(Visibility != EPLATEAUFeatureInfoVisibility::Hidden);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::CreatePanelComponents(const TMap<PredefinedCityModelPackage, int>& MaxLods)
{
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

    for (const auto& FeatureInfoMaterial : FeatureInfoMaterialMap) {
        const auto IconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(FeatureInfoMaterial.Value);
        const auto IconComponent = CreatePanelMeshComponent(IconMaterial);
        IconComponent->SetTranslucentSortPriority(SortPriority_IconComponent);
        IconComponents.Add(IconComponent);

        auto Key = FeatureInfoMaterial.Value;
        Key.bDetailed = true;
        const auto DetailedIconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        const auto DetailedIconComponent = CreatePanelMeshComponent(DetailedIconMaterial);
        DetailedIconComponent->SetTranslucentSortPriority(SortPriority_IconComponent);
        DetailedIconComponents.Add(DetailedIconComponent);
    }
    
    const auto PreviewScene = ViewportClient.Pin()->GetPreviewScene();
    if (PreviewScene == nullptr)
        return;
    
    // アイコン配置
    check(IconComponents.Num() == DetailedIconComponents.Num());
    const auto IconCnt = FMath::Min(IconComponents.Num(), plateau::Feature::MaxIconCnt);
    for (int i = 0; i < IconCnt; ++i) {
        const auto Transform = CalculateIconTransform(Box, i % plateau::Feature::MaxIconCol, i / plateau::Feature::MaxIconCol, IconComponents.Num());
        PreviewScene->AddComponent(IconComponents[i], Transform);
        PreviewScene->AddComponent(DetailedIconComponents[i], Transform);
    }
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadMaxLodAsync(const FPLATEAUFeatureInfoPanelInput& Input, const FBox& InBox) {
    Box = InBox;
    Status = EPLATEAUFeatureInfoPanelStatus::Loading;

    GetMaxLodTask = UE::Tasks::Launch(TEXT("GetMaxLODTask"),
        [Input]() mutable {
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

void FPLATEAUAsyncLoadedFeatureInfoPanel::Tick() {
    if (Status == EPLATEAUFeatureInfoPanelStatus::FullyLoaded)
        return;

    if (!GetMaxLodTask.IsCompleted())
        return;

    CreatePanelComponents(GetMaxLodTask.GetResult());
    ApplyVisibility();

    Status = EPLATEAUFeatureInfoPanelStatus::FullyLoaded;
}

