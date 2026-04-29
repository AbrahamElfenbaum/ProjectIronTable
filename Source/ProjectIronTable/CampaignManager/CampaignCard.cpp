// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "CampaignCard.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

// Broadcasts OnCampaignSelected with the stored campaign ID and game type.
void UCampaignCard::OnLaunchCampaignButtonClicked()
{
	OnCampaignSelected.Broadcast(CampaignID, GameType);
}

// Binds the launch button click delegate.
void UCampaignCard::NativeConstruct()
{
	Super::NativeConstruct();
	if (LaunchCampaignButton)
	{
		LaunchCampaignButton->OnClicked.AddDynamic(this, &UCampaignCard::OnLaunchCampaignButtonClicked);
	}
}

// Updates the campaign name label.
void UCampaignCard::SetCampaignTitle(const FString& Title)
{
	if (CampaignTitle)
	{
		CampaignTitle->SetText(FText::FromString(Title));
	}
}

// Updates the last played date label.
void UCampaignCard::SetLastPlayedDate(const FString& Date)
{
	if (LastPlayedDate)
	{
		LastPlayedDate->SetText(FText::FromString(Date));
	}
}

// Updates the player count label.
void UCampaignCard::SetNumberOfPlayers(int32 NumPlayers)
{
	if (NumberOfPlayers)
	{
		NumberOfPlayers->SetText(FText::AsNumber(NumPlayers));
	}
}

// Stores the campaign ID and game type for broadcast when the card is selected.
void UCampaignCard::SetCampaignData(const FGuid& InCampaignID, const FString& InGameType)
{
	CampaignID = InCampaignID;
	GameType = InGameType;
}
