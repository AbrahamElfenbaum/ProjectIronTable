// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionHUDComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "ChatBox.h"
#include "DiceSelectorManager.h"
#include "PlayerList.h"
#include "DiceSpawnVolume.h"
#include "Taskbar.h"
#include "DraggablePanel.h"
#include "TaskbarButton.h"
#include "PanelLayoutSave.h"
#include "FunctionLibrary.h"

// Disables tick and enables replication so server RPCs function correctly.
USessionHUDComponent::USessionHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Creates and adds the gameplay screen widget, then caches references to child widgets and wires up delegates.
void USessionHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	if (!IsValid(PlayerControllerRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — Owner is not a PlayerController"));
		return;
	}

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		GameplayScreenClass)
	{
		GameplayScreenRef = CreateWidget<UUserWidget>(GetWorld(), GameplayScreenClass);
		GameplayScreenRef->AddToViewport();

		DiceSelectorManagerRef = UFunctionLibrary::GetTypedWidgetFromName<UDiceSelectorManager>(GameplayScreenRef, TEXT("DiceSelectorManager"));
		ChatBoxRef = UFunctionLibrary::GetTypedWidgetFromName<UChatBox>(GameplayScreenRef, TEXT("ChatBox"));
		PlayerListRef = UFunctionLibrary::GetTypedWidgetFromName<UPlayerList>(GameplayScreenRef, TEXT("PlayerList"));
		TaskbarRef = UFunctionLibrary::GetTypedWidgetFromName<UTaskbar>(GameplayScreenRef, TEXT("Taskbar"));

		if (IsValid(DiceSelectorManagerRef))
		{
			UE_LOG(LogTemp, Display, TEXT("USessionHUDComponent::BeginPlay — DiceSelectorManager found"));
			DiceSelectorManagerRef->OnAllDiceRolled.AddDynamic(this, &USessionHUDComponent::AddRollResultToChat);
			DiceSelectorManagerRef->OnDiceFailsafeDestroyed.AddDynamic(this, &USessionHUDComponent::OnDiceFailsafeHandler);
			DiceSelectorManagerRef->OnRollInitiated.AddDynamic(this, &USessionHUDComponent::OnRollInitiated);

			ADiceSpawnVolume* SpawnVolume = Cast<ADiceSpawnVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ADiceSpawnVolume::StaticClass()));
			if (IsValid(SpawnVolume))
			{
				DiceSelectorManagerRef->SpawnVolume = SpawnVolume;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — DiceSpawnVolume not found in level"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — DiceSelectorManager not found"));
		}

		if (!IsValid(ChatBoxRef))
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — ChatBox not found"));
		}

		if (IsValid(PlayerListRef))
		{
			PlayerListRef->OnAddressClicked.AddDynamic(this, &USessionHUDComponent::OnPlayerAddressClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — PlayerList not found"));
		}

		if (IsValid(TaskbarRef))
		{
			DicePanel = FindAndRegisterPanel(TEXT("DicePanel"), TEXT("Dice"));
			ChatPanel = FindAndRegisterPanel(TEXT("ChatPanel"), TEXT("Chat"));;
			PlayersPanel = FindAndRegisterPanel(TEXT("PlayersPanel"), TEXT("Players"));
			//Register other widgets as needed
			LoadPanelLayout();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::BeginPlay — Taskbar not found"));
		}
	}
}

// Delegates focus to the chat box widget.
void USessionHUDComponent::FocusChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->FocusChat();
}

// Delegates exit to the chat box widget.
void USessionHUDComponent::ExitChat()
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->ExitChat();
}

// Delegates scroll direction to the chat box widget.
void USessionHUDComponent::ScrollChat(bool bUp)
{
	if (IsValid(ChatBoxRef)) ChatBoxRef->Scroll(bUp);
}

// Finds a DraggablePanel by widget name, registers it with the Taskbar, assigns its PanelID, and binds save delegates.
UDraggablePanel* USessionHUDComponent::FindAndRegisterPanel(const FName& WidgetName, const FString& Label)
{
	UDraggablePanel* Panel = UFunctionLibrary::GetTypedWidgetFromName<UDraggablePanel>(GameplayScreenRef, WidgetName);
	if (IsValid(Panel))
	{
		UTaskbarButton* Button = TaskbarRef->RegisterWidget(Panel, Label);
		Panel->SetPanelID(Label);
		Panel->OnPanelStateChanged.AddDynamic(this, &USessionHUDComponent::SavePanelLayout);
		Button->OnToggled.AddDynamic(this, &USessionHUDComponent::SavePanelLayout);

		return Panel;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Taskbar not found, cannot register %s panel"), *Label);
		return nullptr;
	}

}

