// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainScreenHUDComponent.generated.h"

class UButton;
class UWidgetSwitcher;
class USettingsSlider;

/** Actor component attached to AMainScreenController that owns and manages all main screen UI. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UMainScreenHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMainScreenHUDComponent();

protected:
	virtual void BeginPlay() override;

public:

	// -- Config --

	/** The root main screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MainScreenClass;

private:

	// -- Runtime References --

#pragma region Main Screen
			/** The instantiated root main screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> MainScreenRef;

	/** The instantiated screen switcher widget. */
	UPROPERTY()
	TObjectPtr<UWidgetSwitcher> ScreenSwitcherRef;
#pragma endregion

#pragma region Home Screen
	/** The instantiated home screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> HomeScreenRef;

	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;

	/** Button that transitions to the gameplay scene. */
	UPROPERTY()
	TObjectPtr<UButton> PlayButtonRef;

	/** Button that opens the join session flow (not yet implemented). */
	UPROPERTY()
	TObjectPtr<UButton> JoinButtonRef;

	/** Button that opens the asset library screen (not yet implemented). */
	UPROPERTY()
	TObjectPtr<UButton> LibraryButtonRef;

	/** Button that opens the settings panel. */
	UPROPERTY()
	TObjectPtr<UButton> SettingsButtonRef;

	/** Button that quits the application. */
	UPROPERTY()
	TObjectPtr<UButton> QuitButtonRef;
#pragma endregion

#pragma region Settings Screen
	/** The instantiated settings screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> SettingsScreenRef;

	/** Slider controlling the maximum camera movement speed. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MaxCamSpeedSliderRef;

	/** Slider controlling the minimum camera movement speed. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MinCamSpeedSliderRef;

	/** Slider controlling the camera sprint speed multiplier. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> CamSpeedMultiplierSliderRef;

	/** Slider controlling the maximum camera pitch angle. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MaxPitchSliderRef;

	/** Slider controlling the minimum camera pitch angle. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MinPitchSliderRef;

	/** Slider controlling the camera pan speed multiplier. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> PanMultiplierSliderRef;

	/** Slider controlling the maximum zoom distance. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MaxZoomSliderRef;

	/** Slider controlling the minimum zoom distance. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> MinZoomSliderRef;

	/** Slider controlling the zoom speed. */
	UPROPERTY()
	TObjectPtr<USettingsSlider> ZoomSpeedSliderRef;

	/** Button that returns to the home screen. */
	UPROPERTY()
	TObjectPtr<UButton> SettingsBackButtonRef;

	/** Button that applies and saves the current slider values. */
	UPROPERTY()
	TObjectPtr<UButton> SettingsApplyButtonRef;

	/** Button that resets all sliders to their defaults. */
	UPROPERTY()
	TObjectPtr<UButton> SettingsResetButtonRef;

	/** All 9 settings sliders — used for batch operations like reset. */
	UPROPERTY()
	TArray<USettingsSlider*> SettingsSliders;

#pragma endregion

private:

	// -- Event Handlers --

	/** Opens the home screen*/
	UFUNCTION()
	void OnBackClicked();

#pragma region Main Screen
	/** Opens the gameplay level. */
	UFUNCTION()
	void OnPlayClicked();

	/** Placeholder — join session flow not yet implemented. */
	UFUNCTION()
	void OnJoinClicked();

	/** Placeholder — asset library screen not yet implemented. */
	UFUNCTION()
	void OnLibraryClicked();

	/** Opens the settings screen */
	UFUNCTION()
	void OnSettingsClicked();

	/** Quits the application. */
	UFUNCTION()
	void OnQuitClicked();
#pragma endregion

#pragma region Settings Screen
	/** Reads all slider values, saves them to the camera settings save slot. */
	UFUNCTION()
	void OnApplyClicked();

	/** Resets all sliders to defaults then calls OnApplyClicked to save. */
	UFUNCTION()
	void OnResetClicked();
#pragma endregion
};
