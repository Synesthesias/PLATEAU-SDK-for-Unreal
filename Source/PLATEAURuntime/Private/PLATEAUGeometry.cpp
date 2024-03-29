// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUGeometry.h"

/**** GeoCoordinate ****/

FPLATEAUGeoCoordinate::FPLATEAUGeoCoordinate() : Latitude(0), Longitude(0), Height(0) {}

FPLATEAUGeoCoordinate::FPLATEAUGeoCoordinate(const plateau::geometry::GeoCoordinate& InGeoCoordinate) : Latitude(InGeoCoordinate.latitude)
, Longitude(InGeoCoordinate.longitude)
, Height(InGeoCoordinate.height) {}

plateau::geometry::GeoCoordinate FPLATEAUGeoCoordinate::GetNativeData() const {
    return plateau::geometry::GeoCoordinate(Latitude, Longitude, Height);
}

bool FPLATEAUGeoCoordinate::operator==(const FPLATEAUGeoCoordinate& other) const {
    return
        Latitude == other.Latitude &&
        Longitude == other.Longitude &&
        Height == other.Height;
}

bool FPLATEAUGeoCoordinate::operator!=(const FPLATEAUGeoCoordinate& other) const {
    return !(*this == other);
}


/**** Extent ****/

FPLATEAUExtent::FPLATEAUExtent(const plateau::geometry::Extent& InExtent)
    : Min(InExtent.min)
    , Max(InExtent.max) {}

plateau::geometry::Extent FPLATEAUExtent::GetNativeData() const {
    return plateau::geometry::Extent(
        Min.GetNativeData(),
        Max.GetNativeData());
}

bool FPLATEAUExtent::IsSet() const {
    return Min != Max;
}

bool FPLATEAUExtent::operator==(const FPLATEAUExtent& other) const {
    return Max == other.Max && Min == other.Min;
}

bool FPLATEAUExtent::operator!=(const FPLATEAUExtent& other) const {
    return !(*this == other);
}


/**** GeoReference ****/

namespace {
    plateau::geometry::GeoReference GetDefaultNativeData() {
        return plateau::geometry::GeoReference(
            9, TVec3d(0, 0, 0), 0.01, plateau::geometry::CoordinateSystem::ESU);
    }
}

FPLATEAUGeoReference::FPLATEAUGeoReference()
    : ZoneID(9)
    , ReferencePoint(FVector::ZeroVector)
    , Data(GetDefaultNativeData()) {}

FPLATEAUGeoReference::FPLATEAUGeoReference(const plateau::geometry::GeoReference& InGeoReference)
    : ZoneID(InGeoReference.getZoneID())
    , Data(InGeoReference) {
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

FPLATEAUExtent UPLATEAUGeoReferenceBlueprintLibrary::UnprojectExtent(FPLATEAUGeoReference& GeoReference,
    const FBox& Box)
{
    auto Min = Unproject(GeoReference, Box.Min);
    auto Max = Unproject(GeoReference, Box.Max);
    auto Tmp = Min;
    Min.Latitude = FMath::Min(Tmp.Latitude, Max.Latitude);
    Min.Longitude = FMath::Min(Tmp.Longitude, Max.Longitude);
    Min.Height = FMath::Min(Tmp.Height, Max.Height);
    Max.Latitude = FMath::Max(Tmp.Latitude, Max.Latitude);
    Max.Longitude = FMath::Max(Tmp.Longitude, Max.Longitude);
    Max.Height = FMath::Max(Tmp.Height, Max.Height);

    return FPLATEAUExtent(plateau::geometry::Extent(Min.GetNativeData(), Max.GetNativeData()));
}

FBox UPLATEAUGeoReferenceBlueprintLibrary::ProjectExtent(FPLATEAUGeoReference& GeoReference,
    const FPLATEAUExtent& Extent)
{
    auto Min = Project(GeoReference, Extent.Min);
    auto Max = Project(GeoReference, Extent.Max);
    auto Tmp = Min;
    Min.X = FMath::Min(Tmp.X, Max.X);
    Min.Y = FMath::Min(Tmp.Y, Max.Y);
    Min.Z = FMath::Min(Tmp.Z, Max.Z);
    Max.X = FMath::Max(Tmp.X, Max.X);
    Max.Y = FMath::Max(Tmp.Y, Max.Y);
    Max.Z = FMath::Max(Tmp.Z, Max.Z);

    return FBox(Min, Max);
}
