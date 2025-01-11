#pragma once
#include <memory>
//#include "CityGML/PLATEAUCityObject.h"
//#include "Serialization/JsonWriter.h"
//#include "Serialization/JsonReader.h"
//#include "Serialization/JsonSerializer.h"
#include "SubDividedCityObject.h"
#include "../../Component/PLATEAUSceneComponent.h"
#include "PLATEAUSubDividedCityObjectGroup.generated.h"

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PLATEAURUNTIME_API UPLATEAUSubDividedCityObjectGroup : public UPLATEAUSceneComponent
{
    GENERATED_BODY()
public:

    UPLATEAUSubDividedCityObjectGroup(){}
    // --------------------
    // start:フィールド
    // --------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    TArray<FSubDividedCityObject> CityObjects;

};
