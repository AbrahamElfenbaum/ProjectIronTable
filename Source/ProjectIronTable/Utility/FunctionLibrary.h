// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiceData.h"
#include "FunctionLibrary.generated.h"

UCLASS()
class PROJECTIRONTABLE_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Dice")
	static FString GetDiceName(EDiceType Type);
};
