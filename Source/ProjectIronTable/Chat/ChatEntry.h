// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ChatEntry.generated.h"

/** A single line of chat text displayed inside a UChatChannel's scroll box. */
UCLASS()
class PROJECTIRONTABLE_API UChatEntry : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

private:

	// -- Widget References --

	/** Text block that displays the chat message. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock;

public:

	// -- Config --

	/** The message string to display; should be set before adding this widget to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FString Message;

};
