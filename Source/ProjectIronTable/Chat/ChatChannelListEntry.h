// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ChatChannelListEntry.generated.h"


class UChatChannel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, UChatChannel*, ClickedChannel);

/** A single row in the closed channel list. Displays the channel name and fires OnEntryClicked when pressed. */
UCLASS()
class PROJECTIRONTABLE_API UChatChannelListEntry : public UUserWidget
{
	GENERATED_BODY()
	
private:

#pragma region Widget References
	/** Button the player clicks to switch to this channel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	/** Label displaying the channel name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EntryLabel;

	/** The channel this entry represents. */
	UPROPERTY()
	TObjectPtr<UChatChannel> Channel;
#pragma endregion

protected:
	/** Binds the entry button click delegate. */
	virtual void NativeConstruct() override;

private:

	/** Broadcasts OnEntryClicked with the stored channel pointer. */
	UFUNCTION()
	void OnEntryButtonClicked();

public:

#pragma region Events
	/** Fired when the tab button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnEntryClicked OnEntryClicked;
#pragma endregion

#pragma region Public Methods
	/** Sets the channel this entry represents and updates the displayed label. */
	void SetChannel(UChatChannel* InChannel);
#pragma endregion
};
