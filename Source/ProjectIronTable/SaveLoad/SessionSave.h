// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SessionSave.generated.h"

/**
 * Per-session save file. One instance per game session, stored under slot "Session_{SessionID}".
 * UCampaignManagerSave is the authoritative index — FCampaignRecord.SessionIDs lists all sessions
 * for a campaign. To open a campaign, read its SessionIDs and load the most recent by LastSaved.
 *
 * Roles: one Server Owner (HostPlayerID), one or more GMs (GMPlayerIDs), remaining players (PlayerIDs).
 * Default GM on creation = campaign creator. GM role is transferable at any time.
 */
UCLASS()
class PROJECTIRONTABLE_API USessionSave : public USaveGame
{
	GENERATED_BODY()

public:

	/** Game system this session belongs to (e.g. "DnD5e"). Matches the key used in UCampaignManagerSave. */
	UPROPERTY()
	FString GameTypeID;

	/** ID of the campaign this session belongs to. */
	UPROPERTY()
	FGuid CampaignID;

	/** Unique ID for this session. Used to build the save slot name: "Session_{SessionID}". */
	UPROPERTY()
	FGuid SessionID;

	/** Player ID of the Server Owner — the machine running the listen server. One per session, non-transferable. */
	UPROPERTY()
	FGuid HostPlayerID;

	/** Player IDs of all active GMs. Supports multiple simultaneous GMs. Transferable; default = campaign creator. */
	UPROPERTY()
	TArray<FGuid> GMPlayerIDs;

	/** Player IDs of all non-GM players in this session. */
	UPROPERTY()
	TArray<FGuid> PlayerIDs;

	/** Timestamp of the last save. Used to sort sessions when loading a campaign (most recent first). */
	UPROPERTY()
	FDateTime LastSaved;
};
