// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseDiceActor.h"
#include "DiceSelectorManager.h"
#include "PlayerList.h"
#include "GameplayHUDComponent.generated.h"

class UChatBox;

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UGameplayHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameplayHUDComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> GameplayScreenClass;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> GameplayScreenRef;

	UPROPERTY()
	TObjectPtr<UDiceSelectorManager> DiceSelectorManagerRef;

	UPROPERTY()
	TObjectPtr<UChatBox> ChatBoxRef;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;

	UPROPERTY()
	TObjectPtr<UPlayerList> PlayerListRef;

public:
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message, const TArray<FString>& Recipients);

	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message, const TArray<FString>& Recipients, bool bIsSender);

	UFUNCTION()
	void AddRollResultToChat(TArray<FRollResult> Results, EDiceRollMode RollMode);

	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

	UFUNCTION()
	void OnPlayerAddressClicked(const FString& PlayerName);

	void FocusChat();
	void ExitChat();
	void ScrollChat(bool bUp);
};
