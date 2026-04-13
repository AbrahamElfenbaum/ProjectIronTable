// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "SessionInstance.h"
#include "SessionSave.h"
#include "SessionGameState.h"
#include "SessionPlayerState.h"

// Load session save and populate GameState before any player logs in
void ASessionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	USessionInstance* SessionInstance = GetGameInstance<USessionInstance>();
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::InitGame — SessionInstance is null; session save will not be loaded"));
		return;
	}

	USessionSave* SessionSave = Cast<USessionSave>(UGameplayStatics::LoadGameFromSlot(
		FString::Printf(TEXT("Session_%s"), *SessionInstance->GetSessionID().ToString()), 0));
	if (!IsValid(SessionSave))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::InitGame — Failed to load session save for ID %s"), *SessionInstance->GetSessionID().ToString());
		return;
	}

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	if (!IsValid(SessionGameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::InitGame — SessionGameState is null; cannot populate session data"));
		return;
	}

	SessionGameState->SetGameTypeID(SessionSave->GameTypeID);
	SessionGameState->SetCampaignID(SessionSave->CampaignID);
	SessionGameState->SetSessionID(SessionSave->SessionID);
	SessionGameState->SetHostPlayerID(SessionSave->HostPlayerID);
	SessionGameState->SetGMPlayerIDs(SessionSave->GMPlayerIDs);
	SessionGameState->SetPlayerIDs(SessionSave->PlayerIDs);
	SessionGameState->SetLastSaved(SessionSave->LastSaved);
}

// Reject the connection if the session is already at capacity
void ASessionGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PreLogin — PreLogin failed with error: %s"), *ErrorMessage);
		return;
	}

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	if (!IsValid(SessionGameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PreLogin — SessionGameState is null; cannot determine if session is full"));
		return;
	}
	
	int32 CurrentPlayers = SessionGameState->PlayerArray.Num();
	if (CurrentPlayers >= MaxPlayers)
	{
		ErrorMessage = TEXT("Session is full");
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PreLogin — Player rejected because session is full (current: %d, max: %d)"), CurrentPlayers, MaxPlayers);
		return;
	}

}

// Assign role flags to the player's SessionPlayerState on login
void ASessionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ASessionPlayerState* SessionPlayerState = NewPlayer->GetPlayerState<ASessionPlayerState>();
	if (!IsValid(SessionPlayerState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PostLogin — SessionPlayerState is null; cannot assign role flags"));
		return;
	}

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	if (!IsValid(SessionGameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PostLogin — SessionGameState is null; cannot determine role flags"));
		return;
	}

	FString sPlayerID = UGameplayStatics::ParseOption(OptionsString, TEXT("PlayerID"));
	if (sPlayerID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PostLogin — PlayerID not found in options string; role assignment will fail"));
		return;
	}

	FGuid SessionPlayerID;
	if (FGuid::Parse(sPlayerID, SessionPlayerID))
	{
		SessionPlayerState->SetSessionPlayerID(SessionPlayerID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::PostLogin — Failed to parse PlayerID from options string; role assignment will fail"));
		return;
	}

	SessionPlayerState->SetIsHost(SessionPlayerID == SessionGameState->GetHostPlayerID());
	SessionPlayerState->SetIsGM(SessionGameState->GetGMPlayerIDs().Contains(SessionPlayerID));

	if (SessionPlayerState->GetIsGM())
	{
		TArray<FGuid> GMPlayerIDs = SessionGameState->GetGMPlayerIDs();
		GMPlayerIDs.AddUnique(SessionPlayerID);
		SessionGameState->SetGMPlayerIDs(GMPlayerIDs);
	}
	else
	{
		TArray<FGuid> PlayerIDs = SessionGameState->GetPlayerIDs();
		PlayerIDs.AddUnique(SessionPlayerID);
		SessionGameState->SetPlayerIDs(PlayerIDs);
	}
}

// Remove the player from the active player list in SessionGameState on logout
void ASessionGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* ExitingPlayerController = Cast<APlayerController>(Exiting);
	if (!IsValid(ExitingPlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::Logout — ExitingPlayerController is null; cannot remove player from active list"));
		return;
	}

	ASessionPlayerState* SessionPlayerState = ExitingPlayerController->GetPlayerState<ASessionPlayerState>();
	if (!IsValid(SessionPlayerState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::Logout — SessionPlayerState is null; cannot remove player from active list"));
		return;
	}

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	if (!IsValid(SessionGameState))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::Logout — SessionGameState is null; cannot remove player from active list"));
		return;
	}

	FGuid PlayerID = SessionPlayerState->GetSessionPlayerID();
	if (SessionPlayerState->GetIsGM())
	{
		TArray<FGuid> GMPlayerIDs = SessionGameState->GetGMPlayerIDs();
		GMPlayerIDs.Remove(PlayerID);
		SessionGameState->SetGMPlayerIDs(GMPlayerIDs);
	}
	else
	{
		TArray<FGuid> PlayerIDs = SessionGameState->GetPlayerIDs();
		PlayerIDs.Remove(PlayerID);
		SessionGameState->SetPlayerIDs(PlayerIDs);
	}
}
