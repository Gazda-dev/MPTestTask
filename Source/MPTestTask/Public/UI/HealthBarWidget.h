// Copyright © 2026 Gazda-dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "HealthBarWidget.generated.h"

UCLASS()
class MPTESTTASK_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Health")
	void UpdateHealthPercent(float Percent);
};
