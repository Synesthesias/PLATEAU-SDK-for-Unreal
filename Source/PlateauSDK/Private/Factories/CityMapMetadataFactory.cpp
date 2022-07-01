#include "Factories/CityMapMetadataFactory.h"
#include "CityMapMetadata.h"

UCityMapMetadataFactory::UCityMapMetadataFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    SupportedClass = UCityMapMetadata::StaticClass();
    bCreateNew = true;
}
bool UCityMapMetadataFactory::DoesSupportClass(UClass* Class) {
    return (Class == UCityMapMetadata::StaticClass());
}
UClass* UCityMapMetadataFactory::ResolveSupportedClass() {
    return UCityMapMetadata::StaticClass();
}
UObject* UCityMapMetadataFactory::FactoryCreateNew(
    UClass* InClass,
    UObject* InParent,
    FName InName,
    EObjectFlags Flags,
    UObject* Context,
    FFeedbackContext* Warn
) {
    return NewObject<UCityMapMetadata>(InParent, InName, Flags);
}