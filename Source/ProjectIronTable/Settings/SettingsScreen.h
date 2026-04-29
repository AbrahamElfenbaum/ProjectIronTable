// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "SettingsScreen.generated.h"

class UWidgetSwitcher;
class UCameraSettingsPanel;

/** Root settings screen widget. Coordinates all settings panels and broadcasts when the user requests to return to the home screen. */
UCLASS()
class PROJECTIRONTABLE_API USettingsScreen : public UBaseScreen
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** The instantiated panel switcher widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> PanelSwitcher;

	/** The camera settings panel widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCameraSettingsPanel> CameraSettingsPanel;
#pragma endregion

protected:

	/** Initializes settings panels. */
	virtual void NativeConstruct() override;
};
