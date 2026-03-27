// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DiceSelectorManager.h"
#include "Kismet/GameplayStatics.h"
#include "DiceSpawnVolume.h"

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
	}
}

void UGameplayHUDComponent::FocusChat()
{
	if (ChatBoxRef) ChatBoxRef->FocusChat();
}

void UGameplayHUDComponent::ExitChat()
{
	if (ChatBoxRef) ChatBoxRef->ExitChat();
}

void UGameplayHUDComponent::ScrollChat(bool bUp)
{
	if (ChatBoxRef) ChatBoxRef->Scroll(bUp);
}

void UGameplayHUDComponent::AddChatMessageOnOwningClient_Implementation(const FString& Message, const TArray<FString>& Recipients, bool bIsSender)
{
	ChatBoxRef->AddChatMessage(Message, Recipients, bIsSender);
}

void UGameplayHUDComponent::SendChatMessageOnServer_Implementation(const FString& Message, const TArray<FString>& Recipients)
{
	//Get sender's player name for prefixing the message. If we can't get it for some reason, default to "Unknown"
	APlayerController* SenderPC = Cast<APlayerController>(GetOwner());
	FString SenderName = SenderPC && SenderPC->PlayerState ? SenderPC->PlayerState->GetPlayerName() : TEXT("Unknown");

	//Get the game state to access the player array
	AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();

	//Send the message to each player's HUD component if they're a recipient(or if Recipients is empty, meaning it's a global message)
	for (APlayerState* Play : GS->PlayerArray)
	{
		bool bIsParticipant = Recipients.IsEmpty() || //Is the recipients array empty?
							  Recipients.Contains(Play->GetPlayerName()) || //Is the player's name in the recipients array?
							  Play->GetPlayerName() == SenderName; //Is the player the sender?
		
		if (!bIsParticipant || !Play->GetPlayerController()) continue;

		UGameplayHUDComponent* HUDComp = Cast<UGameplayHUDComponent>(
			Play->GetPlayerController()->GetComponentByClass(UGameplayHUDComponent::StaticClass()));
		if (HUDComp)
		{
			HUDComp->AddChatMessageOnOwningClient(Message, Recipients, Play->GetPlayerName() == SenderName);
		}
	}
}

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
	SendChatMessageOnServer(Message, {});
}

void UGameplayHUDComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	FString DiceTypeName = UEnum::GetValueAsString(DiceType);
	DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);

	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName), {});
}
