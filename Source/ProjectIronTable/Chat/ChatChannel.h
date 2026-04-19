// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannel.h"
#include "ChatChannel.generated.h"

class UChatEntry;

/** A scrollable list of chat messages belonging to a single conversation channel. */
UCLASS()
class PROJECTIRONTABLE_API UChatChannel : public UBaseChannel
{
	GENERATED_BODY()

public:

#pragma region Config
	/** The widget class instantiated for each new chat message. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;
#pragma endregion

#pragma region Public Methods
	/** Creates a UChatEntry widget with the given message and appends it to the scroll box. */
	void AddChatMessage(const FString& Message);

	/** Sets the entry class used when creating new message widgets. */
	void SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass);

	/** Appends a previously saved message directly to the scroll box, bypassing all routing and notification logic. */
	void RestoreMessage(const FString& SenderName, const FString& Message);
#pragma endregion
};
