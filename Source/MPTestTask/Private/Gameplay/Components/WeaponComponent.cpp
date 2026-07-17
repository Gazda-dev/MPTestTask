// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Gameplay/Components/WeaponComponent.h"

#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponComponent::ServerFireInternal(const FVector& TraceStart, const FVector& TraceEnd)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerPawn);
	CollisionParams.bTraceComplex = false;
	
	const bool bHit = World->LineTraceSingleByChannel(HitResult
		, TraceStart
		, TraceEnd
		, WeaponTraceChannel
		, CollisionParams);
	
	if (bHit && HitResult.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(HitResult.GetActor()
			, Damage
			, (TraceEnd - TraceStart).GetSafeNormal()
			, HitResult
			, OwnerPawn->GetController()
			, OwnerPawn
			, DamageType);
	}
	
	if (CVarDrawDebugWeapon.GetValueOnGameThread())
	{
		DrawDebugLine(World
			, TraceStart
			, TraceEnd
			, bHit ? FColor::Green : FColor::Red
			, false
			, 10.f
			, 0
			, 1.f);
	}
}

bool UWeaponComponent::ComputeAim(FVector& OutStart, FVector& OutEnd)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return false;
	}
	
	// TODO it should be set up with proper aim anims and trace from crosshair but maybe later, now simple
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerPawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	
	OutStart = ViewLocation;
	OutEnd = ViewLocation + ViewRotation.Vector() * MaxRange;
	
	return true;
}

void UWeaponComponent::PlayFireEffects()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}
	
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!IsValid(Mesh))
	{
		return;
	}
	
	if (!MuzzleFlash)
	{
		//TODO report error with proper log
	}
	else
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash
			, Mesh
			, MuzzleSocketName
			, FVector::ZeroVector
			, FRotator::ZeroRotator
			, EAttachLocation::SnapToTarget
			, true);
	}
	
	if (!FireSound)
	{
		//TODO report error with proper log
		return;
	}
	else
	{
		UGameplayStatics::SpawnSoundAttached(FireSound
			, Mesh
			, MuzzleSocketName);
	}
}

void UWeaponComponent::TryFire()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	const float Now = World->GetTimeSeconds();
	if (Now - LastFireLocalTime < FireInterval)
	{
		return;
	}
	LastFireLocalTime = Now;
	
	FVector Start;
	FVector End;
	if (!ComputeAim(Start, End))
	{
		return;
	}
	
	if (OwnerPawn->IsLocallyControlled())
	{
		PlayFireEffects();
	}
	
	Server_Fire(Start, End);
}

void UWeaponComponent::Multicast_PlayFireFX_Implementation()
{
	//shooter already played fxs
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()); OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		return;
	}
	
	PlayFireEffects();
}

void UWeaponComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceStart,
                                                  const FVector_NetQuantize& TraceEnd)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	const float Now = World->GetTimeSeconds();
	if (Now - LastFireServerTime < FireInterval)
	{
		return;
	}
	LastFireServerTime = Now;
	
	// check whether shoot start is kinda close from player (some basic anty cheat)
	const float DistSquared = FVector::DistSquared(OwnerPawn->GetActorLocation(), TraceStart);
	constexpr float MaxAllowedSquare = FMath::Square(300.f);
	if (DistSquared > MaxAllowedSquare)
	{
		return;
	}
	
	ServerFireInternal(TraceStart, TraceEnd);
	Multicast_PlayFireFX();
}

bool UWeaponComponent::Server_Fire_Validate(const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd)
{
	return !TraceStart.ContainsNaN() && !TraceEnd.ContainsNaN();
}

