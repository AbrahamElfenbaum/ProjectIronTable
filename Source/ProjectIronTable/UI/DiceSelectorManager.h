// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "DiceSelectorManager.generated.h"

class UButton;
class ADiceSpawnVolume;

/** Controls whether a single-die roll uses normal, advantage, or disadvantage rules. */
UENUM(BlueprintType)
enum class EDiceRollMode : uint8
{
	Normal,
	Advantage,
	Disadvantage
};

class UDiceSelector;

/** Fired once all spawned dice have settled, passing every result and the active roll mode. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAllDiceRolled, TArray<FRollResult>, Results, EDiceRollMode, RollMode);

/** Fired when a single die is destroyed by its failsafe timer before settling. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceFailsafeDestroyed, EDiceType, DiceType);

/** Fired when a roll is initiated, before any dice are spawned. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRollInitiated);

/** Widget that manages all DiceSelector widgets, handles spawning and rolling, and broadcasts results. */
UCLASS()
class PROJECTIRONTABLE_API UDiceSelectorManager : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Populates selector and button arrays, binds all button delegates, and refreshes initial button states. */
	virtual void NativeConstruct() override;

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

public:

#pragma region Config
	/** The spawn volume actor that defines where dice are placed in the world. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	TObjectPtr<ADiceSpawnVolume> SpawnVolume;

	/** Base direction and magnitude for the launch impulse applied to each die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector Impulse;

	/** Half-range of random noise added to the impulse on each axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseRange = 50.f;

	/** Base direction and magnitude for the angular impulse applied to each die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector AngularImpulse;

	/** Half-range of random noise added to the angular impulse on each axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularImpulseRange = 500.f;

	/** Seconds to wait after all dice settle before destroying them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float TimeBeforeDestroyingDice = 5.0f;
#pragma endregion

#pragma region State
	/** The currently active roll mode; controls advantage/disadvantage behaviour. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	EDiceRollMode RollMode = EDiceRollMode::Normal;
#pragma endregion

private:

	/** Flat array of all selector widgets for iteration. */
	TArray<UDiceSelector*> Selectors;

	/** All dice actors currently in the world from the most recent roll. */
	TArray<ABaseDiceActor*> SpawnedDice;

	/** Results collected as individual dice finish rolling. */
	TArray<FRollResult> PendingResults;

	/** The three advantage-mode buttons, kept together for bulk enable/disable. */
	TArray<UButton*> AdvantageButtons;

	/** Number of dice spawned for the current roll; used to detect when all results are in. */
	int32 ExpectedDiceCount = 0;

	/** Timer that triggers deferred destruction of settled dice. */
	FTimerHandle DestroyDiceTimerHandle;

	/** True while a roll is in progress; used to disable the roll button. */
	bool bRollInProgress = false;

public:

#pragma region Public Methods
	/** Spawns and launches all queued dice according to the current selectors and roll mode. */
	UFUNCTION(BlueprintCallable)
	void RollDice();
#pragma endregion

#pragma region Events
	/** Fired once every spawned die has settled and results are available. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnAllDiceRolled OnAllDiceRolled;

	/** Fired when any individual die is destroyed by its failsafe timer. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceFailsafeDestroyed OnDiceFailsafeDestroyed;

	/** Fired when a roll is initiated, before any dice are spawned. Used to send a private roll message if recipients are in the chat input. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnRollInitiated OnRollInitiated;
#pragma endregion

private:

	/** Collects a result and broadcasts OnAllDiceRolled when all expected results are in. */
	UFUNCTION()
	void OnDiceRolledHandler(FRollResult Result);

	/** Destroys all currently spawned dice actors and clears the array. */
	UFUNCTION()
	void DestroyDice();

	/** Handles a die being destroyed by its failsafe; decrements expected count and finalises early if possible. */
	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

	/** Refreshes the roll and advantage button states whenever a selector count changes. */
	UFUNCTION()
	void OnSelectorCountChanged();

	/** Sets roll mode to Normal and refreshes advantage button states. */
	UFUNCTION()
	void OnNormalClicked();

	/** Sets roll mode to Advantage and refreshes advantage button states. */
	UFUNCTION()
	void OnAdvantageClicked();

	/** Sets roll mode to Disadvantage and refreshes advantage button states. */
	UFUNCTION()
	void OnDisadvantageClicked();

	/** Enables or disables the roll button based on whether any dice are selected and no roll is in progress. */
	void UpdateRollButtonState();

	/** Enables or disables the advantage/disadvantage buttons based on whether exactly one die is selected. */
	void UpdateAdvantageButtonState();

	/** Returns the base vector with a uniform random offset applied to each axis within Range. Z is only randomized when bUseZAxis is true. */
	FVector GetRandomizedVector(const FVector& BaseVector, const float&, bool bUseZAxis);
};
