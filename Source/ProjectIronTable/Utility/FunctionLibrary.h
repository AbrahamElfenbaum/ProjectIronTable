// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiceData.h"
#include "FunctionLibrary.generated.h"

/** General-purpose static utility functions exposed to Blueprints. */
UCLASS()
class PROJECTIRONTABLE_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the display name string for a given EDiceType (e.g. "D20"). */
	UFUNCTION(BlueprintPure, Category = "Dice")
	static FString GetDiceName(EDiceType Type);
};
