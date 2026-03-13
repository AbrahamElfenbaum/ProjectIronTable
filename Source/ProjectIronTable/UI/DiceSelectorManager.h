#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "DiceSelectorManager.generated.h"

class UDiceSelector;

UCLASS()
class PROJECTIRONTABLE_API UDiceSelectorManager : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBeforeDestroyingDice = 5.0f;

private:
	TArray<ABaseDiceActor*> SpawnedDice;
	TArray<FRollResult> PendingResults;
	int32 ExpectedDiceCount = 0;
	
	FTimerHandle DestroyDiceTimerHandle;

public:
	UFUNCTION(BlueprintCallable)
	void RollDice(const TArray<UDiceSelector*>& Selectors, FTransform Transform, FVector Impulse);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dice")
	void OnAllDiceRolled(const TArray<FRollResult>& Results);

private:
	UFUNCTION()
	void OnDiceRolledHandler(FRollResult Result);

	UFUNCTION()
	void DestroyDice();
	
};
