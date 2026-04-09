// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatTab.generated.h"

class UChatChannel;
class UButton;
class UTextBlock;

/** Fired when the tab button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClicked, UChatChannel*, ClickedChannel);

/** Fired when the close button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClosed, UChatChannel*, ClickedChannel);

/** A clickable tab button that represents and activates a single UChatChannel. */
UCLASS()
class PROJECTIRONTABLE_API UChatTab : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Button the player clicks to switch to this channel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabButton;

	/** Button the player clicks to close and hide this tab. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	/** Label displaying the channel name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TabLabel;

	/** Small indicator widget shown when there are unread messages. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NotificationIndicator;
#pragma endregion

#pragma region State
	/** The channel this tab controls. */
	UPROPERTY()
	TObjectPtr<UChatChannel> Channel;
#pragma endregion

protected:
	/** Binds the tab and close button click delegates. */
	virtual void NativeConstruct() override;

public:

#pragma region Events
	/** Fired when the tab button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClicked OnTabClicked;

	/** Fired when the close button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClosed OnTabClosed;
#pragma endregion

#pragma region Public Methods
	/** Assigns the channel this tab represents. */
	void SetChannel(UChatChannel* InChannel);

	/** Updates the displayed tab label text. */
	void SetLabel(const FString& Label);

	/** Makes the notification indicator visible. */
	void ShowNotification();

	/** Hides the notification indicator. */
	void ClearNotification();
#pragma endregion

private:

	/** Broadcasts OnTabClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonClicked();

	/** Broadcasts OnTabClosed with the assigned channel. */
	UFUNCTION()
	void OnCloseButtonClicked();

public:
	/** Enables or disables the tab button — used to prevent clicking the active tab. */
	void SetInteractable(bool bInteractable);

	/** Shows or hides the close button — pass false for the Server tab which cannot be closed. */
	void SetCloseable(bool bShowButton);
};
