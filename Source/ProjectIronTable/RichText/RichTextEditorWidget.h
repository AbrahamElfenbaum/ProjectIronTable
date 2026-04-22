// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "RichTextEditor.h"
#include "RichTextEditorWidget.generated.h"

/** UMG wrapper around SRichTextEditor. Bridges the Slate rich-text editor widget into the UMG widget system. */
UCLASS()
class PROJECTIRONTABLE_API URichTextEditorWidget : public UWidget
{
	GENERATED_BODY()

protected:

	/** Creates the underlying SRichTextEditor Slate widget and returns it for UMG to manage. */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Releases the Slate widget and cleans up the shared pointer when this widget is destroyed. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:

	/** The underlying Slate rich-text editor widget created and owned by this UMG wrapper. */
	TSharedPtr<SRichTextEditor> RichTextEditor;
};
