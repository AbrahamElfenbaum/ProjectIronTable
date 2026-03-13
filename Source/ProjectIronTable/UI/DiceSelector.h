#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "DiceSelector.generated.h"

UCLASS()
class PROJECTIRONTABLE_API UDiceSelector : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TSubclassOf<ABaseDiceActor> DiceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dice")
	EDiceType DiceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dice")
	int32 NumberOfDice = 0;

	UFUNCTION(BlueprintCallable)
	void IncreaseNumberOfDice();

	UFUNCTION(BlueprintCallable)
	void DecreaseNumberOfDice();
};
