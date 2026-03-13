#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseDiceActor.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DiceSelector.generated.h"

UCLASS()
class PROJECTIRONTABLE_API UDiceSelector : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TypeText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IncreaseButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DecreaseButton;

	UFUNCTION()
	void IncreaseDiceCount();

	UFUNCTION()
	void DecreaseDiceCount();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TSubclassOf<ABaseDiceActor> DiceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dice")
	EDiceType DiceType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	int32 DiceCount = 0;
};
