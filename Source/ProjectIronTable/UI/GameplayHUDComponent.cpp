// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayHUDComponent.h"
#include "InputCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DiceSelectorManager.h"

UGameplayHUDComponent::UGameplayHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(true);
}

void UGameplayHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());

	if (PC && 
		PC->IsLocalPlayerController() &&
		GameplayScreenClass)
	{
		GameplayScreenRef = CreateWidget<UUserWidget>(GetWorld(), GameplayScreenClass);
		GameplayScreenRef->AddToViewport();

		DiceSelectorManagerRef = Cast<UDiceSelectorManager>(GameplayScreenRef->GetWidgetFromName(TEXT("DiceSelectorManager")));
		ChatBoxRef = Cast<UChatBox>(GameplayScreenRef->GetWidgetFromName(TEXT("ChatBox")));

		#pragma region Testing
		if (PC->InputComponent)
		{
			PC->InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &UGameplayHUDComponent::Input_FocusChat);
			PC->InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &UGameplayHUDComponent::Input_ScrollUp);
			PC->InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &UGameplayHUDComponent::Input_ScrollDown);
		}
		#pragma endregion
	}
}

#pragma region Testing
void UGameplayHUDComponent::Input_FocusChat()
{
	if (ChatBoxRef) ChatBoxRef->FocusChat();
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
