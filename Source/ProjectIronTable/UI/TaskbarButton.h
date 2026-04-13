// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskbarButton.generated.h"

class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToggled);

/**
 * A single button in the taskbar that tracks and toggles the visibility of one widget.
 */
UCLASS()
class PROJECTIRONTABLE_API UTaskbarButton : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Button the player clicks to toggle the tracked widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToggleButton;

	/** Displays the name of the tracked widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WidgetLabel;
#pragma endregion

#pragma region Runtime References
	/** The widget this button controls. */
	UPROPERTY()
	TObjectPtr<UUserWidget> TrackedWidget;
#pragma endregion

public:

#pragma region Events
	/** Broadcasts when the button is toggled, used to trigger taskbar visibility changes. */
	UPROPERTY(BlueprintAssignable)
	FOnToggled OnToggled;
#pragma endregion

#pragma region Public Methods
	/** Assigns the widget to track and sets the button label. */
	void SetTrackedWidget(UUserWidget* Widget, const FString& Label);

	/** Returns the widget being tracked by this button. */
	UUserWidget* GetTrackedWidget() const;
#pragma endregion

protected:

	/** Binds the toggle button click event. */
	virtual void NativeConstruct() override;

private:

#pragma region Event Handlers
	/** Toggles the tracked widget between Visible and Collapsed. */
	UFUNCTION()
	void OnToggleClicked();
#pragma endregion
};
