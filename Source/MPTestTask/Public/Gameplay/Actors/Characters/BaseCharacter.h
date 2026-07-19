// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "BaseCharacter.generated.h"

/*
 *	A bit copy-paste from template character but lets not overcomplicate it
 */

class USoundCue;
class UInteractionComponent;
class UWeaponComponent;
class UInputAction;
class UHealthComponent;
class UCameraComponent;
class USpringArmComponent;

struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogPlayer, Display, All);

UCLASS()
class MPTESTTASK_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	
protected:
	virtual void BeginPlay() override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Debug|Character")
	void DebugDamageSelf(float Amount);
	
	void Fire();
	void Interact();
	
	UFUNCTION()
	void HandleDamagedServer(float Amount, AController* InstigatedBy);

	UFUNCTION()
	void HandleDeathServer(UHealthComponent* InHealthComponent
	, AController* InstigatedBy
	, AActor* DamageCauser);
	
	UFUNCTION()
	void HandleDeathCosmetic(UHealthComponent* InHealthComponent
		, AController* InstigatedBy
		, AActor* DamageCauser);
	
public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
protected:
	//TODO uprop comments
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWeaponComponent> WeaponComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractionAction;
	
	UPROPERTY(EditAnywhere, Category = "Death")
	TObjectPtr<USoundCue> DeathSound;
};
