// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionChatComponent.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "BaseDiceActor.h"
#include "ChatBox.h"
#include "DiceTray.h"
#include "FunctionLibrary.h"
#include "MacroLibrary.h"
#include "PlayerList.h"
#include "SessionController.h"
#include "SessionInstance.h"
#include "SessionSave.h"
#include "SessionUIComponent.h"

// Disables tick and enables replication so server RPCs function correctly.
USessionChatComponent::USessionChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USessionChatComponent::Init()
{
	SessionControllerRef = Cast<ASessionController>(GetOwner());

	CHECK_IF_VALID(SessionControllerRef, );

	if (SessionControllerRef->IsLocalController())
	{
		ChatBoxRef = SessionControllerRef->UIComponent->GetChatBox();

		if (IsValid(ChatBoxRef))
		{
			ChatBoxRef->SetChatComponent(this);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionChatComponent::Init — ChatBox not found on UIComponent"));
		}

		DiceTrayRef = SessionControllerRef->UIComponent->GetDiceTray();
		if (IsValid(DiceTrayRef))
		{
			DiceTrayRef->OnAllDiceRolled.AddDynamic(this, &USessionChatComponent::AddRollResultToChat);
			DiceTrayRef->OnDiceFailsafeDestroyed.AddDynamic(this, &USessionChatComponent::OnDiceFailsafeHandler);
			DiceTrayRef->OnRollInitiated.AddDynamic(this, &USessionChatComponent::OnRollInitiated);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionChatComponent::Init — DiceTray not found on UIComponent"));
		}

		PlayerListRef = SessionControllerRef->UIComponent->GetPlayerList();
		if (IsValid(PlayerListRef))
		{
			PlayerListRef->OnAddressClicked.AddDynamic(this, &USessionChatComponent::OnPlayerAddressClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionChatComponent::Init — PlayerList not found on UIComponent"));
		}
	}
}

// Resolves sender name, builds the participant list, then delivers the message to each relevant player's HUD component.
void USessionChatComponent::SendChatMessageOnServer_Implementation(const FString& Message, const TArray<FString>& Recipients)
{
	//Get sender's player name for prefixing the message. If we can't get it for some reason, default to "Unknown"
	APlayerController* SenderPC = Cast<APlayerController>(GetOwner());
	FString SenderName = IsValid(SenderPC) && IsValid(SenderPC->PlayerState) ? SenderPC->PlayerState->GetPlayerName() : TEXT("Unknown");

	//Build full participants list (sender + recipients). Empty Recipients means broadcast, keep it empty.
	TArray<FString> Participants;
	if (!Recipients.IsEmpty())
	{
		Participants = Recipients;
		Participants.AddUnique(SenderName);
	}

	//Get the game state to access the player array
	UWorld* World = GetWorld();
	CHECK_IF_VALID(World, );
	AGameStateBase* GS = World->GetGameState<AGameStateBase>();
	CHECK_IF_VALID(GS, );

	//Send the message to each player's HUD component if they're a participant (or if broadcast)
	for (APlayerState* Play : GS->PlayerArray)
	{
		bool bIsParticipant = Participants.IsEmpty() || //Is it a broadcast?
			Participants.Contains(Play->GetPlayerName()); //Is the player a participant?

		if (!bIsParticipant || !Play->GetPlayerController()) continue;

		USessionChatComponent* ChatComp = Cast<USessionChatComponent>(
			Play->GetPlayerController()->GetComponentByClass(USessionChatComponent::StaticClass()));
		if (IsValid(ChatComp))
		{
			ChatComp->AddChatMessageOnOwningClient(Message, Participants, Play->GetPlayerName() == SenderName);
		}
	}

	int32 ColonIndex = Message.Find(TEXT(": "));
	FString Body = Message.RightChop(ColonIndex + 2);
	FString sParticipants = UFunctionLibrary::MakeParticipantKey(Participants);

	FChatMessageRecord MessageRecord = { SenderName , Body };

	USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(GetOwner());
	if (IsValid(SessionSave))
	{
		FChatLogRecord& LogRecord = SessionSave->ChatLog.FindOrAdd(sParticipants);
		LogRecord.Messages.Add(MessageRecord);
		UGameplayStatics::SaveGameToSlot(SessionSave, UFunctionLibrary::GetSessionSaveSlotName(GetOwner()->GetGameInstance<USessionInstance>()), 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionChatComponent::SendChatMessageOnServer � Failed to load session save; message will not be persisted"));
	}
}

// Delivers the incoming message to the chat box on the owning client.
void USessionChatComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->AddChatMessage(Message, Recipients, bIsSender);
}

// Delegates focus to the chat box widget.
void USessionChatComponent::FocusChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->FocusChat();
}

// Delegates exit to the chat box widget.
void USessionChatComponent::ExitChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->ExitChat();
}

// Delegates scroll direction to the chat box widget.
void USessionChatComponent::ScrollChat(bool bUp)
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->Scroll(bUp);
}

// Builds a formatted roll result string and sends it to the server for broadcast.
void USessionChatComponent::AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode)
{
	//Message starts with the player name
	FString Message = UFunctionLibrary::GetLocalPlayerName(GetOwner());

	if (RollMode == EDiceRollMode::Advantage)
	{
		Message += TEXT(" Rolled with Advantage: ");
	}
	else if (RollMode == EDiceRollMode::Disadvantage)
	{
		Message += TEXT(" Rolled with Disadvantage: ");
	}
	else
	{
		Message += TEXT(" Rolled: ");
	}

	//Add in each rolled result on a new line
	for (const FRollResult& Result : Results)
	{
		FString DiceTypeName = UFunctionLibrary::GetEnumDisplayName(Result.DiceType);
		Message += FString::Printf(TEXT("%d on a %s\n"), Result.Value, *DiceTypeName);
	}

	//Trim the last newline off the end of the message and send it to the server to be broadcast to all clients
	Message.TrimEndInline();

	if (IsValid(ChatBoxRef))
	{
		SendChatMessageOnServer(Message, ChatBoxRef->GetActiveChannelParticipants());
	}
}

// Sends a chat message to the server noting that a die of the given type was lost to the failsafe timer.
void USessionChatComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = UFunctionLibrary::GetLocalPlayerName(GetOwner());
	FString DiceTypeName = UFunctionLibrary::GetEnumDisplayName(DiceType);
	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName), {});
}

// Appends the player's name as an @mention in the chat input field.
void USessionChatComponent::OnPlayerAddressClicked(const FString& PlayerName)
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->AppendToInput(TEXT("@") + PlayerName + TEXT(" "));
}

// Notifies the chat box that a roll has been initiated so it can prepare for incoming roll result messages.
void USessionChatComponent::OnRollInitiated()
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->TrySendPrivateRollMessage();
}