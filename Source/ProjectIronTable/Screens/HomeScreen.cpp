// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "HomeScreen.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"

// Broadcasts OnCampaignManagerRequested.
void UHomeScreen::OnCampaignManagerClicked()
{
	OnCampaignManagerRequested.Broadcast();
}

// Broadcasts OnCampaignBrowserRequested.
void UHomeScreen::OnCampaignBrowserClicked()
{
	OnCampaignBrowserRequested.Broadcast();
}

// Broadcasts OnAssetLibraryRequested.
void UHomeScreen::OnAssetLibraryClicked()
{
	OnAssetLibraryRequested.Broadcast();
}

// Broadcasts OnSettingsRequested.
void UHomeScreen::OnSettingsClicked()
{
	OnSettingsRequested.Broadcast();
}

// Quits the application.
void UHomeScreen::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

// Binds all button click delegates.
void UHomeScreen::Init()
{
	if (CampaignManagerButton)
	{
		CampaignManagerButton->OnClicked.AddDynamic(this, &UHomeScreen::OnCampaignManagerClicked);
	}

	if (CampaignBrowserButton)
	{
		CampaignBrowserButton->OnClicked.AddDynamic(this, &UHomeScreen::OnCampaignBrowserClicked);
	}

	if (AssetLibraryButton)
	{
		AssetLibraryButton->OnClicked.AddDynamic(this, &UHomeScreen::OnAssetLibraryClicked);
	}

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UHomeScreen::OnSettingsClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UHomeScreen::OnQuitClicked);
	}
}
