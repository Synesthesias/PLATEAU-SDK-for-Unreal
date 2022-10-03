// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUGeometry.h"

/**** GeoCoordinate ****/

FPLATEAUGeoCoordinate::FPLATEAUGeoCoordinate() : Latitude(0), Longitude(0), Height(0) {}

FPLATEAUGeoCoordinate::FPLATEAUGeoCoordinate(const plateau::geometry::GeoCoordinate& InGeoCoordinate) : Latitude(InGeoCoordinate.latitude)
, Longitude(InGeoCoordinate.longitude)
, Height(InGeoCoordinate.height) {}

plateau::geometry::GeoCoordinate FPLATEAUGeoCoordinate::GetNativeData() const {
    return plateau::geometry::GeoCoordinate(Latitude, Longitude, Height);
}


/**** Extent ****/

FPLATEAUExtent::FPLATEAUExtent(plateau::geometry::Extent InExtent)
    : Min(InExtent.min)
    , Max(InExtent.max) {}

plateau::geometry::Extent FPLATEAUExtent::GetNativeData() const {
    return plateau::geometry::Extent(
        Min.GetNativeData(),
        Max.GetNativeData());
}


/**** GeoReference ****/

namespace {
    plateau::geometry::GeoReference GetDefaultNativeData() {
        return plateau::geometry::GeoReference(
            TVec3d(0, 0, 0), 0.01, plateau::geometry::CoordinateSystem::NWU);
    }
}

FPLATEAUGeoReference::FPLATEAUGeoReference() : ZoneID(9)
, Data(GetDefaultNativeData()) {}

FPLATEAUGeoReference::FPLATEAUGeoReference(const plateau::geometry::GeoReference& InGeoReference) : ZoneID(InGeoReference.getZoneID())
, Data(GetDefaultNativeData()) {
    ReferencePoint.X = InGeoReference.getReferencePoint().x;
    ReferencePoint.Y = InGeoReference.getReferencePoint().y;
    ReferencePoint.Z = InGeoReference.getReferencePoint().z;
}

plateau::geometry::GeoReference& FPLATEAUGeoReference::GetData() {
    UpdateNativeData();
    return Data;
}

void FPLATEAUGeoReference::UpdateNativeData() {
    const TVec3d NativePoint(ReferencePoint.X, ReferencePoint.Y, ReferencePoint.Z);
    Data.setReferencePoint(NativePoint);
    Data.setZoneID(ZoneID);
}

FPLATEAUGeoCoordinate UPLATEAUGeoReferenceBlueprintLibrary::Unproject(FPLATEAUGeoReference& GeoReference,
    const FVector& Point) {
    const TVec3d NativePoint(Point.X, Point.Y, Point.Z);
    return GeoReference.GetData().unproject(NativePoint);
}

FVector UPLATEAUGeoReferenceBlueprintLibrary::Project(FPLATEAUGeoReference& GeoReference,
    const FPLATEAUGeoCoordinate& GeoCoordinate) {
    const auto NativePoint = GeoReference.GetData().project(GeoCoordinate.GetNativeData());
    return FVector(NativePoint.x, NativePoint.y, NativePoint.z);
}
