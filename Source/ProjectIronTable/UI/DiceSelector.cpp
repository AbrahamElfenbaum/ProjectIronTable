// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceSelector.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

// Initializes the type label, count label, and button delegates.
void UDiceSelector::NativeConstruct()
{
	Super::NativeConstruct();

	TypeText->SetText(UEnum::GetDisplayValueAsText(DiceType));
	CountText->SetText(FText::AsNumber(DiceCount));

	IncreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::IncreaseDiceCount);
	DecreaseButton->OnClicked.AddDynamic(this, &UDiceSelector::DecreaseDiceCount);
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

// Resets the count to zero, refreshes the label, and broadcasts the change.
void UDiceSelector::ResetCount()
{
	DiceCount = 0;
	CountText->SetText(FText::AsNumber(DiceCount));
	OnCountChanged.Broadcast();
}
