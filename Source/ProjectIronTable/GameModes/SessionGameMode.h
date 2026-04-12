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

protected:

	/** Loads the session save and populates ASessionGameState before any player logs in. */
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Assigns role flags (bIsHost, bIsGM) to the player's ASessionPlayerState on login. */
	void PostLogin(APlayerController* NewPlayer) override;

	/** Removes the player from the active player list in ASessionGameState on logout. */
	void Logout(AController* Exiting) override;
};