// Collects layout data from all panels and writes it to the PanelLayout save slot.
void USessionHUDComponent::SavePanelLayout()
{
	UPanelLayoutSave* PanelLayoutSave = NewObject<UPanelLayoutSave>();
	if (!IsValid(PanelLayoutSave))
	{
		UE_LOG(LogTemp, Error, TEXT("USessionHUDComponent::SavePanelLayout — Failed to create PanelLayoutSave object"));
		return;
	}
	SavePanelLayout(DicePanel, PanelLayoutSave);
	SavePanelLayout(ChatPanel, PanelLayoutSave);
	SavePanelLayout(PlayersPanel, PanelLayoutSave);
	//Apply other panels as needed

	UGameplayStatics::SaveGameToSlot(PanelLayoutSave, UPanelLayoutSave::SaveSlotName, 0);
}

// Notifies the chat box that a roll has been initiated so it can prepare for incoming roll result messages.
void USessionHUDComponent::OnRollInitiated()
{
	if (!IsValid(ChatBoxRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::OnRollInitiated — ChatBoxRef is null"));
		return;
	}
	ChatBoxRef->TrySendPrivateRollMessage();
}

// Loads the PanelLayout save slot and applies stored position, size, and visibility to each panel.
void USessionHUDComponent::LoadPanelLayout()
{
	if (UGameplayStatics::DoesSaveGameExist(UPanelLayoutSave::SaveSlotName, 0))
	{
		UPanelLayoutSave* LoadedLayout = Cast<UPanelLayoutSave>(UGameplayStatics::LoadGameFromSlot(UPanelLayoutSave::SaveSlotName, 0));
		if (LoadedLayout)
		{
			ApplyPanelLayout(DicePanel, LoadedLayout);
			ApplyPanelLayout(ChatPanel, LoadedLayout);
			ApplyPanelLayout(PlayersPanel, LoadedLayout);
			//Apply other panels as needed
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load panel layout save"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("No existing panel layout save found"));
	}
}

// Adds the panel's current position, size, and visibility to the save object under its PanelID key.
void USessionHUDComponent::SavePanelLayout(const UDraggablePanel* Panel, UPanelLayoutSave* LayoutSave)
{
	if (Panel)
	{
		LayoutSave->PanelLayouts.Add(Panel->GetPanelID(), Panel->GetPanelLayoutData());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Panel not found, cannot apply layout"));
	}
}

// Looks up the panel's saved layout by PanelID and applies position, size, and visibility if found.
void USessionHUDComponent::ApplyPanelLayout(UDraggablePanel* Panel, UPanelLayoutSave* LoadedLayout)
{
	if (Panel)
	{
		if (const FPanelLayoutData* Data = LoadedLayout->PanelLayouts.Find(Panel->GetPanelID()))
		{
			Panel->ApplyPanelLayoutData(*Data);
		}
	}
}

// Delivers the incoming message to the chat box on the owning client.
void USessionHUDComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)
{
	if (!IsValid(ChatBoxRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::AddChatMessageOnOwningClient — ChatBoxRef is null"));
		return;
	}
	ChatBoxRef->AddChatMessage(Message, Recipients, bIsSender);
}

// Resolves sender name, builds the participant list, then delivers the message to each relevant player's HUD component.
void USessionHUDComponent::SendChatMessageOnServer_Implementation(const FString& Message, const TArray<FString>& Recipients)
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
	if (!IsValid(GS))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::SendChatMessageOnServer — GameState is null"));
		return;
	}

	//Send the message to each player's HUD component if they're a participant (or if broadcast)
	for (APlayerState* Play : GS->PlayerArray)
	{
		bool bIsParticipant = Participants.IsEmpty() || //Is it a broadcast?
			Participants.Contains(Play->GetPlayerName()); //Is the player a participant?

		if (!bIsParticipant || !Play->GetPlayerController()) continue;

		USessionHUDComponent* HUDComp = Cast<USessionHUDComponent>(
			Play->GetPlayerController()->GetComponentByClass(USessionHUDComponent::StaticClass()));
		if (HUDComp)
		{
			HUDComp->AddChatMessageOnOwningClient(Message, Participants, Play->GetPlayerName() == SenderName);
		}
	}
}

// Builds a formatted roll result string and sends it to the server for broadcast.
void USessionHUDComponent::AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode)
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

	if (IsValid(ChatBoxRef))
	{
		SendChatMessageOnServer(Message, ChatBoxRef->GetActiveChannelParticipants());
	}
}

// Sends a chat message to the server noting that a die of the given type was lost to the failsafe timer.
void USessionHUDComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	FString DiceTypeName = UEnum::GetValueAsString(DiceType);
	DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);

	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName), {});
}

// Appends the player's name as an @mention in the chat input field.
void USessionHUDComponent::OnPlayerAddressClicked(const FString& PlayerName)
{
	if (!IsValid(ChatBoxRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("USessionHUDComponent::OnPlayerAddressClicked — ChatBoxRef is null"));
		return;
	}
	ChatBoxRef->AppendToInput(TEXT("@") + PlayerName + TEXT(" "));
}
