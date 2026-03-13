#include "DiceSelectorManager.h"
#include "DiceSelector.h"

void UDiceSelectorManager::RollDice(const TArray<UDiceSelector*>& Selectors, FTransform Transform, FVector Impulse)
{
	//Clear array of any data from a previous roll
	SpawnedDice.Empty();
	PendingResults.Empty();
	ExpectedDiceCount = 0;

	for (auto Selector : Selectors)
	{
		if (Selector->NumberOfDice > 0)
		{
			for (int i = 0; i < Selector->NumberOfDice; i++)
			{
				//Spawn the die
				ABaseDiceActor* SpawnedDie = GetWorld()->SpawnActor<ABaseDiceActor>(Selector->DiceClass, Transform);

				if (SpawnedDie)
				{
					//Add the spawned die to the array
					SpawnedDice.Add(SpawnedDie);

					//Increment the ExpectedDiceCount
					ExpectedDiceCount++;

					//Add delegate to wait until all dice have stopped moving before broadcasting the result
					SpawnedDie->OnDiceRolled.AddDynamic(this, &UDiceSelectorManager::OnDiceRolledHandler);
				}
			}

			Selector->NumberOfDice = 0;
		}
	}

	if (!SpawnedDice.IsEmpty())
	{
		for (auto Dice : SpawnedDice)
		{
			//Roll the dice
			Dice->Roll(Impulse);
		}
	}
}

void UDiceSelectorManager::OnDiceRolledHandler(FRollResult Result)
{
	PendingResults.Add(Result);

	// Check if all dice have finished rolling
	if (PendingResults.Num() == ExpectedDiceCount)
	{
		// Broadcast all results at once
		OnAllDiceRolled(PendingResults);

		//Destroy all spawned dice after a delay of TimeBeforeDestroyingDice seconds
		GetWorld()->GetTimerManager().SetTimer(
			DestroyDiceTimerHandle, 
			this, 
			&UDiceSelectorManager::DestroyDice, 
			TimeBeforeDestroyingDice, 
			false);

		// Clear arrays for the next roll
		PendingResults.Empty();
		ExpectedDiceCount = 0;
	}
}

void UDiceSelectorManager::DestroyDice()
{
	for (auto Dice : SpawnedDice)
	{
		if (IsValid(Dice))
		{
			Dice->Destroy();
		}
	}
	SpawnedDice.Empty();
}
