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
    // 仮の範囲情報(53392642の地域メッシュ)
    Extent.Min.Latitude = 35.54136964;
    Extent.Min.Longitude = 139.7755041;
    Extent.Min.Height = -1000;
    Extent.Max.Latitude = 35.5335751;
    Extent.Max.Longitude = 139.78712557;
    Extent.Min.Height = 1000;

    // GeoReference更新
    //GeoReference.ReferencePoint = GeoReference Extent.GetNativeData().centerPoint();

    // ファイル検索
    const auto UdxFileCollection =
        UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
        ->filter(Extent.GetNativeData());
    const auto GmlFiles = UdxFileCollection->getGmlFiles(PredefinedCityModelPackage::Building);

    if (GmlFiles->size() == 0)
        return;

    // 都市モデルパース
    citygml::ParserParams ParserParams;
    ParserParams.tesselate = true;
    const auto CityModel = citygml::load(*GmlFiles->begin(), ParserParams);

    //MeshExtractOptions MeshExtractOptions()
    //MeshExtractor::extract(CityModel, )
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();

}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
