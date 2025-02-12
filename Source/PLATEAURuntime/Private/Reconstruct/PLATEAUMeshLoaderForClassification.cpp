// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForClassification.h"

#include "PLATEAUCachedMaterialArray.h"
#include "PLATEAUMeshLoader.h"
#include "Component/PLATEAUCityObjectGroup.h"

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(FPLATEAUCachedMaterialArray Mats) {
    CachedMaterials = Mats;
    bAutomationTest = false;
}

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(FPLATEAUCachedMaterialArray Mats, const bool InbAutomationTest) {
    CachedMaterials = Mats;
    bAutomationTest = InbAutomationTest;
}

UMaterialInterface* FPLATEAUMeshLoaderForClassification::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component,
    const FLoadInputData& LoadInputData, UTexture2D* Texture, FNodeHierarchy NodeHier) {

    UE_LOG(LogTemp, Log, TEXT("GetMaterialForSubMesh: %d"), SubMeshValue.GameMaterialID);

    if (SubMeshValue.GameMaterialID > -1 && CachedMaterials.Num() > 0 && SubMeshValue.GameMaterialID < CachedMaterials.Num()) {
        const auto MatPtr = CachedMaterials.Get(SubMeshValue.GameMaterialID);
        if (MatPtr != nullptr) {
            const auto& Mat = MatPtr;
            if (Mat != nullptr)
            {
                // ここで UMaterialInstanceDynamic::Create(Mat, Component); としてはいけません。
                // なぜなら、マテリアル分けを複数回実行したときに、キー指定対象外の部分でマテリアルが剥がれるためです。
                return Mat;
            }
                
        } 
    }

    //Defaultマテリアル設定時
    const auto& DefaultMaterial = CachedMaterials.GetDefaultMaterial();
    if (DefaultMaterial != nullptr) {
        return DefaultMaterial;
    }

    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture, NodeHier);
}



