// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Gameplay/Actors/Pickables/HealthPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Scene.h"
#include "Gameplay/Components/HealthComponent.h"

AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = RootSceneComponent;
	
	CollisionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComponent"));
	CollisionSphereComponent->SetupAttachment(RootSceneComponent);
	CollisionSphereComponent->InitSphereRadius(50.f);
	CollisionSphereComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
}

bool AHealthPickup::CanInteract(AActor* Interactor) const
{
	if (!IsValid(Interactor) || bConsumed)
	{
		return false;
	}
	
	if (const UHealthComponent* HealthComponent = Interactor->FindComponentByClass<UHealthComponent>())
	{
		return HealthComponent->GetHealth() < HealthComponent->GetMaxHealth();
	}
	
	return false;
}

void AHealthPickup::Interact(AActor* Interactor)
{
	if (!IsValid(Interactor) || bConsumed)
	{
		return;
	}
	//TODO delete but maybe some proper logs on cvar...
	UE_LOG(LogTemp, Warning, TEXT("healthpickup interact"));
	
	if (UHealthComponent* HealthComponent = Interactor->FindComponentByClass<UHealthComponent>())
	{
		HealthComponent->Heal(HealAmount);
		bConsumed = true;
		
		Destroy();
	}
}

FText AHealthPickup::GetInteractionPrompt() const
{
	return InteractionPrompt;
}

