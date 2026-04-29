// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SessionPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

/** Top-down camera pawn. Owns the spring arm and camera used for the gameplay view. */
UCLASS()
class PROJECTIRONTABLE_API ASessionPawn : public APawn
{
	GENERATED_BODY()

public:

#pragma region Components
	/** Invisible sphere mesh used as the pawn root for movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	TObjectPtr<UStaticMeshComponent> Sphere;

	/** Spring arm that controls camera distance and pitch. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	TObjectPtr<USpringArmComponent> SpringArm;

	/** The gameplay camera attached to the spring arm. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	TObjectPtr<UCameraComponent> Camera;
#pragma endregion

	/** Sets up component hierarchy with root, sphere, spring arm, and camera. */
	ASessionPawn();
};
