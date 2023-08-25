// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUImportModelBtn.h"
#include "PLATEAUInstancedCityModel.h"
#include "ExtentEditor/PLATEAUExtentGizmo.h"
#include "PLATEAUEditor/Public/PLATEAUEditor.h"
#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentEditor.h"
#include <plateau/dataset/dataset_source.h>


class FPLATEAUAutomationTestBase : public FAutomationTestBase {
    FString MyTestName;
    
    struct FGizmoData {
        FGizmoData(): MinX(0), MinY(0), MaxX(0), MaxY(0) {
        }

        FGizmoData(const double InMinX, const double InMinY, const double InMaxX, const double InMaxY): MinX(InMinX), MinY(InMinY), MaxX(InMaxX),
            MaxY(InMaxY) {
        }

        double MinX;
        double MinY;
        double MaxX;
        double MaxY;
    };

    bool WriteToFile(const FString& Path, const FString& Text) const {
        const FString& DirectoryPath = FPaths::GetPath(Path);
        if (!FPaths::DirectoryExists(DirectoryPath)) {
            FFileManagerGeneric::Get().MakeDirectory(*DirectoryPath, true);
        }

        return FFileHelper::SaveStringToFile(Text, *(DirectoryPath + "/" + FPaths::GetBaseFilename(Path) + ".txt"));
    }

    APLATEAUCityModelLoader* GetLocalCityModelLoader(const int ZoneId, const FVector& ReferencePoint, const int64 PackageMask, const FString& SourcePath, const FGizmoData& GizmoData, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData) const {
        const auto& ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();
        ExtentEditor->SetImportFromServer(false);
        ExtentEditor->SetSourcePath(SourcePath);
        ExtentEditor->SetLocalPackageMask(static_cast<plateau::dataset::PredefinedCityModelPackage>(PackageMask));
    
        const auto DatasetSource = plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*SourcePath));
        const std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor = DatasetSource.getAccessor();
        if (DatasetAccessor == nullptr || DatasetAccessor->getMeshCodes().size() == 0)
            return nullptr;
    
        const plateau::geometry::GeoReference RawGeoReference(ZoneId, {}, 1, plateau::geometry::CoordinateSystem::ESU);
        ExtentEditor->SetGeoReference(RawGeoReference);

        auto GeoReference = ExtentEditor->GetGeoReference();
        const auto RawCenterPoint = DatasetAccessor->calculateCenterPoint(GeoReference.GetData());
        GeoReference.ReferencePoint.X = RawCenterPoint.x;
        GeoReference.ReferencePoint.Y = RawCenterPoint.y;
        GeoReference.ReferencePoint.Z = RawCenterPoint.z;
        ExtentEditor->SetGeoReference(RawGeoReference);

        const auto ExtentGizmo = MakeUnique<FPLATEAUExtentGizmo>();
        ExtentGizmo->SetMaxX(GizmoData.MaxX);
        ExtentGizmo->SetMaxY(GizmoData.MaxY);
        ExtentGizmo->SetMinX(GizmoData.MinX);
        ExtentGizmo->SetMinY(GizmoData.MinY);
        ExtentEditor->SetExtent(ExtentGizmo->GetExtent(GeoReference));

        return UPLATEAUImportModelBtn::GetCityModelLoader(ZoneId, ReferencePoint, PackageInfoSettingsData, false);
    }
public:
	FPLATEAUAutomationTestBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask) {
	}
protected:
    void InitializeTest(const FString& TargetTestName) {
        MyTestName = TargetTestName;
        const FString TestLogPath = FPaths::ProjectDir().Append("TestLogs/" + MyTestName + ".log");
        if (FPaths::FileExists(TestLogPath)) {
            if (!FFileManagerGeneric::Get().Delete(*TestLogPath, true, true)) AddError("Failed to DeleteFile");
        }
    }
    
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
            const FPackageInfoSettings PackageInfoSettings(true, true, true, 0, 4, 1);
            TMap<int64, FPackageInfoSettings> PackageInfoSettingsData;
            PackageInfoSettingsData.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building), PackageInfoSettings);
            const auto& Loader = GetLocalCityModelLoader(ZoneId, ReferencePoint, PackageMask, SourcePath, GizmoData, PackageInfoSettingsData);
            if (Loader) {
                return Loader;
            }
        }
        
        AddError(TEXT("Loader is nullptr"));
        return nullptr;
    }

    void FinishTest() {
        const FString TestLogPath = FPaths::ProjectDir().Append("TestLogs/" + MyTestName + ".log");
        if (!WriteToFile(TestLogPath, "Succeeded")) AddError("Failed to WriteToFile");
    }
};
