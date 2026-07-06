// Copyright © 2026 Gazda-dev. All Rights Reserved.

#include "Public/Core/MPTestTaskGameMode.h"

#include "Public/Core/MPTestTaskGameState.h"
#include "Public/Core/MPTestTaskPlayerController.h"
#include "Public/Core/MPTestTaskPlayerState.h"

AMPTestTaskGameMode::AMPTestTaskGameMode()
{
	GameStateClass = AMPTestTaskGameState::StaticClass();
	PlayerStateClass = AMPTestTaskPlayerState::StaticClass();
	PlayerControllerClass = AMPTestTaskPlayerController::StaticClass();
}
