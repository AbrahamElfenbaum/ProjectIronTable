// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionGameState.h"

#include "Net/UnrealNetwork.h"

// Register replicated properties
void ASessionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASessionGameState, GameTypeID);
	DOREPLIFETIME(ASessionGameState, HostPlayerID);
	DOREPLIFETIME(ASessionGameState, GMPlayerIDs);
	DOREPLIFETIME(ASessionGameState, PlayerIDs);
}

// Return the game system identifier
FString ASessionGameState::GetGameTypeID() const
{
	return GameTypeID;
}

// Set the game system identifier
void ASessionGameState::SetGameTypeID(const FString& InGameTypeID)
{
	GameTypeID = InGameTypeID;
}

// Return the campaign ID
FGuid ASessionGameState::GetCampaignID() const
{
	return CampaignID;
}

// Set the campaign ID
void ASessionGameState::SetCampaignID(const FGuid& InCampaignID)
{
	CampaignID = InCampaignID;
}

// Return the session ID
FGuid ASessionGameState::GetSessionID() const
{
	return SessionID;
}

// Set the session ID
void ASessionGameState::SetSessionID(const FGuid& InSessionID)
{
	SessionID = InSessionID;
}

// Return the host player ID
FGuid ASessionGameState::GetHostPlayerID() const
{
	return HostPlayerID;
}

// Set the host player ID
void ASessionGameState::SetHostPlayerID(const FGuid& InHostPlayerID)
{
	HostPlayerID = InHostPlayerID;
}

// Return the list of GM player IDs by reference
const TArray<FGuid>& ASessionGameState::GetGMPlayerIDs() const
{
	return GMPlayerIDs;
}

// Set the full list of GM player IDs
void ASessionGameState::SetGMPlayerIDs(const TArray<FGuid>& InGMPlayerIDs)
{
	GMPlayerIDs = InGMPlayerIDs;
}

// Return the list of non-GM player IDs by reference
const TArray<FGuid>& ASessionGameState::GetPlayerIDs() const
{
	return PlayerIDs;
}

// Set the full list of non-GM player IDs
void ASessionGameState::SetPlayerIDs(const TArray<FGuid>& InPlayerIDs)
{
	PlayerIDs = InPlayerIDs;
}

// Return the last saved timestamp
FDateTime ASessionGameState::GetLastSaved() const
{
	return LastSaved;
}

// Set the last saved timestamp
void ASessionGameState::SetLastSaved(const FDateTime& InLastSaved)
{
	LastSaved = InLastSaved;
}
