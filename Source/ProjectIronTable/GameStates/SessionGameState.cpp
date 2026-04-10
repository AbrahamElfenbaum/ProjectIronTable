// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionGameState.h"
#include "Net/UnrealNetwork.h"

void ASessionGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASessionGameState, GameTypeID);
	DOREPLIFETIME(ASessionGameState, HostPlayerID);
	DOREPLIFETIME(ASessionGameState, GMPlayerIDs);
	DOREPLIFETIME(ASessionGameState, PlayerIDs);
}