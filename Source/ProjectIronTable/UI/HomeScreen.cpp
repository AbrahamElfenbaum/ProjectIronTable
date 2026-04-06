// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "HomeScreen.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Binds all button click delegates.
void UHomeScreen::Init()
{
	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UHomeScreen::OnPlayClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UHomeScreen::OnJoinClicked);
	}

	if (LibraryButton)
	{
		LibraryButton->OnClicked.AddDynamic(this, &UHomeScreen::OnLibraryClicked);
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

// Opens the gameplay level.
void UHomeScreen::OnPlayClicked()
{
	UGameplayStatics::OpenLevel(this, TEXT("L_Gameplay"));
}

// Broadcasts OnJoinRequested.
void UHomeScreen::OnJoinClicked()
{
	OnJoinRequested.Broadcast();
}

// Broadcasts OnLibraryRequested.
void UHomeScreen::OnLibraryClicked()
{
	OnLibraryRequested.Broadcast();
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
