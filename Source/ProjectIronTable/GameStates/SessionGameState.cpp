// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionGameState.h"

#include "Net/UnrealNetwork.h"

// Registers GameTypeID, HostPlayerID, GMPlayerIDs, and PlayerIDs for replication.
void ASessionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASessionGameState, GameTypeID);
	DOREPLIFETIME(ASessionGameState, HostPlayerID);
	DOREPLIFETIME(ASessionGameState, GMPlayerIDs);
	DOREPLIFETIME(ASessionGameState, PlayerIDs);
}

// Returns the game system identifier.
FString ASessionGameState::GetGameTypeID() const
{
	return GameTypeID;
}

// Sets the game system identifier.
void ASessionGameState::SetGameTypeID(const FString& InGameTypeID)
{
	GameTypeID = InGameTypeID;
}

// Returns the campaign ID.
FGuid ASessionGameState::GetCampaignID() const
{
	return CampaignID;
}

// Sets the campaign ID.
void ASessionGameState::SetCampaignID(const FGuid& InCampaignID)
{
	CampaignID = InCampaignID;
}

// Returns the session ID.
FGuid ASessionGameState::GetSessionID() const
{
	return SessionID;
}

// Sets the session ID.
void ASessionGameState::SetSessionID(const FGuid& InSessionID)
{
	SessionID = InSessionID;
}

// Returns the host player ID.
FGuid ASessionGameState::GetHostPlayerID() const
{
	return HostPlayerID;
}

// Sets the host player ID.
void ASessionGameState::SetHostPlayerID(const FGuid& InHostPlayerID)
{
	HostPlayerID = InHostPlayerID;
}

// Returns the GM player ID list by const reference.
const TArray<FGuid>& ASessionGameState::GetGMPlayerIDs() const
{
	return GMPlayerIDs;
}

// Sets the full GM player ID list.
void ASessionGameState::SetGMPlayerIDs(const TArray<FGuid>& InGMPlayerIDs)
{
	GMPlayerIDs = InGMPlayerIDs;
}

// Returns the non-GM player ID list by const reference.
const TArray<FGuid>& ASessionGameState::GetPlayerIDs() const
{
	return PlayerIDs;
}

// Sets the full non-GM player ID list.
void ASessionGameState::SetPlayerIDs(const TArray<FGuid>& InPlayerIDs)
{
	PlayerIDs = InPlayerIDs;
}

// Returns the last saved timestamp.
FDateTime ASessionGameState::GetLastSaved() const
{
	return LastSaved;
}

// Sets the last saved timestamp.
void ASessionGameState::SetLastSaved(const FDateTime& InLastSaved)
{
	LastSaved = InLastSaved;
}
