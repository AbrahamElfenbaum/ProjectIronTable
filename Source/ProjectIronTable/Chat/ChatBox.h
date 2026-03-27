// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "ChatBox.generated.h"

class UGameplayHUDComponent;
class UChatEntry;
class UChatChannel;
class UChatTab;

/** Root chat widget that manages multiple named channels, a tab bar, and the message input field. */
UCLASS()
class PROJECTIRONTABLE_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

public:

	// -- Config --

	/** Widget class used when creating new chat channel widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatChannel> ChannelClass;

	/** Widget class used when creating new channel tab widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatTab> TabClass;

	/** Widget class forwarded to each channel for spawning individual message entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;

protected:
	virtual void NativeConstruct() override;

	/** Focuses the chat box when clicked while unfocused. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:

	// -- Runtime References --

	/** Reference to the owning HUD component, used to send chat messages to the server. */
	UPROPERTY()
	TObjectPtr<UGameplayHUDComponent> HUDComponentRef;

	// -- Widget References --

	/** Horizontal box that holds all channel tab widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TabBar;

	/** Widget switcher that shows one channel's scroll box at a time. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ChannelContainer;

	/** Editable text field where the player types messages. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText;

	// -- State --

	/** All channels that have been created, including the default server channel. */
	UPROPERTY()
	TArray<UChatChannel*> Channels;

	/** Maps each channel to its corresponding tab widget. */
	UPROPERTY()
	TMap<UChatChannel*, UChatTab*> ChannelTabMap;

	/** The channel currently displayed in the widget switcher. */
	UPROPERTY()
	TObjectPtr<UChatChannel> ActiveChannel;

	/** True while the chat input is focused and accepting keyboard input. */
	bool bChatFocused;

public:

	// -- Public Methods --

	/** Scrolls the active channel up or down. */
	void Scroll(bool bUp);

	/** Focuses the editable text field and switches input mode to UI only. */
	void FocusChat();

	/** Clears and disables the input field and restores game-and-UI input mode. */
	void ExitChat();

	/** Creates a new channel for the given participant list, adds it to the tab bar, and returns it. */
	UChatChannel* CreateChannel(TArray<FString> Participants);

	/** Makes the given channel active in the switcher and clears its notification. */
	UFUNCTION()
	void SwitchToChannel(UChatChannel* Channel);

	/** Routes a message to the correct channel (creating one if needed) and shows a notification if not active. */
	void AddChatMessage(const FString& Message, TArray<FString> Participants, bool bIsSender);

	/** Appends text to the current contents of the input field. */
	void AppendToInput(const FString& Text);

	/** Returns the participant list of the currently active channel. */
	TArray<FString> GetActiveChannelParticipants();

private:

	/** Sends the typed message to the server on Enter, or exits chat on focus loss. */
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
