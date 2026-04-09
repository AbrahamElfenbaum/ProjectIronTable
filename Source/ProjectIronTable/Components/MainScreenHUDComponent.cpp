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

		if (!IsValid(ScreenSwitcherRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — ScreenSwitcher not found"));
		}

		if (IsValid(HomeScreenRef))
		{
			HomeScreenRef->Init();
			HomeScreenRef->OnSettingsRequested.AddDynamic(this, &UMainScreenHUDComponent::OnSettingsClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — HomeScreen not found"));
		}

		if (IsValid(SettingsScreenRef))
		{
			SettingsScreenRef->OnSettingsBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — SettingsScreen not found"));
		}
	}
}

// Switches the screen switcher back to the home screen (index 0).
void UMainScreenHUDComponent::OnBackClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnBackClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(0);
}

// Switches the screen switcher to the settings screen (index 1).
void UMainScreenHUDComponent::OnSettingsClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnSettingsClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(1);
}
