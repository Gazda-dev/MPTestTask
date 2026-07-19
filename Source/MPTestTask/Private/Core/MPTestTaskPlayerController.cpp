// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Public/Core/MPTestTaskPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Core/MPTestTaskGameMode.h"
#include "Core/MPTestTaskGameState.h"
#include "Core/MPTestTaskPlayerState.h"
#include "Engine/LocalPlayer.h"

void AMPTestTaskPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (!IsLocalPlayerController())
	{
		return;
	}
	
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		for (const UInputMappingContext* Context : DefaultInputMappingContexts)
		{
			Subsystem->AddMappingContext(Context, DefaultContextsPriority);
		}
	}
	
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void AMPTestTaskPlayerController::Server_ReturnToLobby_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	if (AMPTestTaskGameMode* GM = World->GetAuthGameMode<AMPTestTaskGameMode>())
	{
		GM->ReturnToLobby();
	}
}

void AMPTestTaskPlayerController::RequestSetReady(bool bReady)
{
	Server_SetReady(bReady);
}

void AMPTestTaskPlayerController::RequestStartMatch()
{
	Server_StartMatch();
}

void AMPTestTaskPlayerController::RequestReturnToLobby()
{
	Server_ReturnToLobby();
}

void AMPTestTaskPlayerController::Server_StartMatch_Implementation()
{
	// working on lan only because host is a player & server
	if (!IsLocalController())
	{
		return;
	}
	
	const UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	
	if (AMPTestTaskGameMode* GameMode = World->GetAuthGameMode<AMPTestTaskGameMode>())
	{
		const AMPTestTaskPlayerState* PS = GetPlayerState<AMPTestTaskPlayerState>();
		UE_LOG(LogMatch, Display, TEXT("host %s requested match start")
			, PS ? *PS->GetPlayerName() : TEXT(""));
		
		GameMode->RequestStartMatch();
	}
}

bool AMPTestTaskPlayerController::Server_StartMatch_Validate()
{
	return true;
}

void AMPTestTaskPlayerController::Server_SetReady_Implementation(bool bReady)
{
	if (AMPTestTaskPlayerState* PS = GetPlayerState<AMPTestTaskPlayerState>())
	{
		UE_LOG(LogMatch, Display, TEXT("%s is %s")
			, *PS->GetPlayerName()
			, bReady ? TEXT("ready") : TEXT("not ready"));

		PS->SetReady(bReady);
	}
}

bool AMPTestTaskPlayerController::Server_SetReady_Validate(bool bReady)
{
	return true;
}
