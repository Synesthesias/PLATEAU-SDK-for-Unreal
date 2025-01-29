// // Fill out your copyright notice in the Description page of Project Settings.

// #include "RoadAdjustment/MarkedWayListComposer.h"
// #include "RoadAdjustment/MarkedWayList.h"
// #include "RoadAdjustment/MCLaneLine.h"
// #include "RoadAdjustment/MCShoulderLine.h"
// #include "RoadAdjustment/MCCenterLine.h"
// #include "RoadAdjustment/MCIntersection.h"

// MarkedWayList* UMarkedWayListComposer::ComposeFrom(IRrTarget* Target)
// {
//     // Array of composers for different line types
//     TArray<TScriptInterface<IIMarkedWayListComposer>> Composers;
//     Composers.Add(NewObject<UMCLaneLine>());      // Lines between lanes except center line
//     Composers.Add(NewObject<UMCShoulderLine>());  // Lines between sidewalk and road
//     Composers.Add(NewObject<UMCCenterLine>());    // Center line
//     Composers.Add(NewObject<UMCIntersection>()); // Intersection lines

//     MarkedWayList* Result = new MarkedWayList();
    
//     for (const auto& Composer : Composers)
//     {
//         if (Composer)
//         {
//             MarkedWayList* ComposedList = IIMarkedWayListComposer::Execute_ComposeFrom(Composer.GetObject(), Target);
//             if (ComposedList)
//             {
//                 ComposedList->Translate(FVector(0.0f, 0.0f, HeightOffset));
//                 Result->AddRange(ComposedList);
//             }
//         }
//     }

//     return Result;
// }
