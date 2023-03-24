// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"


/**
 *
 */
class FPLATEAUInstancedCityModelDetails : public IDetailCustomization {
public:
    /**
     * @brief FPLATEAUCityModelLoaderDetailsのインスタンスを生成します。
     */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization インターフェース **/
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    // UIの内部状態
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
};
