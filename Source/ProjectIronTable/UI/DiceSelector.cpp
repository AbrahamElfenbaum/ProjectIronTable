#include "DiceSelector.h"

void UDiceSelector::IncreaseNumberOfDice()
{
	NumberOfDice++;
}

void UDiceSelector::DecreaseNumberOfDice()
{
	NumberOfDice--;
	if (NumberOfDice < 0)
	{
		NumberOfDice = 0;
	}
}
