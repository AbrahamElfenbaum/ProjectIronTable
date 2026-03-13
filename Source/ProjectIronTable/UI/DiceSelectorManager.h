#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "Components/Button.h"
#include "DiceSelectorManager.generated.h"

class UDiceSelector;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Dice")
	void OnAllDiceRolled(const TArray<FRollResult>& Results);

private:
	UFUNCTION()
	void OnDiceRolledHandler(FRollResult Result);

	UFUNCTION()
	void DestroyDice();
};
