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

public:

#pragma region Public Methods
	/** Sets whether newly typed text and any selected text will be rendered in bold. */
	void ToggleBold(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered in italic. */
	void ToggleItalic(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered with an underline. */
	void ToggleUnderline(bool bEnable);

	/** Sets whether newly typed text and any selected text will be rendered with a strikethrough. */
	void ToggleStrikethrough(bool bEnable);

	/** Returns the current document for serialization. */
	FRichTextDocument GetDocument() const;

	/** Replaces the current document with the given one, used when loading saved content. */
	void SetDocument(const FRichTextDocument& InDocument);
#pragma endregion

protected:
	/** Creates the underlying SRichTextEditor Slate widget and returns it for UMG to manage. */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Releases the Slate widget and cleans up the shared pointer when this widget is destroyed. */
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:

#pragma region Runtime References
	/** The underlying Slate rich-text editor widget created and owned by this UMG wrapper. */
	TSharedPtr<SRichTextEditor> RichTextEditor;
#pragma endregion
};
