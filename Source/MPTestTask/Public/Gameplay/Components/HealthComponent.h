// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "HealthComponent.generated.h"

class AController;
class UDamageType;

DECLARE_LOG_CATEGORY_EXTERN(LogHealth, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnHealthChanged
	, UHealthComponent*, HealthComponent
	, float, NewHealth
	, float, Delta
	, AController*, InstigatedBy
	, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeath
	, UHealthComponent*, HealthComponent
	, AController*, InstigatedBy
	, AActor*, DamageCauser);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamagedServer
	, float /*Amount*/
	, AController* /*Instigator*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MPTESTTASK_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_Health(float OldHealth);
	
	UFUNCTION()
	void OnRep_IsDead();
	
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor
		, float Damage
		, const UDamageType* DamageType
		, AController* InstigatedBy
		, AActor* DamageCauser);
	
public:
	UFUNCTION(BlueprintCallable, Category = "MP|Health")
	void ResetHealth();
	
	UFUNCTION(BlueprintCallable, Category = "MP|Health")
	void Heal(float Amount);
	
	UFUNCTION(BlueprintPure, Category = "MP|Health")
	float GetHealth() const
	{
		return Health;
	}
	
	UFUNCTION(BlueprintPure, Category = "MP|Health")
	float GetMaxHealth() const
	{
		return MaxHealth;
	}
	
	UFUNCTION(BlueprintPure, Category = "MP|Health")
	float GetHealthPercent() const;
	
	UFUNCTION(BlueprintPure, Category = "MP|Health")
	bool IsDead() const
	{
		return bIsDead;
	}
	
	UPROPERTY(BlueprintAssignable, Category = "MP|Health")
	FOnHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "MP|Health")
	FOnDeath OnDeath;

	FOnDamagedServer OnDamagedServer;
	
private:
	void ApplyHealthChange(float Delta, AController* InstigatedBy, AActor* DamageCauser);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MP|Health", meta = (ClampMin = 1.f))
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "MP|Health")
	float Health = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "MP|Health")
	bool bIsDead = false;
};
