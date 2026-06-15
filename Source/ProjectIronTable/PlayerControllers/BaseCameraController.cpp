// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseCameraController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

#include "CameraPawn.h"
#include "CameraSettingsSave.h"
#include "MacroLibrary.h"

// Translates the pawn along the XY plane using the scaled movement speed.
void ABaseCameraController::Input_CameraMove(const FInputActionValue& Value)
{
	FVector2D MoveInput = Value.Get<FVector2D>();

	if (IsValid(CameraPawnRef) && bCanCameraMove)
	{
		float Speed = CalculateCameraMovementSpeed() * CurrentCameraSpeedMultiplier;
		FVector Delta = (CameraPawnRef->GetActorRightVector() * MoveInput.X
			+ CameraPawnRef->GetActorForwardVector() * MoveInput.Y) * Speed;
		Delta.Z = 0.f;
		CameraPawnRef->SetActorLocation(CameraPawnRef->GetActorLocation() + Delta);
	}
}

// Locks/unlocks movement and rotates the pawn yaw and pitch using mouse delta while panning.
void ABaseCameraController::Input_CameraPan(const FInputActionValue& Value)
{
	bCanCameraMove = !Value.Get<bool>();

	if (!bCanCameraMove && IsValid(CameraPawnRef))
	{
		float DeltaX, DeltaY;
		GetInputMouseDelta(DeltaX, DeltaY);

		FRotator CurrentRotation = CameraPawnRef->GetActorRotation();
		float NewYaw = CurrentRotation.Yaw + DeltaX * CameraPanSpeedMultiplier;
		float NewPitch = FMath::Clamp(CurrentRotation.Pitch + DeltaY * CameraPanSpeedMultiplier, MinCameraPitch, MaxCameraPitch);
		CameraPawnRef->SetActorRotation(FRotator(NewPitch, NewYaw, CurrentRotation.Roll));
	}
}

// Resets the pawn pitch to -15 degrees while preserving yaw and roll.
void ABaseCameraController::Input_CameraPanReset()
{
	if (IsValid(CameraPawnRef))
	{
		FRotator CurrentRotation = CameraPawnRef->GetActorRotation();
		CameraPawnRef->SetActorRotation(FRotator(-15.f, CurrentRotation.Yaw, CurrentRotation.Roll));
	}
}

// Sets the active camera speed multiplier to CameraSpeedMultiplier while held, and back to 1.0 on release.
void ABaseCameraController::Input_CameraSprint(const FInputActionValue& Value)
{
	CurrentCameraSpeedMultiplier = Value.Get<bool>() ? CameraSpeedMultiplier : 1.f;
}

// Adjusts the spring arm length by ZoomSpeed in the direction of the scroll input, clamped to min/max.
void ABaseCameraController::Input_CameraZoom(const FInputActionValue& Value)
{
	if (IsValid(CameraPawnRef))
	{
		float CurrentArmLength = CameraPawnRef->SpringArm->TargetArmLength;
		CameraPawnRef->SpringArm->TargetArmLength = FMath::Clamp(
			CurrentArmLength - FMath::Sign(Value.Get<float>()) * ZoomSpeed,
			MinZoomLength,
			MaxZoomLength);
	}
}

// Caches the pawn reference, registers the camera input context, and binds the camera input actions.
void ABaseCameraController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CameraPawnRef = Cast<ACameraPawn>(InPawn);

	if (IsLocalController())
	{
		ULocalPlayer* LP = GetLocalPlayer();
		if (IsValid(LP))
		{
			InputSubsystemRef = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (IsValid(InputSubsystemRef))
			{
				InputSubsystemRef->AddMappingContext(IMC_Camera, 0);
			}
		}

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->BindAction(IA_CameraMove, ETriggerEvent::Triggered, this, &ABaseCameraController::Input_CameraMove);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Triggered, this, &ABaseCameraController::Input_CameraPan);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Started, this, &ABaseCameraController::Input_CameraPan);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Completed, this, &ABaseCameraController::Input_CameraPan);
			EIC->BindAction(IA_CameraPanReset, ETriggerEvent::Triggered, this, &ABaseCameraController::Input_CameraPanReset);
			EIC->BindAction(IA_CameraSprint, ETriggerEvent::Triggered, this, &ABaseCameraController::Input_CameraSprint);
			EIC->BindAction(IA_CameraSprint, ETriggerEvent::Completed, this, &ABaseCameraController::Input_CameraSprint);
			EIC->BindAction(IA_CameraZoom, ETriggerEvent::Triggered, this, &ABaseCameraController::Input_CameraZoom);
		}
	}
}

