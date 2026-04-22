// Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
#include "RichTextEditorWidget.h"

// Creates the SRichTextEditor Slate widget and returns it for UMG to display.
TSharedRef<SWidget> URichTextEditorWidget::RebuildWidget()
{
	RichTextEditor = SNew(SRichTextEditor);
	return RichTextEditor.ToSharedRef();
}

// Releases the Slate widget and resets the shared pointer to free memory.
void URichTextEditorWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	RichTextEditor.Reset();
}
