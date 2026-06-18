// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "DiceTray.h"

#include "Components/Button.h"

#include "DiceRollComponent.h"
#include "DiceSelector.h"
#include "SessionController.h"

// Refreshes roll and advantage button states whenever any selector count changes.
void UDiceTray::OnSelectorCountChanged()
{
	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}

// Builds a roll request from the current selectors and roll mode, hands it to the dice roll component, then resets the selectors and refreshes button states.
void UDiceTray::OnRollClicked()
{
	if (!IsValid(DiceRollComponentRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiceTray::OnRollClicked - DiceRollComponentRef is not valid."));
		return;
	}

	FDiceRollRequest RollRequest;

	RollRequest.Mode = ActiveRollMode;

	for(auto Selector : Selectors)
	{
		if (Selector->DiceCount > 0)
		{
			FDiceTypeCount DiceTypeCount;

			DiceTypeCount.Count = Selector->DiceCount;
			DiceTypeCount.DiceClass = Selector->DiceClass;
			DiceTypeCount.DiceType = Selector->DiceType;

			RollRequest.Dice.Add(DiceTypeCount);

			Selector->ResetCount();
		}
	}

	bRollInProgress = true;
	DiceRollComponentRef->RollDice(RollRequest);

	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}

// Sets roll mode to Normal and refreshes advantage button states.
void UDiceTray::OnNormalClicked()
{
	ActiveRollMode = EDiceRollMode::Normal;
	UpdateAdvantageButtonState();
}

// Sets roll mode to Advantage and refreshes advantage button states.
void UDiceTray::OnAdvantageClicked()
{
	ActiveRollMode = EDiceRollMode::Advantage;
	UpdateAdvantageButtonState();
}

// Sets roll mode to Disadvantage and refreshes advantage button states.
void UDiceTray::OnDisadvantageClicked()
{
	ActiveRollMode = EDiceRollMode::Disadvantage;
	UpdateAdvantageButtonState();
}

// Clears the in-progress flag and refreshes button states once the roll completes.
void UDiceTray::OnRollCompleteHandler(TArray<FRollResult> Results, EDiceRollMode RollMode)
{
	bRollInProgress = false;
	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}

// Enables the roll button only when at least one die is selected and no roll is in progress.
void UDiceTray::UpdateRollButtonState()
{
	bool bAnyDiceSelected = Selectors.ContainsByPredicate([](UDiceSelector* S)
		{
			return S && S->DiceCount > 0;
		});

	RollButton->SetIsEnabled(!bRollInProgress && bAnyDiceSelected);
}

// Enables advantage buttons only when exactly one die total is selected; disables the currently active mode button.
void UDiceTray::UpdateAdvantageButtonState()
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

		switch (ActiveRollMode)
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
			ActiveRollMode = EDiceRollMode::Normal;
		}
		for (auto Button : AdvantageButtons)
		{
			Button->SetIsEnabled(false);
		}
	}
}

// Caches the dice roll component, populates selector and button arrays, binds all delegates, and refreshes initial button states.
void UDiceTray::NativeConstruct()
{
	Super::NativeConstruct();

	if (ASessionController* SC = Cast<ASessionController>(GetOwningPlayer()))
	{
		DiceRollComponentRef = SC->DiceRollComponent;
		if (IsValid(DiceRollComponentRef))
		{
			DiceRollComponentRef->OnRollComplete.AddDynamic(this, &UDiceTray::OnRollCompleteHandler);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UDiceTray::NativeConstruct - DiceRollComponentRef is not valid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiceTray::NativeConstruct - Could not get the session controller."));
	}

	Selectors = { D4, D6, D8, D10, D12, D20, D100 };
	AdvantageButtons = { NormalRollButton, AdvantageRollButton, DisadvantageRollButton };
	RollButton->OnClicked.AddDynamic(this, &UDiceTray::OnRollClicked);
	NormalRollButton->OnClicked.AddDynamic(this, &UDiceTray::OnNormalClicked);
	AdvantageRollButton->OnClicked.AddDynamic(this, &UDiceTray::OnAdvantageClicked);
	DisadvantageRollButton->OnClicked.AddDynamic(this, &UDiceTray::OnDisadvantageClicked);

	for (auto Selector : Selectors)
	{
		Selector->OnCountChanged.AddDynamic(this, &UDiceTray::OnSelectorCountChanged);
	}

	UpdateRollButtonState();
	UpdateAdvantageButtonState();
}
