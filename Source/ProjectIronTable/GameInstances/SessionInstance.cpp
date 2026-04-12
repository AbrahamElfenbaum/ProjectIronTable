// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionInstance.h"

#include "Kismet/GameplayStatics.h"

#include "PlayerSave.h"

// Loads PlayerID from UPlayerSave; creates and saves a new one with a generated GUID if none exists.
void USessionInstance::Init()
{
	Super::Init();

	UPlayerSave* PlayerSave = Cast<UPlayerSave>(UGameplayStatics::LoadGameFromSlot(UPlayerSave::SaveSlotName, 0));
	if (!IsValid(PlayerSave))
	{
		UE_LOG(LogTemp, Display, TEXT("USessionInstance::Init - No PlayerSave found; creating new one"));
		PlayerID = FGuid::NewGuid();
		PlayerSave = Cast<UPlayerSave>(UGameplayStatics::CreateSaveGameObject(UPlayerSave::StaticClass()));
		PlayerSave->PlayerID = PlayerID;
		UGameplayStatics::SaveGameToSlot(PlayerSave, UPlayerSave::SaveSlotName, 0);
	}

	PlayerID = PlayerSave->PlayerID;
	if (!PlayerID.IsValid())
	{
		PlayerID = FGuid::NewGuid();
		PlayerSave->PlayerID = PlayerID;
		UGameplayStatics::SaveGameToSlot(PlayerSave, UPlayerSave::SaveSlotName, 0);
	}
}

// Returns the stored campaign ID.
FGuid USessionInstance::GetCampaignID() const
{
	return CampaignID;
}

// Returns the stored session ID.
FGuid USessionInstance::GetSessionID() const
{
	return SessionID;
}

// Returns the persistent player identity loaded from UPlayerSave on startup.
FGuid USessionInstance::GetPlayerID() const
{
	return PlayerID;
}

// Stores the campaign ID for use after level transition.
void USessionInstance::SetCampaignID(const FGuid& InCampaignID)
{
	CampaignID = InCampaignID;
}

// Stores the session ID for use after level transition.
void USessionInstance::SetSessionID(const FGuid& InSessionID)
{
	SessionID = InSessionID;
}

// Stores the player ID. Should not be needed outside of testing — PlayerID is set from UPlayerSave in Init.
void USessionInstance::SetPlayerID(const FGuid& InPlayerID)
{
	PlayerID = InPlayerID;
}
