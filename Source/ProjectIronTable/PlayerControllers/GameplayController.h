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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UGameplayHUDComponent> HUDComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGameplayPawn> GameplayPawnRef;

	UPROPERTY()
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystemRef;

public:
	bool bCanCameraMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinCameraMovementSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraMovementSpeed = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraSpeedMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinCameraPitch = -15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraPitch = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraPanSpeedMultiplier = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoomLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoomLength = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 50.f;

private:
	float CurrentCameraSpeedMultiplier = 1.f;

public:
#pragma region Gameplay Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputMappingContext> IMC_Gameplay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraPan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraPanReset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraSprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_CameraZoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Input")
	TObjectPtr<UInputAction> IA_FocusChat;
#pragma endregion

#pragma region Chat Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputMappingContext> IMC_Chat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ExitChat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat Input")
	TObjectPtr<UInputAction> IA_ScrollChat;
#pragma endregion

protected:
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float CalculateCameraMovementSpeed() const;

private:
	void Input_CameraMove(const FInputActionValue& Value);
	void Input_CameraPan(const FInputActionValue& Value);
	void Input_CameraPanReset();
	void Input_CameraSprint(const FInputActionValue& Value);
	void Input_CameraZoom(const FInputActionValue& Value);
	void Input_FocusChat();
	void Input_ExitChat();
	void Input_ScrollChat(const FInputActionValue& Value);
};
