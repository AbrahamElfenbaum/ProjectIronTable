// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenController.h"

#include "MainScreenUIComponent.h"

// Creates the UI component as a default subobject.
AMainScreenController::AMainScreenController()
{
	HUDComponent = CreateDefaultSubobject<UMainScreenUIComponent>(TEXT("UIComponent"));
}

// Sets UI-only input mode and shows the mouse cursor.
void AMainScreenController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
