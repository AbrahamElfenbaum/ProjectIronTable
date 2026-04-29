// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseChannelPanel.h"
#include "ChatBox.generated.h"

class UEditableText;
class USessionChatComponent;
class UChatEntry;
class UChatChannel;
class UBaseChannel;
class UBaseChannelTab;

/** Root chat widget that manages multiple named channels, a tab bar, and the message input field. */
UCLASS()
class PROJECTIRONTABLE_API UChatBox : public UBaseChannelPanel
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Editable text field where the player types messages. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText;
#pragma endregion

#pragma region State
	/** True while the chat input is focused and accepting keyboard input. */
	bool bChatFocused;

	/** Set before calling FocusChat() on Enter commit to absorb the OnUserMovedFocus Slate fires immediately after. */
	bool bPendingRefocus;
#pragma endregion

#pragma region Runtime References
	/** Reference to the owning chat component, used to send chat messages to the server. */
	UPROPERTY()
	TObjectPtr<USessionChatComponent> ChatComponentRef;
#pragma endregion

public:

#pragma region Config
	/** Widget class forwarded to each channel for spawning individual message entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;
#pragma endregion

private:

#pragma region Private Methods
	/** Splits a message string into @mention recipients and the remaining message body. */
	void ParseMentions(const FString& Message, TArray<FString>& OutRecipients, FString& OutBody) const;

	/** Builds the display label for a chat tab based on the participant list. */
	virtual FString CreateTabLabel(const TArray<FString>& Participants) const override;

	/** Persists the new channel's tab name to the session save. */
	virtual void SaveCreatedTab() override;

	/** Persists the renamed tab label to the session save. */
	virtual void OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey) override;

	/** Clears the input field when switching to a new channel. */
	virtual void OnChannelSwitched(UBaseChannel* Channel) override;
#pragma endregion

#pragma region Event Handlers
	/** Sends the typed message to the server on Enter, or exits chat on focus loss. */
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
#pragma endregion

protected:

	/** Binds the text committed delegate; base handles channel and button setup. */
	virtual void NativeConstruct() override;

	/** Focuses the chat box when clicked while unfocused. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:

#pragma region Public Methods
	/** Focuses the editable text field and switches input mode to UI only. */
	void FocusChat();

	/** Clears and disables the input field and restores game-and-UI input mode. */
	void ExitChat();

	/** Creates a new channel for the given participant list, adds it to the tab bar, and returns it. */
	virtual UBaseChannel* CreateChannel(const TArray<FString>& Participants) override;

	/** Routes a message to the correct channel (creating one if needed) and shows a notification if not active. */
	void AddChatMessage(const FString& Message, const TArray<FString>& Participants, bool bIsSender);

	/** Appends text to the current contents of the input field. */
	void AppendToInput(const FString& Text);

	/** If the current input starts with a private message command, sends it and clears the input. */
	void TrySendPrivateRollMessage();

	/** Sets the chat component reference used to send messages to the server. Called by USessionChatComponent after Init. */
	void SetChatComponent(USessionChatComponent* InChatComponent);
#pragma endregion
};
