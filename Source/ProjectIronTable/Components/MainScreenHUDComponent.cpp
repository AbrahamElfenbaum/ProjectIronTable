// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "SettingsScreen.h"
#include "HomeScreen.h"
#include "FunctionLibrary.h"

// Disables tick.
UMainScreenHUDComponent::UMainScreenHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Creates and adds the main screen widget, then caches button references and binds click handlers.
void UMainScreenHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		MainScreenClass)
	{

		MainScreenRef = CreateWidget<UUserWidget>(GetWorld(), MainScreenClass);
		MainScreenRef->AddToViewport();

		ScreenSwitcherRef = UFunctionLibrary::GetTypedWidgetFromName<UWidgetSwitcher>(MainScreenRef, TEXT("ScreenSwitcher"));
		HomeScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UHomeScreen>(MainScreenRef, TEXT("HomeScreen"));
		SettingsScreenRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsScreen>(MainScreenRef, TEXT("SettingsScreen"));

		if (HomeScreenRef)
		{
			HomeScreenRef->Init();
			HomeScreenRef->OnSettingsRequested.AddDynamic(this, &UMainScreenHUDComponent::OnSettingsClicked);
		}

		if (SettingsScreenRef)
		{
			SettingsScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
		}
	}
}

// Switches the screen switcher back to the home screen (index 0).
void UMainScreenHUDComponent::OnBackClicked()
{
	ScreenSwitcherRef->SetActiveWidgetIndex(0);
}

#pragma region Main Screen
// Switches the screen switcher to the settings screen (index 1).
void UMainScreenHUDComponent::OnSettingsClicked()
{
	ScreenSwitcherRef->SetActiveWidgetIndex(1);
}
#pragma endregion
