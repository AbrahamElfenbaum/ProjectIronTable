// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MainScreenHUDComponent.generated.h"

class UButton;

/** Actor component attached to AMainScreenController that owns and manages all main screen UI. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UMainScreenHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMainScreenHUDComponent();

protected:
	virtual void BeginPlay() override;

public:

	// -- Config --

	/** The root main screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> MainScreenClass;

private:

	// -- Runtime References --

	/** The instantiated root main screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> MainScreenRef;

	/** Cached reference to the owning player controller. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;

	/** Button that transitions to the gameplay scene. */
	UPROPERTY()
	TObjectPtr<UButton> PlayButtonRef;

	/** Button that opens the join session flow (not yet implemented). */
	UPROPERTY()
	TObjectPtr<UButton> JoinButtonRef;

	/** Button that opens the asset library screen (not yet implemented). */
	UPROPERTY()
	TObjectPtr<UButton> LibraryButtonRef;

	/** Button that opens the settings panel. */
	UPROPERTY()
	TObjectPtr<UButton> SettingsButtonRef;

	/** Button that quits the application. */
	UPROPERTY()
	TObjectPtr<UButton> QuitButtonRef;

private:

	// -- Event Handlers --

	/** Opens the gameplay level. */
	UFUNCTION()
	void OnPlayClicked();

	/** Placeholder — join session flow not yet implemented. */
	UFUNCTION()
	void OnJoinClicked();

	/** Placeholder — asset library screen not yet implemented. */
	UFUNCTION()
	void OnLibraryClicked();

	/** Placeholder — settings panel not yet implemented. */
	UFUNCTION()
	void OnSettingsClicked();

	/** Quits the application. */
	UFUNCTION()
	void OnQuitClicked();
};
