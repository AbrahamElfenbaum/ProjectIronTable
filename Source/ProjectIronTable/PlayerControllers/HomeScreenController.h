// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HomeScreenController.generated.h"

class UHomeScreenHUDComponent;

/** Player controller for the home screen. Sets UI-only input mode and owns the home screen HUD component. */
UCLASS()
class PROJECTIRONTABLE_API AHomeScreenController : public APlayerController
{
	GENERATED_BODY()

public:
	AHomeScreenController();

protected:
	virtual void BeginPlay() override;
	
protected:

	// -- Runtime References --

	/** The HUD component owned by this controller, responsible for all UI widgets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UHomeScreenHUDComponent> HUDComponent;
};
