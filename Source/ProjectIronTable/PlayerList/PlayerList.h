// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerRow.h"
#include "PlayerList.generated.h"

class UScrollBox;
class UButton;

/** Collapsible widget that lists all connected players and allows addressing them for private chat. */
UCLASS()
class PROJECTIRONTABLE_API UPlayerList : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Scroll box that holds all player row widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;

	/** Button that expands or collapses the player list. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ToggleButton;
#pragma endregion

public:

#pragma region Config
	/** The widget class instantiated for each player row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPlayerRow> PlayerRowClass;
#pragma endregion

#pragma region Events
	/** Forwarded from any player row's OnAddressClicked; passes the target player's name. */
	UPROPERTY(BlueprintAssignable)
	FOnAddressClicked OnAddressClicked;
#pragma endregion

private:

#pragma region State
	/** Tracks whether the scroll box is currently visible. */
	bool bIsExpanded = false;
#pragma endregion

protected:
	/** Binds the toggle button, populates the list, and collapses the scroll box initially. */
	virtual void NativeConstruct() override;

public:

#pragma region Public Methods
	/** Clears and repopulates the list from the current game state's player array. */
	void PopulateList();
#pragma endregion

private:

	/** Toggles list visibility and repopulates when expanding. */
	UFUNCTION()
	void OnToggleButtonClicked();

	/** Forwards the address click event upward to OnAddressClicked. */
	UFUNCTION()
	void OnPlayerAddressClicked(const FString& PlayerName);
};
