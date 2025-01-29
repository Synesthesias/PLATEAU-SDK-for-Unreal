// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "MWLine.generated.h"

UCLASS()
class PLATEAURUNTIME_API UMWLine : public UObject
{
    GENERATED_BODY()

public:
	UMWLine();
	UMWLine(const TArray<FVector>& InPoints);
	~UMWLine();

	TArray<FVector> GetPoints() const { return Points; }
	void SetPoints(const TArray<FVector>& InPoints) { Points = InPoints; }
	
	FVector operator[](int32 Index) const { return Points[Index]; }
	int32 Num() const { return Points.Num(); }
	
	float SumDistance() const;

private:
	TArray<FVector> Points;
};
