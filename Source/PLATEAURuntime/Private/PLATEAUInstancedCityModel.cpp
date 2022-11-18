// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUInstancedCityModel.h"

// Sets default values
APLATEAUInstancedCityModel::APLATEAUInstancedCityModel() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

FPLATEAUCityObjectInfo APLATEAUInstancedCityModel::GetCityObjectInfo(USceneComponent* Component) {
    FPLATEAUCityObjectInfo Result;
    Result.DatasetName = DatasetName;

    if (Component == nullptr)
        return Result;

    auto ID = Component->GetName();
    {
        int Index = 0;
        if (ID.FindLastChar('_', Index)) {
            if (ID.RightChop(Index + 1).IsNumeric()) {
                ID = ID.LeftChop(ID.Len() - Index);
            }
        }
    }
    Result.ID = ID;

    auto GmlComponent = Component;
    while (GmlComponent->GetAttachParent() != RootComponent) {
        GmlComponent = GmlComponent->GetAttachParent();

        // TODO: エラーハンドリング
        if (GmlComponent == nullptr)
            return Result;
    }

    Result.GmlName = GmlComponent->GetName();

    {
        int Index = 0;
        if (Result.GmlName.FindLastChar('_', Index)) {
            if (Result.GmlName.RightChop(Index + 1).IsNumeric()) {
                Result.GmlName = Result.GmlName.LeftChop(Result.GmlName.Len() - Index);
            }
        }
    }
    Result.GmlName += TEXT(".gml");

    return Result;
}

// Called when the game starts or when spawned
void APLATEAUInstancedCityModel::BeginPlay() {
    Super::BeginPlay();

}

// Called every frame
void APLATEAUInstancedCityModel::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

}

