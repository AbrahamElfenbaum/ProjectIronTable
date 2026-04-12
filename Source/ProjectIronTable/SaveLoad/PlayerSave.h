// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerSave.generated.h"

/**
 * Persistent save game that stores a player's permanent identity GUID.
 * Created on first launch with a newly generated GUID; reloaded every session after.
 * The PlayerID is passed via login options to the server so PostLogin can assign the correct role.
 */
UCLASS()
class PROJECTIRONTABLE_API UPlayerSave : public USaveGame
{
	GENERATED_BODY()

public:

	/** Save slot name used for all load and save calls. */
	static const FString SaveSlotName;

	/** Permanent player identity. Generated once on first launch and reused every session. */
	UPROPERTY()
	FGuid PlayerID;
};
