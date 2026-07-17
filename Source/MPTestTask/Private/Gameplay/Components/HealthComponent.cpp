// Copyright © 2026 Gazda-dev. All Rights Reserved.


#include "Public/Gameplay/Components/HealthComponent.h"

#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogHealth);

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}
	
	Health = MaxHealth;
	Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnTakeAnyDamage);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, bIsDead);
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	const float Delta = Health - OldHealth;
	OnHealthChanged.Broadcast(this
		, Health
		, Delta
		, nullptr
		, nullptr);
}

void UHealthComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast(this, nullptr, nullptr);
	}
}

void UHealthComponent::OnTakeAnyDamage(AActor* DamagedActor
	, float Damage
	, const UDamageType* DamageType
	, AController* InstigatedBy
	, AActor* DamageCauser)
{
	if (Damage <= 0.f || bIsDead)
	{
		return;
	}
	
	ApplyHealthChange(-Damage, InstigatedBy, DamageCauser);
}

void UHealthComponent::ResetHealth()
{
	// server only
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}
	
	const float OldHealth = Health;
	Health = MaxHealth;
	bIsDead = false;
	
	OnRep_Health(OldHealth);
	OnRep_IsDead();
}

void UHealthComponent::Heal(float Amount)
{
	// only server can heal
	if (const AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}
	
	if (bIsDead || Amount <= 0.f)
	{
		return;
	}
	
	ApplyHealthChange(+Amount, nullptr, nullptr);
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.f ? Health / MaxHealth : 0.f;
}

void UHealthComponent::ApplyHealthChange(float Delta, AController* InstigatedBy, AActor* DamageCauser)
{
	const float OldHealth = Health;
	
	Health = FMath::Clamp(Health + Delta, 0.f, MaxHealth);
	
	if (Health == OldHealth)
	{
		return;
	}
	
	UE_LOG(LogHealth, Display, TEXT("Health changed %s | NewHealth: %f")
		, GetOwner() ? *GetOwner()->GetName() : TEXT("null")
		, Health);
	
	// because it not run on server automatically
	OnRep_Health(OldHealth);
	
	if (Health <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnRep_IsDead();
		OnDeath.Broadcast(this, InstigatedBy, DamageCauser);
	}
}

