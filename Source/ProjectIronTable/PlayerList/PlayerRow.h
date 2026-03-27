// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "PlayerRow.generated.h"

/** Fired when the address button is clicked, passing the player's name. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddressClicked, const FString&, PlayerName);

/** A single row in the player list widget showing a player's name and an address button for private messaging. */
UCLASS()
class PROJECTIRONTABLE_API UPlayerRow : public UUserWidget
{
	GENERATED_BODY()

private:

	// -- Widget References --

	/** Displays the player's name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameLabel;

	/** Button that initiates a private message to this player. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AddressButton;

public:

	// -- Events --

	/** Fired when the address button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnAddressClicked OnAddressClicked;

private:

	// -- State --

	/** The player name stored for broadcast when the address button is clicked. */
	FString PlayerName;

protected:
	virtual void NativeConstruct() override;

public:

	// -- Public Methods --

	/** Sets the player name and updates the name label text. */
	void SetPlayerName(const FString& Name);

private:

	/** Broadcasts OnAddressClicked with the stored player name. */
	UFUNCTION()
	void OnAddressButtonClicked();
};
