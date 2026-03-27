// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "ChatChannel.generated.h"

class UChatEntry;

/** A scrollable list of chat messages belonging to a single conversation channel. */
UCLASS()
class PROJECTIRONTABLE_API UChatChannel : public UUserWidget
{
	GENERATED_BODY()

private:

	// -- Widget References --

	/** Scroll box that holds all chat entry widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;

public:

	// -- Config --

	/** The widget class instantiated for each new chat message. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;

	/** Pixels scrolled per Scroll() call. */
	float ScrollMultiplier = 60.f;

	// -- State --

	/** Human-readable name shown on the tab for this channel. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString DisplayName;

	/** Names of players in this private channel; empty means the channel is the public server channel. */
	TArray<FString> Participants;

protected:
	virtual void NativeConstruct() override;

public:

	// -- Public Methods --

	/** Creates a UChatEntry widget with the given message and appends it to the scroll box. */
	void AddChatMessage(const FString& Message);

	/** Sets the entry class used when creating new message widgets. */
	void SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass);

	/** Scrolls the chat box up (bUp = true) or down by ScrollMultiplier pixels. */
	void Scroll(bool bUp);
};
