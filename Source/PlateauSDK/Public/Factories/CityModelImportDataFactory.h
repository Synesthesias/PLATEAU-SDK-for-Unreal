#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CityModelImportDataFactory.generated.h"

/**
 *
 */
UCLASS()
class PLATEAUSDK_API UCityModelImportDataFactory : public UFactory {
    GENERATED_UCLASS_BODY()

    virtual bool DoesSupportClass(UClass* Class) override;
    virtual UClass* ResolveSupportedClass() override;
    virtual UObject* FactoryCreateNew(
        UClass* InClass,
        UObject* InParent,
        FName InName,
        EObjectFlags Flags,
        UObject* Context,
        FFeedbackContext* Warn
    ) override;
};
