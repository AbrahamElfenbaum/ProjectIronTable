// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatTab.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

#include "ChatChannel.h"

// Binds the tab button click delegate.
void UChatTab::NativeConstruct()
{
	Super::NativeConstruct();

	if (TabButton)
	{
		TabButton->OnClicked.AddDynamic(this, &UChatTab::OnTabButtonClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UChatTab::OnCloseButtonClicked);
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

// Broadcasts OnTabClosed with the stored channel pointer.
void UChatTab::OnCloseButtonClicked()
{
	OnTabClosed.Broadcast(Channel);
}

// Broadcasts OnTabRightClicked with the stored channel pointer.
void UChatTab::OnTabButtonRightClicked()
{
	OnTabRightClicked.Broadcast(Channel);
}

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

// Shows or hides the close button.
void UChatTab::SetCloseable(bool bShowButton)
{
	if (bShowButton)
	{
		CloseButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		CloseButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UChatTab::EnterRenameMode()
{
	if (!EditLabel || !TabLabel) return;

	EditLabel->SetText(FText::FromString(TabLabel->GetText().ToString()));
	TabLabel->SetVisibility(ESlateVisibility::Collapsed);
	EditLabel->SetVisibility(ESlateVisibility::Visible);
	EditLabel->SetKeyboardFocus();
}