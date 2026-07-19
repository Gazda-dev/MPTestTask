// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Public/Core/MPTestTaskGameMode.h"

#include "Engine/World.h"
#include "Public/Core/MPTestTaskGameState.h"
#include "Public/Core/MPTestTaskPlayerController.h"
#include "Public/Core/MPTestTaskPlayerState.h"

AMPTestTaskGameMode::AMPTestTaskGameMode()
{
	GameStateClass = AMPTestTaskGameState::StaticClass();
	PlayerStateClass = AMPTestTaskPlayerState::StaticClass();
	PlayerControllerClass = AMPTestTaskPlayerController::StaticClass();
}

void AMPTestTaskGameMode::NotifyPlayerDied(AController* Victim, AController* Killer)
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	if (GS->GetMatchState() != EMatchState::InProgress)
	{
		return;
	}
	
	if (Victim)
	{
		if (AMPTestTaskPlayerState* PS = Victim->GetPlayerState<AMPTestTaskPlayerState>())
		{
			PS->SetAlive(false);
		}
		
		if (APawn* Pawn = Victim->GetPawn())
		{
			Pawn->DisableInput(Cast<APlayerController>(Victim));
			Pawn->SetLifeSpan(DeathLifeSpan);
		}
	}
	
	AMPTestTaskPlayerState* LastAlive = nullptr;
	const int32 Alive = CountAlivePlayers(LastAlive);
	if (Alive <= 1)
	{
		EndMatch(LastAlive);
	}
}

void AMPTestTaskGameMode::RequestStartMatch()
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	const EMatchState Phase = GS->GetMatchState();
	const bool bCanStart = Phase == EMatchState::WaitingForPlayers || Phase == EMatchState::PostMatch;
	
	if (bCanStart && AreAllPlayersReady())
	{
		StartMatch();
	}
}

void AMPTestTaskGameMode::ReturnToLobby()
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	if (GS->GetMatchState() != EMatchState::PostMatch)
	{
		return;
	}
	
	ResetAndRespawnAllPlayers();
	
	GS->SetMatchState(EMatchState::WaitingForPlayers);
}

void AMPTestTaskGameMode::ResetAndRespawnAllPlayers()
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	GS->SetWinner(nullptr);
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	// respawn all players
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AController* Controller = It->Get();
		if (!IsValid(Controller))
		{
			continue;
		}
		
		if (AMPTestTaskPlayerState* PlayerState = Controller->GetPlayerState<AMPTestTaskPlayerState>())
		{
			PlayerState->ResetForNewRound();
		}
		
		RestartPlayer(Controller);
	}
}

void AMPTestTaskGameMode::StartMatch()
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	ResetAndRespawnAllPlayers();
	
	UE_LOG(LogMatch, Display, TEXT("Match started"));
	
	GS->SetMatchState(EMatchState::InProgress);
}

void AMPTestTaskGameMode::EndMatch(AMPTestTaskPlayerState* Winner)
{
	AMPTestTaskGameState* GS = GetGameState<AMPTestTaskGameState>();
	if (!IsValid(GS))
	{
		return;
	}
	
	GS->SetWinner(Winner);
	GS->SetMatchState(EMatchState::PostMatch);
	
	UE_LOG(LogMatch, Display, TEXT("Match ended. Winner %s"), *Winner->GetPlayerName())
}

bool AMPTestTaskGameMode::AreAllPlayersReady() const
{
	int32 Total = 0;
	int32 Ready = 0;
	
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const AController* Controller = It->Get();
		if (!IsValid(Controller))
		{
			continue;
		}
		
		if (const AMPTestTaskPlayerState* PlayerState = Controller->GetPlayerState<AMPTestTaskPlayerState>())
		{
			++Total;
			if (PlayerState->IsReady())
			{
				++Ready;
			}
		}
	}
	
	const bool bAllPlayersReady = Total >= MinPlayersToStart && Ready == Total;
	
	UE_LOG(LogMatch, Display, TEXT("%s")
		, bAllPlayersReady ? TEXT("All Players Ready") : TEXT("Not all players ready"));
	
	return bAllPlayersReady;
}

int32 AMPTestTaskGameMode::CountAlivePlayers(AMPTestTaskPlayerState*& OutLastAlive) const
{
	OutLastAlive = nullptr;
	int32 Alive = 0;
	
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return -1;
	}
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const AController* Controller = It->Get();
		if (!IsValid(Controller))
		{
			continue;
		}
		
		if (AMPTestTaskPlayerState* PlayerState = Controller->GetPlayerState<AMPTestTaskPlayerState>())
		{
			if (PlayerState->IsAlive())
			{
				++Alive;
				OutLastAlive = PlayerState;
			}
		}
	}
	
	return Alive;
}
