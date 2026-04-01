// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "HomeScreenController.h"
#include "HomeScreenHUDComponent.h"

// Creates the HUD component as a default subobject.
AHomeScreenController::AHomeScreenController()
{
	HUDComponent = CreateDefaultSubobject<UHomeScreenHUDComponent>(TEXT("HUDComponent"));
}

// Sets UI-only input mode and shows the mouse cursor.
void AHomeScreenController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
