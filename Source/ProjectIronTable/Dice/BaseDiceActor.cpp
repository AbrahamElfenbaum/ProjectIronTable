// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "BaseDiceActor.h"

#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"

// Creates root and both mesh subobjects, and applies physics properties to each.
ABaseDiceActor::ABaseDiceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh1"));
	Mesh1->SetupAttachment(Root);

	Mesh1->SetSimulatePhysics(true);
	Mesh1->SetMassOverrideInKg(NAME_None, Mass, true);
	Mesh1->SetPhysMaterialOverride(PhysicalMaterial);
	Mesh1->SetLinearDamping(LinearDamping);
	Mesh1->SetAngularDamping(AngularDamping);

	Mesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh2"));
	Mesh2->SetupAttachment(Root);

	Mesh2->SetSimulatePhysics(true);
	Mesh2->SetMassOverrideInKg(NAME_None, Mass, true);
	Mesh2->SetPhysMaterialOverride(PhysicalMaterial);
	Mesh2->SetLinearDamping(LinearDamping);
	Mesh2->SetAngularDamping(AngularDamping);
}

// Returns the roll result by finding the highest dot-product face on each mesh, combining for percentile dice.
FRollResult ABaseDiceActor::GetRolledValue()
{
	FRollResult RollResult = FRollResult();

	RollResult.DiceActor = this;

	// for 1 die
	if (!IsMeshValid(Mesh2) || !DiceFaces2)
	{
		RollResult.DiceType = DiceFaces1->DiceType;
		RollResult.Value = GetFaceValue(Mesh1, DiceFaces1);
		if (RollResult.Value == 0)
		{
			RollResult.Value = 10;
		}
		return RollResult;
	}

	// for 2 dice
	int32 Value1 = GetFaceValue(Mesh1, DiceFaces1);
	int32 Value2 = GetFaceValue(Mesh2, DiceFaces2);
	RollResult.DiceType = DiceFaces2->DiceType;

	if (Value1 == 0 &&
		Value2 == 0)
	{
		RollResult.Value = 100;
		return RollResult;
	}

	RollResult.Value = Value1 + Value2;
	return RollResult;
}

// Applies impulse and angular impulse to each valid mesh and starts the failsafe timer.
void ABaseDiceActor::Roll(FVector Impulse, FVector AngularImpulse)
{
	bMesh1Asleep = false;
	bMesh2Asleep = !IsMeshValid(Mesh2);

	if (IsMeshValid(Mesh1) && Mesh1->IsSimulatingPhysics())
	{
		Mesh1->AddImpulse(Impulse);
		Mesh1->AddAngularImpulseInRadians(AngularImpulse);
	}

	if (IsMeshValid(Mesh2) && Mesh2->IsSimulatingPhysics())
	{
		Mesh2->AddImpulse(Impulse +
			FVector(FMath::FRandRange(-ImpulseRange, ImpulseRange),
				FMath::FRandRange(-ImpulseRange, ImpulseRange),
				0));

		Mesh2->AddAngularImpulseInRadians(AngularImpulse +
			FVector(FMath::FRandRange(-AngularImpulseRange, AngularImpulseRange),
				FMath::FRandRange(-AngularImpulseRange, AngularImpulseRange),
				FMath::FRandRange(-AngularImpulseRange, AngularImpulseRange)));
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABaseDiceActor::Roll — GetWorld() returned null."));
		return;
	}

	World->GetTimerManager().SetTimer(FailsafeTimerHandle,
		this,
		&ABaseDiceActor::FailsafeDestroy,
		FailSafeTime,
		false);
}

// Detaches meshes for independent physics simulation and binds sleep delegates; marks Mesh2 as asleep if unused.
void ABaseDiceActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsMeshValid(Mesh1))
	{
		Mesh1->BodyInstance.bAutoWeld = false;
		Mesh1->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Mesh1->SetSimulatePhysics(true);
		Mesh1->RecreatePhysicsState();
		Mesh1->SetNotifyRigidBodyCollision(true);

		Mesh1->OnComponentSleep.AddDynamic(this, &ABaseDiceActor::OnMeshSleep);
		Mesh1->OnComponentHit.AddDynamic(this, &ABaseDiceActor::OnMeshHit);
	}

	if (IsMeshValid(Mesh2))
	{
		Mesh2->BodyInstance.bAutoWeld = false;
		Mesh2->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Mesh2->SetSimulatePhysics(true);
		Mesh2->RecreatePhysicsState();
		Mesh2->SetNotifyRigidBodyCollision(true);

		Mesh2->OnComponentSleep.AddDynamic(this, &ABaseDiceActor::OnMeshSleep);
		Mesh2->OnComponentHit.AddDynamic(this, &ABaseDiceActor::OnMeshHit);
	}
	else
	{
		bMesh2Asleep = true;
	}
}

// Returns true only if the mesh pointer is valid and has a static mesh asset assigned.
bool ABaseDiceActor::IsMeshValid(UStaticMeshComponent* Mesh) const
{
	return (Mesh && Mesh->GetStaticMesh());
}

// Iterates all faces and returns the value of the face whose transformed normal has the highest dot product with world up.
int32 ABaseDiceActor::GetFaceValue(UStaticMeshComponent* Mesh, UDiceData* DiceFaces) const
{
	if (!IsMeshValid(Mesh) || !DiceFaces)
	{
		return int32();
	}

	int32 Result = int32();
	FTransform World = Mesh->GetComponentTransform();
	float CurrentMaxDot = -1.0f;

	for (const FFaceData& Face : DiceFaces->Faces)
	{
		FVector Direction = World.TransformVectorNoScale(Face.FaceNormal);
		float Dot = FVector::DotProduct(Direction, FVector::UpVector);

		if (Dot > CurrentMaxDot)
		{
			CurrentMaxDot = Dot;
			Result = Face.FaceValue;
		}
	}

	return Result;
}

// Destroys the actor and broadcasts OnFailsafeDestroy if either mesh has not yet finished settling.
void ABaseDiceActor::FailsafeDestroy()
{
	if (!bMesh1Asleep || !bMesh2Asleep)
	{
		EDiceType LostType = (IsMeshValid(Mesh2) && DiceFaces2) ? DiceFaces2->DiceType : DiceFaces1->DiceType;
		OnFailsafeDestroy.Broadcast(LostType);
		Destroy();
	}
}

// Marks the sleeping mesh's flag and broadcasts OnDiceRolled once both meshes are asleep.
void ABaseDiceActor::OnMeshSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	SleepingComponent->SetSimulatePhysics(false);

	if (SleepingComponent == Mesh1)
	{
		bMesh1Asleep = true;
	}
	else if (SleepingComponent == Mesh2)
	{
		bMesh2Asleep = true;
	}

	if (bMesh1Asleep && bMesh2Asleep)
	{
		FRollResult Result = GetRolledValue();
		OnDiceRolled.Broadcast(Result);
	}
}

// Throttles collision sound playback and selects surface or dice sound based on what was hit.
void ABaseDiceActor::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastHitTime < ThrottleInterval)
	{
		return;
	}

	LastHitTime = CurrentTime;
	float Volume = FMath::Clamp(NormalImpulse.Size() / ImpulseVolumeScale, 0.1f, 1.f);

	if (Cast<ABaseDiceActor>(OtherActor))
	{
		if (CollisionSoundDice)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CollisionSoundDice, GetActorLocation(), Volume);
		}
	}
	else
	{
		if (CollisionSoundSurface)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CollisionSoundSurface, GetActorLocation(), Volume);
		}
	}
}
