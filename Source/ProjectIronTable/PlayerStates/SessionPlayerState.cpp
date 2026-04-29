// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionPlayerState.h"

#include "Net/UnrealNetwork.h"

// Registers SessionPlayerID, bIsGM, and bIsHost for replication.
void ASessionPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASessionPlayerState, SessionPlayerID);
	DOREPLIFETIME(ASessionPlayerState, bIsGM);
	DOREPLIFETIME(ASessionPlayerState, bIsHost);
}

// Returns the persistent session identity GUID.
FGuid ASessionPlayerState::GetSessionPlayerID() const
{
	return SessionPlayerID;
}

// Sets the persistent session identity GUID.
void ASessionPlayerState::SetSessionPlayerID(const FGuid& InPlayerID)
{
	SessionPlayerID = InPlayerID;
}

// Returns the GM role flag.
bool ASessionPlayerState::GetIsGM() const
{
	return bIsGM;
}

// Sets the GM role flag.
void ASessionPlayerState::SetIsGM(bool bInIsGM)
{
	bIsGM = bInIsGM;
}

// Returns the host flag.
bool ASessionPlayerState::GetIsHost() const
{
	return bIsHost;
}

// Sets the host flag.
void ASessionPlayerState::SetIsHost(bool bInIsHost)
{
	bIsHost = bInIsHost;
}
