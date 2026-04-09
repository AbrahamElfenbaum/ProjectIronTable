// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerList.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"

// Binds the toggle button, populates the list, and collapses the scroll box initially.
void UPlayerList::NativeConstruct()
{
	Super::NativeConstruct();
	ToggleButton->OnClicked.AddDynamic(this, &UPlayerList::OnToggleButtonClicked);
	PopulateList();
	ScrollBox->SetVisibility(ESlateVisibility::Collapsed);
}

// Clears the scroll box and rebuilds it from the current game state's player array.
void UPlayerList::PopulateList()
{
	ScrollBox->ClearChildren();

	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
	if (!GS || !PlayerRowClass) return;

	for (auto Player : GS->PlayerArray)
	{
		UPlayerRow* PlayerRow = CreateWidget<UPlayerRow>(GetWorld(), PlayerRowClass);
		PlayerRow->SetPlayerName(Player->GetPlayerName());
		PlayerRow->OnAddressClicked.AddDynamic(this, &UPlayerList::OnPlayerAddressClicked);
		ScrollBox->AddChild(PlayerRow);
	}
}

// Toggles the expanded state, repopulating the list when expanding.
void UPlayerList::OnToggleButtonClicked()
{
	bIsExpanded = !bIsExpanded;
	if (bIsExpanded)
	{
		PopulateList();
	}
	ScrollBox->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

// Forwards the player name upward through OnAddressClicked.
void UPlayerList::OnPlayerAddressClicked(const FString& PlayerName)
{
	OnAddressClicked.Broadcast(PlayerName);
}
