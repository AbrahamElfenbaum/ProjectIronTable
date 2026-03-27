// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DiceSelector.generated.h"

/** Broadcast whenever the selected die count changes (increment, decrement, or reset). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiceCountChanged);

/** Widget that lets the player choose how many of a specific die type to roll. */
UCLASS()
class PROJECTIRONTABLE_API UDiceSelector : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

private:

	// -- Widget References --

	/** Displays the die type label (e.g. "D20"). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TypeText;

	/** Displays the current selected count. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

	/** Increments the die count by one. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IncreaseButton;

	/** Decrements the die count by one (minimum zero). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DecreaseButton;

	/** Increments DiceCount and broadcasts OnCountChanged. */
	UFUNCTION()
	void IncreaseDiceCount();

	/** Decrements DiceCount if above zero and broadcasts OnCountChanged. */
	UFUNCTION()
	void DecreaseDiceCount();

public:

	// -- Config --

	/** The actor class to spawn when this die type is rolled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TSubclassOf<ABaseDiceActor> DiceClass;

	/** The die type this selector represents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dice")
	EDiceType DiceType;

	// -- State --

	/** The current number of dice of this type queued to roll. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	int32 DiceCount = 0;

	// -- Events --

	/** Fired whenever the count changes, including on reset. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceCountChanged OnCountChanged;

	// -- Public Methods --

	/** Resets DiceCount to zero and broadcasts OnCountChanged. */
	void ResetCount();
};
