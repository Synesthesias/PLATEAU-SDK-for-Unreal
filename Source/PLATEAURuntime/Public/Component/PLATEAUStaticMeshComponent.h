// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PLATEAUComponentInterface.h"
#include "PLATEAUStaticMeshComponent.generated.h"

/**
 * 属性情報を追加しない場合にPLATEAUCityObjectGroupの代わりに使用
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUStaticMeshComponent : public UStaticMeshComponent, public IPLATEAUComponentInterface
{
	GENERATED_BODY()
	
};
