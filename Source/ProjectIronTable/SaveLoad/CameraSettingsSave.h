// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CameraSettingsSave.generated.h"

/** Persists player camera settings across sessions. Saved to and loaded from slot "CameraSettings", index 0. */
UCLASS()
class PROJECTIRONTABLE_API UCameraSettingsSave : public USaveGame
{
	GENERATED_BODY()

public:

#pragma region Movement
	/** Minimum camera translation speed at full zoom-in. */
	UPROPERTY()
	float MinCameraMovementSpeed = 5.f;

	/** Maximum camera translation speed at full zoom-out. */
	UPROPERTY()
	float MaxCameraMovementSpeed = 20.f;

	/** Speed multiplier applied while the sprint input is held. */
	UPROPERTY()
	float CameraSpeedMultiplier = 2.f;
#pragma endregion

#pragma region Rotation
	/** Minimum pitch angle (degrees) for camera rotation. */
	UPROPERTY()
	float MinCameraPitch = -15.f;

	/** Maximum pitch angle (degrees) for camera rotation. */
	UPROPERTY()
	float MaxCameraPitch = 45.f;

	/** Mouse delta multiplier applied during camera pan rotation. */
	UPROPERTY()
	float CameraPanSpeedMultiplier = 5.f;
#pragma endregion

#pragma region Zoom
	/** Minimum spring arm length (closest zoom). */
	UPROPERTY()
	float MinZoomLength = 100.f;

	/** Maximum spring arm length (furthest zoom). */
	UPROPERTY()
	float MaxZoomLength = 2500.f;

	/** Amount the spring arm changes per scroll tick. */
	UPROPERTY()
	float ZoomSpeed = 50.f;
#pragma endregion
};
