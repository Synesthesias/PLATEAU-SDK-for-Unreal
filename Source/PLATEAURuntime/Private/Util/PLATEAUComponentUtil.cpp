// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Util/PLATEAUComponentUtil.h"
#include "Util/PLATEAUGmlUtil.h"
#include <Misc/DefaultValueHelper.h>
#include <PLATEAUImportSettings.h>

namespace {
    void GetRootCityObjectsRecursive(USceneComponent* SceneComponent, TArray<FPLATEAUCityObject>& RootCityObjects) {
        if (const auto& CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(SceneComponent)) {
            const auto& AllRootCityObjects = CityObjectGroup->GetAllRootCityObjects();
            for (const auto& CityObject : AllRootCityObjects) {
                RootCityObjects.Add(CityObject);
            }
        }

        for (const auto& AttachedComponent : SceneComponent->GetAttachChildren()) {
            GetRootCityObjectsRecursive(AttachedComponent, RootCityObjects);
        }
    }
}

FString FPLATEAUComponentUtil::MakeUniqueGmlObjectName(AActor* Actor, UClass* Class, const FString& BaseName) {
    auto Name = BaseName;
    Name.AppendChar(TEXT('_'));
    return MakeUniqueObjectName(Actor, Class, FName(Name)).ToString();
}

FString FPLATEAUComponentUtil::GetOriginalComponentName(const USceneComponent* const InComponent) {
    auto ComponentName = InComponent->GetName();

    int Index = 0;
    if (ComponentName.FindLastChar('_', Index)) {
        if (ComponentName.RightChop(Index + 1).IsNumeric()) {
            ComponentName = ComponentName.LeftChop(ComponentName.Len() - Index + 1);
        }
    }
    return ComponentName;
}

USceneComponent* FPLATEAUComponentUtil::FindChildComponentWithOriginalName(USceneComponent* ParentComponent, const FString& OriginalName) {
    for (const auto& Component : ParentComponent->GetAttachChildren()) {
        const auto TargetName = GetOriginalComponentName(Component);
        if (TargetName == OriginalName)
            return Component;
    }
    return nullptr;
}

TMap<FString, UPLATEAUCityObjectGroup*> FPLATEAUComponentUtil::CreateComponentsMapWithGmlId(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    TMap<FString, UPLATEAUCityObjectGroup*> Map;
    for (auto Comp : TargetCityObjects) {
        Map.Add(GetOriginalComponentName(Comp), Comp);
    }
    return Map;
}

TMap<FString, UPLATEAUCityObjectGroup*> FPLATEAUComponentUtil::CreateComponentsMapWithNodePath(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    TMap<FString, UPLATEAUCityObjectGroup*> Map;
    for (auto Comp : TargetCityObjects) {
        FString Path = FPLATEAUGmlUtil::GetNodePathString(Comp);
        Map.Add(Path, Comp);
    }
    return Map;
}

void FPLATEAUComponentUtil::DestroyOrHideComponents(TArray<UPLATEAUCityObjectGroup*> Components, bool bDestroy) {
    for (auto Comp : Components) {
        if (bDestroy)
            Comp->DestroyComponent();
        else
            Comp->SetVisibility(false);
    }
}

TArray<FPLATEAUCityObject> FPLATEAUComponentUtil::GetRootCityObjects(USceneComponent* SceneComponent) {
    TArray<FPLATEAUCityObject> RootCityObjects;
    GetRootCityObjectsRecursive(SceneComponent, RootCityObjects);
    return RootCityObjects;
}

TArray<UActorComponent*> FPLATEAUComponentUtil::GetComponentsByPackage(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, EPLATEAUCityModelPackage Pkg) {
    TArray<UActorComponent*> ResultComponents;
    plateau::dataset::PredefinedCityModelPackage Package = UPLATEAUImportSettings::GetPredefinedCityModelPackageFromPLATEAUCityModelPackage(Pkg);
    for (const auto& GmlComponent : GmlComponents) {
        if (FPLATEAUGmlUtil::GetCityModelPackage(GmlComponent) == Package) {
            if (!GmlComponent.GetName().Contains("BillboardComponent"))
                ResultComponents.Add(GmlComponent);
        }
    }
    return ResultComponents;
}

int FPLATEAUComponentUtil::ParseLodComponent(const USceneComponent* const InLodComponent) {
    auto LodString = FPLATEAUComponentUtil::GetOriginalComponentName(InLodComponent);
    // "Lod{数字}"から先頭3文字除外することで数字を抜き出す。
    LodString = LodString.RightChop(3);

    int Lod;
    FDefaultValueHelper::ParseInt(LodString, Lod);

    return Lod;
}

FPLATEAUMinMaxLod FPLATEAUComponentUtil::GetMinMaxLod(const TArray<TObjectPtr<USceneComponent>>& GmlComponents, const plateau::dataset::PredefinedCityModelPackage InPackage) {
    TArray<int> Lods;

    for (const auto& GmlComponent : GmlComponents) {
        if ((FPLATEAUGmlUtil::GetCityModelPackage(GmlComponent) & InPackage) == plateau::dataset::PredefinedCityModelPackage::None)
            continue;

        for (const auto& LodComponent : GmlComponent->GetAttachChildren()) {
            const auto Lod = ParseLodComponent(LodComponent);
            if (Lods.Contains(Lod))
                continue;

            Lods.Add(Lod);
        }
    }
    return { FMath::Min(Lods), FMath::Max(Lods) };
}

TArray<USceneComponent*> FPLATEAUComponentUtil::ConvertArrayToSceneComponentArray(TArray<UActorComponent*> InComponents) {
    TArray<USceneComponent*> OutComponents;
    for (const auto& Component : InComponents)
        OutComponents.Add(StaticCast<USceneComponent*>(Component));
    return OutComponents;
}

TArray<USceneComponent*> FPLATEAUComponentUtil::FindComponentsByName(const AActor* ModelActor, const FString Name) {
    const FRegexPattern pattern = FRegexPattern(FString::Format(*FString(TEXT("^{0}__([0-9]+)")), { Name }));
    TArray<USceneComponent*> Result;
    const auto Components = ModelActor->GetComponents();
    for (auto Component : Components) {
        if (Component->IsA<USceneComponent>()) {
            FRegexMatcher matcher(pattern, Component->GetName());
            if (matcher.FindNext()) {
                Result.Add((USceneComponent*)Component);
            }
        }
    }
    return Result;
}

UPLATEAUCityObjectGroup* FPLATEAUComponentUtil::GetCityObjectGroupByName(const AActor* ModelActor, const FString Name) {
    const auto BaseComponents = FindComponentsByName(ModelActor, Name);
    if (BaseComponents.Num() > 0) {
        UPLATEAUCityObjectGroup* FoundItem;
        int32 ItemIndex;
        if (BaseComponents.FindItemByClass<UPLATEAUCityObjectGroup>(&FoundItem, &ItemIndex)) {
            return FoundItem;
        }
    }
    return nullptr;
}
