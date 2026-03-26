// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiceData.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "BaseDiceActor.generated.h"

class ABaseDiceActor;

USTRUCT(BlueprintType)
struct FRollResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDiceType DiceType;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceRolled, FRollResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFailsafeDestroy, EDiceType, DiceType);

UCLASS()
class PROJECTIRONTABLE_API ABaseDiceActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	UStaticMeshComponent* Mesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	UStaticMeshComponent* Mesh2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	UDiceData* DiceFaces1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	UDiceData* DiceFaces2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float Mass = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	TObjectPtr<UPhysicalMaterial> PhysicalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float LinearDamping = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularDamping = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float ImpulseRange = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float AngularImpulseRange = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dice")
	float FailSafeTime = 10.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dice")
	bool bMesh1Asleep = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Dice")
	bool bMesh2Asleep = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dice")
	bool bWasKept = true;

public:
	ABaseDiceActor();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnDiceRolled OnDiceRolled;

	UPROPERTY(BlueprintAssignable, Category = "Dice")
	FOnFailsafeDestroy OnFailsafeDestroy;

	UFUNCTION(BlueprintCallable)
	FRollResult GetRolledValue();

	UFUNCTION(BlueprintCallable)
	void Roll(FVector Impulse, FVector AngularImpulse);

private:
	UFUNCTION()
	void OnMeshSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);

	bool IsMeshValid(UStaticMeshComponent* Mesh) const;

	int32 GetFaceValue(UStaticMeshComponent* Mesh, UDiceData* DiceFaces) const;
	
	FTimerHandle FailsafeTimerHandle;

	void FailsafeDestroy();
};
