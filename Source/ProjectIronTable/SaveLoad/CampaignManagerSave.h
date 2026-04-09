// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CampaignManagerSave.generated.h"

/** Data for a single campaign entry, including identity, display name, last session time, and player count. */
USTRUCT(BlueprintType)
struct FCampaignRecord
{
	GENERATED_BODY()

	/** Unique identifier for this campaign. Used to reliably locate the campaign regardless of name changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CampaignID;

	/** Display name shown to the campaign owner. Can be changed freely without affecting campaign lookup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CampaignName;

	/** Timestamp of the most recent session played for this campaign. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastPlayed;

	/** Number of players registered to this campaign. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfPlayers;

	FCampaignRecord():
		CampaignID(),
		CampaignName(TEXT("")),
		LastPlayed(),
		NumberOfPlayers(0)
	{
	}

	FCampaignRecord(FGuid InCampaignID, FString InCampaignName, FDateTime InLastPlayed, int32 InNumberOfPlayers):
		CampaignID(InCampaignID),
		CampaignName(InCampaignName),
		LastPlayed(InLastPlayed),
		NumberOfPlayers(InNumberOfPlayers)
	{
	}
};

/** Wrapper holding all campaigns registered under a single game type. Used as the TMap value to work around Unreal's nested container restriction. */
USTRUCT(BlueprintType)
struct FCampaignList
{
	GENERATED_BODY()

	/** All campaigns belonging to this game type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCampaignRecord> Campaigns;

	FCampaignList():
		Campaigns()
	{
	}
};

/**
 * Persists all campaign records across sessions. Campaigns are grouped by game type (registered via the game system plugin).
 * Each game type maps to a list of campaigns, allowing multiple campaigns per game. Saved to slot "CampaignManager", index 0.
 */
UCLASS()
class PROJECTIRONTABLE_API UCampaignManagerSave : public USaveGame
{
	GENERATED_BODY()

public:

	/** Slot name used when saving and loading this object via UGameplayStatics. */
	static const FString SaveSlotName;

	/** All campaigns keyed by game type name. Each entry holds the full list of campaigns for that game type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FCampaignList> CampaignRecords;

};
