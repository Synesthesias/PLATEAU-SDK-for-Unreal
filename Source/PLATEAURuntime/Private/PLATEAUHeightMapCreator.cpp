// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUHeightMapCreator.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include <plateau/texture/heightmap_generator.h>

//#include <iostream>
//#include <fstream>

#include <Landscape.h>
#include <LandscapeProxy.h>



FPLATEAUHeightMapCreator::FPLATEAUHeightMapCreator() {}

FPLATEAUHeightMapCreator::FPLATEAUHeightMapCreator(const bool InbAutomationTest){
    bAutomationTest = InbAutomationTest;
}

void FPLATEAUHeightMapCreator::CreateHeightMap(
    AActor* ModelActor,
    const std::shared_ptr<plateau::polygonMesh::Model> Model,
    const FLoadInputData& LoadInputData,
    const std::shared_ptr<const citygml::CityModel> CityModel) {

    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        LoadNodeRecursiveForHeightMap(Model->getRootNodeAt(i), *ModelActor);
    }
}

void FPLATEAUHeightMapCreator::LoadNodeRecursiveForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor) {
    LoadNodeForHeightMap(InNode, InActor);
    const size_t ChildNodeCount = InNode.getChildCount();
    for (int i = 0; i < ChildNodeCount; i++) {
        const auto& TargetNode = InNode.getChildAt(i);
        LoadNodeRecursiveForHeightMap( TargetNode, InActor);
    }
}

void FPLATEAUHeightMapCreator::LoadNodeForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor) {
    if (InNode.getMesh() == nullptr || InNode.getMesh()->getVertices().size() == 0) {
        const FString DesiredName = FString(UTF8_TO_TCHAR(InNode.getName().c_str()));      
    }
    else {
        CreateHeightMapFromMesh(*InNode.getMesh(), FString(UTF8_TO_TCHAR(InNode.getName().c_str())), InActor);
    }
}

void FPLATEAUHeightMapCreator::CreateLandScape(UWorld* World) {

    const int32 NumSubsections = 1; //1 , 2
    const int32 SubsectionSizeQuads = 127;  //7, 15, 31, 63, 127, 255

    const int32 ComponentCountX = 2;
    const int32 ComponentCountY = 2;

    const int32 SizeX = TextureWidth;
    const int32 SizeY = TextureHeight;

    //double AltMeter = 17.7; //高さ（メートル）
    double ActualHeight = 1770; //高さ（cm）
    double ActualXSize = 26268.1206815294;
    double ActualYSize = 23869.9836173063;

    //Height map 作成
    //FString HeightMapPath = FPaths::ConvertRelativePathToFull(*(FPaths::ProjectContentDir() + FString("PLATEAU/"))) + FString("HeightMap.png");
    FString HeightMapRawPath = FPaths::ConvertRelativePathToFull(*(FPaths::ProjectContentDir() + FString("PLATEAU/"))) + FString("HeightMap.raw");
    
    //std::vector <uint16_t> image_data = plateau::texture::HeightmapGenerator::readPngFile(TCHAR_TO_ANSI(*HeightMapPath), TextureWidth, TextureHeight);
    std::vector <uint16_t> image_data = plateau::texture::HeightmapGenerator::readRawFile(TCHAR_TO_ANSI(*HeightMapRawPath), TextureWidth, TextureHeight);

    const TArray<uint16> HeightData(image_data.data(), image_data.size());

    FString ActorName = FString("DebugLandScape");

    CreateLandScape(World, NumSubsections, SubsectionSizeQuads, ComponentCountX, ComponentCountY, SizeX,  SizeY, 
        TVec3d(-ActualXSize / 2, -ActualYSize / 2, -ActualHeight / 2), TVec3d(ActualXSize / 2, ActualYSize / 2, ActualHeight / 2), HeightData, ActorName);
}

