// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"
#include "ChatEntry.h"
#include "ChatTab.h"
#include "ChatChannel.h"
#include "GameplayHUDComponent.h"
#include "GameFramework/PlayerState.h"

void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		HUDComponentRef = Cast<UGameplayHUDComponent>(
			PC->GetComponentByClass(UGameplayHUDComponent::StaticClass()));
	}

	EditableText->OnTextCommitted.AddDynamic(this, &UChatBox::OnTextCommitted);
	SwitchToChannel(CreateChannel({}));
}

FReply UChatBox::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bChatFocused)
	{
		FocusChat();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UChatBox::FocusChat()
{
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		EditableText->SetUserFocus(PC);
		bChatFocused = true;
		EditableText->SetIsEnabled(true);
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void UChatBox::ExitChat()
{
	bChatFocused = false;
	EditableText->SetIsEnabled(false);
	EditableText->SetText(FText::GetEmpty());
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

UChatChannel* UChatBox::CreateChannel(TArray<FString> Participants)
{
	//Build the label based on the participants
	FString Label;

	if (Participants.IsEmpty())
	{
		Label = TEXT("Server");
	}
	else if (Participants.Num() == 1)
	{
		Label = TEXT("@") + Participants[0];
	}
	else
	{
		Label = FString::Printf(TEXT("@%s +%d"), *Participants[0], Participants.Num() - 1);
	}

	//Channel widget creation and setup
	UChatChannel* Channel = CreateWidget<UChatChannel>(GetOwningPlayer(), ChannelClass);
	Channel->ChatEntryClass = ChatEntryClass;
	Channel->DisplayName = Label;
	Channel->Participants = Participants;

	Channels.Add(Channel);
	ChannelContainer->AddChild(Channel);

	//Tab widget creation and setup
	UChatTab* Tab = CreateWidget<UChatTab>(GetOwningPlayer(), TabClass);
	Tab->SetChannel(Channel);
	Tab->SetLabel(Label);
	Tab->OnTabClicked.AddDynamic(this, &UChatBox::SwitchToChannel);

	ChannelTabMap.Add(Channel, Tab);

	TabBar->AddChild(Tab);

	return Channel;
}

void UChatBox::SwitchToChannel(UChatChannel* Channel)
{
	ActiveChannel = Channel;
	ChannelContainer->SetActiveWidget(Channel);
	ChannelTabMap[Channel]->ClearNotification();
}

void UChatBox::AddChatMessage(const FString& Message, TArray<FString> Participants, bool bIsSender)
{
	UChatChannel* CurrentChannel = nullptr;

	for (UChatChannel* Channel : Channels)
	{
		bool bChannelFound = true;
		for (FString Participant : Participants)
		{
			if(!Channel->Participants.Contains(Participant))
			{
				bChannelFound = false;
				break;
			}
		}
		if (bChannelFound)
		{
			CurrentChannel = Channel;
			break;
		}
	}

	if (!CurrentChannel)
	{
		CurrentChannel = CreateChannel(Participants);
	}

	CurrentChannel->AddChatMessage(Message);

	if (bIsSender)
	{
		SwitchToChannel(CurrentChannel);
	}
	else if (CurrentChannel != ActiveChannel)
	{
		ChannelTabMap[CurrentChannel]->ShowNotification();
	}
}

//void UChatBox::Scroll(bool bUp)
//{
//	int32 scrollDirection = bUp ? 1 : -1;
//
//	ScrollBox->SetScrollOffset(
//		FMath::Clamp(
//			ScrollBox->GetScrollOffset() + (ScrollMultiplier * scrollDirection),
//			0.0f,
//			ScrollBox->GetScrollOffsetOfEnd()));
//}

//void UChatBox::AddChatMessage(const FString& Message)
//{
//	if (!ChatEntryClass) return;
//	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(GetWorld(), ChatEntryClass);
//	ChatEntry->Message = Message;
//	ScrollBox->AddChild(ChatEntry);
//	ScrollBox->ScrollToEnd();
//}

void UChatBox::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString Message = Text.ToString();

		if (!Message.IsEmpty())
		{
			APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
			if (PC && PC->PlayerState)
			{
				FString PlayerName = PC->PlayerState->GetPlayerName();
				FString FullMessage = FString::Printf(TEXT("%s: %s"), *PlayerName, *Message);
				HUDComponentRef->SendChatMessageOnServer(FullMessage);
			}

			EditableText->SetText(FText::GetEmpty());

			FocusChat();
		}
	}
	else if (CommitMethod == ETextCommit::OnUserMovedFocus ||
		     CommitMethod == ETextCommit::OnCleared)
	{
		ExitChat();
	}
}
