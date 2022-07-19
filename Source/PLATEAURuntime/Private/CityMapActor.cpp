// Fill out your copyright notice in the Description page of Project Settings.


#include "CityMapActor.h"

// Sets default values
ACityMapActor::ACityMapActor() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACityMapActor::BeginPlay() {
    Super::BeginPlay();

}

// Called every frame
void ACityMapActor::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

}

void ACityMapActor::PlaceCityModels() {
    check(Metadata != nullptr);
    USceneComponent* ActorRootComponent = NewObject<USceneComponent>(this,
        USceneComponent::GetDefaultSceneRootVariableName());

    check(ActorRootComponent != nullptr);
    ActorRootComponent->Mobility = EComponentMobility::Static;
    ActorRootComponent->bVisualizeComponent = true;
    SetRootComponent(ActorRootComponent);
    AddInstanceComponent(ActorRootComponent);
    ActorRootComponent->RegisterComponent();
    SetFlags(RF_Transactional);
    ActorRootComponent->SetFlags(RF_Transactional);

    for (const auto entry : Metadata->StaticMeshes) {
        for (const auto staticMesh : entry.Value.Value) {
            auto component = NewObject<UStaticMeshComponent>(this, NAME_None);
            component->SetStaticMesh(staticMesh);
            component->DepthPriorityGroup = SDPG_World;
            // TODO: SetStaticMeshComponentOverrideMaterial(StaticMeshComponent, NodeInfo);
            FString NewUniqueName = staticMesh->GetName();
            if (!component->Rename(*NewUniqueName, nullptr, REN_Test)) {
                NewUniqueName = MakeUniqueObjectName(this, USceneComponent::StaticClass(), FName(staticMesh->GetName())).ToString();
            }
            component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);
            AddInstanceComponent(component);
            component->RegisterComponent();
            component->AttachToComponent(ActorRootComponent, FAttachmentTransformRules::KeepWorldTransform);
            component->PostEditChange();
        }
    }
    GEngine->BroadcastLevelActorListChanged();
}
