// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameplayPawn.generated.h"

/** Top-down camera pawn. Owns the spring arm and camera used for the gameplay view. */
UCLASS()
class PROJECTIRONTABLE_API AGameplayPawn : public APawn
{
	GENERATED_BODY()

public:
	/** Sets up component hierarchy with root, sphere, spring arm, and camera. */
	AGameplayPawn();

public:

#pragma region Components
	/** Invisible sphere mesh used as the pawn root for movement. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<UStaticMeshComponent> Sphere;

	/** Spring arm that controls camera distance and pitch. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<class USpringArmComponent> SpringArm;

	/** The gameplay camera attached to the spring arm. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<class UCameraComponent> Camera;
#pragma endregion
};
