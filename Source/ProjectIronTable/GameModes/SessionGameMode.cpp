// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "SessionInstance.h"
#include "SessionSave.h"
#include "SessionGameState.h"
#include "SessionPlayerState.h"
#include "MacroLibrary.h"

// Load session save and populate GameState before any player logs in
void ASessionGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	USessionInstance* SessionInstance = GetGameInstance<USessionInstance>();
	CHECK_IF_VALID(SessionInstance, );

	USessionSave* SessionSave = Cast<USessionSave>(UGameplayStatics::LoadGameFromSlot(
		FString::Printf(TEXT("Session_%s"), *SessionInstance->GetSessionID().ToString()), 0));
	if (!IsValid(SessionSave))
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionGameMode::InitGame — Failed to load session save for ID %s"), *SessionInstance->GetSessionID().ToString());
		return;
	}

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	CHECK_IF_VALID(SessionGameState, );

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
	CHECK_IF_VALID(SessionGameState, );
	
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
	CHECK_IF_VALID(SessionPlayerState, );

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	CHECK_IF_VALID(SessionGameState, );

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
	CHECK_IF_VALID(ExitingPlayerController, );

	ASessionPlayerState* SessionPlayerState = ExitingPlayerController->GetPlayerState<ASessionPlayerState>();
	CHECK_IF_VALID(SessionPlayerState, );

	ASessionGameState* SessionGameState = GetGameState<ASessionGameState>();
	CHECK_IF_VALID(SessionGameState, );

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
