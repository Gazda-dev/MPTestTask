// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"

#include "MPTestTaskDevSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "MPTestTask Settings"))
class MPTESTTASK_API UMPTestTaskDevSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UMPTestTaskDevSettings();
	
	UPROPERTY(Config, EditAnywhere, Category = "Maps")
	TSoftObjectPtr<UWorld> MainMenuMap;
	
	UPROPERTY(Config, EditAnywhere, Category = "Maps")
	TSoftObjectPtr<UWorld> ArenaMap;
};
