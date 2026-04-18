// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionUIComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "ChatBox.h"
#include "ChatChannel.h"
#include "ChatTab.h"
#include "DiceTray.h"
#include "PlayerList.h"
#include "DiceSpawnVolume.h"
#include "Taskbar.h"
#include "DraggablePanel.h"
#include "TaskbarButton.h"
#include "PanelLayoutSave.h"
#include "FunctionLibrary.h"
#include "SessionInstance.h"
#include "SessionNotesPanel.h"
#include "SessionSave.h"
#include "MacroLibrary.h"

// Disables tick and enables replication so server RPCs function correctly.
USessionUIComponent::USessionUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Creates and adds the session screen widget, then caches references to child widgets and wires up delegates.
void USessionUIComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	CHECK_IF_VALID(PlayerControllerRef, );

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		SessionScreenClass)
	{
		SessionScreenRef = CreateWidget<UUserWidget>(PlayerControllerRef, SessionScreenClass);
		if (!IsValid(SessionScreenRef))
		{
			UE_LOG(LogTemp, Error, TEXT("USessionUIComponent::BeginPlay � Failed to create SessionScreen widget"));
			return;
		}
		SessionScreenRef->AddToViewport();

		DiceTrayRef = UFunctionLibrary::GetTypedWidgetFromName<UDiceTray>(SessionScreenRef, TEXT("DiceTray"));
		ChatBoxRef = UFunctionLibrary::GetTypedWidgetFromName<UChatBox>(SessionScreenRef, TEXT("ChatBox"));
		PlayerListRef = UFunctionLibrary::GetTypedWidgetFromName<UPlayerList>(SessionScreenRef, TEXT("PlayerList"));
		SessionNotesPanelRef = UFunctionLibrary::GetTypedWidgetFromName<USessionNotesPanel>(SessionScreenRef, TEXT("SessionNotes"));
		TaskbarRef = UFunctionLibrary::GetTypedWidgetFromName<UTaskbar>(SessionScreenRef, TEXT("Taskbar"));

		if (IsValid(DiceTrayRef))
		{
			UE_LOG(LogTemp, Display, TEXT("USessionUIComponent::BeginPlay � DiceTray found"));
			DiceTrayRef->OnAllDiceRolled.AddDynamic(this, &USessionUIComponent::AddRollResultToChat);
			DiceTrayRef->OnDiceFailsafeDestroyed.AddDynamic(this, &USessionUIComponent::OnDiceFailsafeHandler);
			DiceTrayRef->OnRollInitiated.AddDynamic(this, &USessionUIComponent::OnRollInitiated);

			ADiceSpawnVolume* SpawnVolume = Cast<ADiceSpawnVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ADiceSpawnVolume::StaticClass()));
			if (IsValid(SpawnVolume))
			{
				DiceTrayRef->SpawnVolume = SpawnVolume;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � DiceSpawnVolume not found in level"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � DiceTray not found"));
		}

		if (IsValid(ChatBoxRef))
		{
			USessionSave* SessionSave = UFunctionLibrary::LoadSessionSave(GetOwner());
			if (IsValid(SessionSave))
			{
				for (const TPair<FString, FChatLogRecord>& ChatLog : SessionSave->ChatLog)
				{
					TArray<FString> Recipients;
					ChatLog.Key.ParseIntoArray(Recipients, TEXT("|"), 1);

					UChatChannel* Channel = ChatBoxRef->FindOrCreateChannel(Recipients);

					const TArray<FChatMessageRecord>& Messages = ChatLog.Value.Messages;
					for (const FChatMessageRecord& Message : Messages)
					{
						Channel->RestoreMessage(Message.SenderName, Message.Message);
					}
				}

				FString PlayerName = UFunctionLibrary::GetLocalPlayerName(GetOwner());
				for (const TPair <FString, FString>& ChatTabName : SessionSave->ChatTabNames)
				{
					TArray<FString> Names;
					ChatTabName.Key.ParseIntoArray(Names, TEXT("|"), 1);
					Names.Remove(PlayerName);
					if (!Names.IsEmpty())
					{
						UChatChannel* Channel = ChatBoxRef->FindOrCreateChannel(Names);
						UChatTab* Tab = ChatBoxRef->GetTabForChannel(Channel);
						if (IsValid(Tab))
						{
							Tab->SetLabel(ChatTabName.Value);
						}
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � Failed to load session save; chat log and tab names will not be restored"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � ChatBox not found"));
		}

		if (IsValid(PlayerListRef))
		{
			PlayerListRef->OnAddressClicked.AddDynamic(this, &USessionUIComponent::OnPlayerAddressClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � PlayerList not found"));
		}

		if (IsValid(SessionNotesPanelRef))
		{

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � SessionNotesPanel not found"));
		}

		if (IsValid(TaskbarRef))
		{
			DiceTrayPanel = FindAndRegisterPanel(TEXT("DiceTrayPanel"), TEXT("Dice"));
			ChatPanel = FindAndRegisterPanel(TEXT("ChatPanel"), TEXT("Chat"));
			PlayersPanel = FindAndRegisterPanel(TEXT("PlayersPanel"), TEXT("Players"));
			SessionNotesPanel = FindAndRegisterPanel(TEXT("SessionNotesPanel"), TEXT("Session Notes"));
			Panels = { DiceTrayPanel, ChatPanel, PlayersPanel, SessionNotesPanel };
			//Register and add other widgets to Panels array as needed
			LoadPanelLayout();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::BeginPlay � Taskbar not found"));
		}
	}
}

