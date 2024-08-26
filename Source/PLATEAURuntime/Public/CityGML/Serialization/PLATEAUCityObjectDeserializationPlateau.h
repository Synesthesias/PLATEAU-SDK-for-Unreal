// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUCityObjectSerializationBase.h"

struct FPLATEAUCityObject;

class PLATEAURUNTIME_API FPLATEAUCityObjectDeserializationPlateau : public IPLATEAUCityObjectSerializationBase {

public:

    void DeserializeCityObjects(const FString InSerializedCityObjects, const TArray<TObjectPtr<USceneComponent>> InAttachChildren, TArray<FPLATEAUCityObject>& OutRootCityObjects, FString& OutOutsideParent );

protected:

    /**
    * @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
    * @param InCityObject CityModelから得られるシティオブジェクト情報
    * @param CityObjectIndex CityObjectListが持つインデックス情報
    * @return シティオブジェクト情報
    */
    FPLATEAUCityObject GetCityObject(TSharedPtr<FJsonValue> CityJsonValue);

};


