// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainScreenController.generated.h"

class UMainScreenHUDComponent;

/** Player controller for the main screen. Sets UI-only input mode and owns the main screen HUD component. */
UCLASS()
class PROJECTIRONTABLE_API AMainScreenController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainScreenController();

protected:
	virtual void BeginPlay() override;

protected:

	// -- Runtime References --

	/** The HUD component owned by this controller, responsible for all UI widgets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMainScreenHUDComponent> HUDComponent;
};
