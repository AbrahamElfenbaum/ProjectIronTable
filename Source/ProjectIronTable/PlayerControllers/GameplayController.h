// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameplayController.generated.h"

class UGameplayHUDComponent;
class AGameplayPawn;
class UEnhancedInputLocalPlayerSubsystem;

/** Main player controller. Drives camera movement and routes input to the HUD. */
UCLASS()
class PROJECTIRONTABLE_API AGameplayController : public APlayerController
{
	GENERATED_BODY()

public:
	AGameplayController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	// -- Runtime References --

	/** The HUD component owned by this controller, responsible for all UI widgets. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGameplayHUDComponent> HUDComponent;

	/** Cached reference to the possessed gameplay pawn. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGameplayPawn> GameplayPawnRef;

	/** Reference to the Enhanced Input subsystem for managing mapping contexts. */
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystemRef;

public:

	// -- Camera State --

	/** When false, panning input is active and WASD movement is suppressed. */
	bool bCanCameraMove = true;

	// -- Camera Config --

	/** Minimum camera translation speed at full zoom-in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinCameraMovementSpeed = 5.f;

	/** Maximum camera translation speed at full zoom-out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraMovementSpeed = 20.f;

	/** Speed multiplier applied while the sprint input is held. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraSpeedMultiplier = 2.f;

	/** Minimum pitch angle (degrees) for camera rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinCameraPitch = -15.f;

	/** Maximum pitch angle (degrees) for camera rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraPitch = 45.f;

	/** Mouse delta multiplier applied during camera pan rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraPanSpeedMultiplier = 5.f;

	/** Minimum spring arm length (closest zoom). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoomLength = 100.f;

	/** Maximum spring arm length (furthest zoom). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoomLength = 2500.f;

	/** Amount the spring arm changes per scroll tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 50.f;

private:

	/** Active speed multiplier; 1.0 normally, CameraSpeedMultiplier while sprinting. */
	float CurrentCameraSpeedMultiplier = 1.f;

public:

#pragma region Gameplay Input

	/** Input mapping context active during normal gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputMappingContext> IMC_Gameplay;

	/** WASD camera translation input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraMove;

	/** Mouse-drag camera pan input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraPan;

	/** Resets camera pitch to default while held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraPanReset;

	/** Hold to apply the camera sprint speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraSprint;

	/** Scroll-wheel camera zoom input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraZoom;

	/** Triggers chat focus mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_FocusChat;

#pragma endregion

#pragma region Chat Input

	/** Input mapping context active while the chat box is focused. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputMappingContext> IMC_Chat;

	/** Exits chat focus mode and returns to gameplay input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ExitChat;

	/** Scrolls the active chat channel up or down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ScrollChat;

#pragma endregion

protected:

	/** Returns a movement speed scaled by the current spring arm length, clamped to the configured min/max. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float CalculateCameraMovementSpeed() const;

private:

	/** Translates the pawn on the XY plane based on WASD input. */
	void Input_CameraMove(const FInputActionValue& Value);

	/** Rotates the camera yaw and pitch while the pan button is held. */
	void Input_CameraPan(const FInputActionValue& Value);

	/** Resets camera pitch to a default angle. */
	void Input_CameraPanReset();

	/** Scales movement speed up or down based on whether sprint is held. */
	void Input_CameraSprint(const FInputActionValue& Value);

	/** Adjusts the spring arm length to zoom in or out. */
	void Input_CameraZoom(const FInputActionValue& Value);

	/** Adds the chat input mapping context and focuses the chat box. */
	void Input_FocusChat();

	/** Removes the chat input mapping context and unfocuses the chat box. */
	void Input_ExitChat();

	/** Forwards scroll direction to the HUD chat scroll handler. */
	void Input_ScrollChat(const FInputActionValue& Value);
};
