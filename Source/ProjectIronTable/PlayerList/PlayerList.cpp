// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerList.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#include "PlayerRow.h"

// Toggles the expanded state, repopulating the list when expanding.
void UPlayerList::OnToggleButtonClicked()
{
	bIsExpanded = !bIsExpanded;
	if (bIsExpanded)
	{
		PopulateList();
	}
	if (ScrollBox)
	{
		ScrollBox->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// Forwards the player name upward through OnAddressClicked.
void UPlayerList::OnPlayerAddressClicked(const FString& PlayerName)
{
	OnAddressClicked.Broadcast(PlayerName);
}

// Binds the toggle button, populates the list, and collapses the scroll box initially.
void UPlayerList::NativeConstruct()
{
	Super::NativeConstruct();
	if (ToggleButton)
	{
		ToggleButton->OnClicked.AddDynamic(this, &UPlayerList::OnToggleButtonClicked);
	}
	PopulateList();
	if (ScrollBox)
	{
		ScrollBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Clears the scroll box and rebuilds it from the current game state's player array.
void UPlayerList::PopulateList()
{
	if (!ScrollBox)
	{
		return;
	}
	ScrollBox->ClearChildren();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlayerList::PopulateList — GetWorld() returned null."));
		return;
	}

	AGameStateBase* GS = World->GetGameState<AGameStateBase>();
	if (!IsValid(GS) || !PlayerRowClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPlayerList::PopulateList — GameState is invalid or PlayerRowClass is not set"));
		return;
	}

	for (APlayerState* Player : GS->PlayerArray)
	{
		if (!IsValid(Player))
		{
			continue;
		}
		UPlayerRow* PlayerRow = CreateWidget<UPlayerRow>(this, PlayerRowClass);
		if (!IsValid(PlayerRow))
		{
			UE_LOG(LogTemp, Warning, TEXT("UPlayerList::PopulateList — Failed to create PlayerRow."));
			continue;
		}
		PlayerRow->SetPlayerName(Player->GetPlayerName());
		PlayerRow->OnAddressClicked.AddDynamic(this, &UPlayerList::OnPlayerAddressClicked);
		ScrollBox->AddChild(PlayerRow);
	}
}
