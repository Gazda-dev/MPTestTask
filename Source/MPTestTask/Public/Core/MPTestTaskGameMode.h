// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MPTestTaskGameMode.generated.h"

class AMPTestTaskPlayerState;

UCLASS(abstract)
class AMPTestTaskGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMPTestTaskGameMode();
	
	void NotifyPlayerDied(AController* Victim, AController* Killer);
	void RequestStartMatch();
	void ReturnToLobby();
	void ResetAndRespawnAllPlayers();
	
private:
	void StartMatch();
	void EndMatch(AMPTestTaskPlayerState* Winner);
	bool AreAllPlayersReady() const;
	int32 CountAlivePlayers(AMPTestTaskPlayerState*& OutLastAlive) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "2"))
	int32 MinPlayersToStart = 2;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match", meta = (ClampMin = "2"))
	float DeathLifeSpan = 5.f;
};



