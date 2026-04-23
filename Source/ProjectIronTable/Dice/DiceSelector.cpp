// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceSelector.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

// Resets the count to zero, refreshes the label, and broadcasts the change.
void UDiceSelector::ResetCount()
{
	DiceCount = 0;
	CountText->SetText(FText::AsNumber(DiceCount));
	OnCountChanged.Broadcast();
}

// Initializes the type label, count label, and button delegates.
void UDiceSelector::NativeConstruct()
{
	Super::NativeConstruct();

	if (TypeText)
	{
		TypeText->SetText(UEnum::GetDisplayValueAsText(DiceType));
	}
	if (CountText)
	{
		CountText->SetText(FText::AsNumber(DiceCount));
	}
	if (IncreaseButton)
	{
		IncreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::IncreaseDiceCount);
	}
	if (DecreaseButton)
	{
		DecreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::DecreaseDiceCount);
	}
}

// Increments the count, refreshes the label, and broadcasts the change.
void UDiceSelector::IncreaseDiceCount()
{
	DiceCount++;
	CountText->SetText(FText::AsNumber(DiceCount));
	OnCountChanged.Broadcast();
}

// Decrements the count if above zero, refreshes the label, and broadcasts the change.
void UDiceSelector::DecreaseDiceCount()
{
	if (DiceCount > 0)
	{
		DiceCount--;
		CountText->SetText(FText::AsNumber(DiceCount));
		OnCountChanged.Broadcast();
	}
}
