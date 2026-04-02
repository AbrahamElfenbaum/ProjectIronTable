// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayController.h"
#include "GameplayHUDComponent.h"
#include "GameplayPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CameraSettingsSave.h"
#include "Kismet/GameplayStatics.h"

// Creates and attaches the HUD component subobject.
AGameplayController::AGameplayController()
{
	HUDComponent = CreateDefaultSubobject<UGameplayHUDComponent>(TEXT("HUDComponent"));
}

// Caches the pawn reference, registers the gameplay input context, and binds all input actions.
void AGameplayController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	GameplayPawnRef = Cast<AGameplayPawn>(InPawn);

	if (IsLocalController())
	{
		ULocalPlayer* LP = GetLocalPlayer();
		if (LP)
		{
			InputSubsystemRef = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (InputSubsystemRef)
			{
				InputSubsystemRef->AddMappingContext(IMC_Gameplay, 0);
			}
		}

		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->BindAction(IA_CameraMove, ETriggerEvent::Triggered, this, &AGameplayController::Input_CameraMove);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Triggered, this, &AGameplayController::Input_CameraPan);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Started, this, &AGameplayController::Input_CameraPan);
			EIC->BindAction(IA_CameraPan, ETriggerEvent::Completed, this, &AGameplayController::Input_CameraPan);
			EIC->BindAction(IA_CameraPanReset, ETriggerEvent::Triggered, this, &AGameplayController::Input_CameraPanReset);
			EIC->BindAction(IA_CameraSprint, ETriggerEvent::Triggered, this, &AGameplayController::Input_CameraSprint);
			EIC->BindAction(IA_CameraSprint, ETriggerEvent::Completed, this, &AGameplayController::Input_CameraSprint);
			EIC->BindAction(IA_CameraZoom, ETriggerEvent::Triggered, this, &AGameplayController::Input_CameraZoom);
			EIC->BindAction(IA_FocusChat, ETriggerEvent::Triggered, this, &AGameplayController::Input_FocusChat);
			EIC->BindAction(IA_ExitChat, ETriggerEvent::Triggered, this, &AGameplayController::Input_ExitChat);
			EIC->BindAction(IA_ScrollChat, ETriggerEvent::Triggered, this, &AGameplayController::Input_ScrollChat);
		}
	}
}

// Clamps all camera config properties to valid ranges; shared between editor validation and runtime apply.
void AGameplayController::ValidateCameraSettings()
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
void AGameplayController::ApplyCameraSettings(const UCameraSettingsSave* Settings)
{
	if (!Settings) return;

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
void AGameplayController::SaveCameraSettings()
{
	UCameraSettingsSave* Save = NewObject<UCameraSettingsSave>();
	Save->MinCameraMovementSpeed = MinCameraMovementSpeed;
	Save->MaxCameraMovementSpeed = MaxCameraMovementSpeed;
	Save->CameraSpeedMultiplier = CameraSpeedMultiplier;
	Save->MinCameraPitch = MinCameraPitch;
	Save->MaxCameraPitch = MaxCameraPitch;
	Save->CameraPanSpeedMultiplier = CameraPanSpeedMultiplier;
	Save->MinZoomLength = MinZoomLength;
	Save->MaxZoomLength = MaxZoomLength;
	Save->ZoomSpeed = ZoomSpeed;

	UGameplayStatics::SaveGameToSlot(Save, TEXT("CameraSettings"), 0);
}

#if WITH_EDITOR
// Delegates to ValidateCameraSettings so editor and runtime share the same validation logic.
void AGameplayController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ValidateCameraSettings();
}
#endif

// Sets input mode and cursor, then loads and applies saved camera settings if a save exists.
void AGameplayController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (UGameplayStatics::DoesSaveGameExist(TEXT("CameraSettings"), 0))
	{
		UCameraSettingsSave* LoadedSettings = Cast<UCameraSettingsSave>(UGameplayStatics::LoadGameFromSlot(TEXT("CameraSettings"), 0));
		ApplyCameraSettings(LoadedSettings);
	}
}

