// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ChatTab.generated.h"

class UChatChannel;

/** Fired when the tab button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClicked, UChatChannel*, ClickedChannel);

/** A clickable tab button that represents and activates a single UChatChannel. */
UCLASS()
class PROJECTIRONTABLE_API UChatTab : public UUserWidget
{
	GENERATED_BODY()

private:

	// -- Widget References --

	/** Button the player clicks to switch to this channel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabButton;

	/** Label displaying the channel name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TabLabel;

	/** Small indicator widget shown when there are unread messages. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NotificationIndicator;

	// -- State --

	/** The channel this tab controls. */
	UPROPERTY()
	TObjectPtr<UChatChannel> Channel;

protected:
	virtual void NativeConstruct() override;

public:

	// -- Events --

	/** Fired when the tab button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClicked OnTabClicked;

	// -- Public Methods --

	/** Assigns the channel this tab represents. */
	void SetChannel(UChatChannel* InChannel);

	/** Updates the displayed tab label text. */
	void SetLabel(const FString& Label);

	/** Makes the notification indicator visible. */
	void ShowNotification();

	/** Hides the notification indicator. */
	void ClearNotification();

private:

	/** Broadcasts OnTabClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonClicked();

public:
	void SetInteractable(bool bInteractable);
};
