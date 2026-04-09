// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SettingsScreen.h"

#include "Components/Button.h"

#include "CameraSettingsPanel.h"

// Binds the back button click delegate and initializes the camera settings panel.
void USettingsScreen::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &USettingsScreen::OnBackClicked);
	}

	if (CameraSettingsPanel)
	{
		CameraSettingsPanel->Init();
	}
}

// Broadcasts OnBackRequested to notify the parent to return to the home screen.
void USettingsScreen::OnBackClicked()
{
	OnSettingsBackRequested.Broadcast();
}
