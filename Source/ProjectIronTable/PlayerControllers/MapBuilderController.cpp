// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MapBuilderController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

#include "MapGrid.h"
#include "TileActor.h"
#include "TilePlacementComponent.h"

// Creates the tile placement component.
AMapBuilderController::AMapBuilderController()
{
	TilePlacementComponent = CreateDefaultSubobject<UTilePlacementComponent>(TEXT("TilePlacementComponent"));
}

// Raises or lowers the active build level by the input value.
void AMapBuilderController::Input_ChangeLevel(const FInputActionValue& Value)
{
	ActiveBuildLevel += static_cast<int32>(Value.Get<float>());
}

// Deletes the placed tile under the cursor.
void AMapBuilderController::Input_DeleteTile(const FInputActionValue& Value)
{
	if (!IsValid(MapGridRef))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::Input_DeleteTile — Map Grid is null"));
		return;
	}

	ATileActor* Tile = FindTileActor();
	if (IsValid(Tile) &&
		!Tile->GetIsLocked())
	{
		TilePlacementComponent->DeleteTile(MapGridRef->WorldToGrid(Tile->GetActorLocation()));
	}
}

// Places a tile at the grid cell under the cursor.
void AMapBuilderController::Input_PlaceTile(const FInputActionValue& Value)
{
	FIntVector Cell;
	if (GetGridCellUnderCursor(Cell))
	{
		TilePlacementComponent->PlaceTile(Cell);
	}
}

// Adds a 90-degree step to the current placement rotation.
void AMapBuilderController::Input_RotateTile(const FInputActionValue& Value)
{
	TilePlacementComponent->RotateTile(Value.Get<float>() * 90.f);
}

// Toggles the locked state of the tile under the cursor.
void AMapBuilderController::Input_ToggleTileLock(const FInputActionValue& Value)
{
	ATileActor* Tile = FindTileActor();
	if (IsValid(Tile))
	{
		Tile->SetIsLocked(!Tile->GetIsLocked());
	}
}

// Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane.
bool AMapBuilderController::GetGridCellUnderCursor(FIntVector& OutCell) const
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

	const float PlaneZ = MapGridRef->GetActorLocation().Z + ActiveBuildLevel * MapGridRef->TileHeight;
	const float HitDistance = (PlaneZ - WorldOrigin.Z) / WorldDirection.Z;

	if (HitDistance < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — Hit Distance is < 0"));
		return false;
	}

	const FIntVector XY = MapGridRef->WorldToGrid(WorldOrigin + WorldDirection * HitDistance);
	OutCell = FIntVector(XY.X, XY.Y, ActiveBuildLevel);

	if (!MapGridRef->IsValidCell(OutCell))
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::GetGridCellUnderCursor — Not a Valid Cell"));
		return false;
	}

	return true;
}

// Returns the tile under the cursor via a line trace on the Tiles channel; nullptr if nothing is hit.
ATileActor* AMapBuilderController::FindTileActor()
{
	FVector Start;
	FVector Direction;
	const bool bDeprojectSuccess = DeprojectMousePositionToWorld(Start, Direction);

	if (!bDeprojectSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::FindTileActor — Deproject failed"));
		return nullptr;
	}

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMapBuilderController::FindTileActor — World is Null "));
		return nullptr;
	}

	FHitResult Hit;
	FVector End = Start + Direction * 100000.f;

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);

	if (!bHit)
	{
		UE_LOG(LogTemp, Display, TEXT("AMapBuilderController::FindTileActor — No Hit Detected"));
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();

	return Cast<ATileActor>(HitActor);
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
			EIC->BindAction(IA_ChangeLevel, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_ChangeLevel);
			EIC->BindAction(IA_DeleteTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_DeleteTile);
			EIC->BindAction(IA_RotateTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_RotateTile);
			EIC->BindAction(IA_PlaceTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_PlaceTile);
			EIC->BindAction(IA_ToggleTileLock, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_ToggleTileLock);
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

	TilePlacementComponent->Init(MapGridRef);
}

// Updates the ghost tile to follow the grid cell under the cursor each frame.
void AMapBuilderController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FIntVector Cell;
	if (GetGridCellUnderCursor(Cell))
	{
		TilePlacementComponent->UpdateGhostTile(Cell);
	}
}