void FPLATEAUHeightMapCreator::CreateLandScape(UWorld* World, const int32 NumSubsections, const int32 SubsectionSizeQuads, const  int32 ComponentCountX, const int32 ComponentCountY, const  int32 SizeX, const int32 SizeY,
    const TVec3d Min, const TVec3d Max, TArray<uint16> HeightData, const FString ActorName ) {

    // Weightmap is sized the same as the component
    const int32 WeightmapSize = (SubsectionSizeQuads + 1) * NumSubsections;
    // Should be power of two
    if (!FMath::IsPowerOfTwo(WeightmapSize)) {
        UE_LOG(LogTemp, Error, TEXT("WeightmapSize not POT:%d"), WeightmapSize);
        return;
    }

    double ActualHeight = abs(Max.z - Min.z);
    double ActualXSize = abs(Max.x - Min.x);
    double ActualYSize = abs(Max.y - Min.y);

    float HeightScale = ActualHeight / 511.992;

    float XScale = ActualXSize / (SizeX - 1 ); 
    float YScale = ActualYSize / (SizeY - 1 );

    FTransform LandscapeTransform;
    FVector LandscapeScale = FVector(XScale, YScale, HeightScale);
    //FVector LandscapePosition = FVector(Min.x, Min.y, Max.z - ActualHeight / 2);
    FVector LandscapePosition = FVector(Min.x, Min.y, Min.z + ActualHeight / 2);
    LandscapeTransform.SetLocation(LandscapePosition);
    LandscapeTransform.SetScale3D(LandscapeScale);      

    UE_LOG(LogTemp, Error, TEXT("SubsectionSizeQuads:%d SizeX:%d SizeY:%d"), SubsectionSizeQuads, SizeX, SizeY);
    
    TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
    HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData)); //ENewLandscapePreviewMode.NewLandscape

    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
    TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
    MaterialImportLayers.Reserve(0);
    MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(MaterialImportLayers));

    FActorSpawnParameters Param;
    ALandscape* Landscape = World->SpawnActor<ALandscape>(Param);
    Landscape->bCanHaveLayersContent = false;
    Landscape->LandscapeMaterial = nullptr;
    Landscape->SetActorTransform(LandscapeTransform);

    Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1 , SizeY - 1 , NumSubsections, SubsectionSizeQuads, HeightDataPerLayers, nullptr, MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);

    Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1), (uint32)2);

    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();

    LandscapeInfo->UpdateLayerInfoMap(Landscape);
    Landscape->RegisterAllComponents();

    FPropertyChangedEvent MaterialPropertyChangedEvent(FindFieldChecked< FProperty >(Landscape->GetClass(), FName("LandscapeMaterial")));
    Landscape->PostEditChangeProperty(MaterialPropertyChangedEvent);
    Landscape->PostEditChange();
    Landscape->SetActorLabel(FString(ActorName));
}


void FPLATEAUHeightMapCreator::CreateHeightMapFromMesh( 
    const plateau::polygonMesh::Mesh& InMesh, const FString NodeName, AActor& Actor) {

    plateau::texture::HeightmapGenerator generator;
    TVec3d ExtMin, ExtMax;
    std::vector<uint16_t> heightMapData = generator.generateFromMesh(InMesh, TextureWidth, TextureHeight, TVec2d(500, 500), plateau::geometry::CoordinateSystem::ESU, ExtMin, ExtMax);

    UE_LOG(LogTemp, Error, TEXT("Ext Min (%f, %f, %f ) Max (%f, %f, %f )"), ExtMin.x, ExtMin.y, ExtMin.z, ExtMax.x, ExtMax.y, ExtMax.z);

    // Debug Image Output 
    FString SavePath = FPaths::ConvertRelativePathToFull(*(FPaths::ProjectContentDir() + FString("PLATEAU/"))) + FString("HeightMap.png");
    UE_LOG(LogTemp, Error, TEXT("Save png %s"), *SavePath);
    plateau::texture::HeightmapGenerator::savePngFile(TCHAR_TO_ANSI(*SavePath), TextureWidth, TextureHeight, heightMapData.data());

    FString RawSavePath = FPaths::ConvertRelativePathToFull(*(FPaths::ProjectContentDir() + FString("PLATEAU/"))) + FString("HeightMap.raw");
    UE_LOG(LogTemp, Error, TEXT("Save raw %s"), *RawSavePath);
    plateau::texture::HeightmapGenerator::saveRawFile(TCHAR_TO_ANSI(*RawSavePath), TextureWidth, TextureHeight, heightMapData.data());
   

    TArray<uint16> HeightData(heightMapData.data(), heightMapData.size());

    //LandScape  
    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&, ExtMin, ExtMax, HeightData, NodeName] {
            const int32 NumSubsections = 1; //1 , 2
            const int32 SubsectionSizeQuads = 127;  //7, 15, 31, 63, 127, 255
            const int32 ComponentCountX = 2;
            const int32 ComponentCountY = 2;
            CreateLandScape(Actor.GetWorld(), NumSubsections, SubsectionSizeQuads, ComponentCountX, ComponentCountY, TextureWidth, TextureHeight, ExtMin, ExtMax, HeightData, NodeName);
        }, TStatId(), nullptr, ENamedThreads::GameThread);
}
