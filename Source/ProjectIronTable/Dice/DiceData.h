// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DiceData.generated.h"

UENUM(BlueprintType)
enum class EDiceType : uint8
{
	D4 UMETA(DisplayName = "D4"),
	D6 UMETA(DisplayName = "D6"),
	D8 UMETA(DisplayName = "D8"),
	D10 UMETA(DisplayName = "D10"),
	D12 UMETA(DisplayName = "D12"),
	D20 UMETA(DisplayName = "D20"),
	D100 UMETA(DisplayName = "D100"),
};

USTRUCT(BlueprintType)
struct FFaceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FaceValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FaceNormal;

	FFaceData()
		:FaceValue(0),
		FaceNormal(FVector::ZeroVector)
	{
	}

	FFaceData(FVector InFaceNormal, int32 InFaceValue)
		:FaceValue(InFaceValue),
		FaceNormal(InFaceNormal)

	{
	}
};

UCLASS()
class PROJECTIRONTABLE_API UDiceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (ToolTip = "Determines which type of die this is (e.g. D4, D6, D12)."))
	EDiceType DiceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (ToolTip = "Holds the Normal Vector and Value of each die face."))
	TArray<FFaceData> Faces;
	
};
