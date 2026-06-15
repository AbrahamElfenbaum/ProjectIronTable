// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "BaseCameraController.generated.h"

class ACameraPawn;
class UCameraSettingsSave;
class UEnhancedInputLocalPlayerSubsystem;

/** Base player controller providing free-look camera control — WASD movement, mouse-drag pan, zoom, sprint, and camera-settings persistence. Mode-specific controllers (session, map builder) subclass this. */
UCLASS()
class PROJECTIRONTABLE_API ABaseCameraController : public APlayerController
{
	GENERATED_BODY()

private:

#pragma region State
	/** Active speed multiplier; 1.0 normally, CameraSpeedMultiplier while sprinting. */
	float CurrentCameraSpeedMultiplier = 1.f;
#pragma endregion

protected:

#pragma region Runtime References
	/** Cached reference to the possessed camera pawn. */
	UPROPERTY()
	TObjectPtr<ACameraPawn> CameraPawnRef;

	/** Reference to the Enhanced Input subsystem for managing mapping contexts. */
	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystemRef;
#pragma endregion

#pragma region Camera State
	/** When false, panning input is active and WASD movement is suppressed. */
	bool bCanCameraMove = true;
#pragma endregion

public:

#pragma region Camera Config
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
#pragma endregion

#pragma region Camera Input
	/** Input mapping context active during normal gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputMappingContext> IMC_Camera;

	/** WASD camera translation input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputAction> IA_CameraMove;

	/** Mouse-drag camera pan input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputAction> IA_CameraPan;

	/** Resets camera pitch to default while held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputAction> IA_CameraPanReset;

	/** Hold to apply the camera sprint speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputAction> IA_CameraSprint;

	/** Scroll-wheel camera zoom input action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Input")
	TObjectPtr<UInputAction> IA_CameraZoom;
#pragma endregion

private:

#pragma region Input Handlers
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
#pragma endregion

protected:

	/** Caches the pawn reference, registers the camera input context, and binds the camera input actions. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Sets input mode, shows cursor, and loads saved camera settings. */
	virtual void BeginPlay() override;

#if WITH_EDITOR
	/** Delegates to ValidateCameraSettings so editor validation uses the same rules as runtime. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Returns a movement speed scaled by the current spring arm length, clamped to the configured min/max. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float CalculateCameraMovementSpeed() const;

public:

#pragma region Public Methods
	/** Clamps all camera config properties to valid ranges. Called at runtime and in the editor. */
	void ValidateCameraSettings();

	/** Copies values from the save object into camera config properties, then validates. */
	void ApplyCameraSettings(const UCameraSettingsSave* Settings);

	/** Writes current camera config properties to a new save object and saves to slot "CameraSettings". */
	void SaveCameraSettings();
#pragma endregion
};
