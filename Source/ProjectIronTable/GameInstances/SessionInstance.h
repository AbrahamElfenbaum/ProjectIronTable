// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SessionInstance.generated.h"

/**
 * Persistent game instance that survives level transitions. Carries the minimum session context
 * needed to bootstrap ASessionGameMode on the gameplay level — specifically the SessionID used
 * to locate and load USessionSave, and the CampaignID for campaign context.
 */
UCLASS()
class PROJECTIRONTABLE_API USessionInstance : public UGameInstance
{
	GENERATED_BODY()

public:

#pragma region Public Methods
	/** Returns the ID of the campaign associated with the pending session. */
	FGuid GetCampaignID() const;

	/** Returns the ID of the session to load on the gameplay level. */
	FGuid GetSessionID() const;

	/** Returns the persistent player identity loaded from UPlayerSave on startup. */
	FGuid GetPlayerID() const;

	/** Sets the campaign ID before traveling to the gameplay level. */
	void SetCampaignID(const FGuid& InCampaignID);

	/** Sets the session ID before traveling to the gameplay level. */
	void SetSessionID(const FGuid& InSessionID);

	/** Sets the player ID. Should not be needed outside of testing — PlayerID is set from UPlayerSave in Init. */
	void SetPlayerID(const FGuid& InPlayerID);
#pragma endregion

protected:
	/** Loads or generates the persistent PlayerID from UPlayerSave. Called once on game startup. */
	virtual void Init() override;

#pragma region State
	/** ID of the campaign this session belongs to. */
	UPROPERTY()
	FGuid CampaignID;

	/** Unique ID for this session. Used to build the save slot name: "Session_{SessionID}". */
	UPROPERTY()
	FGuid SessionID;

	/** Persistent player identity loaded from UPlayerSave on startup. Passed via login options to the server on travel. */
	UPROPERTY()
	FGuid PlayerID;
#pragma endregion
};
