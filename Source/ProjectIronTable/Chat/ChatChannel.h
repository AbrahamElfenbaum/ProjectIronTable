// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "ChatChannel.generated.h"

class UChatEntry;

UCLASS()
class PROJECTIRONTABLE_API UChatChannel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	float ScrollMultiplier = 60.f;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UChatEntry> ChatEntryClass;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString DisplayName;
	TArray<FString> Participants;

protected:
	virtual void NativeConstruct() override;

public:
	void AddChatMessage(const FString& Message);

	void SetChatEntryClass(TSubclassOf<UChatEntry> EntryClass);

	void Scroll(bool bUp);
};
