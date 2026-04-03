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
	/** Creates the HUD component as a default subobject. */
	AMainScreenController();

protected:
	/** Sets UI-only input mode and shows the mouse cursor. */
	virtual void BeginPlay() override;

protected:

#pragma region Runtime References
	/** The HUD component owned by this controller, responsible for all UI widgets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMainScreenHUDComponent> HUDComponent;
#pragma endregion
};
