// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MPTestTaskPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class MPTESTTASK_API AMPTestTaskPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void SetupInputComponent() override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetReady(bool bReady);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartMatch();	
	
	UFUNCTION(Server, Reliable)
	void Server_ReturnToLobby();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Match")
	void RequestSetReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void RequestStartMatch();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void RequestReturnToLobby();
	
protected:
	// Input mapping contexts
	UPROPERTY(EditDefaultsOnly, Category = "MPTask|Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultInputMappingContexts;

	// Priority for input mapping contexts 
	// TODO prolly should be per context but for now minor (or just put all IAs in one IMC)
	UPROPERTY(EditDefaultsOnly, Category = "MPTask|Input")
	int32 DefaultContextsPriority = 0;
	
	UPROPERTY(EditDefaultsOnly, Category = "MPTask")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
private:
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;
};
