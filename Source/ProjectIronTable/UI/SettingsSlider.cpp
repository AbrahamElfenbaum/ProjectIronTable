// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "SettingsSlider.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"

// Applies config to widgets and binds slider and text delegates.
void USettingsSlider::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleText)
	{
		TitleText->SetText(Title);
	}

	if (ValueSlider)
	{
		ValueSlider->SetMinValue(SliderMin);
		ValueSlider->SetMaxValue(SliderMax);
		ValueSlider->OnValueChanged.AddDynamic(this, &USettingsSlider::OnSliderValueChanged);
	}

	if (ValueText)
	{
		ValueText->OnTextCommitted.AddDynamic(this, &USettingsSlider::OnTextCommitted);
	}
}

// Returns the current slider value, or 0 if the slider is not bound.
float USettingsSlider::GetValue() const
{
	if (ValueSlider)
	{
		return ValueSlider->GetValue();
	}
	return 0.f;
}

// Resets the slider to the configured default value.
void USettingsSlider::ResetToDefault()
{
	SetValue(DefaultValue);
}

// Sets the text field and slider directly; used for loading saved values and reset without triggering paired clamping.
void USettingsSlider::SetValue(float Value)
{
	if (ValueText)
	{
		ValueText->SetText(FText::FromString(FString::SanitizeFloat(Value)));
	}
	
	if (ValueSlider)
	{
		ValueSlider->SetValue(Value);
	}
}

// Syncs the text field to the clamped value and corrects the slider if pairing requires it.
void USettingsSlider::OnSliderValueChanged(float Value)
{
	float ClampedValue = ClampToPair(Value);

	if (ValueText)
	{
		ValueText->SetText(FText::FromString(FString::SanitizeFloat(ClampedValue)));
	}

	if (ClampedValue != Value && ValueSlider)
	{
		ValueSlider->SetValue(ClampedValue);
	}
}

// Parses the committed text, clamps to slider bounds and pair gap, then applies the result.
void USettingsSlider::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!ValueSlider) return;

	float Value = FCString::Atof(*Text.ToString());
	SetValue(ClampToPair(FMath::Clamp(Value, ValueSlider->GetMinValue(), ValueSlider->GetMaxValue())));
}

// Clamps Value to stay below or above the paired slider's value by at least PairGap; returns Value unchanged if no pair is set.
float USettingsSlider::ClampToPair(float Value)
{
	if (!PairedSlider)
	{
		return Value;
	}

	if (bIsMin)
	{
		return FMath::Min(Value, PairedSlider->GetValue() - PairGap);
	}
	else
	{
		return FMath::Max(Value, PairedSlider->GetValue() + PairGap);
	}
}
