// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HAL/IConsoleManager.h"

#include "InteractionComponent.generated.h"

static TAutoConsoleVariable<bool> CVarDrawDebugInteraction(TEXT("MPTT.debug.interaction")
	, false
	, TEXT("draw interaction debug info"));

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, NewFocus);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MPTESTTASK_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void TryInteract();
	
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(AActor* Target);
	
private:
	void DetectFocus();
	bool ComputerAim(FVector& OutStart, FVector& OutEnd);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusChanged OnFocusChanged;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "10"))
	float InteractionRange = 250.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "0.05"))
	float DetectionInterval = 0.15f;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> FocusedActor = nullptr;
	
	FTimerHandle DetectTimerHandle;
};
