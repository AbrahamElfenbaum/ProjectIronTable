// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SettingsScreen.h"
#include "Components/Button.h"
#include "CameraSettingsPanel.h"

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

void USettingsScreen::OnBackClicked()
{
	OnBackRequested.Broadcast();
}
