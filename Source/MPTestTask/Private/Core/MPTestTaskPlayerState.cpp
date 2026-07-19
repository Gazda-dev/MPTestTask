// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Public/Core/MPTestTaskPlayerState.h"

#include "Core/MPTestTaskGameState.h"
#include "Net/UnrealNetwork.h"

void AMPTestTaskPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMPTestTaskPlayerState, bIsReady);
	DOREPLIFETIME(AMPTestTaskPlayerState, bIsAlive);
	DOREPLIFETIME(AMPTestTaskPlayerState, DamageDealt);
}

void AMPTestTaskPlayerState::SetReady(bool bInReady)
{
	if (!HasAuthority() || bInReady == bIsReady)
	{
		return;
	}
	
	bIsReady = bInReady;
	
	OnRep_Data();
}

void AMPTestTaskPlayerState::SetAlive(bool bInAlive)
{
	if (!HasAuthority() || bInAlive == bIsAlive)
	{
		return;
	}
	
	UE_LOG(LogMatch, Display, TEXT("%s is %s")
		, *GetPlayerName()
		, bInAlive ? TEXT("alive") : TEXT("dead"));
	
	bIsAlive = bInAlive;
	
	OnRep_Data();
}

void AMPTestTaskPlayerState::AddDamageDealt(float Amount)
{
	if (!HasAuthority() || Amount <= 0.f)
	{
		return;
	}
	
	DamageDealt += Amount;
	
	OnRep_Data();
}

void AMPTestTaskPlayerState::ResetForNewRound()
{
	if (!HasAuthority())
	{
		return;
	}
	
	DamageDealt = 0.f;
	bIsReady = false;
	bIsAlive = true;
	
	OnRep_Data();
}

void AMPTestTaskPlayerState::OnRep_Data()
{
	OnPlayerStateDataChanged.Broadcast();
}
