
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/RoadMarkingGenerator.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"

#include "Misc/ScopedSlowTask.h"

URoadMarkingGenerator::URoadMarkingGenerator()
{
    TargetBeforeCopy = nullptr;
    CrosswalkFrequency = ECrosswalkFrequency::BigRoad;
}

void URoadMarkingGenerator::Initialize(IIRrTarget* Target, ECrosswalkFrequency CrosswalkFreq)
{
    if (Target != nullptr)
    {
        TargetBeforeCopy = Target;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("target road network is null."));
    }

    CrosswalkFrequency = CrosswalkFreq;
}

void URoadMarkingGenerator::Generate()
{
    if (TargetBeforeCopy == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Road network not found."));
        return;
    }

    // Create progress dialog equivalent
    FScopedSlowTask Progress(100.f, NSLOCTEXT("RoadMarkingGenerator", "GeneratingRoadMarkings", "Generating Road Markings"));
    Progress.MakeDialog();

    Progress.EnterProgressFrame(5.f, NSLOCTEXT("RoadMarkingGenerator", "CopyingRoadNetwork", "Copying Road Network"));
    
    //IRrTarget* Target = TargetBeforeCopy->Copy();

    Progress.EnterProgressFrame(5.f, NSLOCTEXT("RoadMarkingGenerator", "SmoothingRoadNetwork", "Smoothing Road Network"));
    
    //// Implement road network smoothing
    //URoadNetworkLineSmoother::Smooth(Target);

    //USceneComponent* DstParent = URoadReproducer::GenerateDstParent();

    //TArray<URoadBase*> RoadBases = Target->GetRoadBases();
    //TArray<RoadReproduceSource> RoadSources;
    //
    //// Convert road bases to sources
    //for (URoadBase* RoadBase : RoadBases)
    //{
    //    RoadSources.Add(RoadReproduceSource(RoadBase));
    //}

    //// Generate road markings for each road
    //for (int32 i = 0; i < RoadSources.Num(); i++)
    //{
    //    const RoadReproduceSource& RoadSource = RoadSources[i];
    //    FString RoadName = RoadSource.GetName();

    //    float ProgressPercent = (float)i / RoadSources.Num() * 70.f;
    //    Progress.EnterProgressFrame(ProgressPercent, FText::Format(NSLOCTEXT("RoadMarkingGenerator", "ProcessingRoad", "Processing Road {0}/{1}: {2}"),
    //        FText::AsNumber(i + 1), FText::AsNumber(RoadSources.Num()), FText::FromString(RoadName)));

    //    // Continue implementation...
    //}

    // Generate crosswalks
    Progress.EnterProgressFrame(20.f, NSLOCTEXT("RoadMarkingGenerator", "GeneratingCrosswalks", "Generating Crosswalks"));
    
    // Implement crosswalk generation...
}

TArray<TSharedPtr<RoadMarkingInstance>> URoadMarkingGenerator::GenerateRoadLines(class IIRrTarget* InnerTarget)
{
    TArray<TSharedPtr<RoadMarkingInstance>> Results;
    
    // Implement road line generation...
    
    return Results;
}

//void URoadMarkingGenerator::GenerateGameObj(UStaticMesh* Mesh,
//    const TArray<UMaterialInterface*>& Materials,
//    USceneComponent* DstParent,
//    const RoadReproduceSource& SrcRoad,
//    EReproducedRoadType ReproducedType,
//    EReproducedRoadDirection Direction)
//{
//    FString TargetName = SrcRoad.GetName();
//    FString DstName = FString::Printf(TEXT("%s-%s"), *GetReproducedRoadTypeName(ReproducedType), *TargetName);
//    
//    if (Direction != EReproducedRoadDirection::None)
//    {
//        DstName += FString::Printf(TEXT("-%s"), *GetReproducedRoadDirectionName(Direction));
//    }
//
//    // Implementation for game object generation...
//}
