// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CameraSettingsPanel.generated.h"

class UButton;
class USettingsSlider;

/** Self-contained camera settings panel. Binds Apply and Reset buttons and handles loading and saving camera settings via the CameraSettings save slot. */
UCLASS()
class PROJECTIRONTABLE_API UCameraSettingsPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Populates the sliders array, binds button delegates, and loads saved camera settings (or resets to defaults if no save exists). */
	void Init();

private:

#pragma region Settings Sliders
	/** Slider controlling the maximum camera movement speed. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MaxCamSpeed;

	/** Slider controlling the minimum camera movement speed. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MinCamSpeed;

	/** Slider controlling the camera sprint speed multiplier. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> CamSpeedMultiplier;

	/** Slider controlling the maximum camera pitch angle. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MaxPitch;

	/** Slider controlling the minimum camera pitch angle. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MinPitch;

	/** Slider controlling the camera pan speed multiplier. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> PanMultiplier;

	/** Slider controlling the maximum zoom distance. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MaxZoom;

	/** Slider controlling the minimum zoom distance. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> MinZoom;

	/** Slider controlling the zoom speed. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USettingsSlider> ZoomSpeed;

	/** All 9 settings sliders � used for batch operations like reset. */
	UPROPERTY()
	TArray<USettingsSlider*> SettingsSliders;
#pragma endregion

#pragma region Buttons
	/** Button that applies and saves the current slider values. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyButton;

	/** Button that resets all sliders to their defaults. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResetButton;
#pragma endregion

#pragma region Event Functions
	/** Reads all slider values, saves them to the camera settings save slot. */
	UFUNCTION()
	void OnApplyClicked();

	/** Resets all sliders to defaults then calls OnApplyClicked to save. */
	UFUNCTION()
	void OnResetClicked();
#pragma endregion
};
