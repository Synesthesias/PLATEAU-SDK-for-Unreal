// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "FeaturePlacementSettingsPropertyRow.h"
#include "IDetailCustomization.h"
#include "PLATEAUCityMap.h"

/**
 *
 */
class FPLATEAUCityMapDetails : public IDetailCustomization {
public:
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    FReply OnClickPlace();

    void PlaceMeshes(APLATEAUCityMap& Actor);
    void PlaceCityModel(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, const FPLATEAUImportedCityModelInfo& CityModelInfo, int TargetLOD, bool bShouldPlaceLowerLODs);
    UStaticMeshComponent* PlaceStaticMesh(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, UStaticMesh* StaticMesh);
    USceneComponent* PlaceEmptyComponent(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, const FName& Name);

    FString GetMeshName(int LOD, FString CityObjectID);

    TSharedPtr<IPropertyHandle> BuildingPlacementModeProperty;
    TSharedPtr<IPropertyHandle> BuildingLODProperty;

    // UIの内部状態
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;

    TMap<ECityModelPackage, FFeaturePlacementRow> FeaturePlacementRows;
};
