// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Reconstruct/PLATEAUMeshLoaderForLandscape.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include <plateau/height_map_generator/heightmap_generator.h>
#include <Landscape.h>
#include <PLATEAUTextureLoader.h>
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/SavePackage.h"

FPLATEAUMeshLoaderForLandscape::FPLATEAUMeshLoaderForLandscape() {}

FPLATEAUMeshLoaderForLandscape::FPLATEAUMeshLoaderForLandscape(const bool InbAutomationTest){
    bAutomationTest = InbAutomationTest;
}

TArray<HeightmapCreationResult> FPLATEAUMeshLoaderForLandscape::CreateHeightMap(
    AActor* ModelActor,
    const std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param) {
    TArray<HeightmapCreationResult> CreationResults;
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        LoadNodeRecursiveForHeightMap(Model->getRootNodeAt(i), *ModelActor, Param, CreationResults);
    }
    return CreationResults;
}

void FPLATEAUMeshLoaderForLandscape::LoadNodeRecursiveForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results) {
    LoadNodeForHeightMap(InNode, InActor, Param, Results);
    const size_t ChildNodeCount = InNode.getChildCount();
    for (int i = 0; i < ChildNodeCount; i++) {
        const auto& TargetNode = InNode.getChildAt(i);
        LoadNodeRecursiveForHeightMap( TargetNode, InActor, Param, Results);
    }
}

void FPLATEAUMeshLoaderForLandscape::LoadNodeForHeightMap(
    const plateau::polygonMesh::Node& InNode,
    AActor& InActor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results) {
    if (InNode.getMesh() == nullptr || InNode.getMesh()->getVertices().size() == 0) {
        const FString DesiredName = FString(UTF8_TO_TCHAR(InNode.getName().c_str()));      
    }
    else {
        auto Result = CreateHeightMapFromMesh(*InNode.getMesh(), FString(UTF8_TO_TCHAR(InNode.getName().c_str())), InActor, Param);
        Results.Add(Result);
    }
}

HeightmapCreationResult FPLATEAUMeshLoaderForLandscape::CreateHeightMapFromMesh(
    const plateau::polygonMesh::Mesh& InMesh, const FString NodeName, AActor& Actor, FPLATEAULandscapeParam Param) {

    plateau::heightMapGenerator::HeightmapGenerator generator;
    TVec3d ExtMin, ExtMax;
    TVec2f UVMin, UVMax;
    TVec2d Offset(Param.Offset.X, Param.Offset.Y);
    std::vector<uint16_t> heightMapData = generator.generateFromMesh(InMesh, Param.TextureWidth, Param.TextureHeight, Offset, plateau::geometry::CoordinateSystem::ESU, Param.FillEdges, ExtMin, ExtMax, UVMin, UVMax);
    
    // Heightmap Image Output 
    if (Param.HeightmapImageOutput == EPLATEAULandscapeHeightmapImageOutput::PNG || Param.HeightmapImageOutput == EPLATEAULandscapeHeightmapImageOutput::PNG_RAW) {
        FString PngSavePath = FString::Format(*FString(TEXT("{0}PLATEAU/HM_{1}_{2}_{3}.png")), { FPaths::ProjectContentDir(),NodeName,Param.TextureWidth, Param.TextureHeight });
        plateau::heightMapGenerator::HeightmapGenerator::savePngFile(TCHAR_TO_ANSI(*PngSavePath), Param.TextureWidth, Param.TextureHeight, heightMapData.data());
        UE_LOG(LogTemp, Log, TEXT("height map png saved: %s"), *PngSavePath);
    }
    if (Param.HeightmapImageOutput == EPLATEAULandscapeHeightmapImageOutput::RAW || Param.HeightmapImageOutput == EPLATEAULandscapeHeightmapImageOutput::PNG_RAW) {
        FString RawSavePath = FString::Format(*FString(TEXT("{0}PLATEAU/HM_{1}_{2}_{3}.raw")), { FPaths::ProjectContentDir(),NodeName,Param.TextureWidth, Param.TextureHeight });
        plateau::heightMapGenerator::HeightmapGenerator::saveRawFile(TCHAR_TO_ANSI(*RawSavePath), Param.TextureWidth, Param.TextureHeight, heightMapData.data());
        UE_LOG(LogTemp, Log, TEXT("height map raw saved: %s"), *RawSavePath);
    }

    TArray<uint16> HeightData(heightMapData.data(), heightMapData.size());

    //Texture
    FString TexturePath;
    const auto& subMeshes = InMesh.getSubMeshes();
    if (subMeshes.size() > 0) {
        const auto& subMesh = subMeshes.at(0);
        TexturePath = FString(subMesh.getTexturePath().c_str());
    }

    //LandScape  
    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&, ExtMin, ExtMax, UVMin, UVMax, TexturePath, HeightData, NodeName, Param] {
            CreateLandScape(Actor.GetWorld(), Param.NumSubsections, Param.SubsectionSizeQuads, Param.ComponentCountX, Param.ComponentCountY, Param.TextureWidth, Param.TextureHeight, ExtMin, ExtMax, UVMin, UVMax, TexturePath, HeightData, NodeName);
        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();

    HeightmapCreationResult Result{ NodeName, heightMapData ,ExtMin, ExtMax , UVMin, UVMax };
    return Result;
}

