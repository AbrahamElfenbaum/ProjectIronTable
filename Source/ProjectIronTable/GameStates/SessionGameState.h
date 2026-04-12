// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SessionGameState.generated.h"

/**
 * Runtime session state replicated to all clients. Mirrors USessionSave at runtime — loaded from disk
 * on session start, written back on save. Holds role assignments (Server Owner, GMs, players) and
 * session identity fields. Server is authoritative; clients read via accessors (to be added).
 */
UCLASS()
class PROJECTIRONTABLE_API ASessionGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	/** Registers replicated properties with the Unreal replication system. */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Game system this session belongs to (e.g. "DnD5e"). Matches the key used in UCampaignManagerSave. */
	UPROPERTY(Replicated)
	FString GameTypeID;

	/** ID of the campaign this session belongs to. */
	UPROPERTY()
	FGuid CampaignID;

	/** Unique ID for this session. Used to build the save slot name: "Session_{SessionID}". */
	UPROPERTY()
	FGuid SessionID;

	/** Player ID of the Server Owner — the machine running the listen server. One per session, non-transferable. */
	UPROPERTY(Replicated)
	FGuid HostPlayerID;

	/** Player IDs of all active GMs. Supports multiple simultaneous GMs. Transferable; default = campaign creator. */
	UPROPERTY(Replicated)
	TArray<FGuid> GMPlayerIDs;

	/** Player IDs of all non-GM players in this session. */
	UPROPERTY(Replicated)
	TArray<FGuid> PlayerIDs;

	/** Timestamp of the last save. Used to sort sessions when loading a campaign (most recent first). */
	UPROPERTY()
	FDateTime LastSaved;

public:

	/** Returns the game system identifier for this session (e.g. "DnD5e"). */
	FString GetGameTypeID() const;

	/** Sets the game system identifier for this session. */
	void SetGameTypeID(const FString& InGameTypeID);

	/** Returns the ID of the campaign this session belongs to. */
	FGuid GetCampaignID() const;

	/** Sets the campaign ID for this session. */
	void SetCampaignID(const FGuid& InCampaignID);

	/** Returns the unique ID for this session. */
	FGuid GetSessionID() const;

	/** Sets the unique ID for this session. */
	void SetSessionID(const FGuid& InSessionID);

	/** Returns the player ID of the Server Owner. */
	FGuid GetHostPlayerID() const;

	/** Sets the player ID of the Server Owner. */
	void SetHostPlayerID(const FGuid& InHostPlayerID);

	/** Returns the player IDs of all active GMs. */
	const TArray<FGuid>& GetGMPlayerIDs() const;

	/** Sets the full list of GM player IDs. */
	void SetGMPlayerIDs(const TArray<FGuid>& InGMPlayerIDs);

	/** Returns the player IDs of all non-GM players in this session. */
	const TArray<FGuid>& GetPlayerIDs() const;

	/** Sets the full list of non-GM player IDs. */
	void SetPlayerIDs(const TArray<FGuid>& InPlayerIDs);

	/** Returns the timestamp of the last save. */
	FDateTime GetLastSaved() const;

	/** Sets the timestamp of the last save. */
	void SetLastSaved(const FDateTime& InLastSaved);
};
