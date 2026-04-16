// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "CampaignManagerScreen.h"

#include "Kismet/GameplayStatics.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"

#include "GameTypeButton.h"
#include "CampaignCard.h"
#include "SessionController.h"
#include "SessionInstance.h"
#include "MacroLibrary.h"

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
		CHECK_IF_VALID(CampaignData, );
	}

	bool bGridPopulated = false;
	for (const TPair<FString, FCampaignList>& Game : CampaignData->CampaignRecords)
	{
		UGameTypeButton* TypeButton = CreateWidget<UGameTypeButton>(this, GameTypeButtonClass);
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
		UCampaignCard* CampaignCard = CreateWidget<UCampaignCard>(this, CampaignCardClass);
		if (!IsValid(CampaignCard))
		{
			UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::PopulateCampaigns — Failed to create CampaignCard"));
			continue;
		}

		CampaignCard->SetCampaignTitle(Campaign.CampaignName);
		CampaignCard->SetLastPlayedDate(Campaign.LastPlayed.ToString(TEXT("%Y-%m-%d")));
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
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("The Lost Mines"),         FDateTime(2026, 3, 15), 4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Curse of Strahd"),        FDateTime(2026, 4, 1),  5));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Tomb of Annihilation"),   FDateTime(2026, 1, 10), 3));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Dragon of Icespire Peak"),FDateTime(2025, 12, 5), 2));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Descent into Avernus"),   FDateTime(2026, 2, 28), 6));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Waterdeep: Dragon Heist"),FDateTime(2025, 11, 20),4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Out of the Abyss"),       FDateTime(2026, 3, 30), 5));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Ghosts of Saltmarsh"),    FDateTime(2026, 4, 5),  3));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Rime of the Frostmaiden"),FDateTime(2026, 1, 22), 4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Stormwreck Isle"),          FDateTime(2026, 4, 8),  2));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Princes of the Apocalypse"),FDateTime(2026, 2, 11), 5));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Storm King's Thunder"),     FDateTime(2026, 1, 3),  4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Dungeon of the Mad Mage"),  FDateTime(2025, 11, 8), 3));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Tales from the Yawning Portal"), FDateTime(2025, 10, 15), 4));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Candlekeep Mysteries"),     FDateTime(2026, 3, 21), 2));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Spelljammer: Light of Xaryxis"), FDateTime(2026, 2, 1), 5));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Keys from the Golden Vault"),FDateTime(2026, 3, 27), 3));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Dragonlance: Shadow of the Dragon Queen"), FDateTime(2026, 1, 18), 6));
	DnDList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("The Wild Beyond the Witchlight"), FDateTime(2025, 12, 28), 4));
	Records.Add(TEXT("DnD5e"), DnDList);

	FCampaignList PathfinderList;
	PathfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Age of Ashes"),      FDateTime(2026, 2, 20), 3));
	PathfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Extinction Curse"),  FDateTime(2026, 1, 14), 4));
	PathfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Abomination Vaults"),FDateTime(2026, 3, 9),  5));
	PathfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Kingmaker"),         FDateTime(2025, 12, 1), 6));
	Records.Add(TEXT("Pathfinder2e"), PathfinderList);

	FCampaignList CthulhuList;
	CthulhuList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Masks of Nyarlathotep"),FDateTime(2026, 3, 18), 3));
	CthulhuList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Shadows of Yog-Sothoth"),FDateTime(2026, 2, 7), 4));
	Records.Add(TEXT("CallOfCthulhu"), CthulhuList);

	FCampaignList StarfinderList;
	StarfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Dead Suns"),        FDateTime(2026, 1, 30), 4));
	StarfinderList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Against the Aeon Throne"),FDateTime(2026, 3, 3), 3));
	Records.Add(TEXT("Starfinder"), StarfinderList);

	FCampaignList VampireList;
	VampireList.Campaigns.Add(FCampaignRecord(FGuid::NewGuid(), TEXT("Chicago by Night"), FDateTime(2026, 4, 2), 5));
	Records.Add(TEXT("VtM"), VampireList);

	Records.Add(TEXT("Shadowrun"),   FCampaignList());
	Records.Add(TEXT("WFRP"),        FCampaignList());
	Records.Add(TEXT("Cyberpunk RED"),FCampaignList());
	Records.Add(TEXT("Mothership"),  FCampaignList());

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
	CHECK_IF_VALID(CampaignData, );

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
// TODO: ServerTravel must be called on the server, not the client. When this is fully implemented,
// route through a server RPC. Also replace "MapName" with the actual session level path.
void UCampaignManagerScreen::OnCampaignSelected(const FGuid& CampaignID, const FString& GameType)
{
	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(this));
	CHECK_IF_VALID(SessionInstance, );

	FGuid PlayerID = SessionInstance->GetPlayerID();
	if (!PlayerID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UCampaignManagerScreen::OnCampaignSelected — PlayerID is invalid"));
		return;
	}

	UWorld* World = GetWorld();
	CHECK_IF_VALID(World, );

	FString TravelURL = FString::Printf(TEXT("MapName?PlayerID=%s"), *PlayerID.ToString());

	GET_OWNING_PC(PC, );
	ASessionController* SessionPC = Cast<ASessionController>(PC);

	CHECK_IF_VALID(SessionPC, );
	SessionPC->Server_TravelToSession(TravelURL);
}

