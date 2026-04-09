// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampaignCard.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCampaignSelected, const FGuid&, CampaignID, const FString&, GameType);

/** A card widget representing a single campaign entry. Displays the campaign name, last played date, and player count. Fires OnCampaignSelected when the launch button is clicked. */
UCLASS()
class PROJECTIRONTABLE_API UCampaignCard : public UUserWidget
{
	GENERATED_BODY()

private:

#pragma region Widget References
	/** Button the player clicks to open this campaign. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LaunchCampaignButton;

	/** Displays the campaign's display name. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CampaignTitle;

	/** Displays the date and time of the most recent session. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LastPlayedDate;

	/** Displays the number of players registered to this campaign. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NumberOfPlayers;
#pragma endregion

#pragma region State
	/** Unique identifier for this campaign, broadcast when the card is selected. */
	FGuid CampaignID;

	/** Game type key this campaign belongs to, broadcast when the card is selected. */
	FString GameType;
#pragma endregion

protected:

	/** Binds the launch button click delegate. */
	virtual void NativeConstruct() override;

public:

	/** Fired when the launch button is clicked, passing the campaign ID and game type. */
	UPROPERTY(BlueprintAssignable)
	FOnCampaignSelected OnCampaignSelected;

	/** Sets the displayed campaign name. */
	void SetCampaignTitle(const FString& Title);

	/** Sets the displayed last played date string. */
	void SetLastPlayedDate(const FString& Date);

	/** Sets the displayed player count. */
	void SetNumberOfPlayers(int32 NumPlayers);

	/** Stores the campaign ID and game type to be broadcast when the card is selected. */
	void SetCampaignData(const FGuid& InCampaignID, const FString& InGameType);

private:

	/** Broadcasts OnCampaignSelected with the stored campaign ID and game type. */
	UFUNCTION()
	void OnLaunchCampaignButtonClicked();
};
