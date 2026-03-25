#include "DiceSelectorManager.h"
#include "DiceSelector.h"
#include "Kismet/KismetMathLibrary.h"

void UDiceSelectorManager::NativeConstruct()
{
	Super::NativeConstruct();

	Selectors = { D4, D6, D8, D10, D12, D20, D100 };
	RollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::RollDice);

	for (auto Selector : Selectors)
	{
		Selector->OnCountChanged.AddDynamic(this, &UDiceSelectorManager::OnSelectorCountChanged);
	}

	UpdateRollButtonState();
}

void UDiceSelectorManager::RollDice()
{
	// Cancel any pending destroy timer from a previous roll
	GetWorld()->GetTimerManager().ClearTimer(DestroyDiceTimerHandle);

	// Destroy any dice still in the world from a previous roll
	for (auto Dice : SpawnedDice)
	{
		if (IsValid(Dice))
		{
			Dice->Destroy();
		}
	}

	// Clear arrays of any data from the previous roll
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

					//Failsafe delegate in case something goes wrong and the dice don't stop moving within a reasonable time frame
					SpawnedDie->OnFailsafeDestroy.AddDynamic(this, &UDiceSelectorManager::OnDiceFailsafeHandler);
				}
			}

			Selector->ResetCount();
		}
	}

	if (!SpawnedDice.IsEmpty())
	{
		bRollInProgress = true;
		UpdateRollButtonState();

		for (auto Dice : SpawnedDice)
		{
			//Roll the dice
			Dice->Roll(GetRandomizedVector(Impulse, ImpulseRange, false),
					   GetRandomizedVector(AngularImpulse, AngularImpulseRange, true));
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

		bRollInProgress = false;
		UpdateRollButtonState();

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

void UDiceSelectorManager::OnDiceFailsafeHandler(EDiceType DiceType)
{
	ExpectedDiceCount--;
	OnDiceFailsafeDestroyed.Broadcast(DiceType);

	if (ExpectedDiceCount > 0 && PendingResults.Num() == ExpectedDiceCount)
	{
		OnAllDiceRolled.Broadcast(PendingResults);

		bRollInProgress = false;
		UpdateRollButtonState();

		GetWorld()->GetTimerManager().SetTimer(
			DestroyDiceTimerHandle,
			this,
			&UDiceSelectorManager::DestroyDice,
			TimeBeforeDestroyingDice,
			false);

		PendingResults.Empty();
		ExpectedDiceCount = 0;
	}
}

void UDiceSelectorManager::OnSelectorCountChanged()
{
	UpdateRollButtonState();
}

void UDiceSelectorManager::UpdateRollButtonState()
{
	bool bAnyDiceSelected = Selectors.ContainsByPredicate([](UDiceSelector* S)
	{
		return S && S->DiceCount > 0;
	});

	RollButton->SetIsEnabled(!bRollInProgress && bAnyDiceSelected);
}

FVector UDiceSelectorManager::GetRandomizedVector(const FVector& BaseVector, const float& Range, bool bUseZAxis)
{
	float Z = bUseZAxis ? FMath::FRandRange(-Range, Range) : 0.f;
	return BaseVector + FVector(FMath::FRandRange(-Range, Range), 
								FMath::FRandRange(-Range, Range), 
								Z);
}
