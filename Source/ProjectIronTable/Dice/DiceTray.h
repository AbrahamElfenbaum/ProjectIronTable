// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "DiceRollComponent.h"
#include "DiceTray.generated.h"

class UButton;
class UDiceSelector;
class UDiceRollComponent;

/** Presentation widget for the dice tray: lets the player choose dice counts and roll mode, then builds a FDiceRollRequest and hands it to the session's UDiceRollComponent. Owns no roll logic itself. */
UCLASS()
class PROJECTIRONTABLE_API UDiceTray : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Selector widget for D4 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D4;

	/** Selector widget for D6 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D6;

	/** Selector widget for D8 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D8;

	/** Selector widget for D10 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D10;

	/** Selector widget for D12 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D12;

	/** Selector widget for D20 dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D20;

	/** Selector widget for D100 (percentile) dice. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D100;

	/** Button that sets roll mode to Normal. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NormalRollButton;

	/** Button that sets roll mode to Advantage. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AdvantageRollButton;

	/** Button that sets roll mode to Disadvantage. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DisadvantageRollButton;

	/** Button that triggers the actual dice roll. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RollButton;
#pragma endregion

	/** Cached dice roll component on the owning session controller; target of roll requests. */
	UPROPERTY()
	TObjectPtr<UDiceRollComponent> DiceRollComponentRef;

#pragma region State
	/** Flat array of all selector widgets for iteration. */
	UPROPERTY()
	TArray<TObjectPtr<UDiceSelector>> Selectors;

	/** The three advantage-mode buttons, kept together for bulk enable/disable. */
	UPROPERTY()
	TArray<TObjectPtr<UButton>> AdvantageButtons;

	/** True while a roll is in progress */
	bool bRollInProgress = false;

	/** The currently selected roll mode; drives which advantage button is highlighted and is copied into the roll request. */
	EDiceRollMode ActiveRollMode = EDiceRollMode::Normal;
#pragma endregion

private:

#pragma region Event Handlers
	/** Refreshes the roll and advantage button states whenever a selector count changes. */
	UFUNCTION()
	void OnSelectorCountChanged();

	/** Builds a roll request from the current selectors and roll mode, hands it to the dice roll component, then resets the selectors and refreshes button states. */
	UFUNCTION()
	void OnRollClicked();

	/** Sets roll mode to Normal and refreshes advantage button states. */
	UFUNCTION()
	void OnNormalClicked();

	/** Sets roll mode to Advantage and refreshes advantage button states. */
	UFUNCTION()
	void OnAdvantageClicked();

	/** Sets roll mode to Disadvantage and refreshes advantage button states. */
	UFUNCTION()
	void OnDisadvantageClicked();

	/** Clears the in-progress flag and refreshes button states once the dice roll component reports the roll complete. */
	UFUNCTION()
	void OnRollCompleteHandler(TArray<FRollResult> Results, EDiceRollMode RollMode);
#pragma endregion

#pragma region Private Methods
	/** Enables or disables the roll button based on whether any dice are selected and no roll is in progress. */
	void UpdateRollButtonState();

	/** Enables or disables the advantage/disadvantage buttons based on whether exactly one die is selected. */
	void UpdateAdvantageButtonState();
#pragma endregion

protected:

	/** Populates selector and button arrays, binds all button delegates, and refreshes initial button states. */
	virtual void NativeConstruct() override;
};
