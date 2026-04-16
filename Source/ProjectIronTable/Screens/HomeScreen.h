// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HomeScreen.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampaignManagerRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCampaignBrowserRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAssetLibraryRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsRequested);

/** Home screen widget. Binds all home screen buttons and broadcasts delegates for navigation events that require parent coordination. */
UCLASS()
class PROJECTIRONTABLE_API UHomeScreen : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Binds all button click delegates. */
	void Init();

private:

#pragma region Widget References
	/** Button that opens the campaign manager screen. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CampaignManagerButton;

	/** Button that opens the join session flow (not yet implemented). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CampaignBrowserButton;

	/** Button that opens the asset library screen (not yet implemented). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AssetLibraryButton;

	/** Button that opens the settings panel. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	/** Button that quits the application. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;
#pragma endregion

public:

#pragma region Events
	/** Broadcasts when the user requests to open the campaign manager screen. */
	UPROPERTY(BlueprintAssignable)
	FOnCampaignManagerRequested OnCampaignManagerRequested;

	/** Broadcasts when the user requests to open the campaign browser screen. */
	UPROPERTY(BlueprintAssignable)
	FOnCampaignBrowserRequested OnCampaignBrowserRequested;

	/** Broadcasts when the user requests to open the asset library screen. */
	UPROPERTY(BlueprintAssignable)
	FOnAssetLibraryRequested OnAssetLibraryRequested;

	/** Broadcasts when the user requests to open the settings screen. */
	UPROPERTY(BlueprintAssignable)
	FOnSettingsRequested OnSettingsRequested;
#pragma endregion

private:

#pragma region Event Handlers
	/** Broadcasts OnCampaignManagerRequested. */
	UFUNCTION()
	void OnCampaignManagerClicked();

	/** Broadcasts OnCampaignBrowserRequested. */
	UFUNCTION()
	void OnCampaignBrowserClicked();

	/** Broadcasts OnAssetLibraryRequested. */
	UFUNCTION()
	void OnAssetLibraryClicked();

	/** Opens the settings screen. */
	UFUNCTION()
	void OnSettingsClicked();

	/** Quits the application. */
	UFUNCTION()
	void OnQuitClicked();
#pragma endregion
};
