// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceRollComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "DiceSpawnVolume.h"

// Sets default values for this component's properties
UDiceRollComponent::UDiceRollComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Collects a result and broadcasts OnRollComplete once all expected results have arrived, applying advantage/disadvantage keep-discard first.
void UDiceRollComponent::OnDiceRolledHandler(FRollResult Result)
{
	PendingResults.Add(Result);

	if (PendingResults.Num() == ExpectedDiceCount)
	{
		if (ActiveRollMode != EDiceRollMode::Normal && PendingResults.Num() == 2)
		{
			bool bKeepFirst = (ActiveRollMode == EDiceRollMode::Advantage)
				? PendingResults[0].Value >= PendingResults[1].Value
				: PendingResults[0].Value <= PendingResults[1].Value;

			int32 LoserIndex = bKeepFirst ? 1 : 0;
			int32 KeeperIndex = bKeepFirst ? 0 : 1;

			if (IsValid(PendingResults[LoserIndex].DiceActor))
			{
				PendingResults[LoserIndex].DiceActor->bWasKept = false;
			}

			TArray<FRollResult> KeptResult = { PendingResults[KeeperIndex] };
			OnRollComplete.Broadcast(KeptResult, ActiveRollMode);
		}
		else
		{
			for (const FRollResult& RollResult : PendingResults)
			{
				FString DiceTypeName = UEnum::GetValueAsString(RollResult.DiceType);
				UE_LOG(LogTemp, Display, TEXT("Type: %s | Value: %d"), *DiceTypeName, RollResult.Value);
			}

			OnRollComplete.Broadcast(PendingResults, ActiveRollMode);
		}

		FinalizeRoll();
	}
}

// Destroys all actors in SpawnedDice and empties the array.
void UDiceRollComponent::DestroyDice()
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

// Decrements the expected count and finalises the roll early once all remaining dice have reported.
void UDiceRollComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	ExpectedDiceCount--;
	OnDiceFailsafeDestroyed.Broadcast(DiceType);

	if (PendingResults.Num() == ExpectedDiceCount)
	{
		if (ExpectedDiceCount > 0)
		{
			OnRollComplete.Broadcast(PendingResults, ActiveRollMode);
		}
		FinalizeRoll();
	}
}

// Adds a uniform random offset within [-Range, Range] to each component of the base vector.
FVector UDiceRollComponent::GetRandomizedVector(const FVector& BaseVector, const float& Range, bool bUseZAxis)
{
	float Z = bUseZAxis ? FMath::FRandRange(-Range, Range) : 0.f;
	return BaseVector + FVector(FMath::FRandRange(-Range, Range),
		FMath::FRandRange(-Range, Range),
		Z);
}

// Tears down roll state and schedules deferred destruction of the settled dice.
void UDiceRollComponent::FinalizeRoll()
{
	bRollInProgress = false;

	if (UWorld* TimerWorld = GetWorld())
	{
		TimerWorld->GetTimerManager().SetTimer(
			DestroyDiceTimerHandle,
			this,
			&UDiceRollComponent::DestroyDice,
			TimeBeforeDestroyingDice,
			false);
	}

	PendingResults.Empty();
	ExpectedDiceCount = 0;
}

// Stashes the roll mode, then spawns and launches the dice described by Request, clearing leftover dice from the previous roll first.
void UDiceRollComponent::RollDice(const FDiceRollRequest& Request)
{
	if (!IsValid(SpawnVolume))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiceRollComponent::RollDice - SpawnVolume is not set."));
		bRollInProgress = false;
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiceRollComponent::RollDice - GetWorld() returned null."));
		return;
	}

	World->GetTimerManager().ClearTimer(DestroyDiceTimerHandle);

	for (auto Dice : SpawnedDice)
	{
		if (IsValid(Dice))
		{
			Dice->Destroy();
		}
	}

	OnRollInitiated.Broadcast();

	SpawnedDice.Empty();
	PendingResults.Empty();
	ExpectedDiceCount = 0;
	bRollInProgress = true;
	ActiveRollMode = Request.Mode;

	for (auto Die : Request.Dice)
	{
		if (Die.Count > 0)
		{
			bool bAdvantageRoll = ActiveRollMode != EDiceRollMode::Normal;
			int32 SpawnCount = (bAdvantageRoll) ? 2 : Die.Count;

			for (int i = 0; i < SpawnCount; i++)
			{
				FRotator RandomRot = UKismetMathLibrary::RandomRotator(true);
				FVector SpawnPoint = FMath::RandPointInBox(SpawnVolume->GetSpawnBox());
				FTransform T(FQuat(RandomRot), SpawnPoint, FVector::OneVector);

				ABaseDiceActor* SpawnedDie = World->SpawnActor<ABaseDiceActor>(Die.DiceClass, T);

				if (IsValid(SpawnedDie))
				{
					SpawnedDice.Add(SpawnedDie);
					ExpectedDiceCount++;

					// Failsafe delegate in case something goes wrong and the dice don't stop moving within a reasonable time frame
					SpawnedDie->OnDiceRolled.AddDynamic(this, &UDiceRollComponent::OnDiceRolledHandler);
					SpawnedDie->OnFailsafeDestroy.AddDynamic(this, &UDiceRollComponent::OnDiceFailsafeHandler);
				}
			}
		}
	}

	if (!SpawnedDice.IsEmpty())
	{
		for (auto Dice : SpawnedDice)
		{
			Dice->Roll(GetRandomizedVector(Impulse, ImpulseRange, false),
				GetRandomizedVector(AngularImpulse, AngularImpulseRange, true));
		}
	}
	else
	{
		bRollInProgress = false;
	}
}
