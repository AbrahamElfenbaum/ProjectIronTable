// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenController.h"
#include "MainScreenHUDComponent.h"

// Creates the HUD component as a default subobject.
AMainScreenController::AMainScreenController()
{
	HUDComponent = CreateDefaultSubobject<UMainScreenHUDComponent>(TEXT("HUDComponent"));
}

// Sets UI-only input mode and shows the mouse cursor.
void AMainScreenController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
