// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "InteractionPromptWidget.generated.h"

UCLASS()
class MPTESTTASK_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Interaction")
	void SetLabel(const FText& Label);
};
