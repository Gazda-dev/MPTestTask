// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interfaces/InteractableInterface.h"

#include "HealthPickup.generated.h"

class USphereComponent;

UCLASS()
class MPTESTTASK_API AHealthPickup : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AHealthPickup();

	/** IInteractableInterface Interface */
	virtual bool CanInteract(AActor* Interactor) const override;
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractionPrompt() const override;
	/** End IInteractableInterface Interface */
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HealthPickup")
	TObjectPtr<USceneComponent> RootSceneComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HealthPickup")
	TObjectPtr<USphereComponent> CollisionSphereComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HealthPickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthPickup")
	float HealAmount = 20.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthPickup")
	FText InteractionPrompt = FText::FromString(TEXT("Pickup medkit"));
	
	bool bConsumed = false;
};
