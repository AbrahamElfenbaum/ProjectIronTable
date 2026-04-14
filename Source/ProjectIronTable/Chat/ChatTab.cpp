// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatTab.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableText.h"

#include "ChatChannel.h"

// Binds tab button and rename field delegates.
void UChatTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UChatTab::OnTabButtonClicked);
	}

	if (EditLabel)
	{
		EditLabel->OnTextCommitted.AddDynamic(this, &UChatTab::OnTabRenamedCompleted);
		EditLabel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Detects right-click to broadcast OnTabRightClicked; falls through to Super for all other inputs.
FReply UChatTab::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

// Returns the channel this tab controls
UChatChannel* UChatTab::GetChannel() const
{
	return Channel;
}

// Stores the channel reference used when broadcasting tab click events.
void UChatTab::SetChannel(UChatChannel* InChannel)
{
	Channel = InChannel;
}

// Updates the tab label text to the given string.
void UChatTab::SetLabel(const FString& Label)
{
	TabLabel->SetText(FText::FromString(Label));
}

// Makes the notification indicator visible.
void UChatTab::ShowNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Visible);
}

// Hides the notification indicator.
void UChatTab::ClearNotification()
{
	NotificationIndicator->SetVisibility(ESlateVisibility::Collapsed);
}

// Broadcasts OnTabClicked with the stored channel pointer.
void UChatTab::OnTabButtonClicked()
{
	OnTabClicked.Broadcast(Channel);
}

// Broadcasts OnTabRightClicked with the stored channel pointer.
void UChatTab::OnTabButtonRightClicked()
{
	OnTabRightClicked.Broadcast(Channel);
}

// Hides the rename field and updates the tab label if the user committed with Enter.
void UChatTab::OnTabRenamedCompleted(const FText& Text, ETextCommit::Type CommitMethod)
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

// Enables or disables the tab button.
void UChatTab::SetInteractable(bool bInteractable)
{
	TabButton->SetIsEnabled(bInteractable);
}

// Populates the rename field with the current label, hides the label, and focuses the field.
void UChatTab::EnterRenameMode()
{
	if (!EditLabel || !TabLabel) return;

	EditLabel->SetText(FText::FromString(TabLabel->GetText().ToString()));
	TabLabel->SetVisibility(ESlateVisibility::Collapsed);
	EditLabel->SetVisibility(ESlateVisibility::Visible);
	EditLabel->SetKeyboardFocus();
}