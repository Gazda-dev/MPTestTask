// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Gameplay/Actors/Pickables/HealthPickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Scene.h"
#include "Gameplay/Components/HealthComponent.h"
#include "UI/InteractionPromptWidget.h"

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
	
	InteractionPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidget"));
	InteractionPromptWidget->SetupAttachment(RootSceneComponent);
	InteractionPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPromptWidget->SetDrawSize(FVector2D(200.f, 40.f));
	InteractionPromptWidget->SetVisibility(false);
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (UInteractionPromptWidget* PromptWidget 
		= Cast<UInteractionPromptWidget>(InteractionPromptWidget->GetUserWidgetObject()))
	{
		PromptWidget->SetLabel(InteractionPrompt);
	}
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


void AHealthPickup::SetInteractionPromptVisible(bool bVisible)
{
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(bVisible);
	}
}
