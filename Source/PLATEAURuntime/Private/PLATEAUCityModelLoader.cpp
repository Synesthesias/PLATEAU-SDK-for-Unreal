// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelLoader.h"

#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"
#include "citygml/citygml.h"

#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"

using namespace plateau::udx;
using namespace plateau::polygonMesh;

APLATEAUCityModelLoader::APLATEAUCityModelLoader() {
    PrimaryActorTick.bCanEverTick = false;
}

void APLATEAUCityModelLoader::Load() {
#if WITH_EDITOR
    UE_LOG(LogTemp, Log, TEXT("-----Load Start-----"));
    // 仮の範囲情報(53392642の地域メッシュ)
    Extent.Min.Latitude = 35.5335751;
    Extent.Min.Longitude = 139.7755041;
    Extent.Min.Height = -10000;
    Extent.Max.Latitude = 35.54136964;
    Extent.Max.Longitude = 139.78712557;
    Extent.Max.Height = 10000;

    // GeoReferenceを選択範囲の中心に更新
    const auto MinPoint = GeoReference.GetData().project(Extent.GetNativeData().min);
    const auto MaxPoint = GeoReference.GetData().project(Extent.GetNativeData().max);
    const auto NativeReferencePoint = (MinPoint + MaxPoint) / 2.0;
    GeoReference.ReferencePoint.X = NativeReferencePoint.x;
    GeoReference.ReferencePoint.Y = NativeReferencePoint.y;
    GeoReference.ReferencePoint.Z = NativeReferencePoint.z;

    // ファイル検索
    const auto UdxFileCollection =
        UdxFileCollection::find(TCHAR_TO_UTF8(*Source))
        ->filter(Extent.GetNativeData());
    const auto GmlFiles = UdxFileCollection->getGmlFiles(PredefinedCityModelPackage::Building);

    if (GmlFiles->size() == 0)
        return;
    UE_LOG(LogTemp, Log, TEXT("GmlFiles size : %zu"), GmlFiles->size());

    // 都市モデルパース
    citygml::ParserParams ParserParams;
    ParserParams.tesselate = true;
    const auto CityModel = citygml::load(*GmlFiles->begin(), ParserParams);
    UE_LOG(LogTemp, Log, TEXT("CityModel ID : %s"), CityModel->getId().c_str());

    // ポリゴンメッシュ抽出
    const MeshExtractOptions MeshExtractOptions(
        NativeReferencePoint, CoordinateSystem::NWU,
        MeshGranularity::PerPrimaryFeatureObject,
        3, 0, true,
        1, 0.01, Extent.GetNativeData());
    const auto Model = MeshExtractor::extract(*CityModel, MeshExtractOptions);
    UE_LOG(LogTemp, Log, TEXT("Model RootNode Count : %d"), Model->getRootNodeCount());

    // アクター生成
    AActor* modelActor = GetWorld()->SpawnActor<AActor>(FVector(0, 0, 0), FRotator(0, 0, 0));
    modelActor->RegisterAllComponents();
    modelActor->SetActorLabel("Model");
    int nodeCount = Model->getRootNodeCount();
    if (nodeCount > 0)
    {
        bool bSetRootComponent = false;
        USceneComponent* targetComponent = nullptr;
        for (int i = 0; i < nodeCount; i++)
        {
            //ProseduralMeshComponentを付与
            auto& nodeData = Model->getRootNodeAt(i);
            USceneComponent* cmp = Cast<USceneComponent>(modelActor->AddComponentByClass(USceneComponent::StaticClass(), false, modelActor->GetActorTransform(), false));
            UProceduralMeshComponent* proceduralMeshComponent = Cast<UProceduralMeshComponent>(modelActor->AddComponentByClass(UProceduralMeshComponent::StaticClass(), false, modelActor->GetActorTransform(), false));
            FString name = nodeData.getName().c_str();
            const TCHAR* aTChar = *name;
            proceduralMeshComponent->Rename(aTChar);
            if (bSetRootComponent == false)
            {
                modelActor->SetRootComponent(cmp);
                targetComponent = proceduralMeshComponent;
                bSetRootComponent = true;
            }
            else
            {
                proceduralMeshComponent->AttachToComponent(targetComponent, FAttachmentTransformRules::KeepRelativeTransform,"");
                targetComponent = proceduralMeshComponent;
            }
            auto& modelData = nodeData.getMesh();
            UE_LOG(LogTemp, Log, TEXT("-----MeshData Load-----"), modelData->getVertices().size());
            UE_LOG(LogTemp, Log, TEXT("vectorData size : %zu"), modelData->getVertices().size());
            UE_LOG(LogTemp, Log, TEXT("indicesData size : %zu"), modelData->getIndices().size());
            if (true)
            {
                // 動的メッシュ用の変数
                TArray<FVector> vertices; // 頂点群
                TArray<int32> indices; // インデックス群
                TArray<FVector> normals; // 法線群（今回は空っぽのままだが、立体を作成するときには設定しないとエッジ部分が変になる）
                TArray<FVector2D> texcoords0; // テクスチャー座標群
                TArray<FLinearColor> vertex_colors; // 頂点カラー群（今回は空っぽのまま）
                TArray<FProcMeshTangent> tangents; // 接線群（今回は空っぽのままだが、立体を作成するときには設定しないとエッジ部分が変になる

                //各種データを取得、メッシュに反映
                for (int j = 0; j < modelData->getVertices().size(); j++)
                {
                    //vertices.Add(FVector(modelData->getVertices()[j].x, modelData->getVertices()[j].y, modelData->getVertices()[j].z));
                    //UE_LOG(LogTemp, Log, TEXT("X:%f, Y:%f, Z:%f"), modelData->getVertices().at(j).x, modelData->getVertices().at(j).y, modelData->getVertices().at(j).z);
                }
                for (int k = 0; k < modelData->getIndices().size(); k++)
                {
                    //indices.Add(modelData->getIndices()[k]);
                }

                //メッシュ生成
                proceduralMeshComponent->CreateMeshSection_LinearColor(0, vertices, indices, normals, texcoords0, vertex_colors, tangents, true);
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("-----Load End-----"));
#endif
}

void APLATEAUCityModelLoader::BeginPlay() {
    Super::BeginPlay();

}

void APLATEAUCityModelLoader::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
