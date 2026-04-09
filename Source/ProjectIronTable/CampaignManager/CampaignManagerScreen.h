// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CampaignManagerSave.h"
#include "DelegateLibrary.h"
#include "CampaignManagerScreen.generated.h"

class UButton;
class UScrollBox;
class UWrapBox;
class UGameTypeButton;
class UCampaignCard;

/** Root widget for the campaign manager screen. Loads saved campaigns, populates game type tabs, and displays campaign cards. */
UCLASS()
class PROJECTIRONTABLE_API UCampaignManagerScreen : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Binds the back button click delegate. */
	virtual void NativeConstruct() override;

public:

#pragma region Config
	/** Widget class used when creating game type tab buttons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameTypeButton> GameTypeButtonClass;

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
	/** Button that returns to the home screen. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

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

	/** Cached references to the active game type buttons. */
	UPROPERTY()
	TArray<UGameTypeButton*> ActiveButtons;
#pragma endregion

public:

#pragma region Events
	/** Fired when the back button is clicked; parent should return to the home screen. */
	UPROPERTY(BlueprintAssignable)
	FOnBackRequested OnBackRequested;
#pragma endregion

#pragma region Public Methods
	/** Loads the campaign save, populates game type tabs, and displays the first available campaign list. */
	void Init();
#pragma endregion

private:

#pragma region Event Handlers
	/** Clears the campaign grid and repopulates it with cards for the given game type. */
	void PopulateCampaigns(const TArray<FCampaignRecord>& Campaigns, const FString& GameType);

	/** Updates all active game type buttons — marks the one matching SelectedGameType as selected, all others as unselected. */
	void SetSelectedGameButton();

	/** Builds and returns a hardcoded TMap of campaign records for testing the campaign manager UI. */
	TMap<FString, FCampaignList> BuildFakeData() const;

	/** Switches the active game type and refreshes the campaign grid. */
	UFUNCTION()
	void OnGameTypeSelected(const FString& GameType);

	/** Called when a campaign card is clicked; launches the selected campaign. */
	UFUNCTION()
	void OnCampaignSelected(const FGuid& CampaignID, const FString& GameType);

	/** Broadcasts OnBackRequested to signal the parent to return to the home screen. */
	UFUNCTION()
	void OnBackClicked();
#pragma endregion

};
