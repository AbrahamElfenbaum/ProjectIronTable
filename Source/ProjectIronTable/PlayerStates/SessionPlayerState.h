// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SessionPlayerState.generated.h"

/**
 * Per-player runtime state replicated to all clients. Holds role flags derived from ASessionGameState —
 * bIsGM and bIsHost are set by the server when the session starts or when roles change, and must stay
 * in sync with GMPlayerIDs and HostPlayerID in ASessionGameState. Not persisted to disk; repopulated
 * each session from USessionSave via the server.
 */
UCLASS()
class PROJECTIRONTABLE_API ASessionPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** True if this player currently holds the GM role. Set by the server; derived from ASessionGameState::GMPlayerIDs. */
	UPROPERTY(Replicated)
	bool bIsGM;

	/** True if this player is the Server Owner (host machine). Set by the server; derived from ASessionGameState::HostPlayerID. */
	UPROPERTY(Replicated)
	bool bIsHost;
};
