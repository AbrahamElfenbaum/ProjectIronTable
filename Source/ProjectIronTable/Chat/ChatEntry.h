// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatEntry.generated.h"

class UTextBlock;

/** A single line of chat text displayed inside a UChatChannel's scroll box. */
UCLASS()
class PROJECTIRONTABLE_API UChatEntry : public UUserWidget
{
	GENERATED_BODY()
protected:
	/** Populates the text block with the message string. */
	virtual void NativeConstruct() override;

private:

#pragma region Widget References
	/** Text block that displays the chat message. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock;
#pragma endregion

public:

#pragma region Config
	/** The message string to display; should be set before adding this widget to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FString Message;
#pragma endregion

};
