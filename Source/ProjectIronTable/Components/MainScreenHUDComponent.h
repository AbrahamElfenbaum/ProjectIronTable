// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainScreenHUDComponent.generated.h"

class UWidgetSwitcher;
class UHomeScreen;
class UCampaignManagerScreen;
class UCampaignBrowserScreen;
class UAssetLibraryScreen;
class USettingsScreen;


/** Actor component attached to AMainScreenController that owns and manages all main screen UI. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UMainScreenHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Disables tick. */
	UMainScreenHUDComponent();

protected:
	/** Creates and adds the main screen widget, then caches widget references and binds click handlers. */
	virtual void BeginPlay() override;

public:

#pragma region Config
	/** The root main screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MainScreenClass;
#pragma endregion

private:

#pragma region Runtime References
	/** The instantiated root main screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> MainScreenRef;

	/** The instantiated screen switcher widget. */
	UPROPERTY()
	TObjectPtr<UWidgetSwitcher> ScreenSwitcherRef;

	/** The instantiated home screen widget. */
	UPROPERTY()
	TObjectPtr<UHomeScreen> HomeScreenRef;

	/** The instantiated campaign manager screen widget. */
	UPROPERTY()
	TObjectPtr<UCampaignManagerScreen> CampaignManagerScreenRef;

	/** The instantiated campaign browser screen widget. */
	UPROPERTY()
	TObjectPtr<UCampaignBrowserScreen> CampaignBrowserScreenRef;

	/** The instantiated asset library screen widget. */
	UPROPERTY()
	TObjectPtr<UAssetLibraryScreen> AssetLibraryScreenRef;

	/** The instantiated settings screen widget. */
	UPROPERTY()
	TObjectPtr<USettingsScreen> SettingsScreenRef;

	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;
#pragma endregion

#pragma region Event Handlers
	/** Switches the screen switcher back to the home screen. */
	UFUNCTION()
	void OnBackClicked();

	/** Switches the screen switcher to the campaign manager screen. */
	UFUNCTION()
	void OnCampaignManagerClicked();

	/** Switches the screen switcher to the campaign browser screen. */
	UFUNCTION()
	void OnCampaignBrowserClicked();

	/** Switches the screen switcher to the asset library screen. */
	UFUNCTION()
	void OnAssetLibraryClicked();

	/** Switches the screen switcher to the settings screen. */
	UFUNCTION()
	void OnSettingsClicked();
#pragma endregion
};
