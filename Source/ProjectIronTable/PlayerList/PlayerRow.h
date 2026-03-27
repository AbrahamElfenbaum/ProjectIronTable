// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "PlayerRow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddressClicked, const FString&, PlayerName);

UCLASS()
class PROJECTIRONTABLE_API UPlayerRow : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AddressButton;

public:
	UPROPERTY(BlueprintAssignable)
	FOnAddressClicked OnAddressClicked;

private:
	FString PlayerName;

protected:
	virtual void NativeConstruct() override;

public:
	void SetPlayerName(const FString& Name);

private:
	UFUNCTION()
	void OnAddressButtonClicked();
};
