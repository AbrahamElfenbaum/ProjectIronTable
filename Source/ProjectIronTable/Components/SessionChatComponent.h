// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SessionChatComponent.generated.h"

class ASessionController;
class UChatBox;
class UDiceTray;
class UDraggablePanel;
class UPlayerList;

/** Actor component attached to ASessionController that manages chat routing, dice roll messages, and player addressing. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API USessionChatComponent : public UActorComponent
{
	GENERATED_BODY()

private:

#pragma region Runtime References
	/** Cached reference to the owning session controller. */
	UPROPERTY()
	TObjectPtr<ASessionController> SessionControllerRef;

	/** Reference to the ChatBox widget found inside the session screen. */
	UPROPERTY()
	TObjectPtr<UChatBox> ChatBoxRef;

	/** Reference to the DiceTray widget found inside the session screen. */
	UPROPERTY()
	TObjectPtr<UDiceTray> DiceTrayRef;

	/** Reference to the PlayerList widget found inside the session screen. */
	UPROPERTY()
	TObjectPtr<UPlayerList> PlayerListRef;
#pragma endregion

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
#pragma endregion

public:

	/** Disables tick and enables replication so server RPCs function correctly. */
	USessionChatComponent();

	/** Caches widget references from the UI component and binds delegates for dice roll results and player addressing. */
	void Init();

#pragma region Public Methods
	/** Server RPC: validates and routes a chat message to all relevant clients. */
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message, const TArray<FString>& Recipients);

	/** Client RPC: delivers a chat message to this client's chat box. */
	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message, const TArray<FString>& Recipients, bool bIsSender);

	/** Focuses the chat input box and switches to UI-only input mode. */
	void FocusChat();

	/** Exits chat focus, clears the input field, and restores game-and-UI input mode. */
	void ExitChat();

	/** Scrolls the active chat channel up or down. */
	void ScrollChat(bool bUp);
#pragma endregion
};
