// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseDiceActor.h"
#include "GameplayHUDComponent.generated.h"

class UDiceSelectorManager;
class UChatBox;
class UInputAction;
class UInputMappingContext;

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
	TObjectPtr<class UEnhancedInputLocalPlayerSubsystem> InputSubsystemRef;

protected:
#pragma region Gameplay Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputMappingContext> IMC_Gameplay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_FocusChat;
#pragma endregion

#pragma region Chat Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputMappingContext> IMC_Chat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ExitChat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ScrollChatUp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ScrollChatDown;
#pragma endregion

public:
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message);

	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message);

	UFUNCTION()
	void AddRollResultToChat(TArray<FRollResult> Results);

	UFUNCTION()
	void OnDiceFailsafeHandler(EDiceType DiceType);

#pragma region Testing
private:
	void Input_FocusChat();
	void Input_ExitChat();
	void Input_ScrollUp();
	void Input_ScrollDown();
#pragma endregion
};
