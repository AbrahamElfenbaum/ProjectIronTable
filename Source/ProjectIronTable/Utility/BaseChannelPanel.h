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

/** Shared base widget for channel panel types; manages the tab bar, channel switcher, and closed-channel list. */
UCLASS()
class PROJECTIRONTABLE_API UBaseChannelPanel : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Widget class used when creating new channel widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannel> ChannelClass;

	/** Widget class used when creating new tab widgets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannelTab> TabClass;

	/** Widget class used when creating entries in the closed channel list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UBaseChannelListEntry> ChannelListEntryClass;

	/** Widget class used for the context menu when right-clicking a channel tab. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UContextMenu> ContextMenuClass;
#pragma endregion

#pragma region Public Methods
	/** Scrolls the active channel up or down. */
	void Scroll(bool bUp);

	/** Creates a new channel for the given participant list, adds it to the tab bar, and returns it. */
	virtual UBaseChannel* CreateChannel(const TArray<FString>& Participants);

	/** Returns the participant list of the currently active channel. */
	TArray<FString> GetActiveChannelParticipants();

	/** Returns the channel matching the given participant list, creating one if none exists. */
	UBaseChannel* FindOrCreateChannel(const TArray<FString>& Participants);

	/** Returns the tab associated with the given channel, or nullptr if not found. */
	UBaseChannelTab* GetTabForChannel(UBaseChannel* Channel) const;
#pragma endregion

protected:
	/** Binds the channel list button delegate, collapses the closed list, and creates the default server channel. */
	virtual void NativeConstruct() override;

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

#pragma region Private Methods
	/** Builds and returns the display label for a new tab based on its participant list. */
	virtual FString CreateTabLabel(const TArray<FString>& Participants) const;

	/** Called after channel creation; subclasses override to persist tab data to their save format. */
	virtual void SaveCreatedTab();

	/** Called after a tab rename commits; subclasses override to persist the new name to their save format. */
	virtual void OnChannelRenamed(UBaseChannelTab* Tab, const FString& NewName, const FString& ParticipantsKey);

	/** Called after SwitchToChannel completes; subclasses override for panel-specific switch behavior. */
	virtual void OnChannelSwitched(UBaseChannel* Channel);

	/** Clears and repopulates the closed channel list panel from the current ClosedChannels set. */
	void RefreshChannelList();
#pragma endregion

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
};
