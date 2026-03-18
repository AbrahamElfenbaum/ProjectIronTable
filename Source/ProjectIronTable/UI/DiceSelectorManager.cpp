#include "DiceSelectorManager.h"
#include "DiceSelector.h"
#include "Kismet/KismetMathLibrary.h"

void UDiceSelectorManager::NativeConstruct()
{
	Super::NativeConstruct();

	Selectors = { D4, D6, D8, D10, D12, D20, D100 };
	RollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::RollDice);
}

void UDiceSelectorManager::RollDice()
{
	//Clear array of any data from a previous roll
	SpawnedDice.Empty();
	PendingResults.Empty();
	ExpectedDiceCount = 0;

	for (auto Selector : Selectors)
	{
		if (Selector->DiceCount > 0)
		{
			for (int i = 0; i < Selector->DiceCount; i++)
			{
				FRotator RandomRot = UKismetMathLibrary::RandomRotator(true);
				FTransform T(
					FQuat(RandomRot),
					StartingLocation,
					FVector::OneVector
				);

				//Spawn the die
				ABaseDiceActor* SpawnedDie = GetWorld()->SpawnActor<ABaseDiceActor>(Selector->DiceClass, T);

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

			Selector->DiceCount = 0;
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
		// Debug: log each result to the output log
		for (const FRollResult& RollResult : PendingResults)
		{
			FString DiceTypeName = UEnum::GetValueAsString(RollResult.DiceType);
			UE_LOG(LogTemp, Display, TEXT("Type: %s | Value: %d"), *DiceTypeName, RollResult.Value);
		}

		// Broadcast all results at once
		OnAllDiceRolled.Broadcast(PendingResults);

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
