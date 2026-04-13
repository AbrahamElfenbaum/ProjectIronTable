// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenHUDComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"

#include "HomeScreen.h"
#include "CampaignManagerScreen.h"
#include "CampaignBrowserScreen.h"
#include "AssetLibraryScreen.h"
#include "SettingsScreen.h"
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
		MainScreenRef = CreateWidget<UUserWidget>(PlayerControllerRef, MainScreenClass);
		if (!IsValid(MainScreenRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — Failed to create MainScreen widget"));
			return;
		}
		MainScreenRef->AddToViewport();

		ScreenSwitcherRef = UFunctionLibrary::GetTypedWidgetFromName<UWidgetSwitcher>(MainScreenRef, TEXT("ScreenSwitcher"));
		HomeScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UHomeScreen>(MainScreenRef, TEXT("HomeScreen"));
		CampaignManagerScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UCampaignManagerScreen>(MainScreenRef, TEXT("CampaignManagerScreen"));
		CampaignBrowserScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UCampaignBrowserScreen>(MainScreenRef, TEXT("CampaignBrowserScreen"));
		AssetLibraryScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UAssetLibraryScreen>(MainScreenRef, TEXT("AssetLibraryScreen"));
		SettingsScreenRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsScreen>(MainScreenRef, TEXT("SettingsScreen"));
		
		if (!IsValid(ScreenSwitcherRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — ScreenSwitcher not found"));
		}

		if (IsValid(HomeScreenRef))
		{
			HomeScreenRef->Init();
			HomeScreenRef->OnCampaignManagerRequested.AddDynamic(this, &UMainScreenHUDComponent::OnCampaignManagerClicked);
			HomeScreenRef->OnCampaignBrowserRequested.AddDynamic(this, &UMainScreenHUDComponent::OnCampaignBrowserClicked);
			HomeScreenRef->OnAssetLibraryRequested.AddDynamic(this, &UMainScreenHUDComponent::OnAssetLibraryClicked);
			HomeScreenRef->OnSettingsRequested.AddDynamic(this, &UMainScreenHUDComponent::OnSettingsClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — HomeScreen not found"));
		}

		if (IsValid(CampaignManagerScreenRef))
		{
			CampaignManagerScreenRef->Init();
			CampaignManagerScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — CampaignManagerScreen not found"));
		}

		if (IsValid(CampaignBrowserScreenRef))
		{
			CampaignBrowserScreenRef->Init();
			CampaignBrowserScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — CampaignBrowserScreen not found"));
		}

		if (IsValid(AssetLibraryScreenRef))
		{
			AssetLibraryScreenRef->Init();
			AssetLibraryScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::BeginPlay — AssetLibraryScreen not found"));
		}

		if (IsValid(SettingsScreenRef))
		{
			SettingsScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenHUDComponent::OnBackClicked);
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

// Switches the screen switcher to the campaign manager screen (index 1).
void UMainScreenHUDComponent::OnCampaignManagerClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnCampaignManagerClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(1);
}

// Switches the screen switcher to the campaign browser screen (index 2).
void UMainScreenHUDComponent::OnCampaignBrowserClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnCampaignBrowserClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(2);
}

// Switches the screen switcher to the asset library screen (index 3).
void UMainScreenHUDComponent::OnAssetLibraryClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnAssetLibraryClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(3);
}

// Switches the screen switcher to the settings screen (index 4).
void UMainScreenHUDComponent::OnSettingsClicked()
{
	if (!IsValid(ScreenSwitcherRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMainScreenHUDComponent::OnSettingsClicked — ScreenSwitcherRef is null"));
		return;
	}
	ScreenSwitcherRef->SetActiveWidgetIndex(4);
}
