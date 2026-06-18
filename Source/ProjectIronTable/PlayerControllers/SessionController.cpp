// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SessionController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "DiceRollComponent.h"
#include "SessionChatComponent.h"
#include "SessionUIComponent.h"

// Adds the chat input mapping context and tells the HUD to focus the chat box.
void ASessionController::Input_FocusChat()
{
	if (IsValid(InputSubsystemRef)) InputSubsystemRef->AddMappingContext(IMC_Chat, 1);
	if (IsValid(ChatComponent)) ChatComponent->FocusChat();
}

// Removes the chat input mapping context and tells the HUD to exit chat.
void ASessionController::Input_ExitChat()
{
	if (IsValid(InputSubsystemRef)) InputSubsystemRef->RemoveMappingContext(IMC_Chat);
	if (IsValid(ChatComponent)) ChatComponent->ExitChat();
}

// Adds the session input context and binds the chat focus/exit actions, after the base camera setup.
void ASessionController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		if (IsValid(InputSubsystemRef))
		{
			InputSubsystemRef->AddMappingContext(IMC_Session, 0);
		}
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->BindAction(IA_FocusChat, ETriggerEvent::Triggered, this, &ASessionController::Input_FocusChat);
			EIC->BindAction(IA_ExitChat, ETriggerEvent::Triggered, this, &ASessionController::Input_ExitChat);
		}
	}
}

// Initializes the UI and chat components after the base camera setup.
void ASessionController::BeginPlay()
{
	Super::BeginPlay();

	UIComponent->Init();
	ChatComponent->Init();
}

// Validates the world reference and calls ServerTravel with the provided URL.
void ASessionController::Server_TravelToSession_Implementation(const FString& TravelURL)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASessionController::Server_TravelToSession — GetWorld() returned null."));
		return;
	}

	World->ServerTravel(TravelURL);
}

// Creates and attaches the UI, chat, and dice roll component subobjects.
ASessionController::ASessionController()
{
	ChatComponent = CreateDefaultSubobject<USessionChatComponent>(TEXT("ChatComponent"));
	DiceRollComponent = CreateDefaultSubobject<UDiceRollComponent>(TEXT("DiceRollComponent"));
	UIComponent = CreateDefaultSubobject<USessionUIComponent>(TEXT("UIComponent"));
}
