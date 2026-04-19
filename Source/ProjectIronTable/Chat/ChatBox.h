// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

class UHorizontalBox;
class UEditableText;
class UVerticalBox;
class UWidgetSwitcher;
class UButton;
class USessionChatComponent;
class UChatEntry;
class UChatChannel;
class UChatTab;
class UChatChannelListEntry;
class UContextMenu;

/** Root chat widget that manages multiple named channels, a tab bar, and the message input field. */
UCLASS()
class PROJECTIRONTABLE_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Widget class used when creating new chat channel widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatChannel> ChannelClass;

	/** Widget class used when creating new channel tab widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatTab> TabClass;

	/** Widget class forwarded to each channel for spawning individual message entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;

	/** Widget class used when creating entries in the closed channel list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatChannelListEntry> ChannelListEntryClass;

	/** Widget class used for the context menu when right-clicking a channel tab. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UContextMenu> ContextMenuClass;
#pragma endregion

protected:
	/** Caches the HUD component reference, binds delegates, and creates the default server channel. */
	virtual void NativeConstruct() override;

	/** Focuses the chat box when clicked while unfocused. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:

#pragma region Widget References
	/** Horizontal box that holds all channel tab widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TabBar;

	/** Vertical box that holds the list of closed channel entries, shown when the list button is toggled. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ClosedChannelContainer;

	/** Widget switcher that shows one channel's scroll box at a time. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ChannelContainer;

	/** Editable text field where the player types messages. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText;

	/** Button that toggles the closed channel list panel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChannelListButton;
#pragma endregion

#pragma region State
	/** All channels that have been created, including the default server channel. */
	UPROPERTY()
	TArray<TObjectPtr<UChatChannel>> Channels;

	/** Maps each channel to its corresponding tab widget. */
	UPROPERTY()
	TMap<UChatChannel*, UChatTab*> ChannelTabMap;

	/** The channel currently displayed in the widget switcher. */
	UPROPERTY()
	TObjectPtr<UChatChannel> ActiveChannel;

	/** True while the chat input is focused and accepting keyboard input. */
	bool bChatFocused;

	/** Set before calling FocusChat() on Enter commit to absorb the OnUserMovedFocus Slate fires immediately after. */
	bool bPendingRefocus;

	/** Set of channels that have been closed and hidden from the tab bar. */
	UPROPERTY()
	TSet<TObjectPtr<UChatChannel>> ClosedChannels;
#pragma endregion

public:

#pragma region Public Methods
	/** Scrolls the active channel up or down. */
	void Scroll(bool bUp);

	/** Focuses the editable text field and switches input mode to UI only. */
	void FocusChat();

	/** Clears and disables the input field and restores game-and-UI input mode. */
	void ExitChat();

	/** Creates a new channel for the given participant list, adds it to the tab bar, and returns it. */
	UChatChannel* CreateChannel(const TArray<FString>& Participants);

	/** Routes a message to the correct channel (creating one if needed) and shows a notification if not active. */
	void AddChatMessage(const FString& Message, const TArray<FString>& Participants, bool bIsSender);

	/** Appends text to the current contents of the input field. */
	void AppendToInput(const FString& Text);

	/** Returns the participant list of the currently active channel. */
	TArray<FString> GetActiveChannelParticipants();

	/** If the current input starts with a private message command, sends it and clears the input. */
	void TrySendPrivateRollMessage();

	/** Returns the channel matching the given participant list, creating one if none exists. */
	UChatChannel* FindOrCreateChannel(const TArray<FString>& Participants);

	/** Returns the tab associated with the given channel, or nullptr if not found. */
	UChatTab* GetTabForChannel(UChatChannel* Channel) const;

	/** Sets the chat component reference used to send messages to the server. Called by USessionChatComponent after Init. */
	void SetChatComponent(USessionChatComponent* InChatComponent);
#pragma endregion

private:

#pragma region Runtime References
	/** Reference to the owning chat component, used to send chat messages to the server. */
	UPROPERTY()
	TObjectPtr<USessionChatComponent> ChatComponentRef;

	/** Cached reference to the currently visible context menu, used to dismiss it before spawning a new one. */
	UPROPERTY()
	TObjectPtr<UContextMenu> ActiveContextMenuRef;
#pragma endregion

#pragma region Event Handlers
	/** Makes the given channel active in the switcher and clears its notification. */
	UFUNCTION()
	void SwitchToChannel(UChatChannel* Channel);

	/** Sends the typed message to the server on Enter, or exits chat on focus loss. */
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/** Toggles the closed channel list panel between visible and collapsed. */
	UFUNCTION()
	void OnChannelListButtonClicked();

	/** Hides the tab for the given channel and adds it to the closed set; switches to Server if it was active. */
	UFUNCTION()
	void CloseChannel(UChatChannel* Channel);

	/** Removes the given channel from the closed set, restores its tab, and switches to it. */
	UFUNCTION()
	void ReopenChannel(UChatChannel* Channel);

	/** Spawns a context menu at the cursor position when a non-Server tab is right-clicked. */
	UFUNCTION()
	void OnTabRightClickedHandler(UChatChannel* Channel);

	/** Persists the new tab label to the session save when the user commits a rename. */
	UFUNCTION()
	void OnTabRenamedHandler(UChatTab* Tab, const FString& NewName);
#pragma endregion

#pragma region Private Methods
	/** Splits a message string into @mention recipients and the remaining message body. */
	void ParseMentions(const FString& Message, TArray<FString>& OutRecipients, FString& OutBody) const;

	/** Clears and repopulates the closed channel list panel from the current ClosedChannels set. */
	void RefreshChannelList();
#pragma endregion
};
