
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PLATEAUReproducedRoad.generated.h"

UENUM(BlueprintType)
enum class EReproducedRoadDirection : uint8
{
    None UMETA(DisplayName = "None"),
    Prev UMETA(DisplayName = "Prev"),
    Next UMETA(DisplayName = "Next")
};

UENUM(BlueprintType)
enum class EReproducedRoadType : uint8
{
    RoadMesh UMETA(DisplayName = "RoadMesh"),
    LaneLineAndArrow UMETA(DisplayName = "LaneLineAndArrow"),
    Crosswalk UMETA(DisplayName = "Crosswalk")
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FRoadReproduceSource
{
    GENERATED_BODY()

    UPROPERTY()
    USceneComponent* Transform;

    UPROPERTY()
    int64 RoadNetworkID;

    FRoadReproduceSource();
    explicit FRoadReproduceSource(class URnRoadBase* Road);

    FString GetName() const;
    bool IsSourceExists() const;

    bool IsMatch(const FRoadReproduceSource& Other) const;
};

UCLASS()
class PLATEAURUNTIME_API APLATEAUReproducedRoad : public AActor
{
    GENERATED_BODY()

public:
    APLATEAUReproducedRoad();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    EReproducedRoadType RoadType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    FRoadReproduceSource ReproduceSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    EReproducedRoadDirection RoadDirection;

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    void Init(EReproducedRoadType RoadTypeArg, const FRoadReproduceSource& SourceRoadArg, EReproducedRoadDirection RoadDirectionArg);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    static AActor* Find(EReproducedRoadType RoadTypeArg, const FRoadReproduceSource& SourceRoadArg, EReproducedRoadDirection RoadDirectionArg);

    static FString GetGameObjName(EReproducedRoadType Type);
};
