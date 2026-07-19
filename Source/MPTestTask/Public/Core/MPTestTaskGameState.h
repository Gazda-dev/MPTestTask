// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "MPTestTaskGameState.generated.h"

class AMPTestTaskPlayerState;

DECLARE_LOG_CATEGORY_EXTERN(LogMatch, Display, All);

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingForPlayers,
	InProgress,
	PostMatch
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStateChanged);

UCLASS()
class MPTESTTASK_API AMPTestTaskGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetMatchState(EMatchState NewState);
	void SetWinner(AMPTestTaskPlayerState* InWinner);
	
	UFUNCTION(BlueprintPure, Category = "Match")
	EMatchState GetMatchState() const
	{
		return MatchState;
	}
	
	UFUNCTION(BlueprintPure, Category = "Match")
	AMPTestTaskPlayerState* GetWinner() const
	{
		return Winner;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Match")
	TArray<AMPTestTaskPlayerState*> GetPlayerStatesSortedByScore() const;
	
protected:
	UFUNCTION()
	void OnRep_MatchState();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnMatchStateChanged OnMatchStateChanged;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "Match")
	EMatchState MatchState = EMatchState::WaitingForPlayers;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "Match")
	TObjectPtr<AMPTestTaskPlayerState> Winner = nullptr;
};
