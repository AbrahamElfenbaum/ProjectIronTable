// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerRow.generated.h"

class UTextBlock;
class UButton;

/** Fired when the address button is clicked, passing the player's name. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddressClicked, const FString&, PlayerName);

/** A single row in the player list widget showing a player's name and an address button for private messaging. */
UCLASS()
class PROJECTIRONTABLE_API UPlayerRow : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Displays the player's name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameLabel;

	/** Button that initiates a private message to this player. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AddressButton;
#pragma endregion

public:

#pragma region Events
	/** Fired when the address button is clicked. */
	UPROPERTY(BlueprintAssignable)
	FOnAddressClicked OnAddressClicked;
#pragma endregion

private:

#pragma region State
	/** The player name stored for broadcast when the address button is clicked. */
	FString PlayerName;
#pragma endregion

protected:
	/** Binds the address button click delegate. */
	virtual void NativeConstruct() override;

public:

#pragma region Public Methods
	/** Sets the player name and updates the name label text. */
	void SetPlayerName(const FString& Name);
#pragma endregion

private:

	/** Broadcasts OnAddressClicked with the stored player name. */
	UFUNCTION()
	void OnAddressButtonClicked();
};
