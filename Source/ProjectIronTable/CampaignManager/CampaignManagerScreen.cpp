// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "CampaignManagerScreen.h"

#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"

#include "GameTypeButton.h"
#include "CampaignCard.h"

// Binds the back button click delegate.
void UCampaignManagerScreen::NativeConstruct()
{
	Super::NativeConstruct();
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UCampaignManagerScreen::OnBackClicked);
	}
}

// Loads the campaign save, builds game type tabs, and populates the grid with the first available game type's campaigns.
void UCampaignManagerScreen::Init()
{
	if (UGameplayStatics::DoesSaveGameExist(UCampaignManagerSave::SaveSlotName, 0))
	{
		USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(UCampaignManagerSave::SaveSlotName, 0);
		CampaignData = Cast<UCampaignManagerSave>(LoadedSave);
		if (IsValid(CampaignData))
		{
			bool bGridPopulated = false;
			for (const TPair<FString, FCampaignList>& Game : CampaignData->CampaignRecords)
			{
				UGameTypeButton* TypeButton = CreateWidget<UGameTypeButton>(GetWorld(), GameTypeButtonClass);
				if (!IsValid(TypeButton))
				{
					UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::Init — Failed to create GameTypeButton for %s"), *Game.Key);
					continue;
				}

				TypeButton->SetLabel(Game.Key);
				if (Game.Value.Campaigns.Num() > 0)
				{
					TypeButton->SetInteractable(true);

					if (!bGridPopulated)
					{
						SelectedGameType = Game.Key;
						PopulateCampaigns(Game.Value.Campaigns, Game.Key);
						bGridPopulated = true;
					}
				}
				else
				{
					TypeButton->SetInteractable(false);
				}

				TypeButton->OnGameTypeSelected.AddDynamic(this, &UCampaignManagerScreen::OnGameTypeSelected);
				GameTypeTabBar->AddChild(TypeButton);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::Init — Failed to cast loaded save to UCampaignManagerSave"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::Init — No Campaign Manager save found"));
	}
}

// Clears the campaign grid and creates a card for each record in the given list.
void UCampaignManagerScreen::PopulateCampaigns(const TArray<FCampaignRecord>& Campaigns, const FString& GameType)
{
	CampaignGrid->ClearChildren();
	for (const FCampaignRecord& Campaign : Campaigns)
	{
		UCampaignCard* CampaignCard = CreateWidget<UCampaignCard>(GetWorld(), CampaignCardClass);
		if (!IsValid(CampaignCard))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::PopulateCampaigns — Failed to create CampaignCard"));
			continue;
		}

		CampaignCard->SetCampaignTitle(Campaign.CampaignName);
		CampaignCard->SetLastPlayedDate(Campaign.LastPlayed.ToString());
		CampaignCard->SetNumberOfPlayers(Campaign.NumberOfPlayers);
		CampaignCard->SetCampaignData(Campaign.CampaignID, GameType);
		CampaignCard->OnCampaignSelected.AddDynamic(this, &UCampaignManagerScreen::OnCampaignSelected);

		CampaignGrid->AddChild(CampaignCard);
	}
}

// Updates the selected game type and refreshes the campaign grid.
void UCampaignManagerScreen::OnGameTypeSelected(const FString& GameType)
{
	if (!IsValid(CampaignData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::OnGameTypeSelected — CampaignData is null"));
		return;
	}

	SelectedGameType = GameType;
	if (const FCampaignList* CampaignList = CampaignData->CampaignRecords.Find(GameType))
	{
		PopulateCampaigns(CampaignList->Campaigns, GameType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::OnGameTypeSelected — Game type not found in campaign records"));
	}
}

// Placeholder — launches the selected campaign with the given ID and game type.
void UCampaignManagerScreen::OnCampaignSelected(const FGuid& CampaignID, const FString& GameType)
{
}

// Broadcasts OnCampaignBackRequested to signal the parent to return to the home screen.
void UCampaignManagerScreen::OnBackClicked()
{
	OnBackRequested.Broadcast();
}
