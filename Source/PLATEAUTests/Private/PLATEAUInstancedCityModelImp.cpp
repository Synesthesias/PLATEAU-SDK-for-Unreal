// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUInstancedCityModelImp.h"
#include "Kismet/GameplayStatics.h"
#include <plateau/dataset/dataset_source.h>
#include "PLATEAUImportModelBtn.h"
#include "PLATEAUInstancedCityModel.h"
#include "ExtentEditor/PLATEAUExtentGizmo.h"
#include "PLATEAUEditor/Public/PLATEAUEditor.h"
#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentEditor.h"
#include "PLATEAURuntime/Public/PLATEAUCityModelLoader.h"


APLATEAUCityModelLoader* UPLATEAUInstancedCityModelImp::GetLocalCityModelLoader(const int ZoneId, const FVector& ReferencePoint, const int64 PackageMask, const FString& SourcePath, const FGizmoData& GizmoData, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData) {
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