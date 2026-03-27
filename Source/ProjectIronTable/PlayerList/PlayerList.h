// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "PlayerRow.h"
#include "PlayerList.generated.h"

UCLASS()
class PROJECTIRONTABLE_API UPlayerList : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToggleButton;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPlayerRow> PlayerRowClass;

	UPROPERTY(BlueprintAssignable)
	FOnAddressClicked OnAddressClicked;

private:
	bool bIsExpanded = false;

protected:
	virtual void NativeConstruct() override;

public:
	void PopulateList();

private:
	UFUNCTION()
	void OnToggleButtonClicked();

	UFUNCTION()
	void OnPlayerAddressClicked(const FString& PlayerName);
};
