// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASessionPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASessionPlayerState, bIsGM);
	DOREPLIFETIME(ASessionPlayerState, bIsHost);
}