// Sets input mode and cursor, then loads and applies saved camera settings if a save exists.
void ABaseCameraController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (UGameplayStatics::DoesSaveGameExist(UCameraSettingsSave::SaveSlotName, 0))
	{
		UCameraSettingsSave* LoadedSettings = Cast<UCameraSettingsSave>(UGameplayStatics::LoadGameFromSlot(UCameraSettingsSave::SaveSlotName, 0));
		CHECK_IF_VALID(LoadedSettings, );
		ApplyCameraSettings(LoadedSettings);
	}
}

#if WITH_EDITOR
// Delegates to ValidateCameraSettings so editor and runtime share the same validation logic.
void ABaseCameraController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ValidateCameraSettings();
}
#endif

// Returns movement speed proportional to spring arm length, clamped between min and max.
float ABaseCameraController::CalculateCameraMovementSpeed() const
{
	if (IsValid(CameraPawnRef))
	{
		return FMath::Clamp(CameraPawnRef->SpringArm->TargetArmLength / 100.f,
			MinCameraMovementSpeed,
			MaxCameraMovementSpeed);
	}

	return 10.f;
}

// Clamps all camera config properties to valid ranges; shared between editor validation and runtime apply.
void ABaseCameraController::ValidateCameraSettings()
{
	MinCameraMovementSpeed = FMath::Max(MinCameraMovementSpeed, 0.1f);
	MaxCameraMovementSpeed = FMath::Max(MaxCameraMovementSpeed, 0.1f);
	if (MinCameraMovementSpeed >= MaxCameraMovementSpeed)
		MaxCameraMovementSpeed = MinCameraMovementSpeed + 1.f;

	MinZoomLength = FMath::Max(MinZoomLength, 1.f);
	MaxZoomLength = FMath::Max(MaxZoomLength, 1.f);
	if (MinZoomLength >= MaxZoomLength)
		MaxZoomLength = MinZoomLength + 100.f;

	ZoomSpeed = FMath::Max(ZoomSpeed, 1.f);
	CameraPanSpeedMultiplier = FMath::Max(CameraPanSpeedMultiplier, 0.1f);
	CameraSpeedMultiplier = FMath::Max(CameraSpeedMultiplier, 0.1f);

	if (MinCameraPitch >= MaxCameraPitch)
		MaxCameraPitch = MinCameraPitch + 1.f;
}

// Copies all values from the save object into camera config properties, then validates.
void ABaseCameraController::ApplyCameraSettings(const UCameraSettingsSave* Settings)
{
	if (!Settings)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABaseCameraController::ApplyCameraSettings — Settings was null"));
		return;
	}

	MinCameraMovementSpeed = Settings->MinCameraMovementSpeed;
	MaxCameraMovementSpeed = Settings->MaxCameraMovementSpeed;
	CameraSpeedMultiplier = Settings->CameraSpeedMultiplier;
	MinCameraPitch = Settings->MinCameraPitch;
	MaxCameraPitch = Settings->MaxCameraPitch;
	CameraPanSpeedMultiplier = Settings->CameraPanSpeedMultiplier;
	MinZoomLength = Settings->MinZoomLength;
	MaxZoomLength = Settings->MaxZoomLength;
	ZoomSpeed = Settings->ZoomSpeed;

	ValidateCameraSettings();
}

// Creates a new save object, writes current camera config values into it, and saves to slot "CameraSettings".
void ABaseCameraController::SaveCameraSettings()
{
	UCameraSettingsSave* Save = NewObject<UCameraSettingsSave>();
	CHECK_IF_VALID(Save, );
	Save->MinCameraMovementSpeed = MinCameraMovementSpeed;
	Save->MaxCameraMovementSpeed = MaxCameraMovementSpeed;
	Save->CameraSpeedMultiplier = CameraSpeedMultiplier;
	Save->MinCameraPitch = MinCameraPitch;
	Save->MaxCameraPitch = MaxCameraPitch;
	Save->CameraPanSpeedMultiplier = CameraPanSpeedMultiplier;
	Save->MinZoomLength = MinZoomLength;
	Save->MaxZoomLength = MaxZoomLength;
	Save->ZoomSpeed = ZoomSpeed;

	UGameplayStatics::SaveGameToSlot(Save, UCameraSettingsSave::SaveSlotName, 0);
}
