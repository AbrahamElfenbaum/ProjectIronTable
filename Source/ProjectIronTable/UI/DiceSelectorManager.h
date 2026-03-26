// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "Components/Button.h"
#include "DiceSelectorManager.generated.h"

UENUM(BlueprintType)
enum class EDiceRollMode : uint8
{
	Normal,
	Advantage,
	Disadvantage
};

class UDiceSelector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAllDiceRolled, TArray<FRollResult>, Results, EDiceRollMode, RollMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceFailsafeDestroyed, EDiceType, DiceType);

UCLASS()
class PROJECTIRONTABLE_API UDiceSelectorManager : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D6;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D8;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D10;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D12;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D20;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDiceSelector> D100;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NormalRollButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AdvantageRollButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DisadvantageRollButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RollButton;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector StartingLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector Impulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseRange = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	FVector AngularImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularImpulseRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float TimeBeforeDestroyingDice = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	EDiceRollMode RollMode = EDiceRollMode::Normal;

private:
	TArray<UDiceSelector*> Selectors;
	TArray<ABaseDiceActor*> SpawnedDice;
	TArray<FRollResult> PendingResults;
	TArray<UButton*> AdvantageButtons;
	int32 ExpectedDiceCount = 0;
	
	FTimerHandle DestroyDiceTimerHandle;
	bool bRollInProgress = false;

public:
	UFUNCTION(BlueprintCallable)
	void RollDice();

	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnAllDiceRolled OnAllDiceRolled;

	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceFailsafeDestroyed OnDiceFailsafeDestroyed;
private:
	UFUNCTION()
	void OnDiceRolledHandler(FRollResult Result);

	UFUNCTION()
	void DestroyDice();

	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

	UFUNCTION()
	void OnSelectorCountChanged();

	UFUNCTION()
	void OnNormalClicked();

	UFUNCTION()
	void OnAdvantageClicked();

	UFUNCTION()
	void OnDisadvantageClicked();

	void UpdateRollButtonState();

	void UpdateAdvantageButtonState();

	FVector GetRandomizedVector(const FVector& BaseVector, const float&, bool bUseZAxis);
};
