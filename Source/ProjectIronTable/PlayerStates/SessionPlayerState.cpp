// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionPlayerState.h"

#include "Net/UnrealNetwork.h"

// Return the persistent session identity GUID
FGuid ASessionPlayerState::GetSessionPlayerID() const
{
	return SessionPlayerID;
}

// Set the persistent session identity GUID
void ASessionPlayerState::SetSessionPlayerID(const FGuid& InPlayerID)
{
	SessionPlayerID = InPlayerID;
}

// Return the GM role flag
bool ASessionPlayerState::GetIsGM() const
{
	return bIsGM;
}

// Set the GM role flag
void ASessionPlayerState::SetIsGM(bool bInIsGM)
{
	bIsGM = bInIsGM;
}

// Return the host flag
bool ASessionPlayerState::GetIsHost() const
{
	return bIsHost;
}

// Set the host flag
void ASessionPlayerState::SetIsHost(bool bInIsHost)
{
	bIsHost = bInIsHost;
}

// Register replicated properties
void ASessionPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASessionPlayerState, SessionPlayerID);
	DOREPLIFETIME(ASessionPlayerState, bIsGM);
	DOREPLIFETIME(ASessionPlayerState, bIsHost);
}
