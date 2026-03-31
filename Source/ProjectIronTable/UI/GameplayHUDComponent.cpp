// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DiceSelectorManager.h"
#include "Kismet/GameplayStatics.h"
#include "DiceSpawnVolume.h"
#include "Taskbar.h"
#include "DraggablePanel.h"

// Disables tick and enables replication so server RPCs function correctly.
UGameplayHUDComponent::UGameplayHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

// Creates and adds the gameplay screen widget, then caches references to child widgets and wires up delegates.
void UGameplayHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		GameplayScreenClass)
	{
		GameplayScreenRef = CreateWidget<UUserWidget>(GetWorld(), GameplayScreenClass);
		GameplayScreenRef->AddToViewport();

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerControllerRef->SetInputMode(InputMode);
		PlayerControllerRef->bShowMouseCursor = true;


		DiceSelectorManagerRef = Cast<UDiceSelectorManager>(GameplayScreenRef->GetWidgetFromName(TEXT("DiceSelectorManager")));
		ChatBoxRef = Cast<UChatBox>(GameplayScreenRef->GetWidgetFromName(TEXT("ChatBox")));
		PlayerListRef = Cast<UPlayerList>(GameplayScreenRef->GetWidgetFromName(TEXT("PlayerList")));
		TaskbarRef = Cast<UTaskbar>(GameplayScreenRef->GetWidgetFromName(TEXT("Taskbar")));

		if (DiceSelectorManagerRef)
		{
			UE_LOG(LogTemp, Display, TEXT("Dice Selector Found"));
			DiceSelectorManagerRef->OnAllDiceRolled.AddDynamic(this, &UGameplayHUDComponent::AddRollResultToChat);
			DiceSelectorManagerRef->OnDiceFailsafeDestroyed.AddDynamic(this, &UGameplayHUDComponent::OnDiceFailsafeHandler);

			ADiceSpawnVolume* SpawnVolume = Cast<ADiceSpawnVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ADiceSpawnVolume::StaticClass()));
			if (SpawnVolume)
			{
				DiceSelectorManagerRef->SpawnVolume = SpawnVolume;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DiceSpawnVolume not found in level!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dice Selector Not Found"));
		}

		if (!ChatBoxRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("Chat Box Not Found"));
		}

		if (PlayerListRef)
		{
			PlayerListRef->OnAddressClicked.AddDynamic(this, &UGameplayHUDComponent::OnPlayerAddressClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player List Not Found"));
		}

		if (TaskbarRef)
		{
			DicePanel = FindAndRegisterPanel(TEXT("DicePanel"), TEXT("Dice"));
			ChatPanel = FindAndRegisterPanel(TEXT("ChatPanel"), TEXT("Chat"));;
			PlayersPanel = FindAndRegisterPanel(TEXT("PlayersPanel"), TEXT("Players"));
			//Register other widgets as needed
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Taskbar Not Found"));
		}
	}
}

// Delegates focus to the chat box widget.
void UGameplayHUDComponent::FocusChat()
{
	if (ChatBoxRef) ChatBoxRef->FocusChat();
}

// Delegates exit to the chat box widget.
void UGameplayHUDComponent::ExitChat()
{
	if (ChatBoxRef) ChatBoxRef->ExitChat();
}

// Delegates scroll direction to the chat box widget.
void UGameplayHUDComponent::ScrollChat(bool bUp)
{
	if (ChatBoxRef) ChatBoxRef->Scroll(bUp);
}

UDraggablePanel* UGameplayHUDComponent::FindAndRegisterPanel(const FName& WidgetName, const FString& Label)
{
	UDraggablePanel* Panel = Cast<UDraggablePanel>(GameplayScreenRef->GetWidgetFromName(WidgetName));
	if (Panel)
	{
		TaskbarRef->RegisterWidget(Panel, Label);
		return Panel;		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Taskbar not found, cannot register %s panel"), *Label);
		return nullptr;
	}
	
}

// Delivers the incoming message to the chat box on the owning client.
void UGameplayHUDComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)
{
	ChatBoxRef->AddChatMessage(Message, Recipients, bIsSender);
}

// Resolves sender name, builds the participant list, then delivers the message to each relevant player's HUD component.
void UGameplayHUDComponent::SendChatMessageOnServer_Implementation(const FString& Message, const TArray<FString>& Recipients)
{
	//Get sender's player name for prefixing the message. If we can't get it for some reason, default to "Unknown"
	APlayerController* SenderPC = Cast<APlayerController>(GetOwner());
	FString SenderName = SenderPC && SenderPC->PlayerState ? SenderPC->PlayerState->GetPlayerName() : TEXT("Unknown");

	//Build full participants list (sender + recipients). Empty Recipients means broadcast, keep it empty.
	TArray<FString> Participants;
	if (!Recipients.IsEmpty())
	{
		Participants = Recipients;
		Participants.AddUnique(SenderName);
	}

	//Get the game state to access the player array
	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();

	//Send the message to each player's HUD component if they're a participant (or if broadcast)
	for (APlayerState* Play : GS->PlayerArray)
	{
		bool bIsParticipant = Participants.IsEmpty() || //Is it a broadcast?
			Participants.Contains(Play->GetPlayerName()); //Is the player a participant?

		if (!bIsParticipant || !Play->GetPlayerController()) continue;

		UGameplayHUDComponent* HUDComp = Cast<UGameplayHUDComponent>(
			Play->GetPlayerController()->GetComponentByClass(UGameplayHUDComponent::StaticClass()));
		if (HUDComp)
		{
			HUDComp->AddChatMessageOnOwningClient(Message, Participants, Play->GetPlayerName() == SenderName);
		}
	}
}

// Builds a formatted roll result string and sends it to the server for broadcast.
void UGameplayHUDComponent::AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode)
{
	//Message starts with the player name
	FString Message = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

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
		FString DiceTypeName = UEnum::GetValueAsString(Result.DiceType);
		DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);
		Message += FString::Printf(TEXT("%d on a %s\n"), Result.Value, *DiceTypeName);
	}

	//Trim the last newline off the end of the message and send it to the server to be broadcast to all clients
	Message.TrimEndInline();

	if (ChatBoxRef)
	{
		SendChatMessageOnServer(Message, ChatBoxRef->GetActiveChannelParticipants());
	}
}

// Sends a chat message to the server noting that a die of the given type was lost to the failsafe timer.
void UGameplayHUDComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	FString DiceTypeName = UEnum::GetValueAsString(DiceType);
	DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);

	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName), {});
}

// Appends the player's name as an @mention in the chat input field.
void UGameplayHUDComponent::OnPlayerAddressClicked(const FString& PlayerName)
{
	ChatBoxRef->AppendToInput(TEXT("@") + PlayerName + TEXT(" "));
}
