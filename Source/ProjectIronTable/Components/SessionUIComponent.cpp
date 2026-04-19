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

// Creates and initializes all session UI widgets, binds delegates, and loads the saved panel layout.
void USessionUIComponent::Init()
{
	PlayerControllerRef = Cast<APlayerController>(GetOwner());

	CHECK_IF_VALID(PlayerControllerRef, );

	if (PlayerControllerRef &&
		PlayerControllerRef->IsLocalPlayerController() &&
		SessionScreenClass)
	{
		SessionScreenRef = CreateWidget<UUserWidget>(PlayerControllerRef, SessionScreenClass);
		if (!IsValid(SessionScreenRef))
		{
			UE_LOG(LogTemp, Error, TEXT("USessionUIComponent::Init� Failed to create SessionScreen widget"));
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
			ADiceSpawnVolume* SpawnVolume = Cast<ADiceSpawnVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), ADiceSpawnVolume::StaticClass()));
			if (IsValid(SpawnVolume))
			{
				DiceTrayRef->SpawnVolume = SpawnVolume;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init — DiceSpawnVolume not found in level"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init — DiceTray not found"));
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
				UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init — Failed to load session save; chat log and tab names will not be restored"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init — ChatBox not found"));
		}

		if (IsValid(PlayerListRef))
		{
			//Placeholder
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init� PlayerList not found"));
		}

		if (IsValid(SessionNotesPanelRef))
		{
			//Placeholder
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init� SessionNotesPanel not found"));
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
			UE_LOG(LogTemp, Warning, TEXT("USessionUIComponent::Init� Taskbar not found"));
		}
	}
}

// Returns the chat box reference.
UChatBox* USessionUIComponent::GetChatBox() const
{
	return ChatBoxRef;
}

// Returns the dice tray reference.
UDiceTray* USessionUIComponent::GetDiceTray() const
{
	return DiceTrayRef;
}

// Returns the player list reference.
UPlayerList* USessionUIComponent::GetPlayerList() const
{
	return PlayerListRef;
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