// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiceData.h"
#include "BaseDiceActor.generated.h"

class UPhysicalMaterial;
class USoundBase;

/** Stores the result of a single die roll, including the value, type, and a pointer to the actor that produced it. */
USTRUCT(BlueprintType)
struct FRollResult
{
	GENERATED_BODY()

	/** The numeric result of the roll. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value;

	/** The type of die that was rolled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDiceType DiceType;

	/** The dice actor instance that produced this result. */
	UPROPERTY(BlueprintReadOnly)
	ABaseDiceActor* DiceActor = nullptr;

	FRollResult()
		:Value(0),
		DiceType(),
		DiceActor(nullptr)
	{
	}

	FRollResult(int32 InValue, EDiceType InDiceType)
		:Value(InValue),
		DiceType(InDiceType),
		DiceActor(nullptr)
	{
	}
};

/** Fired when both meshes have settled and a final roll value has been determined. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceRolled, FRollResult, Result);

/** Fired when the failsafe timer expires before the die finishes rolling. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFailsafeDestroy, EDiceType, DiceType);

/** Physics-based dice actor. Supports one or two meshes for standard and percentile dice. */
UCLASS()
class PROJECTIRONTABLE_API ABaseDiceActor : public AActor
{
	GENERATED_BODY()

public:

#pragma region Components
	/** Primary die mesh; always present. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<UStaticMeshComponent> Mesh1;

	/** Secondary die mesh; used for percentile dice (D100). Null for all other dice. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<UStaticMeshComponent> Mesh2;
#pragma endregion

#pragma region Config
	/** Face data asset for the primary mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<UDiceData> DiceFaces1;

	/** Face data asset for the secondary mesh (percentile only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<UDiceData> DiceFaces2;

	/** Sound played when the die strikes a non-die surface (e.g. the table). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<USoundBase> CollisionSoundSurface;

	/** Sound played when the die strikes another die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<USoundBase> CollisionSoundDice;

	/** Mass (kg) applied to both mesh physics bodies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float Mass = 1.f;

	/** Physical material applied to both meshes for friction and restitution. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<UPhysicalMaterial> PhysicalMaterial;

	/** Linear damping applied to both meshes to slow translational movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float LinearDamping = 0.1f;

	/** Angular damping applied to both meshes to slow rotational movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularDamping = 0.2f;

	/** Half-range added as random noise to the launch impulse vector. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseRange = 500.f;

	/** Half-range added as random noise to the angular impulse vector. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularImpulseRange = 50.f;

	/** Seconds after Roll() before the failsafe destroy fires if the die has not settled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float FailSafeTime = 10.0f;

	/** Minimum seconds between collision sounds; prevents rapid-fire hits from spamming audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ThrottleInterval = 0.1f;

	/** Divisor applied to the collision impulse magnitude to derive the volume multiplier (0.1–1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseVolumeScale = 1000.f;
#pragma endregion

#pragma region State
	/** True once Mesh1 has entered sleep and physics have been disabled for it. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dice")
	bool bMesh1Asleep = false;

	/** True once Mesh2 has entered sleep, or immediately if Mesh2 is not used. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dice")
	bool bMesh2Asleep = false;

	/** False if this die was discarded during an advantage/disadvantage roll. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	bool bWasKept = true;
#pragma endregion

public:
	/** Creates root and both mesh subobjects, and applies physics properties to each. */
	ABaseDiceActor();

protected:
	/** Detaches meshes for independent physics simulation and binds sleep and hit delegates. */
	virtual void BeginPlay() override;

public:

#pragma region Events
	/** Fired when all meshes have settled and a result is ready. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceRolled OnDiceRolled;

	/** Fired when the failsafe timer expires before the die finishes settling. */
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnFailsafeDestroy OnFailsafeDestroy;
#pragma endregion

#pragma region Public Methods
	/** Determines the face value by finding the mesh face whose normal is most aligned with world up. */
	UFUNCTION(BlueprintCallable)
	FRollResult GetRolledValue();

	/** Applies the given impulse and angular impulse to the mesh(es) and starts the failsafe timer. */
	UFUNCTION(BlueprintCallable)
	void Roll(FVector Impulse, FVector AngularImpulse);
#pragma endregion

private:

	/** Called when a mesh physics body enters sleep; fires OnDiceRolled once both meshes are asleep. */
	UFUNCTION()
	void OnMeshSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	/** Plays a collision sound when a mesh strikes another object; throttled to prevent rapid-fire hits. */
	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns true if the mesh is non-null and has a valid static mesh asset assigned. */
	bool IsMeshValid(UStaticMeshComponent* Mesh) const;

	/** Returns the value of the face whose normal is most aligned with world up for the given mesh and data asset. */
	int32 GetFaceValue(UStaticMeshComponent* Mesh, UDiceData* DiceFaces) const;

	/** Timer handle for the failsafe destroy. */
	FTimerHandle FailsafeTimerHandle;

	/** World time of the last collision sound played; used to throttle hit sounds. */
	float LastHitTime = 0.f;

	/** Destroys the actor if it has not fully settled, broadcasting OnFailsafeDestroy first. */
	void FailsafeDestroy();
};
