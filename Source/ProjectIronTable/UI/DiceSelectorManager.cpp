// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceSelectorManager.h"
#include "DiceSelector.h"
#include "Kismet/KismetMathLibrary.h"

// Populates selector and button arrays, binds all button delegates, and refreshes initial button states.
void UDiceSelectorManager::NativeConstruct()
{
	Super::NativeConstruct();

	Selectors = { D4, D6, D8, D10, D12, D20, D100 };
	AdvantageButtons = { NormalRollButton, AdvantageRollButton, DisadvantageRollButton };
	RollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::RollDice);
	NormalRollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::OnNormalClicked);
	AdvantageRollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::OnAdvantageClicked);
	DisadvantageRollButton->OnClicked.AddDynamic(this, &UDiceSelectorManager::OnDisadvantageClicked);

	for (auto Selector : Selectors)
	{
		Selector->OnCountChanged.AddDynamic(this, &UDiceSelectorManager::OnSelectorCountChanged);
	}

	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}

// Spawns and launches all selected dice, clearing leftover dice from the previous roll first.
void UDiceSelectorManager::RollDice()
{
	if (!IsValid(SpawnVolume))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiceSelectorManager: SpawnVolume is not set!"));
		bRollInProgress = false;
		return;
	}

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

	OnRollInitiated.Broadcast();

	// Clear arrays of any data from the previous roll
	SpawnedDice.Empty();
	PendingResults.Empty();
	ExpectedDiceCount = 0;
	bRollInProgress = true;

	for (auto Selector : Selectors)
	{
		if (Selector->DiceCount > 0)
		{
			bool bAdvantageRoll = RollMode != EDiceRollMode::Normal;
			int32 SpawnCount = (bAdvantageRoll) ? 2 : Selector->DiceCount;

			for (int i = 0; i < SpawnCount; i++)
			{
				FRotator RandomRot = UKismetMathLibrary::RandomRotator(true);

				FVector SpawnPoint = FMath::RandPointInBox(SpawnVolume->GetSpawnBox());

				FTransform T(
					FQuat(RandomRot),
					SpawnPoint,
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
		UpdateRollButtonState();
		UpdateAdvantageButtonState();

		for (auto Dice : SpawnedDice)
		{
			//Roll the dice
			Dice->Roll(GetRandomizedVector(Impulse, ImpulseRange, false),
					   GetRandomizedVector(AngularImpulse, AngularImpulseRange, true));
		}
	}
	else
	{
		bRollInProgress = false;
		UpdateRollButtonState();
		UpdateAdvantageButtonState();
	}
}

// Accumulates results; once all dice report in, resolves advantage/disadvantage and broadcasts OnAllDiceRolled.
void UDiceSelectorManager::OnDiceRolledHandler(FRollResult Result)
{
	PendingResults.Add(Result);

	// Check if all dice have finished rolling
	if (PendingResults.Num() == ExpectedDiceCount)
	{
		if (RollMode != EDiceRollMode::Normal && PendingResults.Num() == 2)
		{
			bool bKeepFirst = (RollMode == EDiceRollMode::Advantage)
				? PendingResults[0].Value >= PendingResults[1].Value
				: PendingResults[0].Value <= PendingResults[1].Value;

			int32 LoserIndex = bKeepFirst ? 1 : 0;
			int32 KeeperIndex = bKeepFirst ? 0 : 1;

			if (IsValid(PendingResults[LoserIndex].DiceActor))
			{
				PendingResults[LoserIndex].DiceActor->bWasKept = false;
			}

			TArray<FRollResult> KeptResult = { PendingResults[KeeperIndex] };
			OnAllDiceRolled.Broadcast(KeptResult, RollMode);
		}
		else
		{
			// Debug: log each result to the output log
			for (const FRollResult& RollResult : PendingResults)
			{
				FString DiceTypeName = UEnum::GetValueAsString(RollResult.DiceType);
				UE_LOG(LogTemp, Display, TEXT("Type: %s | Value: %d"), *DiceTypeName, RollResult.Value);
			}

			// Broadcast all results at once
			OnAllDiceRolled.Broadcast(PendingResults, RollMode);
		}

		bRollInProgress = false;
		UpdateRollButtonState();
		UpdateAdvantageButtonState();

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

// Destroys all actors in SpawnedDice and empties the array.
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

// Decrements the expected count and finalises the roll early if remaining dice have all reported results.
void UDiceSelectorManager::OnDiceFailsafeHandler(EDiceType DiceType)
{
	ExpectedDiceCount--;
	OnDiceFailsafeDestroyed.Broadcast(DiceType);

	if (ExpectedDiceCount > 0 && PendingResults.Num() == ExpectedDiceCount)
	{
		OnAllDiceRolled.Broadcast(PendingResults, RollMode);

		bRollInProgress = false;
		UpdateRollButtonState();
		UpdateAdvantageButtonState();

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

// Refreshes roll and advantage button states whenever any selector count changes.
void UDiceSelectorManager::OnSelectorCountChanged()
{
	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}

// Sets roll mode to Normal and refreshes advantage button states.
void UDiceSelectorManager::OnNormalClicked()
{
	RollMode = EDiceRollMode::Normal;
	UpdateAdvantageButtonState();
}

// Sets roll mode to Advantage and refreshes advantage button states.
void UDiceSelectorManager::OnAdvantageClicked()
{
	RollMode = EDiceRollMode::Advantage;
	UpdateAdvantageButtonState();
}

// Sets roll mode to Disadvantage and refreshes advantage button states.
void UDiceSelectorManager::OnDisadvantageClicked()
{
	RollMode = EDiceRollMode::Disadvantage;
	UpdateAdvantageButtonState();
}

// Enables the roll button only when at least one die is selected and no roll is in progress.
void UDiceSelectorManager::UpdateRollButtonState()
{
	bool bAnyDiceSelected = Selectors.ContainsByPredicate([](UDiceSelector* S)
	{
		return S && S->DiceCount > 0;
	});

	RollButton->SetIsEnabled(!bRollInProgress && bAnyDiceSelected);
}

// Enables advantage buttons only when exactly one die total is selected; disables the currently active mode button.
void UDiceSelectorManager::UpdateAdvantageButtonState()
{
	int32 TotalDiceCount = 0;
	bool bAnyDiceSelected = true;
	for (auto Selector : Selectors)
	{
		if (Selector->DiceCount > 1)
		{
			bAnyDiceSelected = false;
			break;
		}

		if (Selector->DiceCount == 1)
		{
			TotalDiceCount++;
			if (TotalDiceCount > 1)
			{
				bAnyDiceSelected = false;
				break;
			}
		}
	}

	if (!bRollInProgress &&
		bAnyDiceSelected &&
		TotalDiceCount == 1)
	{
		for (auto Button : AdvantageButtons)
		{
			Button->SetIsEnabled(true);
		}

		switch (RollMode)
		{
		case EDiceRollMode::Normal:
			NormalRollButton->SetIsEnabled(false);
			break;
		case EDiceRollMode::Advantage:
			AdvantageRollButton->SetIsEnabled(false);
			break;
		case EDiceRollMode::Disadvantage:
			DisadvantageRollButton->SetIsEnabled(false);
			break;
		default:
			break;
		}
	}
	else
	{
		if (!bRollInProgress)
		{
			RollMode = EDiceRollMode::Normal;
		}
		for (auto Button : AdvantageButtons)
		{
			Button->SetIsEnabled(false);
		}
	}
}

// Adds a uniform random offset within [-Range, Range] to each component of the base vector.
FVector UDiceSelectorManager::GetRandomizedVector(const FVector& BaseVector, const float& Range, bool bUseZAxis)
{
	float Z = bUseZAxis ? FMath::FRandRange(-Range, Range) : 0.f;
	return BaseVector + FVector(FMath::FRandRange(-Range, Range),
								FMath::FRandRange(-Range, Range),
								Z);
}
