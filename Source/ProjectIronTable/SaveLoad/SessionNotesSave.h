// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RichTextDocument.h"
#include "SessionNotesSave.generated.h"

/** Stores all data for a single notes document — identity, content, edit timestamp, and access model. */
USTRUCT(BlueprintType)
struct FNoteRecord
{
	GENERATED_BODY()

	/** Unique identifier for this note. Stable across renames and content edits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid NoteID;

	/** User-visible tab label for this note. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/** Rich text document body of the note. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRichTextDocument Content;

	/** Timestamp of the most recent content change. Most recently modified copy wins on conflict. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastEdited;

	/** Player ID of the note creator. Metadata only — does not grant special runtime authority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CreatorPlayerID;

	/** Player IDs of all editors. Empty means private (creator-only). Non-empty means shared — all listed players are peers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> EditorPlayerIDs;
};


/**
 * Per-player notes save file. Stores all note records and tab ordering for a single player in a single session.
 * Save slot: "Notes_{PlayerID}_{SessionID}".
 */
UCLASS()
class PROJECTIRONTABLE_API USessionNotesSave : public USaveGame
{
	GENERATED_BODY()

public:

	/** All note records owned by or shared with this player for this session. */
	UPROPERTY()
	TArray<FNoteRecord> SessionNoteRecords;
};
