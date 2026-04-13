// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DiceData.generated.h"

/** Identifies the type of die, used to select the correct data asset and display name. */
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

/** Stores the face normal and integer value for a single face on a die mesh. */
USTRUCT(BlueprintType)
struct FFaceData
{
	GENERATED_BODY()

	/** The integer value printed on this face. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FaceValue;

	/** The local-space normal vector pointing away from this face. Used to determine the upward-facing face after a roll. */
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

/** Data asset that defines the type and face layout of a single die. */
UCLASS()
class PROJECTIRONTABLE_API UDiceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** Determines which type of die this data asset represents (e.g. D4, D6, D12). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EDiceType DiceType;

	/** Array of face normals and values for every face on this die. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FFaceData> Faces;

};
