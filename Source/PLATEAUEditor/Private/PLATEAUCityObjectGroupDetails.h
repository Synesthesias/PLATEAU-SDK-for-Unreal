// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"


class FPLATEAUCityObjectGroupDetails : public IDetailCustomization {
public:
    /**
     * @brief FPLATEAUCityObjectGroupDetailsインスタンス生成
     */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /**
     * @brief IDetailCustomizationインターフェース
     */    
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
    TSharedPtr<IPropertyHandle> BuildingPlacementModeProperty;
    TSharedPtr<IPropertyHandle> BuildingLODProperty;

    // UIの内部状態
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
};
