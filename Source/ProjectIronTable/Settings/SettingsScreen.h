// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DelegateLibrary.h"
#include "SettingsScreen.generated.h"

class UButton;
class UWidgetSwitcher;
class UCameraSettingsPanel;

/** Root settings screen widget. Coordinates all settings panels and broadcasts when the user requests to return to the home screen. */
UCLASS()
class PROJECTIRONTABLE_API USettingsScreen : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:

#pragma region Widget References
	/** The instantiated panel switcher widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> PanelSwitcher;

	/** Button that returns to the home screen. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	/** The camera settings panel widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCameraSettingsPanel> CameraSettingsPanel;
#pragma endregion

public:

#pragma region Events
	/** Broadcasts when the user requests to return to the home screen. */
	UPROPERTY(BlueprintAssignable)
	FOnBackRequested OnBackRequested;
#pragma endregion

private:

#pragma region Event Handlers
	/** Broadcasts OnBackRequested to notify the parent to return to the home screen. */
	UFUNCTION()
	void OnBackClicked();
#pragma endregion

};
