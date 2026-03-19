// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "ChatBox.h"
#include "ChatEntry.h"
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
}

void UChatBox::FocusChat()
{
	APlayerController* PC = Cast<APlayerController>(GetOwningPlayer());
	if (PC)
	{
		EditableText->SetUserFocus(PC);
		bChatFocused = true;
		EditableText->SetIsEnabled(true);
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

void UChatBox::Scroll(bool bUp)
{
	int32 scrollDirection = bUp ? 1 : -1;

	ScrollBox->SetScrollOffset(
		FMath::Clamp(
			ScrollBox->GetScrollOffset() + (ScrollMultiplier * scrollDirection),
			0.0f, 
			ScrollBox->GetScrollOffsetOfEnd()));
}

void UChatBox::AddChatMessage(const FString& Message)
{
	if (!ChatEntryClass) return;
	UChatEntry* ChatEntry = CreateWidget<UChatEntry>(GetWorld(), ChatEntryClass);
	ChatEntry->Message = Message;
	ScrollBox->AddChild(ChatEntry);
	ScrollBox->ScrollToEnd();
}

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
		}

		ExitChat();
	}
}
