// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HomeScreenHUDComponent.generated.h"

class UButton;

/** Actor component attached to AHomeScreenController that owns and manages all home screen UI. */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTIRONTABLE_API UHomeScreenHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHomeScreenHUDComponent();

protected:
	virtual void BeginPlay() override;

public:

	// -- Config --

	/** The root gameplay screen widget class to instantiate and add to the viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> HomeScreenClass;

private:

	// -- Runtime References --

	/** The instantiated root home screen screen widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> HomeScreenRef;

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

	/** Button that opens the settings panel (not yet implemented). */
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
