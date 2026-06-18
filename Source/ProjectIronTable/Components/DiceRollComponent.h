// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseDiceActor.h"
#include "DiceRollComponent.generated.h"

/** Controls whether a single-die roll uses normal, advantage, or disadvantage rules. */
UENUM(BlueprintType)
enum class EDiceRollMode : uint8
{
	Normal,
	Advantage,
	Disadvantage
};

/** One die type's worth of a roll request: which actor to spawn, which die it is, and how many. */
USTRUCT(BlueprintType)
struct PROJECTIRONTABLE_API FDiceTypeCount
{
	GENERATED_BODY()

	/** The actor class to spawn for this die type. */
	UPROPERTY()
	TSubclassOf<ABaseDiceActor> DiceClass;

	/** The die type this entry represents. */
	UPROPERTY()
	EDiceType DiceType;

	/** How many dice of this type to roll. */
	UPROPERTY()
	int32 Count;
};

/** A complete description of a dice roll: the dice to spawn and the roll mode. Built by the tray widget (or a chat parser) and handed to UDiceRollComponent::RollDice. */
USTRUCT(BlueprintType)
struct PROJECTIRONTABLE_API FDiceRollRequest
{
	GENERATED_BODY()

	/** One entry per selected die type. */
	UPROPERTY()
	TArray<FDiceTypeCount> Dice;

	/** Whether the roll uses normal, advantage, or disadvantage rules. */
	UPROPERTY()
	EDiceRollMode Mode;
};

/** Fired once every die in a roll has settled, passing the kept results and the active roll mode. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRollComplete, TArray<FRollResult>, Results, EDiceRollMode, RollMode);

/** Fired when a single die is destroyed by its failsafe timer before settling. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceFailsafeDestroyed, EDiceType, DiceType);

/** Fired when a roll is initiated, before any dice are spawned. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRollInitiated);

class ADiceSpawnVolume;

/** Runs a dice roll independent of any UI: spawns the dice described by a FDiceRollRequest, waits for them to settle, and broadcasts OnRollComplete. Owned by the session controller so chat commands and networked rolls can reuse the same path. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UDiceRollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDiceRollComponent();

#pragma region State
	/** All dice actors currently in the world from the most recent roll. */
	UPROPERTY()
	TArray<TObjectPtr<ABaseDiceActor>> SpawnedDice;

	/** Results collected as individual dice finish rolling. */
	TArray<FRollResult> PendingResults;

	/** Number of dice spawned for the current roll; used to detect when all results are in. */
	int32 ExpectedDiceCount = 0;

	/** Timer that triggers deferred destruction of settled dice. */
	FTimerHandle DestroyDiceTimerHandle;

	/** True while a roll is in progress */
	bool bRollInProgress = false;

	/** The roll mode of the in-flight roll; stashed from the request so the settle handler can apply advantage/disadvantage rules. */
	EDiceRollMode ActiveRollMode = EDiceRollMode::Normal;
#pragma endregion

#pragma region Config
	/** The spawn volume actor that defines where dice are placed in the world. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	TObjectPtr<ADiceSpawnVolume> SpawnVolume;

	/** Base direction and magnitude for the launch impulse applied to each die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector Impulse = { 3000.f, 3000.f, 0.f };

	/** Half-range of random noise added to the impulse on each axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseRange = 100.f;

	/** Base direction and magnitude for the angular impulse applied to each die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector AngularImpulse = { 500.f, 500.f, 500.f };

	/** Half-range of random noise added to the angular impulse on each axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularImpulseRange = 200.f;

	/** Seconds to wait after all dice settle before destroying them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float TimeBeforeDestroyingDice = 5.0f;
#pragma endregion

#pragma region Events
	/** Fired once the roll resolves; consumers (e.g. chat) bind here to display results. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnRollComplete OnRollComplete;

	/** Fired when any individual die is destroyed by its failsafe timer. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceFailsafeDestroyed OnDiceFailsafeDestroyed;

	/** Fired when a roll is initiated, before any dice are spawned. Used to send a private roll message if recipients are in the chat input. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnRollInitiated OnRollInitiated;
#pragma endregion

private:

#pragma region Event Handlers
	/** Collects a result and broadcasts OnRollComplete when all expected results are in. */
	UFUNCTION()
	void OnDiceRolledHandler(FRollResult Result);

	/** Destroys all currently spawned dice actors and clears the array. */
	UFUNCTION()
	void DestroyDice();

	/** Handles a die being destroyed by its failsafe; decrements expected count and finalises early if possible. */
	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);
#pragma endregion

	/** Returns the base vector with a uniform random offset applied to each axis within Range. Z is only randomized when bUseZAxis is true. */
	FVector GetRandomizedVector(const FVector& BaseVector, const float& Range, bool bUseZAxis);

	/** Tears down shared roll state after results broadcast: clears the in-progress flag, schedules deferred dice destruction, and resets pending results and expected count. */
	void FinalizeRoll();

public:

	/** Spawns and launches the dice described by Request; broadcasts OnRollComplete once they settle. */
	void RollDice(const FDiceRollRequest& Request);
};
