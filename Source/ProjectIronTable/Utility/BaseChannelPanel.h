// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseChannelPanel.generated.h"

class UBaseChannel;
class UBaseChannelTab;
class UBaseChannelListEntry;
class UContextMenu;
class UHorizontalBox;
class UVerticalBox;
class UWidgetSwitcher;
class UButton;

/**
 * 
 */
UCLASS()
class PROJECTIRONTABLE_API UBaseChannelPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:

#pragma region Config
	/** Widget class used when creating new session notes channel widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannel> ChannelClass;

	/** Widget class used when creating new session notes tab widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannelTab> TabClass;

	/** Widget class used when creating entries in the closed channel list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannelListEntry> ChannelListEntryClass;

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

	/** Button that toggles the closed channel list panel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChannelListButton;
#pragma endregion

#pragma region State
	/** All channels that have been created, including the default server channel. */
	UPROPERTY()
	TArray<TObjectPtr<UBaseChannel>> Channels;

	/** Maps each channel to its corresponding tab widget. */
	UPROPERTY()
	TMap<UBaseChannel*, UBaseChannelTab*> ChannelTabMap;

	/** The channel currently displayed in the widget switcher. */
	UPROPERTY()
	TObjectPtr<UBaseChannel> ActiveChannel;

	/** Set of channels that have been closed and hidden from the tab bar. */
	UPROPERTY()
	TSet<TObjectPtr<UBaseChannel>> ClosedChannels;
#pragma endregion

public:

#pragma region Public Methods
	/** Scrolls the active channel up or down. */
	void Scroll(bool bUp);

	/** Creates a new channel for the given participant list, adds it to the tab bar, and returns it. */
	UBaseChannel* CreateChannel(const TArray<FString>& Participants);

	/** Returns the participant list of the currently active channel. */
	TArray<FString> GetActiveChannelParticipants();

	/** Returns the channel matching the given participant list, creating one if none exists. */
	UBaseChannel* FindOrCreateChannel(const TArray<FString>& Participants);

	/** Returns the tab associated with the given channel, or nullptr if not found. */
	UBaseChannelTab* GetTabForChannel(UBaseChannel* Channel) const;
#pragma endregion

protected:

#pragma region Runtime References
	/** Cached reference to the currently visible context menu, used to dismiss it before spawning a new one. */
	UPROPERTY()
	TObjectPtr<UContextMenu> ActiveContextMenuRef;
#pragma endregion

#pragma region Event Handlers
	/** Makes the given channel active in the switcher and clears its notification. */
	UFUNCTION()
	void SwitchToChannel(UBaseChannel* Channel);

	/** Toggles the closed channel list panel between visible and collapsed. */
	UFUNCTION()
	void OnChannelListButtonClicked();

	/** Hides the tab for the given channel and adds it to the closed set; switches to Server if it was active. */
	UFUNCTION()
	void CloseChannel(UBaseChannel* Channel);

	/** Removes the given channel from the closed set, restores its tab, and switches to it. */
	UFUNCTION()
	void ReopenChannel(UBaseChannel* Channel);

	/** Spawns a context menu at the cursor position when a non-Server tab is right-clicked. */
	UFUNCTION()
	void OnTabRightClickedHandler(UBaseChannel* Channel);

	/** Persists the new tab label to the session save when the user commits a rename. */
	UFUNCTION()
	void OnTabRenamedHandler(UBaseChannelTab* Tab, const FString& NewName);
#pragma endregion

#pragma region Private Methods
	/** Clears and repopulates the closed channel list panel from the current ClosedChannels set. */
	void RefreshChannelList();
#pragma endregion
};
