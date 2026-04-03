// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "SettingsSlider.generated.h"

/** A labeled slider widget paired with an editable text field. Supports optional pairing with another slider to enforce a min/max gap. */
UCLASS()
class PROJECTIRONTABLE_API USettingsSlider : public UUserWidget
{
	GENERATED_BODY()

public:

#pragma region Config
	/** Display name shown in the title label. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Minimum value the slider can be set to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SliderMin = 0.f;

	/** Maximum value the slider can be set to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SliderMax = 1.f;

	/** Value the slider resets to when ResetToDefault is called. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultValue = 0.f;

	/** Optional slider whose value is used to enforce a min/max gap with this one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USettingsSlider> PairedSlider;

	/** Minimum gap enforced between this slider's value and its paired slider's value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PairGap = .1f;

	/** If true, this slider is the minimum of the pair and will be capped below the paired slider's value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMin;
#pragma endregion

protected:
	/** Applies config to widgets and binds slider and text delegates. */
	virtual void NativeConstruct() override;

private:

#pragma region Widget References
	/** Displays the title label. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** The slider widget. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> ValueSlider;

	/** Editable text field that mirrors the slider value. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ValueText;
#pragma endregion

public:

	/** Returns the current slider value. */
	float GetValue() const;

	/** Sets the slider and text to the given value without triggering paired clamping. */
	void SetValue(float Value);

	/** Resets the slider to DefaultValue. */
	void ResetToDefault();

private:

	/** Syncs the text field and enforces paired clamping when the slider value changes. */
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	/** Parses the committed text, clamps it to slider bounds and pair gap, then applies it. */
	UFUNCTION()
	void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/** Returns Value clamped relative to the paired slider's value, or unchanged if no pair is set. */
	float ClampToPair(float Value);
	
};
