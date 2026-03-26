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
	SendChatMessageOnServer(Message);
}

void UGameplayHUDComponent::OnDiceFailsafeHandler(EDiceType DiceType)
{
	FString PlayerName = PlayerControllerRef && PlayerControllerRef->PlayerState ? PlayerControllerRef->PlayerState->GetPlayerName() : TEXT("Unknown");

	FString DiceTypeName = UEnum::GetValueAsString(DiceType);
	DiceTypeName = DiceTypeName.RightChop(DiceTypeName.Find(TEXT("::")) + 2);

	SendChatMessageOnServer(FString::Printf(TEXT("%s lost a %s to the void"), *PlayerName, *DiceTypeName));
}
