// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "MPTestTaskPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStateDataChanged);

UCLASS()
class MPTESTTASK_API AMPTestTaskPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetReady(bool bInReady);
	void SetAlive(bool bInAlive);
	void AddDamageDealt(float Amount);
	void ResetForNewRound();
	
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	FORCEINLINE bool IsReady() const
	{
		return bIsReady;
	}
	
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	FORCEINLINE bool IsAlive() const
	{
		return bIsAlive;
	}
	
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	FORCEINLINE float GetDamageDealt() const
	{
		return DamageDealt;
	}
	
protected:
	UFUNCTION()
	void OnRep_Data();
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateDataChanged OnPlayerStateDataChanged;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_Data, BlueprintReadOnly, Category = "PlayerState")
	bool bIsReady = false;
	
	UPROPERTY(ReplicatedUsing = OnRep_Data, BlueprintReadOnly, Category = "PlayerState")
	bool bIsAlive = true;
	
	// damage dealt == score
	UPROPERTY(ReplicatedUsing = OnRep_Data, BlueprintReadOnly, Category = "PlayerState")
	float DamageDealt = 0.f;
};
