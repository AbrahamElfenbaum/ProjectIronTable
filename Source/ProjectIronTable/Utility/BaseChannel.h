// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseChannel.generated.h"

class UScrollBox;

/** Base widget class for a named channel with a scrollable content area. Shared by chat and notes channels. */
UCLASS()
class PROJECTIRONTABLE_API UBaseChannel : public UUserWidget
{
	GENERATED_BODY()

protected:

#pragma region Widget References
	/** Scroll box that holds all channel content widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;
#pragma endregion

public:

#pragma region State
	/** Human-readable name shown on the tab for this channel. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FString DisplayName;

	/** Names of players in this private channel; empty means the channel is the public server channel. */
	TArray<FString> Participants;
#pragma endregion

#pragma region Config
	/** Pixels scrolled per Scroll() call. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScrollMultiplier = 60.f;
#pragma endregion

#pragma region Public Methods
	/** Scrolls the channel up (bUp = true) or down by ScrollMultiplier pixels. */
	void Scroll(bool bUp);
#pragma endregion
};
