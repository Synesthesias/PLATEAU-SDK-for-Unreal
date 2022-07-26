// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "FeaturePlacementSettingsPropertyRow.h"
#include "IDetailCustomization.h"
#include "PLATEAUCityModelLoader.h"

/**
 *
 */
class FPLATEAUCityMapDetails : public IDetailCustomization {
public:
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

    // TODO: APLATEAUCityMapへ移動
    static void PlaceMeshes(APLATEAUCityModelLoader& Actor);
    static void PlaceCityModel(APLATEAUCityModelLoader& Actor, USceneComponent& ParentComponent, const FPLATEAUImportedCityModelInfo& CityModelInfo, int TargetLOD, bool bShouldPlaceLowerLODs);
    static UStaticMeshComponent* PlaceStaticMesh(APLATEAUCityModelLoader& Actor, USceneComponent& ParentComponent, UStaticMesh* StaticMesh);
    static USceneComponent* PlaceEmptyComponent(APLATEAUCityModelLoader& Actor, USceneComponent& ParentComponent, const FName& Name);

private:
    FReply OnClickPlace();


    static FString GetMeshName(int LOD, FString CityObjectID);

    TSharedPtr<IPropertyHandle> BuildingPlacementModeProperty;
    TSharedPtr<IPropertyHandle> BuildingLODProperty;

    // UIの内部状態
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;

    TMap<ECityModelPackage, FFeaturePlacementRow> FeaturePlacementRows;
};
