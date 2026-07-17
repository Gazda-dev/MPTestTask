// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "InteractableInterface.generated.h"

UINTERFACE()
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class MPTESTTASK_API IInteractableInterface
{
	GENERATED_BODY()

public:
	virtual bool CanInteract(AActor* Interactor) const
	{
		return true;
	}
	
	virtual void Interact(AActor* Interactor) = 0;
	
	virtual FText GetInteractionPrompt() const
	{
		return FText::GetEmpty();
	}
};
