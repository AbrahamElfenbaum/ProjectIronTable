// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "CameraSettingsPanel.h"

#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"

#include "SettingsSlider.h"
#include "CameraSettingsSave.h"

// Populates the sliders array, binds button delegates, and loads saved camera settings (or resets to defaults if no save exists).
void UCameraSettingsPanel::Init()
{
	SettingsSliders =
	{
		MaxCamSpeed,
		MinCamSpeed,
		CamSpeedMultiplier,
		MaxPitch,
		MinPitch,
		PanMultiplier,
		MaxZoom,
		MinZoom,
		ZoomSpeed
	};

	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &UCameraSettingsPanel::OnApplyClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UCameraSettingsPanel::OnResetClicked);
	}

	if (UGameplayStatics::DoesSaveGameExist(UCameraSettingsSave::SaveSlotName, 0))
	{
		USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(UCameraSettingsSave::SaveSlotName, 0);
		if (UCameraSettingsSave* CameraSettingsSave = Cast<UCameraSettingsSave>(LoadedSave))
		{
			MaxCamSpeed->SetValue(CameraSettingsSave->MaxCameraMovementSpeed);
			MinCamSpeed->SetValue(CameraSettingsSave->MinCameraMovementSpeed);
			CamSpeedMultiplier->SetValue(CameraSettingsSave->CameraSpeedMultiplier);
			MaxPitch->SetValue(CameraSettingsSave->MaxCameraPitch);
			MinPitch->SetValue(CameraSettingsSave->MinCameraPitch);
			PanMultiplier->SetValue(CameraSettingsSave->CameraPanSpeedMultiplier);
			MaxZoom->SetValue(CameraSettingsSave->MaxZoomLength);
			MinZoom->SetValue(CameraSettingsSave->MinZoomLength);
			ZoomSpeed->SetValue(CameraSettingsSave->ZoomSpeed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UCameraSettingsPanel::Init — Failed to cast loaded save to UCameraSettingsSave."));
		}
	}
	else
	{
		OnResetClicked();
	}
}

// Reads all slider values, populates a UCameraSettingsSave, and saves to slot "CameraSettings".
void UCameraSettingsPanel::OnApplyClicked()
{
	UCameraSettingsSave* SaveObject = Cast<UCameraSettingsSave>(UGameplayStatics::CreateSaveGameObject(UCameraSettingsSave::StaticClass()));

	if (SaveObject)
	{
		SaveObject->MaxCameraMovementSpeed = MaxCamSpeed->GetValue();
		SaveObject->MinCameraMovementSpeed = MinCamSpeed->GetValue();
		SaveObject->CameraSpeedMultiplier = CamSpeedMultiplier->GetValue();
		SaveObject->MaxCameraPitch = MaxPitch->GetValue();
		SaveObject->MinCameraPitch = MinPitch->GetValue();
		SaveObject->CameraPanSpeedMultiplier = PanMultiplier->GetValue();
		SaveObject->MaxZoomLength = MaxZoom->GetValue();
		SaveObject->MinZoomLength = MinZoom->GetValue();
		SaveObject->ZoomSpeed = ZoomSpeed->GetValue();

		UGameplayStatics::SaveGameToSlot(SaveObject, UCameraSettingsSave::SaveSlotName, 0);
	}
}

// Resets all sliders to their default values then applies and saves the defaults.
void UCameraSettingsPanel::OnResetClicked()
{
	for (USettingsSlider* Slider : SettingsSliders)
	{
		if (Slider)
		{
			Slider->ResetToDefault();
		}
	}
	OnApplyClicked();
}