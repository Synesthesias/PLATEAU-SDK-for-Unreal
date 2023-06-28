// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "PLATEAUCityModelLoader.h"
#include "PLATEAUImportModelBtn.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUInstancedCityModelImp.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"


class FPLATEAUAutomationTestBase : public FAutomationTestBase
{
public:
	FPLATEAUAutomationTestBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask) {
	}
protected:
    UWorld* GetWorld() {
        for (auto WorldContext : GEngine->GetWorldContexts()) {
            if (WorldContext.World() != nullptr) {
                return WorldContext.World();
            }
        }
        AddError("World == nullptr");
        return nullptr;
    }
    
    bool OpenNewMap() const {
        const FString TemplateMap = FPaths::EngineContentDir().Append("Maps/Templates/Template_Default.umap");
        return FEditorFileUtils::LoadMap(TemplateMap, true, true);
    }
    
    APLATEAUCityModelLoader* GetInstancedCityLoader(const UWorld& World) {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(&World, APLATEAUInstancedCityModel::StaticClass(), FoundActors);
        if (0 < FoundActors.Num()) {
            AddError(TEXT("0 < FoundActors.Num()"));
        } else {
            constexpr int ZoneId = 9;
            const FVector ReferencePoint = FVector(-472995.5625, 5131221.5, 0);
            constexpr int64 PackageMask = static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building);
            const FString SourcePath = UKismetSystemLibrary::GetProjectContentDirectory().Append("data");
            const FGizmoData GizmoData(4486.303033, 4728.534916, 6486.303033, 6728.534916);
            const FPackageInfoSettings PackageInfoSettings(true, true, 0, 4, 1);
            TMap<int64, FPackageInfoSettings> PackageInfoSettingsData;
            PackageInfoSettingsData.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building), PackageInfoSettings);
            const auto& Loader = UPLATEAUInstancedCityModelImp::GetLocalCityModelLoader(ZoneId, ReferencePoint, PackageMask, SourcePath, GizmoData, PackageInfoSettingsData);
            if (Loader) {
                return Loader;
            }
        }
        
        AddError(TEXT("Loader is nullptr"));
        return nullptr;
    }
};
