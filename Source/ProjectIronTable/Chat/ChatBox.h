// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
//#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableText.h"
#include "ChatBox.generated.h"

class UGameplayHUDComponent;
class UChatEntry;
class UChatChannel;
class UChatTab;

UCLASS()
class PROJECTIRONTABLE_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

public:
	//float ScrollMultiplier = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatChannel> ChannelClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatTab> TabClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY()
	TObjectPtr<UGameplayHUDComponent> HUDComponentRef;

	//UPROPERTY(meta = (BindWidget))
	//TObjectPtr<UScrollBox> ScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TabBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ChannelContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> EditableText;

	UPROPERTY()
	TArray<UChatChannel*> Channels;

	UPROPERTY()
	TMap<UChatChannel*, UChatTab*> ChannelTabMap;

	UPROPERTY()
	TObjectPtr<UChatChannel> ActiveChannel;

	bool bChatFocused;

public:
	void FocusChat();

	void ExitChat();

	UChatChannel* CreateChannel(TArray<FString> Participants);

	void SwitchToChannel(UChatChannel* Channel);

	void AddChatMessage(const FString& Message, TArray<FString> Participants, bool bIsSender);

	//void Scroll(bool bUp);

	//void AddChatMessage(const FString& Message);

private:
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
};
