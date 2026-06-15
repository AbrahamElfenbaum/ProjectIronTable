// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "MapBuilderController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

#include "MapGrid.h"

// Spawns a tile at the grid cell under the cursor.
void AMapBuilderController::Input_PlaceTile(const FInputActionValue& Value)
{
}

// Returns the grid cell under the mouse cursor; false if the cursor ray misses the grid plane.
bool AMapBuilderController::GetGridCellUnderCursor(FIntPoint& OutCell) const
{
	return false;
}

// Adds the build input context and binds the place-tile action, after the base camera setup.
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
			EIC->BindAction(IA_PlaceTile, ETriggerEvent::Triggered, this, &AMapBuilderController::Input_PlaceTile);
		}
	}
}

// Caches the map grid; logs a warning if none is found in the level.
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
}