void FPLATEAUMeshLoaderForLandscape::CreateLandScape(UWorld* World, const int32 NumSubsections, const int32 SubsectionSizeQuads, const  int32 ComponentCountX, const int32 ComponentCountY, const  int32 SizeX, const int32 SizeY,
    const TVec3d Min, const TVec3d Max, const TVec2f MinUV, const TVec2f MaxUV, const FString TexturePath, TArray<uint16> HeightData, const FString ActorName ) {

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
    FVector LandscapePosition = FVector(Min.x, Min.y, Min.z + ActualHeight / 2);
    LandscapeTransform.SetLocation(LandscapePosition);
    LandscapeTransform.SetScale3D(LandscapeScale);      

    UE_LOG(LogTemp, Log, TEXT("Create Landscape SizeX:%d SizeY:%d SubsectionSizeQuads:%d  NumSubsections:%d ComponentCount(%d,%d)"), SizeX, SizeY, SubsectionSizeQuads, NumSubsections, ComponentCountX, ComponentCountY);
    
    TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
    HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData)); 

    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
    TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
    MaterialImportLayers.Reserve(0);
    MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(MaterialImportLayers));

#if WITH_EDITOR
    FActorSpawnParameters Param;
    ALandscape* Landscape = World->SpawnActor<ALandscape>(Param);
    Landscape->bCanHaveLayersContent = false;
    Landscape->SetActorTransform(LandscapeTransform);

    Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1 , SizeY - 1 , NumSubsections, SubsectionSizeQuads, HeightDataPerLayers, nullptr, MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);
    
    //Create Package
    FString PackageName = TEXT("/Game/PLATEAU/Materials/");
    PackageName += FString::Format(*FString(TEXT("{0}_{1}_{2}")), { ActorName,FPaths::GetBaseFilename(TexturePath).Replace(TEXT("."), TEXT("_")), SizeX });
    UPackage* Package = CreatePackage(*PackageName);
    Package->FullyLoad();

    //Create Material (LandscapeはDynamicを使用するとうまく動作しないのでConstantを使用(Editorのみ動作))
    const auto SourceMaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/Materials/PLATEAULandscapeMaterial");
    UMaterial* BaseMat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, SourceMaterialPath));
    UMaterialInstanceConstant* MatIns = NewObject<UMaterialInstanceConstant>(Package, NAME_None, RF_Public | RF_Standalone | RF_MarkAsRootSet);
    MatIns->SetParentEditorOnly(BaseMat, true);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("SizeX")), SizeX);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("SizeY")), SizeY);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("MinU")), MinUV.x);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("MinV")), MinUV.y);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("MaxU")), MaxUV.x);
    MatIns->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName("MaxV")), MaxUV.y);

    if (!TexturePath.IsEmpty()) {
        const auto& Texture = FPLATEAUTextureLoader::Load(TexturePath, OverwriteTexture());
        MatIns->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(FName("MainTexture")), Texture);
    }

    //Save Material
    const FString PackageFileName = FPackageName::LongPackageNameToFilename(
        PackageName, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs Args;
    Args.SaveFlags = SAVE_NoError;
    Args.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    Args.Error = GError;
    auto result = UPackage::Save(Package, MatIns, *PackageFileName, Args);
    if(result.Result != ESavePackageResult::Success)
        UE_LOG(LogTemp, Warning, TEXT("Save Material Failed: %s %s %d"), *PackageName, *PackageFileName, result.Result);

    Landscape->LandscapeMaterial = MatIns;

    Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1), (uint32)2);

    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    LandscapeInfo->UpdateLayerInfoMap(Landscape);
    Landscape->RegisterAllComponents();

    FPropertyChangedEvent MaterialPropertyChangedEvent(FindFieldChecked< FProperty >(Landscape->GetClass(), FName("LandscapeMaterial")));
    Landscape->PostEditChangeProperty(MaterialPropertyChangedEvent);
    Landscape->PostEditChange();
    Landscape->SetActorLabel(FString(ActorName));
#endif   
}

bool FPLATEAUMeshLoaderForLandscape::OverwriteTexture() {
    return false;
}
