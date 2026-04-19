// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseChannelListEntry.generated.h"

class UTextBlock;
class UButton;
class UBaseChannel;

/** Fired when the entry button is clicked, passing the channel it represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, UBaseChannel*, ClickedChannel);

/** A single row in the closed channel list. Displays the channel name and fires OnEntryClicked when pressed. */
UCLASS()
class PROJECTIRONTABLE_API UBaseChannelListEntry : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Button the player clicks to reopen this channel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	/** Label displaying the channel name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EntryLabel;
#pragma endregion

#pragma region State
	/** The channel this entry represents. */
	UPROPERTY()
	TObjectPtr<UBaseChannel> Channel;
#pragma endregion

protected:
	/** Binds the entry button click delegate. */
	virtual void NativeConstruct() override;

public:

#pragma region Events
	/** Fired when the entry button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnEntryClicked OnEntryClicked;
#pragma endregion

#pragma region Public Methods
	/** Sets the channel this entry represents and updates the displayed label. */
	void SetChannel(UBaseChannel* InChannel);
#pragma endregion

private:

#pragma region Event Handlers
	/** Broadcasts OnEntryClicked with the stored channel pointer. */
	UFUNCTION()
	void OnEntryButtonClicked();
#pragma endregion
};
