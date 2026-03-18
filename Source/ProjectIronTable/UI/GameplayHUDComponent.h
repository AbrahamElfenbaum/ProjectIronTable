// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayHUDComponent.generated.h"

class UDiceSelectorManager;
class UChatBox;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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

public:
	UFUNCTION(Reliable, Server)
	void SendChatMessageOnServer(const FString& Message);

	UFUNCTION(Reliable, Client)
	void AddChatMessageOnOwningClient(const FString& Message);

#pragma region Testing
private:
	void Input_FocusChat();
	void Input_ScrollUp();
	void Input_ScrollDown();
#pragma endregion
};
