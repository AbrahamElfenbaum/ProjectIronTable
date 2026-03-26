// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "ChatTab.generated.h"

class UChatChannel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabClicked, UChatChannel*, ClickedChannel);

UCLASS()
class PROJECTIRONTABLE_API UChatTab : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TabLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> NotificationIndicator;

private:
	UPROPERTY()
	TObjectPtr<UChatChannel> Channel;

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnTabClicked OnTabClicked;

	void SetChannel(UChatChannel* InChannel);
	void SetLabel(const FString& Label);
	void ShowNotification();
	void ClearNotification();

private:
	UFUNCTION()
	void OnTabButtonClicked();

};
