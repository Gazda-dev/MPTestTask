// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/IConsoleManager.h"

#include "WeaponComponent.generated.h"

class USoundCue;
class UNiagaraSystem;
class UDamageType;
struct FVector_NetQuantize;

//TODO default false
static TAutoConsoleVariable<bool> CVarDrawDebugWeapon(TEXT("MPTT.debug.weapon")
	, true
	, TEXT("draw weapon's debug info"));

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MPTESTTASK_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire(const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireFX();
	
private:
	void ServerFireInternal(const FVector& TraceStart, const FVector& TraceEnd);
	bool ComputeAim(FVector& OutStart, FVector& OutEnd);
	void PlayFireEffects();
	
public:
	UFUNCTION()
	void TryFire();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 1.f))
	float MaxRange = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 1.f))
	float Damage = 30.f;	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0.05f))
	float FireInterval = 0.25f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	TObjectPtr<UNiagaraSystem> MuzzleFlash;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	TObjectPtr<USoundCue> FireSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|FX")
	FName MuzzleSocketName = FName(TEXT("Muzzle"));
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<ECollisionChannel> WeaponTraceChannel = ECC_GameTraceChannel1;
	
private:
	float LastFireServerTime = -1000.f;
};
