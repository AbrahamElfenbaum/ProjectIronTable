// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SessionGameMode.generated.h"

class APlayerController;
class AController;

/**
 * Game mode for active gameplay sessions. Responsible for loading USessionSave into ASessionGameState
 * on startup, assigning roles to players on login, and cleaning up player state on logout.
 */
UCLASS()
class PROJECTIRONTABLE_API ASessionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Maximum number of players allowed in this session. Incoming connections are rejected once this limit is reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;
#pragma endregion

protected:

	/** Loads the session save and populates ASessionGameState before any player logs in. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Rejects incoming connections if the session has reached MaxPlayers. */
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	/** Assigns role flags (bIsHost, bIsGM) to the player's ASessionPlayerState on login. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Removes the player from the active player list in ASessionGameState on logout. */
	virtual void Logout(AController* Exiting) override;
};
