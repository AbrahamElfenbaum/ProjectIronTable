// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "DiceSpawnVolume.generated.h"

UCLASS()
class PROJECTIRONTABLE_API ADiceSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ADiceSpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	TObjectPtr<UBoxComponent> SpawnArea;

public:	
	FBox GetSpawnBox() const;
};
