#include "Factories/CityModelImportDataFactory.h"
#include "CityModelImportData.h"

UCityModelImportDataFactory::UCityModelImportDataFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    SupportedClass = UCityModelImportData::StaticClass();
    bCreateNew = true;
}
bool UCityModelImportDataFactory::DoesSupportClass(UClass* Class) {
    return (Class == UCityModelImportData::StaticClass());
}
UClass* UCityModelImportDataFactory::ResolveSupportedClass() {
    return UCityModelImportData::StaticClass();
}
UObject* UCityModelImportDataFactory::FactoryCreateNew(
    UClass* InClass,
    UObject* InParent,
    FName InName,
    EObjectFlags Flags,
    UObject* Context,
    FFeedbackContext* Warn
) {
    return NewObject<UCityModelImportData>(InParent, InName, Flags);
}