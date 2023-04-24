// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Blutility/Classes/EditorUtilityWidget.h"
#include "PLATEAUSDKEditorUtilityWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAreaSelectSuccessDelegate, FVector3d, ReferencePoint, int64, PackageMask);

UCLASS(Blueprintable)
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidget : public UEditorUtilityWidget {
	GENERATED_BODY()
public:
    void AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const;	

    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FAreaSelectSuccessDelegate AreaSelectSuccessDelegate;
};
