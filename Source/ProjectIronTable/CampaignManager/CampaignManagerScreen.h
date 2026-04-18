// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "CampaignManagerSave.h"
#include "CampaignManagerScreen.generated.h"

class UScrollBox;
class UWrapBox;
class UGameTypeButton;
class UGameTypeTab;
class UCampaignCard;

/** Root widget for the campaign manager screen. Loads saved campaigns, populates game type tabs, and displays campaign cards. */
UCLASS()
class PROJECTIRONTABLE_API UCampaignManagerScreen : public UBaseScreen
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Widget class used when creating game type tabs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameTypeTab> GameTypeTabClass;

	/** Widget class used when creating campaign card entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCampaignCard> CampaignCardClass;

	/** Background color applied to the selected game type tab button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SelectedTabColor;

	/** Background color applied to unselected game type tab buttons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnselectedTabColor;

	/** When true, Init populates the screen with hardcoded test data instead of the saved campaign data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFakeData;
#pragma endregion

private:

#pragma region Widget References
	/** Button that begins the new campaign creation flow. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NewCampaignButton;

	/** Horizontal scroll box holding all game type tab buttons. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> GameTypeTabBar;

	/** Wrap box that holds the campaign cards for the active game type. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> CampaignGrid;

	/** Scroll box wrapping the campaign grid, allowing it to scroll vertically when cards overflow. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> CampaignScroll;
#pragma endregion

#pragma region State
	/** The game type key currently displayed in the campaign grid. */
	FString SelectedGameType;

	/** Cached reference to the loaded campaign save game object. */
	UPROPERTY()
	TObjectPtr<UCampaignManagerSave> CampaignData;

	/** Cached references to the active game type tabs. */
	UPROPERTY()
	TArray<TObjectPtr<UGameTypeTab>> ActiveTabs;
#pragma endregion

public:

#pragma region Public Methods
	/** Loads the campaign save, populates game type tabs, and displays the first available campaign list. */
	virtual void Init() override;
#pragma endregion

private:

#pragma region Private Methods
	/** Clears the campaign grid and creates a card for each record in the given list. */
	void PopulateCampaigns(const TArray<FCampaignRecord>& Campaigns, const FString& GameType);

	/** Updates all active game type buttons — marks the one matching SelectedGameType as selected, all others as unselected. */
	void SetSelectedGameButton();

	/** Builds and returns a hardcoded TMap of campaign records for testing the campaign manager UI. */
	TMap<FString, FCampaignList> BuildFakeData() const;
#pragma endregion

#pragma region Event Handlers
	/** Switches the active game type and refreshes the campaign grid. */
	UFUNCTION()
	void OnGameTypeSelected(const FString& GameType);

	/** Called when a campaign card is clicked; launches the selected campaign. */
	UFUNCTION()
	void OnCampaignSelected(const FGuid& CampaignID, const FString& GameType);
#pragma endregion
};
