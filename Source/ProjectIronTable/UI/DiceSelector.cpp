// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#include "DiceSelector.h"

void UDiceSelector::NativeConstruct()
{
	Super::NativeConstruct();

	TypeText->SetText(UEnum::GetDisplayValueAsText(DiceType));
	CountText->SetText(FText::AsNumber(DiceCount));

	IncreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::IncreaseDiceCount);
	DecreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::DecreaseDiceCount);
}

void UDiceSelector::IncreaseDiceCount()
{
	DiceCount++;
	CountText->SetText(FText::AsNumber(DiceCount));
}

void UDiceSelector::DecreaseDiceCount()
{
	if (DiceCount > 0)
	{
		DiceCount--;
		CountText->SetText(FText::AsNumber(DiceCount));
	}
}
