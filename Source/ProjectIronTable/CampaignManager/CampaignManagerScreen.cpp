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

// Loads campaign data (real or fake), builds game type tabs, and populates the grid with the first available game type's campaigns.
void UCampaignManagerScreen::Init()
{
	if (bUseFakeData)
	{
		CampaignData = NewObject<UCampaignManagerSave>(this);
		CampaignData->CampaignRecords = BuildFakeData();
	}
	else
	{
		if (!UGameplayStatics::DoesSaveGameExist(UCampaignManagerSave::SaveSlotName, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::Init — No Campaign Manager save found"));
			return;
		}

		USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(UCampaignManagerSave::SaveSlotName, 0);
		CampaignData = Cast<UCampaignManagerSave>(LoadedSave);
		if (!IsValid(CampaignData))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::Init — Failed to cast loaded save to UCampaignManagerSave"));
			return;
		}
	}

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
		TypeButton->SetTabColors(SelectedTabColor, UnselectedTabColor);

		if (Game.Value.Campaigns.Num() > 0)
		{
			TypeButton->SetInteractable(true);
			ActiveButtons.Add(TypeButton);

			if (!bGridPopulated)
			{
				SelectedGameType = Game.Key;
				PopulateCampaigns(Game.Value.Campaigns, SelectedGameType);
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

	SetSelectedGameButton();
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

// Returns a hardcoded map of campaign records for testing.
TMap<FString, FCampaignList> UCampaignManagerScreen::BuildFakeData() const
{
	TMap<FString, FCampaignList> Records;

	FCampaignList DnDList;
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("The Lost Mines"),   FDateTime(2026, 3, 15), 4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Curse of Strahd"),  FDateTime(2026, 4, 1),  5));
	Records.Add(TEXT("DnD5e"), DnDList);

	FCampaignList PathfinderList;
	PathfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Age of Ashes"), FDateTime(2026, 2, 20), 3));
	Records.Add(TEXT("Pathfinder2e"), PathfinderList);

	Records.Add(TEXT("CallOfCthulhu"), FCampaignList());
	Records.Add(TEXT("Starfinder"),    FCampaignList());

	return Records;
}

// Iterates all active buttons and sets their selected state based on SelectedGameType.
void UCampaignManagerScreen::SetSelectedGameButton()
{
	for (UGameTypeButton* Button : ActiveButtons)
	{
		if (IsValid(Button))
		{
			Button->SetSelected(Button->GetLabel() == SelectedGameType);
		}
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
		SetSelectedGameButton();
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

// Broadcasts OnBackRequested to signal the parent to return to the home screen.
void UCampaignManagerScreen::OnBackClicked()
{
	OnBackRequested.Broadcast();
}
