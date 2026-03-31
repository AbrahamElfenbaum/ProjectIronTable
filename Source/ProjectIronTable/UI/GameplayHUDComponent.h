// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseDiceActor.h"
#include "DiceSelectorManager.h"
#include "PlayerList.h"
#include "GameplayHUDComponent.generated.h"

class UChatBox;
class UTaskbar;
class UDraggablePanel;

/**
 * Actor component attached to AGameplayController that owns and manages all gameplay UI.
 * Handles widget creation, dice result routing, and replicated chat messaging.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UGameplayHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameplayHUDComponent();

protected:
	virtual void BeginPlay() override;

public:

	// -- Config --

	/** The root gameplay screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GameplayScreenClass;

private:

	// -- Runtime References --

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

public:

	// -- Replicated Chat Methods --

	/** Server RPC: validates and routes a chat message to all relevant clients. */
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message, const TArray<FString>& Recipients);

	/** Client RPC: delivers a chat message to this client's chat box. */
	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message, const TArray<FString>& Recipients, bool bIsSender);

	// -- Event Handlers --

	/** Converts dice roll results into a formatted chat message and sends it to the server. */
	UFUNCTION()
	void AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode);

	/** Sends a chat message noting that a die of the given type was lost to the failsafe. */
	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

	/** Appends the clicked player's name as an @mention in the chat input field. */
	UFUNCTION()
	void OnPlayerAddressClicked(const FString& PlayerName);

	// -- Chat Passthrough Methods --

	/** Focuses the chat input box and switches to UI-only input mode. */
	void FocusChat();

	/** Exits chat focus, clears the input field, and restores game-and-UI input mode. */
	void ExitChat();

	/** Scrolls the active chat channel up or down. */
	void ScrollChat(bool bUp);

private:
	/** Finds a DraggablePanel by widget name, registers it with the Taskbar, and returns it. Logs a warning if not found. */
	UDraggablePanel* FindAndRegisterPanel(const FName& WidgetName, const FString& Label);
};
