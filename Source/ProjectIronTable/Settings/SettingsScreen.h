// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsScreen.generated.h"

class UButton;
class UWidgetSwitcher;
class UCameraSettingsPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/** Root settings screen widget. Coordinates all settings panels and broadcasts when the user requests to return to the home screen. */
UCLASS()
class PROJECTIRONTABLE_API USettingsScreen : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:

	/** The instantiated panel switcher widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> PanelSwitcher;

	/** Button that returns to the home screen. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	/** The camera settings panel widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCameraSettingsPanel> CameraSettingsPanel;

	//Add more settings panels here as needed.

public: 

	/** Broadcasts when the user requests to return to the home screen. */
	UPROPERTY(BlueprintAssignable)
	FOnBackRequested OnBackRequested;

private:

	/** Broadcasts OnBackRequested to notify the parent to return to the home screen. */
	UFUNCTION()
	void OnBackClicked();

};
