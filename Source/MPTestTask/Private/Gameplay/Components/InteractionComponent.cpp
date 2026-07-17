// Copyright © 2026 Gazda-dev. All Rights Reserved.


#include "Gameplay/Components/InteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/Interfaces/InteractableInterface.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()); 
		!IsValid(OwnerPawn) || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	World->GetTimerManager().SetTimer(DetectTimerHandle
		, this
		, &UInteractionComponent::DetectFocus
		, DetectionInterval
		, true);
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetectTimerHandle);
		DetectTimerHandle.Invalidate();
	}
}

void UInteractionComponent::TryInteract()
{
	if (FocusedActor)
	{
		Server_Interact(FocusedActor);
	}
}

void UInteractionComponent::DetectFocus()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	FVector StartLocation;
	FVector EndLocation;
	if (!ComputerAim(StartLocation, EndLocation))
	{
		return;
	}
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	
	const bool bHit = World->LineTraceSingleByChannel(HitResult
		, StartLocation
		, EndLocation
		, ECC_Visibility
		, CollisionParams);
	
	if (CVarDrawDebugInteraction.GetValueOnGameThread())
	{
		DrawDebugLine(World
				, StartLocation
				, EndLocation
				, bHit ? FColor::Green : FColor::Red
				, false
				, 10.f
				, 0
				, 1.f);
	}
	
	AActor* NewFocusActor = nullptr;
	if (bHit && HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractableInterface>())
	{
		NewFocusActor = HitResult.GetActor();

		if (CVarDrawDebugInteraction.GetValueOnGameThread())
		{
			GEngine->AddOnScreenDebugMessage(0
			, 5.f
			, FColor::Emerald
			, FString::Printf(TEXT("interact focus actor found: %s")
				, *NewFocusActor->GetName()));
		}
	}
	
	if (NewFocusActor != FocusedActor)
	{
		FocusedActor = NewFocusActor;
		OnFocusChanged.Broadcast(FocusedActor);
	}
}

bool UInteractionComponent::ComputerAim(FVector& OutStart, FVector& OutEnd)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn) || !OwnerPawn->IsLocallyControlled())
	{
		return false;
	}
	
	FVector Location;
	FRotator Rotation;
	OwnerPawn->GetActorEyesViewPoint(Location, Rotation);
	
	OutStart = Location;
	OutEnd = Location + Rotation.Vector() * InteractionRange;
	
	return true;
}

void UInteractionComponent::Server_Interact_Implementation(AActor* Target)
{
	if (!IsValid(Target))
	{
		return;
	}
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}
	
	IInteractableInterface* Interactable = Cast<IInteractableInterface>(Target);
	if (!Interactable)
	{
		return;
	}
	
	// distance check
	const float DistSquared = FVector::DistSquared(OwnerPawn->GetActorLocation(), Target->GetActorLocation());
	if (DistSquared > FMath::Square(InteractionRange + 100.f))
	{
		return;
	}
	
	if (!Interactable->CanInteract(OwnerPawn))
	{
		return;
	}
	
	Interactable->Interact(OwnerPawn);
}

bool UInteractionComponent::Server_Interact_Validate(AActor* Target)
{
	return Target && Target->Implements<UInteractableInterface>(); 
}


