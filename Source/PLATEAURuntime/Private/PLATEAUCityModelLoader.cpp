// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "citygml/citygml.h"

using namespace plateau::udx;
using namespace plateau::polygonMesh;

APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
}

void APLATEAUCityModelLoader::Load() {
    // ���͈̔͏��(53392642�̒n�惁�b�V��)
    Extent.Min.Latitude = 35.5335751;
    Extent.Min.Longitude = 139.7755041;
    Extent.Min.Height = -10000;
    Extent.Max.Latitude = 35.54136964;
    Extent.Max.Longitude = 139.78712557;
    Extent.Max.Height = 10000;

    // GeoReference��I��͈͂̒��S�ɍX�V
    const auto MinPoint = GeoReference.GetData().project(Extent.GetNativeData().min);
    const auto MaxPoint = GeoReference.GetData().project(Extent.GetNativeData().max);
    const auto NativeReferencePoint = (MinPoint + MaxPoint) / 2.0;
    GeoReference.ReferencePoint.X = NativeReferencePoint.x;
    GeoReference.ReferencePoint.Y = NativeReferencePoint.y;
    GeoReference.ReferencePoint.Z = NativeReferencePoint.z;

    // �t�@�C������
    const auto UdxFileCollection =
        UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
        ->filter(Extent.GetNativeData());
    const auto GmlFiles = UdxFileCollection->getGmlFiles(PredefinedCityModelPackage::Building);

    if (GmlFiles->size() == 0)
        return;

    // �s�s���f���p�[�X
    citygml::ParserParams ParserParams;
    ParserParams.tesselate = true;
    const auto CityModel = citygml::load(*GmlFiles->begin(), ParserParams);

    // �|���S�����b�V�����o
    const MeshExtractOptions MeshExtractOptions(
        NativeReferencePoint, CoordinateSystem::NWU,
        MeshGranularity::PerPrimaryFeatureObject,
        3, 0, true,
        1, 0.01, 9, Extent.GetNativeData());
    const auto Model = MeshExtractor::extract(*CityModel, MeshExtractOptions);

    // StaticMesh����
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();

}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
