// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseChannelTab.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

// Broadcasts OnTabClicked with the assigned channel.
void UBaseChannelTab::OnTabButtonClicked()
{
	OnTabClicked.Broadcast(Channel);
}

// Broadcasts OnTabRightClicked with the assigned channel.
void UBaseChannelTab::OnTabButtonRightClicked()
{
	OnTabRightClicked.Broadcast(Channel);
}

// Collapses the edit field, restores the label, and broadcasts OnTabRenamed if committed with a non-empty name.
void UBaseChannelTab::OnTabRenamedCompleted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!EditLabel || !TabLabel) return;

	EditLabel->SetVisibility(ESlateVisibility::Collapsed);
	TabLabel->SetVisibility(ESlateVisibility::Visible);

	if (CommitMethod == ETextCommit::OnEnter && !Text.IsEmpty())
	{
		TabLabel->SetText(Text);
		OnTabRenamed.Broadcast(this, Text.ToString());
	}
}

// Binds tab button click and rename field commit delegates; collapses the edit label by default.
void UBaseChannelTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UBaseChannelTab::OnTabButtonClicked);
	}

	if (EditLabel)
	{
		EditLabel->OnTextCommitted.AddDynamic(this, &UBaseChannelTab::OnTabRenamedCompleted);
		EditLabel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Detects right-click to broadcast OnTabRightClicked; left-click falls through to Super.
FReply UBaseChannelTab::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnTabButtonRightClicked();
	}
	else
	{
		Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}
	return FReply::Handled();
}

// Returns the channel this tab represents.
UBaseChannel* UBaseChannelTab::GetChannel() const
{
	return Channel;
}

// Assigns the channel this tab represents.
void UBaseChannelTab::SetChannel(UBaseChannel* InChannel)
{
	Channel = InChannel;
}

// Updates the displayed tab label text.
void UBaseChannelTab::SetLabel(const FString& Label)
{
	TabLabel->SetText(FText::FromString(Label));
}

// Makes the notification indicator visible.
void UBaseChannelTab::ShowNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Visible);
}

// Hides the notification indicator.
void UBaseChannelTab::ClearNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Collapsed);
}

// Enables or disables the tab button — used to prevent clicking the active tab.
void UBaseChannelTab::SetInteractable(bool bInteractable)
{
	TabButton->SetIsEnabled(bInteractable);
}

// Shows the editable text field and hides the label, placing focus on the field for immediate input.
void UBaseChannelTab::EnterRenameMode()
{
	if (!EditLabel || !TabLabel) return;

	EditLabel->SetText(FText::FromString(TabLabel->GetText().ToString()));
	TabLabel->SetVisibility(ESlateVisibility::Collapsed);
	EditLabel->SetVisibility(ESlateVisibility::Visible);
	EditLabel->SetKeyboardFocus();
}
