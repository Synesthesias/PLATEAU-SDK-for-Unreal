
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/RoadReproducer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "RoadAdjustment/RoadMarkingGenerator.h"

URoadReproducer::URoadReproducer()
{
}

void URoadReproducer::Generate(class IIRrTarget* Target, ECrosswalkFrequency CrosswalkFrequency)
{
    //// 道路ネットワークから道路メッシュを生成
    //URoadNetworkToMesh* Rnm = NewObject<URoadNetworkToMesh>();
    //Rnm->Initialize(Target, ERnmLineSeparateType::Combine);
    //Rnm->Generate();

    // 道路標示を生成
    TObjectPtr<URoadMarkingGenerator> Rm = NewObject<URoadMarkingGenerator>();
    Rm->Initialize(Target, CrosswalkFrequency);
    Rm->Generate();
}

USceneComponent* URoadReproducer::GenerateDstParent()
{
    UWorld* World = GEngine->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Find existing ReproducedRoad actor
    AActor* ExistingActor = nullptr;
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName(TEXT("ReproducedRoad")), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        ExistingActor = FoundActors[0];
    }

    // If not found, create new one
    if (!ExistingActor)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(TEXT("ReproducedRoad"));
        ExistingActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        ExistingActor->Tags.Add(FName(TEXT("ReproducedRoad")));
    }

    return ExistingActor->GetRootComponent();
}
