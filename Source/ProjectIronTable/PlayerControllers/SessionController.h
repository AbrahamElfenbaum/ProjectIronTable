// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseCameraController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "SessionController.generated.h"

class UDiceRollComponent;
class USessionChatComponent;
class USessionUIComponent;

/** Main player controller for active sessions. Drives camera movement and routes input to the HUD. */
UCLASS()
class PROJECTIRONTABLE_API ASessionController : public ABaseCameraController
{
	GENERATED_BODY()

public:

#pragma region Components
	/** The chat component owned by this controller, responsible for replicated chat messaging. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USessionChatComponent> ChatComponent;

	/** The dice roll component owned by this controller, responsible for rolling dice. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDiceRollComponent> DiceRollComponent;

	/** The HUD component owned by this controller, responsible for all UI widgets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USessionUIComponent> UIComponent;
#pragma endregion

#pragma region Session Input
	/** Input mapping context active during normal gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session Input")
	TObjectPtr<UInputMappingContext> IMC_Session;

	/** Triggers chat focus mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Session Input")
	TObjectPtr<UInputAction> IA_FocusChat;
#pragma endregion

#pragma region Chat Input
	/** Input mapping context active while the chat box is focused. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputMappingContext> IMC_Chat;

	/** Exits chat focus mode and returns to gameplay input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ExitChat;

#pragma endregion

private:

#pragma region Input Handlers
	/** Adds the chat input mapping context and focuses the chat box. */
	void Input_FocusChat();

	/** Removes the chat input mapping context and unfocuses the chat box. */
	void Input_ExitChat();

#pragma endregion

protected:

	/** Adds the session input context and binds the chat focus/exit actions, after the base camera setup. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Initializes the UI and chat components, after the base camera setup. */
	virtual void BeginPlay() override;

public:

#pragma region Public Methods
	/** Sends the server a request to call ServerTravel with the given URL, moving all players to the target level. */
	UFUNCTION(Server, Reliable)
	void Server_TravelToSession(const FString& TravelURL);
#pragma endregion

	/** Creates and attaches the UI, chat, and dice roll component subobjects. */
	ASessionController();
};
