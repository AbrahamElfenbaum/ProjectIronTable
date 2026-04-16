// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseDiceActor.h"
#include "SessionHUDComponent.generated.h"

class UChatBox;
class UDiceSelectorManager;
class UTaskbar;
class UDraggablePanel;
class UPanelLayoutSave;
class UPlayerList;

/**
 * Actor component attached to ASessionController that owns and manages all gameplay UI.
 * Handles widget creation, dice result routing, and replicated chat messaging.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API USessionHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Disables tick and enables replication so server RPCs function correctly. */
	USessionHUDComponent();

protected:
	/** Creates and adds the gameplay screen widget, then caches widget references and wires up delegates. */
	virtual void BeginPlay() override;

public:

#pragma region Config
	/** The root gameplay screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GameplayScreenClass;
#pragma endregion

private:

#pragma region Runtime References
	/** The instantiated root gameplay screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> GameplayScreenRef;

	/** Reference to the DiceSelectorManager widget found inside the gameplay screen. */
	UPROPERTY()
	TObjectPtr<UDiceSelectorManager> DiceSelectorManagerRef;

	/** Reference to the ChatBox widget found inside the gameplay screen. */
	UPROPERTY()
	TObjectPtr<UChatBox> ChatBoxRef;

	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;

	/** Reference to the PlayerList widget found inside the gameplay screen. */
	UPROPERTY()
	TObjectPtr<UPlayerList> PlayerListRef;

	/** Reference to the Taskbar widget found inside the gameplay screen. */
	UPROPERTY()
	TObjectPtr<UTaskbar> TaskbarRef;

	/** Reference to the DraggablePanel wrapping the DiceSelectorManager, registered with the Taskbar for toggling. */
	UPROPERTY()
	TObjectPtr<UDraggablePanel> DicePanel;

	/** Reference to the DraggablePanel wrapping the ChatBox, registered with the Taskbar for toggling. */
	UPROPERTY()
	TObjectPtr<UDraggablePanel> ChatPanel;

	/** Reference to the DraggablePanel wrapping the PlayerList, registered with the Taskbar for toggling. */
	UPROPERTY()
	TObjectPtr<UDraggablePanel> PlayersPanel;

	/** Array of all draggable panels in the HUD, used for saving and loading layout. */
	UPROPERTY()
	TArray<TObjectPtr<UDraggablePanel>> Panels;
#pragma endregion

public:

#pragma region Replicated Chat Methods
	/** Server RPC: validates and routes a chat message to all relevant clients. */
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message, const TArray<FString>& Recipients);

	/** Client RPC: delivers a chat message to this client's chat box. */
	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message, const TArray<FString>& Recipients, bool bIsSender);
#pragma endregion

#pragma region Chat Passthrough Methods
	/** Focuses the chat input box and switches to UI-only input mode. */
	void FocusChat();

	/** Exits chat focus, clears the input field, and restores game-and-UI input mode. */
	void ExitChat();

	/** Scrolls the active chat channel up or down. */
	void ScrollChat(bool bUp);
#pragma endregion

private:

#pragma region Event Handlers
	/** Converts dice roll results into a formatted chat message and sends it to the server. */
	UFUNCTION()
	void AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode);

	/** Sends a chat message noting that a die of the given type was lost to the failsafe. */
	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

	/** Appends the clicked player's name as an @mention in the chat input field. */
	UFUNCTION()
	void OnPlayerAddressClicked(const FString& PlayerName);

	/** Called when a roll is initiated; forwards to the chat box to send a private roll message if recipients are present in the input. */
	UFUNCTION()
	void OnRollInitiated();

	/** Saves the current layout of all draggable panels to a save game object. Called when a panel is dragged, resized, or toggled. */
	UFUNCTION()
	void SavePanelLayout();
#pragma endregion

	/** Finds a DraggablePanel by widget name, registers it with the Taskbar, and returns it. Logs a warning if not found. */
	UDraggablePanel* FindAndRegisterPanel(const FName& WidgetName, const FString& Label);

	/** Loads the saved layout of all draggable panels from a save game object and applies it. Called on HUD initialization. */
	void LoadPanelLayout();

	/** Writes the given panel's current layout data into the save object using its PanelID as the key. */
	void SavePanelLayout(const UDraggablePanel* Panel, UPanelLayoutSave* LayoutSave);

	/** Reads the given panel's layout data from the save object and applies it if a matching key exists. */
	void ApplyPanelLayout(UDraggablePanel* Panel, UPanelLayoutSave* LoadedLayout);
};
