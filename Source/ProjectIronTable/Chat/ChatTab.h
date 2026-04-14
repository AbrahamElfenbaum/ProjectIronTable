// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatTab.generated.h"

class UChatChannel;
class UButton;
class UTextBlock;
class UEditableText;

/** Fired when the tab button is clicked, passing the channel this tab represents. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClicked, UChatChannel*, ClickedChannel);

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

	/** Label displaying the channel name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TabLabel;

	/** Inline text field shown during rename mode; hidden by default. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditLabel;

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
	/** Binds tab button and rename field delegates. */
	virtual void NativeConstruct() override;

	/** Detects right-click to broadcast OnTabRightClicked; left-click falls through to Super. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:

#pragma region Events
	/** Fired when the tab button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabClicked OnTabClicked;

	/** Fired when the tab button is right-clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnTabRightClicked OnTabRightClicked;

	/** Fired when the tab is renamed by the user. */
	UPROPERTY(BlueprintAssignable)
	FOnTabRenamed OnTabRenamed;
#pragma endregion

#pragma region Public Methods
	/** Returns the channel this tab represents. */
	UChatChannel* GetChannel() const;
	
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

	/** Shows the editable text field and hides the label, placing focus on the field for immediate input. */
	void EnterRenameMode();
#pragma endregion

private:

#pragma region Event Handlers
	/** Broadcasts OnTabClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonClicked();

	/** Broadcasts OnTabRightClicked with the assigned channel. */
	UFUNCTION()
	void OnTabButtonRightClicked();

	/** Broadcasts OnTabRenamed with this tab as the parameter. */
	UFUNCTION()
	void OnTabRenamedCompleted(const FText& Text, ETextCommit::Type CommitMethod);
#pragma endregion
};
