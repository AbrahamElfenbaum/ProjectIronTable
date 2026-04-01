// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "GameplayController.h"
#include "GameplayHUDComponent.h"
#include "GameplayPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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

#if WITH_EDITOR
// Clamps all camera config properties to sane ranges whenever a property is changed in the editor.
void AGameplayController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

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
#endif

void AGameplayController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
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
