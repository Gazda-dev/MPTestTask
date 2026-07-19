// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Public/Core/MPTestTaskGameState.h"

#include "Core/MPTestTaskPlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogMatch);

void AMPTestTaskGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMPTestTaskGameState, MatchState);
	DOREPLIFETIME(AMPTestTaskGameState, Winner);
}

void AMPTestTaskGameState::SetMatchState(EMatchState NewState)
{
	if (!HasAuthority() || MatchState == NewState)
	{
		return;
	}
	
	MatchState = NewState;
	
	OnRep_MatchState();
}

void AMPTestTaskGameState::SetWinner(AMPTestTaskPlayerState* InWinner)
{
	if (!HasAuthority())
	{
		return;
	}
	
	UE_LOG(LogMatch, Display, TEXT("Set new winner %s")
		, InWinner ? *InWinner->GetName() : TEXT(""));
	
	Winner = InWinner;
}

TArray<AMPTestTaskPlayerState*> AMPTestTaskGameState::GetPlayerStatesSortedByScore() const
{
	TArray<AMPTestTaskPlayerState*> Result;
	
	for (APlayerState* PS : PlayerArray)
	{
		if (AMPTestTaskPlayerState* TaskPS = Cast<AMPTestTaskPlayerState>(PS))
		{
			Result.Add(TaskPS);
		}
	}
	
	Result.Sort([](const AMPTestTaskPlayerState& A, const AMPTestTaskPlayerState& B)
	{
		return A.GetDamageDealt() > B.GetDamageDealt();
	});
	
	return Result;
}

void AMPTestTaskGameState::OnRep_MatchState()
{
	OnMatchStateChanged.Broadcast();
}