// Translates the pawn along the XY plane using the scaled movement speed.
void AGameplayController::Input_CameraMove(const FInputActionValue& Value)
{
	FVector2D MoveInput = Value.Get<FVector2D>();

	if (GameplayPawnRef && bCanCameraMove)
	{
		float Speed = CalculateCameraMovementSpeed() * CurrentCameraSpeedMultiplier;
		FVector Delta = (GameplayPawnRef->GetActorRightVector() * MoveInput.X
			+ GameplayPawnRef->GetActorForwardVector() * MoveInput.Y) * Speed;
		Delta.Z = 0.f;
		GameplayPawnRef->SetActorLocation(GameplayPawnRef->GetActorLocation() + Delta);
	}
}

// Locks/unlocks movement and rotates the pawn yaw and pitch using mouse delta while panning.
void AGameplayController::Input_CameraPan(const FInputActionValue& Value)
{
	bCanCameraMove = !Value.Get<bool>();

	if (!bCanCameraMove && GameplayPawnRef)
	{
		float DeltaX, DeltaY;
		GetInputMouseDelta(DeltaX, DeltaY);

		FRotator CurrentRotation = GameplayPawnRef->GetActorRotation();
		float NewYaw = CurrentRotation.Yaw + DeltaX * CameraPanSpeedMultiplier;
		float NewPitch = FMath::Clamp(CurrentRotation.Pitch + DeltaY * CameraPanSpeedMultiplier, MinCameraPitch, MaxCameraPitch);
		GameplayPawnRef->SetActorRotation(FRotator(NewPitch, NewYaw, CurrentRotation.Roll));
	}
}

// Resets the pawn pitch to -15 degrees while preserving yaw and roll.
void AGameplayController::Input_CameraPanReset()
{
	if (GameplayPawnRef)
	{
		FRotator CurrentRotation = GameplayPawnRef->GetActorRotation();
		GameplayPawnRef->SetActorRotation(FRotator(-15.f, CurrentRotation.Yaw, CurrentRotation.Roll));
	}
}

// Sets the active camera speed multiplier to CameraSpeedMultiplier while held, and back to 1.0 on release.
void AGameplayController::Input_CameraSprint(const FInputActionValue& Value)
{
	CurrentCameraSpeedMultiplier = Value.Get<bool>() ? CameraSpeedMultiplier : 1.f;
}

// Adjusts the spring arm length by ZoomSpeed in the direction of the scroll input, clamped to min/max.
void AGameplayController::Input_CameraZoom(const FInputActionValue& Value)
{
	if (GameplayPawnRef)
	{
		float CurrentArmLength = GameplayPawnRef->SpringArm->TargetArmLength;
		GameplayPawnRef->SpringArm->TargetArmLength = FMath::Clamp(
			CurrentArmLength - FMath::Sign(Value.Get<float>()) * ZoomSpeed,
			MinZoomLength,
			MaxZoomLength);
	}
}

// Adds the chat input mapping context and tells the HUD to focus the chat box.
void AGameplayController::Input_FocusChat()
{
	if (InputSubsystemRef) InputSubsystemRef->AddMappingContext(IMC_Chat, 1);
	if (HUDComponent) HUDComponent->FocusChat();
}

// Removes the chat input mapping context and tells the HUD to exit chat.
void AGameplayController::Input_ExitChat()
{
	if (InputSubsystemRef) InputSubsystemRef->RemoveMappingContext(IMC_Chat);
	if (HUDComponent) HUDComponent->ExitChat();
}

// Forwards the scroll direction (positive = up) to the HUD chat scroll handler.
void AGameplayController::Input_ScrollChat(const FInputActionValue& Value)
{
	float ScrollInput = Value.Get<float>();

	if (HUDComponent) HUDComponent->ScrollChat(ScrollInput > 0);
}

// Returns movement speed proportional to spring arm length, clamped between min and max.
float AGameplayController::CalculateCameraMovementSpeed() const
{
	if (GameplayPawnRef)
	{
		return FMath::Clamp(GameplayPawnRef->SpringArm->TargetArmLength / 100.f,
							MinCameraMovementSpeed,
							MaxCameraMovementSpeed);
	}

	return 10.f;
}
