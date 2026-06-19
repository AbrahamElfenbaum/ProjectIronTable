// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MapBuilderController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

#include "MapGrid.h"
#include "TilePlacementComponent.h"

// Creates the tile placement component.
AMapBuilderController::AMapBuilderController()
{
	TilePlacementComponent = CreateDefaultSubobject<UTilePlacementComponent>(TEXT("TilePlacementComponent"));
}

// Adds a 90-degree step to the current placement rotation.
void AMapBuilderController::Input_RotateTile(const FInputActionValue& Value)
{
	TilePlacementComponent->RotateTile(Value.Get<float>() * 90.f);
}

// Places a tile at the grid cell under the cursor.
void AMapBuilderController::Input_PlaceTile(const FInputActionValue& Value)
{
	FIntPoint Cell;
	if (GetGridCellUnderCursor(Cell))
	{
		TilePlacementComponent->PlaceTile(Cell);
	}
}

// Deletes the placed tile under the cursor.
void AMapBuilderController::Input_DeleteTile(const FInputActionValue& Value)
{
	FIntPoint Cell;
	if (GetGridCellUnderCursor(Cell))
	{
		TilePlacementComponent->DeleteTile(Cell);
	}
}

// Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane.
bool AMapBuilderController::GetGridCellUnderCursor(FIntPoint& OutCell) const
{
	if (!IsValid(MapGridRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — Map Grid is null"));
		return false;
	}

	FVector WorldOrigin;
	FVector WorldDirection;
	const bool bDeprojectSuccess = DeprojectMousePositionToWorld(WorldOrigin, WorldDirection);

	if (!bDeprojectSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — Deproject failed"));
		return false;
	}

	if (FMath::IsNearlyZero(WorldDirection.Z))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — World Direction Z is near to or is at 0"));
		return false;
	}

	const float PlaneZ = MapGridRef->GetActorLocation().Z;
	const float HitDistance = (PlaneZ - WorldOrigin.Z) / WorldDirection.Z;

	if (HitDistance < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — Hit Distance is < 0"));
		return false;
	}

	OutCell = MapGridRef->WorldToGrid(WorldOrigin + WorldDirection * HitDistance);
	return true;
}

// Adds the build input context and binds the rotate, place, and delete actions, after the base camera setup.
void AMapBuilderController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		if (IsValid(InputSubsystemRef))
		{
			InputSubsystemRef->AddMappingContext(IMC_Build, 0);
		}
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EIC->BindAction(IA_RotateTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_RotateTile);
			EIC->BindAction(IA_PlaceTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_PlaceTile);
			EIC->BindAction(IA_DeleteTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_DeleteTile);
		}
	}
}

// Caches the map grid used for cursor-to-cell conversion.
void AMapBuilderController::BeginPlay()
{
	Super::BeginPlay();

	AActor* Found = UGameplayStatics::GetActorOfClass(this, AMapGrid::StaticClass());
	MapGridRef = Cast<AMapGrid>(Found);

	if (!IsValid(MapGridRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::BeginPlay — Map Grid is null"));
		return;
	}
}

// Updates the ghost tile to follow the grid cell under the cursor each frame.
void AMapBuilderController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FIntPoint Cell;
	if (GetGridCellUnderCursor(Cell))
	{
		TilePlacementComponent->UpdateGhostTile(Cell);
	}
}
