// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "Components/Button.h"
#include "DiceSelectorManager.generated.h"

class UDiceSelector;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllDiceRolled, TArray<FRollResult>, Results);
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

private:
	TArray<UDiceSelector*> Selectors;
	TArray<ABaseDiceActor*> SpawnedDice;
	TArray<FRollResult> PendingResults;
	int32 ExpectedDiceCount = 0;
	
	FTimerHandle DestroyDiceTimerHandle;

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

	FVector GetRandomizedVector(const FVector& BaseVector, const float&, bool bUseZAxis);
};
