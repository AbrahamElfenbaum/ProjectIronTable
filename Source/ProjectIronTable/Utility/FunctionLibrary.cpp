// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#include "FunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "SessionInstance.h"
#include "SessionSave.h"
#include "GameFramework/PlayerState.h"

// Returns the short string name for each EDiceType value.
FString UFunctionLibrary::GetDiceName(EDiceType Type)
{
	switch (Type)
	{
	case EDiceType::D4:
		return TEXT("D4");
	case EDiceType::D6:
		return TEXT("D6");
	case EDiceType::D8:
		return TEXT("D8");
	case EDiceType::D10:
		return TEXT("D10");
	case EDiceType::D12:
		return TEXT("D12");
	case EDiceType::D20:
		return TEXT("D20");
	case EDiceType::D100:
		return TEXT("D100");
	default:
		return FString();
	}
}

// Returns the session save slot name for the given session instance, or an empty string if the instance is invalid.
FString UFunctionLibrary::GetSessionSaveSlotName(USessionInstance* SessionInstance)
{
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::GetSessionSaveSlotName � SessionInstance is null; message will not be persisted"));
		return FString();
	}

	return FString::Printf(TEXT("Session_%s"), *SessionInstance->GetSessionID().ToString());
	
}

// Loads and returns the session save for the current session, or nullptr if the instance or save is not found.
USessionSave* UFunctionLibrary::LoadSessionSave(UObject* WorldContext)
{
	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(WorldContext));

	FString SlotName = GetSessionSaveSlotName(SessionInstance);

	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionSave � Failed to get valid slot name; cannot load session save"));
		return nullptr;
	}

	USessionSave* SessionSave = Cast<USessionSave>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (IsValid(SessionSave))
	{
		return SessionSave;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionSave � Failed to get valid session save"));
	return nullptr;
}

// Returns the local player's name from their PlayerState, or "Unknown" if the controller or state is not available.
FString UFunctionLibrary::GetLocalPlayerName(UObject* WorldContext)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContext, 0);
	if (IsValid(PC) && IsValid(PC->PlayerState))
	{
		return PC->PlayerState->GetPlayerName();
	}

	UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::GetLocalPlayerName � Failed to get local player name"));
	return TEXT("Unknown");
}
