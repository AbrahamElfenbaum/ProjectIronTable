// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "PlayerList.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void UPlayerList::NativeConstruct()
{
	Super::NativeConstruct();
	ToggleButton->OnClicked.AddDynamic(this, &UPlayerList::OnToggleButtonClicked);
	PopulateList();
	ScrollBox->SetVisibility(ESlateVisibility::Collapsed);
}

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

void UPlayerList::OnToggleButtonClicked()
{
	bIsExpanded = !bIsExpanded;
	if (bIsExpanded)
	{
		PopulateList();
	}
	ScrollBox->SetVisibility(bIsExpanded ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UPlayerList::OnPlayerAddressClicked(const FString& PlayerName)
{
	OnAddressClicked.Broadcast(PlayerName);
}
