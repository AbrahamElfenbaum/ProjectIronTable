// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MainScreenUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"

#include "AssetLibraryScreen.h"
#include "CampaignBrowserScreen.h"
#include "CampaignManagerScreen.h"
#include "FunctionLibrary.h"
#include "HomeScreen.h"
#include "MacroLibrary.h"
#include "SettingsScreen.h"

// Switches the screen switcher back to the home screen (index 0).
void UMainScreenUIComponent::OnBackClicked()
{
	SwitchScreen(0);
}

// Switches the screen switcher to the campaign manager screen (index 1).
void UMainScreenUIComponent::OnCampaignManagerClicked()
{
	SwitchScreen(1);
}

// Switches the screen switcher to the campaign browser screen (index 2).
void UMainScreenUIComponent::OnCampaignBrowserClicked()
{
	SwitchScreen(2);
}

// Switches the screen switcher to the asset library screen (index 3).
void UMainScreenUIComponent::OnAssetLibraryClicked()
{
	SwitchScreen(3);
}

// Switches the screen switcher to the settings screen (index 4).
void UMainScreenUIComponent::OnSettingsClicked()
{
	SwitchScreen(4);
}

// Switches the screen switcher to the screen at the given index.
void UMainScreenUIComponent::SwitchScreen(int32 ScreenIndex)
{
	CHECK_IF_VALID(ScreenSwitcherRef, );
	ScreenSwitcherRef->SetActiveWidgetIndex(ScreenIndex);
}

// Creates and adds the main screen widget, then caches button references and binds click handlers.
void UMainScreenUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		MainScreenClass)
	{
		MainScreenRef = CreateWidget<UUserWidget>(PlayerControllerRef, MainScreenClass);
		CHECK_IF_VALID(MainScreenRef, );
		MainScreenRef->AddToViewport();

		ScreenSwitcherRef = UFunctionLibrary::GetTypedWidgetFromName<UWidgetSwitcher>(MainScreenRef, TEXT("ScreenSwitcher"));
		HomeScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UHomeScreen>(MainScreenRef, TEXT("HomeScreen"));
		CampaignManagerScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UCampaignManagerScreen>(MainScreenRef, TEXT("CampaignManagerScreen"));
		CampaignBrowserScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UCampaignBrowserScreen>(MainScreenRef, TEXT("CampaignBrowserScreen"));
		AssetLibraryScreenRef = UFunctionLibrary::GetTypedWidgetFromName<UAssetLibraryScreen>(MainScreenRef, TEXT("AssetLibraryScreen"));
		SettingsScreenRef = UFunctionLibrary::GetTypedWidgetFromName<USettingsScreen>(MainScreenRef, TEXT("SettingsScreen"));

		if (!IsValid(ScreenSwitcherRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — ScreenSwitcher not found"));
		}

		if (IsValid(HomeScreenRef))
		{
			HomeScreenRef->Init();
			HomeScreenRef->OnCampaignManagerRequested.AddDynamic(this, &UMainScreenUIComponent::OnCampaignManagerClicked);
			HomeScreenRef->OnCampaignBrowserRequested.AddDynamic(this, &UMainScreenUIComponent::OnCampaignBrowserClicked);
			HomeScreenRef->OnAssetLibraryRequested.AddDynamic(this, &UMainScreenUIComponent::OnAssetLibraryClicked);
			HomeScreenRef->OnSettingsRequested.AddDynamic(this, &UMainScreenUIComponent::OnSettingsClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — HomeScreen not found"));
		}

		if (IsValid(CampaignManagerScreenRef))
		{
			CampaignManagerScreenRef->Init();
			CampaignManagerScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenUIComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — CampaignManagerScreen not found"));
		}

		if (IsValid(CampaignBrowserScreenRef))
		{
			CampaignBrowserScreenRef->Init();
			CampaignBrowserScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenUIComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — CampaignBrowserScreen not found"));
		}

		if (IsValid(AssetLibraryScreenRef))
		{
			AssetLibraryScreenRef->Init();
			AssetLibraryScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenUIComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — AssetLibraryScreen not found"));
		}

		if (IsValid(SettingsScreenRef))
		{
			SettingsScreenRef->OnBackRequested.AddDynamic(this, &UMainScreenUIComponent::OnBackClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMainScreenUIComponent::BeginPlay — SettingsScreen not found"));
		}
	}
}

// Disables tick.
UMainScreenUIComponent::UMainScreenUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
