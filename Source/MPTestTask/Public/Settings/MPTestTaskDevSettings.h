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
	
	// after how many seconds joining to server will be aborted
	UPROPERTY(Config, EditAnywhere, Category = "Online")
	float JoinTimeoutSeconds = 10.f;
	
	UPROPERTY(Config, EditAnywhere, Category = "Online")
	bool bUseEOS = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "Online|EOS")
	FString DevAuthCredentialName = TEXT("Player1");
};
