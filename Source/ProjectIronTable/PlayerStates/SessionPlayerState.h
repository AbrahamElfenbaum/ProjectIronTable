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

	/** Registers replicated properties with the Unreal replication system. */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region State
	/** Persistent player identity GUID. Used to look up role assignments in ASessionGameState. */
	UPROPERTY(Replicated)
	FGuid SessionPlayerID;

	/** True if this player currently holds the GM role. Set by the server; derived from ASessionGameState::GMPlayerIDs. */
	UPROPERTY(Replicated)
	bool bIsGM;

	/** True if this player is the Server Owner (host machine). Set by the server; derived from ASessionGameState::HostPlayerID. */
	UPROPERTY(Replicated)
	bool bIsHost;
#pragma endregion

public:

#pragma region Public Methods
	/** Returns this player's persistent session identity GUID. */
	FGuid GetSessionPlayerID() const;

	/** Sets this player's persistent session identity GUID. */
	void SetSessionPlayerID(const FGuid& InPlayerID);

	/** Returns true if this player currently holds the GM role. */
	bool GetIsGM() const;

	/** Sets the GM role flag. Should only be called by the server. */
	void SetIsGM(bool bInIsGM);

	/** Returns true if this player is the Server Owner. */
	bool GetIsHost() const;

	/** Sets the host flag. Should only be called by the server. */
	void SetIsHost(bool bInIsHost);
#pragma endregion
};