// Resolves sender name, builds the participant list, then delivers the message to each relevant player's HUD component.
void USessionUIComponent::SendChatMessageOnServer_Implementation(const FString& Message, const TArray<FString>& Recipients)
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

		USessionUIComponent* UIComp = Cast<USessionUIComponent>(
			Play->GetPlayerController()->GetComponentByClass(USessionUIComponent::StaticClass()));
		if (IsValid(UIComp))
		{
			UIComp->AddChatMessageOnOwningClient(Message, Participants, Play->GetPlayerName() == SenderName);
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
		UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::SendChatMessageOnServer � Failed to load session save; message will not be persisted"));
	}
}

// Delivers the incoming message to the chat box on the owning client.
void USessionUIComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->AddChatMessage(Message, Recipients, bIsSender);
}

// Delegates focus to the chat box widget.
void USessionUIComponent::FocusChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->FocusChat();
}

// Delegates exit to the chat box widget.
void USessionUIComponent::ExitChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->ExitChat();
}

// Delegates scroll direction to the chat box widget.
void USessionUIComponent::ScrollChat(bool bUp)
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->Scroll(bUp);
}

// Builds a formatted roll result string and sends it to the server for broadcast.
void USessionUIComponent::AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode)
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
void USessionUIComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = UFunctionLibrary::GetLocalPlayerName(GetOwner());
	FString DiceTypeName = UFunctionLibrary::GetEnumDisplayName(DiceType);
	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName), {});
}

// Appends the player's name as an @mention in the chat input field.
void USessionUIComponent::OnPlayerAddressClicked(const FString& PlayerName)
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->AppendToInput(TEXT("@") + PlayerName + TEXT(" "));
}

// Notifies the chat box that a roll has been initiated so it can prepare for incoming roll result messages.
void USessionUIComponent::OnRollInitiated()
{
	CHECK_IF_VALID(ChatBoxRef, );
	ChatBoxRef->TrySendPrivateRollMessage();
}

// Adds the panel's current position, size, and visibility to the save object under its PanelID key.
void USessionUIComponent::SavePanelLayout()
{
	UPanelLayoutSave* PanelLayoutSave = NewObject<UPanelLayoutSave>();
	if (!IsValid(PanelLayoutSave))
	{
		UE_LOG(LogTemp, Error, TEXT("USessionUIComponent::SavePanelLayout � Failed to create PanelLayoutSave object"));
		return;
	}

	for (UDraggablePanel* Panel : Panels)
	{
		SavePanelLayout(Panel, PanelLayoutSave);
	}

	UGameplayStatics::SaveGameToSlot(PanelLayoutSave, UPanelLayoutSave::SaveSlotName, 0);
}

// Finds a DraggablePanel by widget name, registers it with the Taskbar, assigns its PanelID, and binds save delegates.
UDraggablePanel* USessionUIComponent::FindAndRegisterPanel(const FName& WidgetName, const FString& Label)
{
	UDraggablePanel* Panel = UFunctionLibrary::GetTypedWidgetFromName<UDraggablePanel>(SessionScreenRef, WidgetName);
	if (IsValid(Panel))
	{
		UTaskbarButton* Button = TaskbarRef->RegisterWidget(Panel, Label);
		Panel->SetPanelID(Label);
		Panel->OnPanelStateChanged.AddDynamic(this, &USessionUIComponent::SavePanelLayout);
		Button->OnToggled.AddDynamic(this, &USessionUIComponent::SavePanelLayout);

		return Panel;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::FindAndRegisterPanel � Panel '%s' not found in session screen"), *Label);
		return nullptr;
	}
}

// Loads the PanelLayout save slot and applies stored position, size, and visibility to each panel.
void USessionUIComponent::LoadPanelLayout()
{
	if (UGameplayStatics::DoesSaveGameExist(UPanelLayoutSave::SaveSlotName, 0))
	{
		UPanelLayoutSave* LoadedLayout = Cast<UPanelLayoutSave>(UGameplayStatics::LoadGameFromSlot(UPanelLayoutSave::SaveSlotName, 0));
		if (IsValid(LoadedLayout))
		{
			for (UDraggablePanel* Panel : Panels)
			{
				ApplyPanelLayout(Panel, LoadedLayout);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::LoadPanelLayout � Failed to load panel layout save"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("USessionUIComponent::LoadPanelLayout � No existing panel layout save found"));
	}
}

// Adds the panel's current position, size, and visibility to the save object under its PanelID key.
void USessionUIComponent::SavePanelLayout(const UDraggablePanel* Panel, UPanelLayoutSave* LayoutSave)
{
	if (IsValid(Panel))
	{
		LayoutSave->PanelLayouts.Add(Panel->GetPanelID(), Panel->GetPanelLayoutData());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::SavePanelLayout � Panel is null; layout data will not be saved"));
	}
}

// Looks up the panel's saved layout by PanelID and applies position, size, and visibility if found.
void USessionUIComponent::ApplyPanelLayout(UDraggablePanel* Panel, UPanelLayoutSave* LoadedLayout)
{
	if (IsValid(Panel))
	{
		if (const FPanelLayoutData* Data = LoadedLayout->PanelLayouts.Find(Panel->GetPanelID()))
		{
			Panel->ApplyPanelLayoutData(*Data);
		}
	}
}