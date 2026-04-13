// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatTab.generated.h"

class UChatChannel;
class UButton;
class UTextBlock;
class UEditableTextBox;

/** Fired when the tab button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClicked, UChatChannel*, ClickedChannel);

/** Fired when the close button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClosed, UChatChannel*, ClickedChannel);

/** Fired when the tab button is right-clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabRightClicked, UChatChannel*, ClickedChannel);

/** Fired when the tab is renamed, passing this tab and the new name. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabRenamed, UChatTab*, SelectedTab, const FString&, NewName);

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

	/** */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditLabel;

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

	/** Detects right-click to broadcast OnTabRightClicked; left-click falls through to Super. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:

#pragma region Events
	/** Fired when the tab button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClicked OnTabClicked;

	/** Fired when the close button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClosed OnTabClosed;

	/** Fired when the tab button is right-clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabRightClicked OnTabRightClicked;

	/** Fired when the tab is renamed by the user. */
	UPROPERTY(BlueprintAssignable)
	FOnTabRenamed OnTabRenamed;
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

	/** Enables or disables the tab button — used to prevent clicking the active tab. */
	void SetInteractable(bool bInteractable);

	/** Shows or hides the close button — pass false for the Server tab which cannot be closed. */
	void SetCloseable(bool bShowButton);

	/** */
	void EnterRenameMode();
#pragma endregion

private:

#pragma region Event Handlers
	/** Broadcasts OnTabClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonClicked();

	/** Broadcasts OnTabClosed with the assigned channel. */
	UFUNCTION()
	void OnCloseButtonClicked();

	/** Broadcasts OnTabRightClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonRightClicked();

	/** Broadcasts OnTabRenamed with this tab as the parameter. */
	UFUNCTION()
	void OnTabRenamedCompleted(const FText& Text, ETextCommit::Type CommitMethod);
#pragma endregion
};
