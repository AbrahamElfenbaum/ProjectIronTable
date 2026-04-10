// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SettingsScreen.h"

#include "CameraSettingsPanel.h"

// Initializes settings panels.
void USettingsScreen::NativeConstruct()
{
	Super::NativeConstruct();

	if (CameraSettingsPanel)
	{
		CameraSettingsPanel->Init();
	}
}
