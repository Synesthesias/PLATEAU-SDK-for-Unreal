// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyRow.h"
#include "PLATEAUCityMap.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"

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
    TSharedRef<SWidget> OnGetFeaturePlacementModeComboContent() const;
    void CommitPlacementMode(EFeaturePlacementMode NewMode);

    FReply OnClickPlace();

    void PlaceMeshes(APLATEAUCityMap& Actor);
    void PlaceCityModel(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, int SourceGmlIndex, int TargetLOD, bool bShouldPlaceLowerLODs);
    UStaticMeshComponent* PlaceStaticMesh(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, UStaticMesh* StaticMesh);
    USceneComponent* PlaceEmptyComponent(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, const FName& Name);

    FString GetMeshName(int LOD, FString CityObjectID);

    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    IDetailPropertyRow* BuildingLODPropertyRow;
    
    TSharedPtr<IPropertyHandle> BuildingPlacementModeProperty;

    TMap<EFeaturePlacementMode, FText> FeaturePlacementTexts() const
    {
        TMap<EFeaturePlacementMode, FText> Items;
        Items.Add(EFeaturePlacementMode::DontPlace, LOCTEXT("DontPlace", "配置無し"));
        Items.Add(EFeaturePlacementMode::PlaceMaxLOD, LOCTEXT("PlaceMaxLOD", "最大LODを配置"));
        return Items;
    }
};

#undef LOCTEXT_NAMESPACE
