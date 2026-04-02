// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/WidgetSwitcher.h"
#include "SettingsSlider.h"
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
		HomeScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UUserWidget>(MainScreenRef, TEXT("HomeScreen"));
		SettingsScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UUserWidget>(MainScreenRef, TEXT("SettingsScreen"));

		if (HomeScreenRef)
		{
			PlayButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(HomeScreenRef, TEXT("PlayButton"));
			JoinButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(HomeScreenRef, TEXT("JoinButton"));
			LibraryButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(HomeScreenRef, TEXT("LibraryButton"));
			SettingsButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(HomeScreenRef, TEXT("SettingsButton"));
			QuitButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(HomeScreenRef, TEXT("QuitButton"));

			if (PlayButtonRef)
			{
				PlayButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnPlayClicked);
			}

			if (JoinButtonRef)
			{
				JoinButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnJoinClicked);
			}

			if (LibraryButtonRef)
			{
				LibraryButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnLibraryClicked);
			}

			if (SettingsButtonRef)
			{
				SettingsButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnSettingsClicked);
			}

			if (QuitButtonRef)
			{
				QuitButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnQuitClicked);
			}
		}

		if (SettingsScreenRef)
		{
			MaxCamSpeedSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MaxCamSpeed"));
			MinCamSpeedSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MinCamSpeed"));
			CamSpeedMultiplierSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("CamSpeedMultiplier"));
			MaxPitchSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MaxPitch"));
			MinPitchSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MinPitch"));
			PanMultiplierSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("PanMultiplier"));
			MaxZoomSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MaxZoom"));
			MinZoomSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("MinZoom"));
			ZoomSpeedSliderRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsSlider>(SettingsScreenRef, TEXT("ZoomSpeed"));
			SettingsBackButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(SettingsScreenRef, TEXT("BackButton"));
			SettingsApplyButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(SettingsScreenRef, TEXT("ApplyButton"));
			SettingsResetButtonRef = UFunctionLibrary::GetTypedWidgetFromName<UButton>(SettingsScreenRef, TEXT("ResetButton"));

			SettingsSliders = 
			{ 
				MaxCamSpeedSliderRef, 
				MinCamSpeedSliderRef, 
				CamSpeedMultiplierSliderRef, 
				MaxPitchSliderRef, 
				MinPitchSliderRef, 
				PanMultiplierSliderRef, 
				MaxZoomSliderRef, 
				MinZoomSliderRef, 
				ZoomSpeedSliderRef 
			};

			if (SettingsBackButtonRef)
			{
				SettingsBackButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
			}

			if (SettingsApplyButtonRef)
			{
				SettingsApplyButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnApplyClicked);
			}

			if (SettingsResetButtonRef)
			{
				SettingsResetButtonRef->OnClicked.AddDynamic(this, &UMainScreenHUDComponent::OnResetClicked);
			}
		}
	}
}

// Switches the screen switcher back to the home screen (index 0).
void UMainScreenHUDComponent::OnBackClicked()
{
	ScreenSwitcherRef->SetActiveWidgetIndex(0);
}

#pragma region Main Screen
// Opens the gameplay level.
void UMainScreenHUDComponent::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_Gameplay"));
}

// Placeholder — join session flow not yet implemented.
void UMainScreenHUDComponent::OnJoinClicked()
{
}

// Placeholder — asset library screen not yet implemented.
void UMainScreenHUDComponent::OnLibraryClicked()
{
}

// Switches the screen switcher to the settings screen (index 1).
void UMainScreenHUDComponent::OnSettingsClicked()
{
	ScreenSwitcherRef->SetActiveWidgetIndex(1);
}

// Quits the application.
void UMainScreenHUDComponent::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, PlayerControllerRef, EQuitPreference::Quit, false);
}
#pragma endregion

#pragma region Settings Screen
// Reads all slider values, populates a UCameraSettingsSave, and saves to slot "CameraSettings".
void UMainScreenHUDComponent::OnApplyClicked()
{
	
}

// Resets all sliders to their default values then applies and saves the defaults.
void UMainScreenHUDComponent::OnResetClicked()
{
	for (USettingsSlider* Slider : SettingsSliders)
	{
		Slider->ResetToDefault();
	}

	OnApplyClicked();
}
#pragma endregion


