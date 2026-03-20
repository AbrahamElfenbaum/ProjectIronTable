// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DiceSelectorManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UGameplayHUDComponent::UGameplayHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

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

		DiceSelectorManagerRef = Cast<UDiceSelectorManager>(GameplayScreenRef->GetWidgetFromName(TEXT("DiceSelectorManager")));
		ChatBoxRef = Cast<UChatBox>(GameplayScreenRef->GetWidgetFromName(TEXT("ChatBox")));

		if (DiceSelectorManagerRef)
		{
			UE_LOG(LogTemp, Display, TEXT("Dice Selector Found"));
			DiceSelectorManagerRef->OnAllDiceRolled.AddDynamic(this, &UGameplayHUDComponent::AddRollResultToChat);
			DiceSelectorManagerRef->OnDiceFailsafeDestroyed.AddDynamic(this, &UGameplayHUDComponent::OnDiceFailsafeHandler);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dice Selector Not Found"));
		}

		ULocalPlayer* LP = PlayerControllerRef->GetLocalPlayer();
		if (LP)
		{
			InputSubsystemRef = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (InputSubsystemRef)
			{
				InputSubsystemRef->AddMappingContext(IMC_Gameplay, 0);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Subsystem not found on Local Player"));
			}
		}

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerControllerRef->InputComponent))
		{
			EIC->BindAction(IA_FocusChat, ETriggerEvent::Triggered, this, &UGameplayHUDComponent::Input_FocusChat);

			EIC->BindAction(IA_ExitChat, ETriggerEvent::Triggered, this, &UGameplayHUDComponent::Input_ExitChat);

			EIC->BindAction(IA_ScrollChatUp, ETriggerEvent::Triggered, this, &UGameplayHUDComponent::Input_ScrollUp);

			EIC->BindAction(IA_ScrollChatDown, ETriggerEvent::Triggered, this, &UGameplayHUDComponent::Input_ScrollDown);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Component not found on Player Controller"));
		}
	}
}

#pragma region Testing
void UGameplayHUDComponent::Input_FocusChat()
{
	if (InputSubsystemRef)
	{
		InputSubsystemRef->AddMappingContext(IMC_Chat, 1);
	}
	if (ChatBoxRef) ChatBoxRef->FocusChat();
}

void UGameplayHUDComponent::Input_ExitChat()
{
	if (InputSubsystemRef)
	{
		InputSubsystemRef->RemoveMappingContext(IMC_Chat);
	}
	if (ChatBoxRef) ChatBoxRef->ExitChat();
}

void UGameplayHUDComponent::Input_ScrollUp()
{
	if (ChatBoxRef) ChatBoxRef->Scroll(true);
}

void UGameplayHUDComponent::Input_ScrollDown()
{
	if (ChatBoxRef) ChatBoxRef->Scroll(false);
}
#pragma endregion

void UGameplayHUDComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message)
{
	ChatBoxRef->AddChatMessage(Message);
}

void UGameplayHUDComponent::SendChatMessageOnServer_Implementation(const FString& Message)
{
	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
	for (auto Play : GS->PlayerArray)
	{
		if (Play->GetPlayerController())
		{
			UGameplayHUDComponent* HUDComp = Cast<UGameplayHUDComponent>(
				Play->GetPlayerController()->GetComponentByClass(UGameplayHUDComponent::StaticClass()));
			if (HUDComp)
			{
				HUDComp->AddChatMessageOnOwningClient(Message);
			}
		}
	}
}

void UGameplayHUDComponent::AddRollResultToChat(TArray<FRollResult> Results)
{
	//Message starts with the player name
	FString Message = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	Message += TEXT(" Rolled:\n");

	//Add in each rolled result on a new line
	for (const FRollResult& Result : Results)
	{
		FString DiceTypeName = UEnum::GetValueAsString(Result.DiceType);
		DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);
		Message += FString::Printf(TEXT("%d on a %s\n"), Result.Value, *DiceTypeName);
	}

	//Trim the last newline off the end of the message and send it to the server to be broadcast to all clients
	Message.TrimEndInline();
	SendChatMessageOnServer(Message);
}

void UGameplayHUDComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	FString DiceTypeName = UEnum::GetValueAsString(DiceType);
	DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);

	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName));
}
