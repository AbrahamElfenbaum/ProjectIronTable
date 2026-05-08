// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "FunctionLibrary.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"

#include "RichTextDocument.h"
#include "RichTextRun.h"
#include "SessionInstance.h"
#include "SessionSave.h"
#include "SessionNotesSave.h"

// Returns the session save slot name for the given session instance, or an empty string if the instance is invalid.
FString UFunctionLibrary::GetSessionSaveSlotName(USessionInstance* SessionInstance)
{
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::GetSessionSaveSlotName — SessionInstance is null; message will not be persisted"));
		return FString();
	}

	return FString::Printf(TEXT("Session_%s"), *SessionInstance->GetSessionID().ToString());
}

// Returns the formatted notes save slot name for the given player and session IDs.
FString UFunctionLibrary::GetNotesSaveSlotName(const FGuid& InPlayerID, const FGuid& InSessionID)
{
	return FString::Printf(TEXT("Notes_%s_%s"), *InPlayerID.ToString(), *InSessionID.ToString());
}

// Loads and returns the session save for the current session, or nullptr if the instance or save is not found.
USessionSave* UFunctionLibrary::LoadSessionSave(UObject* WorldContext)
{
	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionSave — Failed to get valid SessionInstance"));
		return nullptr;
	}

	FString SlotName = GetSessionSaveSlotName(SessionInstance);

	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionSave — Failed to get valid slot name; cannot load session save"));
		return nullptr;
	}

	USessionSave* SessionSave = Cast<USessionSave>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (IsValid(SessionSave))
	{
		return SessionSave;
	}

	UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionSave — Failed to get valid session save"));
	return nullptr;
}

// Loads and returns the notes save for the local player in the current session, or nullptr if the instance or save is not found.
USessionNotesSave* UFunctionLibrary::LoadSessionNotesSave(UObject* WorldContext)
{
	USessionInstance* SessionInstance = Cast<USessionInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	if (!IsValid(SessionInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionNotesSave — Failed to get valid SessionInstance"));
		return nullptr;
	}

	FString SlotName = GetNotesSaveSlotName(SessionInstance->GetPlayerID(), SessionInstance->GetSessionID());

	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionNotesSave — Failed to get valid slot name; cannot load session notes save"));
		return nullptr;
	}

	USessionNotesSave* SessionNotesSave = Cast<USessionNotesSave>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (IsValid(SessionNotesSave))
	{
		return SessionNotesSave;
	}

	UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::LoadSessionNotesSave — Failed to get valid session notes save"));
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

	UE_LOG(LogTemp, Warning, TEXT("UFunctionLibrary::GetLocalPlayerName — Failed to get local player name"));
	return TEXT("Unknown");
}

// Copies the participant list, sorts it, then joins with '|' for a deterministic key regardless of input order.
FString UFunctionLibrary::MakeParticipantKey(const TArray<FString>& Participants)
{
	TArray<FString> Sorted = Participants;
	Sorted.Sort();
	return FString::Join(Sorted, TEXT("|"));
}

// Iterates all runs, finds the tallest character height at the given scale, and optionally writes the winning FontInfo to OutFontInfo.
float UFunctionLibrary::GetDocumentLineHeight(const FRichTextDocument& InDocument, float Scale, FSlateFontInfo* OutFontInfo)
{
	float LineHeight = 0;
	FSlateFontInfo BestFontInfo;

	const TArray<FRichTextRun>& Runs = InDocument.Runs;

	for (const FRichTextRun& Run : Runs)
	{
		float NewLineHeight = FSlateApplication::Get().GetRenderer()
												 ->GetFontMeasureService()
												 ->GetMaxCharacterHeight(Run.FontInfo, Scale);

		if (NewLineHeight > LineHeight)
		{
			LineHeight = NewLineHeight;
			BestFontInfo = Run.FontInfo;
		}
	}

	if (OutFontInfo)
	{ 
		*OutFontInfo = BestFontInfo; 
	}

	return LineHeight;
}
