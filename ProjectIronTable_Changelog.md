# ProjectIronTable — Changelog

> Most recent entries at top. For implementation detail see TechDoc; for design rationale see GDD.

---

## 2026-06-19

**Implementation:**
- **Map builder core placement complete.** `AMapBuilderController` Part B implemented, then all placement logic extracted into a new controller-owned `UTilePlacementComponent`.
  - `AMapBuilderController::GetGridCellUnderCursor` — deprojects the mouse, intersects the grid's Z-plane (`T = (PlaneZ - Origin.Z) / Dir.Z`), returns the cell via `WorldToGrid`. Guards: null grid, failed deproject, parallel ray (`Dir.Z≈0`), behind-camera (`HitDistance<0`).
  - `UTilePlacementComponent` (`Components/`, `UActorComponent`) — owns `TileClass`, `RotationInterpSpeed`, ghost (`GhostTileRef`, collision disabled), `MapGridRef`, `WorldRef`, `CurrentRotation`, and the `TMap<FIntPoint, TObjectPtr<ATileActor>> PlacedTiles` occupancy map. Public ops `PlaceTile`/`DeleteTile`/`RotateTile`/`UpdateGhostTile`, all keyed on `FIntPoint Cell`. Place rejects occupied cells; delete is a cell lookup (`Find`→`Destroy`→`Remove`), no raycast; rotate steps 90° (`UnwindDegrees`-wrapped) with the ghost eased via `RInterpTo`; ghost spawned in `BeginPlay`, snapped to the cursor cell each tick.
  - Controller slimmed to input + cursor: handlers and `PlayerTick` delegate to the component; dropped its `WorldRef`/`TileClass`/`CurrentRotation` and the `TileActor.h` include.
- **Architecture decision** — placement orchestration extracted into a controller-owned component, mirroring the dice `UDiceRollComponent` pattern, but timed *before* delete/occupancy were written so they landed in the right home (no write-then-move churn). The seam is cell-based: the controller owns the cursor (deproject is a PC API) and produces an `FIntPoint`; the component owns grid↔world conversion, spawning, and occupancy — chosen over passing a raw world point because the component needs the grid regardless and it avoids a redundant round-trip. Free-place props (raw world point) will be a separate path. Occupancy keyed `FIntPoint` now; migrates to `FIntVector` for vertical stacking via an "active build level" plane.
- Format pass on the 8 map-builder files (constructor-first, doc comments, component Runtime Refs → `private` before Config, `TickComponent` → `protected`, UCLASS doc, `TileActor` constructor-first). `MapGrid.cpp` left unformatted pending removal of its debug-draw scaffolding.
- New editor assets: `PC_MapBuilder`, `GM_MapBuilder`, `A_MapGrid`, `A_GhostTile`, `IMC_Build`, `IA_PlaceTile`/`IA_RotateTile`/`IA_DeleteTile`, `Dev_MapBuilder` level.

---

## 2026-06-18

**Implementation:**
- **Dice system re-architected** — roll orchestration extracted from the `UDiceTray` widget into a new controller-owned `UDiceRollComponent` (`Components/`, `UActorComponent`, `Blueprintable`):
  - Component owns the moved `EDiceRollMode` enum, new `FDiceTypeCount` + `FDiceRollRequest` structs (its input contract), spawn/settle/teardown logic (`RollDice(const FDiceRollRequest&)`, `OnDiceRolledHandler`, `OnDiceFailsafeHandler`, `DestroyDice`, `FinalizeRoll`, `GetRandomizedVector`), the impulse/spawn config, `ActiveRollMode` state, and the `OnRollComplete` / `OnDiceFailsafeDestroyed` / `OnRollInitiated` delegates.
  - `UDiceTray` shrank to pure presentation. `OnRollClicked` builds a `FDiceRollRequest` from the selectors (one `FDiceTypeCount` per selected type) and calls `DiceRollComponentRef->RollDice()`; `NativeConstruct` acquires the component via `GetOwningPlayer()` → `Cast<ASessionController>` → `DiceRollComponent` and binds `OnRollComplete` → `OnRollCompleteHandler`. Retains `bRollInProgress` + `ActiveRollMode` and all button-state logic as presentation.
  - `ASessionController` creates the `UDiceRollComponent` subobject. `USessionChatComponent` repointed from the tray's delegates to the component's (`OnAllDiceRolled`→`OnRollComplete`) — chat no longer reaches into the tray widget. `USessionUIComponent` assigns `SpawnVolume` to the component instead of the tray.
- **Two pre-existing `UDiceTray` bugs fixed** before extraction: extracted a shared `FinalizeRoll()` (the broadcast-teardown block was duplicated across the rolled/failsafe handlers); fixed a failsafe hang where an all-failed or last-die-failed roll never finalized (nested the completion check — outer "roll resolved" always runs `FinalizeRoll`, inner "results exist" gates only the broadcast).
- Bug fix (editor-side): camera input regression — `IA_CameraMove` Negate/Swizzle modifiers had blanked out during the camera refactor, collapsing all WASD movement to +X. Re-added in the editor.
- Architecture review of the session/component layer recorded (mostly small safe cleanups queued — dead `DiceTrayRef`/`GetDiceTray()`, panel-array redundancy).

---

## 2026-06-15

**Implementation:**
- **Map builder foundation started** (`Source/ProjectIronTable/MapBuilder/`):
  - `AMapGrid` (`AActor`) — scene-root grid actor. `GridToWorld(FIntPoint)` returns a tile's world-space center; `WorldToGrid(FVector)` is its verified inverse (inverse-transform → divide by `TileSize` → `FloorToInt`; holds for non-square grids and any actor transform). Config: `TileSize` (100), `GridDimensions`. `BeginPlay` draws a debug grid (lines + per-tile center spheres, persistent, drawn once) — temporary scaffolding to be replaced by a rendered grid.
  - `ATileActor` (`AActor`) — static-mesh tile; mesh root forced `Movable`; GC-safe `UPROPERTY` on `TileMesh`; ticking disabled. Mesh to be assigned via a `BP_Tile` subclass.
- **Camera controller base-class extraction** — new `ABaseCameraController` (`PlayerControllers/`) owns all camera state, config (9 properties), input (`IMC_Camera` + move/pan/zoom/sprint/pan-reset), `OnPossess` camera binding, `BeginPlay` input-mode + settings load, and `Validate/Apply/SaveCameraSettings`. `CameraPawnRef`/`InputSubsystemRef` are `protected` for subclass reuse.
- **`ASessionController` reparented** to `ABaseCameraController` — reduced to session-only surface (`UIComponent`, `ChatComponent`, `IMC_Session`, `IA_FocusChat`, `IMC_Chat`, `IA_ExitChat`, chat handlers, `Server_TravelToSession`). Removed the duplicate `InputSubsystemRef` (now inherited); `OnPossess`/`BeginPlay` call `Super` then do session-only work; dead includes/forward-declares removed.
- **`AMapBuilderController`** (`PlayerControllers/`) — skeleton (Part A): subclasses `ABaseCameraController`, adds `IMC_Build` + `IA_PlaceTile` in `OnPossess`, caches `AMapGrid` in `BeginPlay`. `GetGridCellUnderCursor`, `Input_PlaceTile`, ghost-follow `PlayerTick` are stubs pending Part B.
- **`ASessionPawn` → `ACameraPawn`** — renamed (no session-specific code); `[CoreRedirects]` entry added to `Config/DefaultEngine.ini` so Blueprint/level/GameMode references resolve.
- Bug fix: `ABaseCameraController::PostEditChangeProperty` definition now wrapped in `#if WITH_EDITOR` (was guarded only in the header — would break a packaged build).

**Design:**
- Coding standard: a class's constructor now goes **first**, above all field regions — the one exception to "fields before functions" (updated in `CLAUDE.md`, `feedback_coding_standards.md`, and `ProjectIronTable_CodingStandards.md`).
- Map builder reuses the existing free-fly camera (via the shared base) rather than a fixed top-down view — required for inspecting 3D/multi-floor maps from any angle.
- Tiles spawn from a Blueprint subclass (`TSubclassOf<ATileActor>`), not a hardcoded C++ mesh; `UTileData` deferred until multiple tile types exist.

---

## 2026-05-31

**Implementation:**
- `SRichTextArea::GetLineHeightForVisualLine(int32 VisualLineIndex, float InScale) const` — new private helper. Walks `Document->Runs` with `RunStart`/`RunEnd` overlap check; returns max `GetMaxCharacterHeight` across overlapping runs; falls back to default 12pt if no runs overlap.
- `SRichTextArea::RebuildVisualLines` — removed `InFontInfo` parameter; every character measurement now uses `FindFontAtIndex(*Document, i)` so wrap points reflect each character's actual font size rather than the document-wide max.
- `SRichTextArea::ComputeDesiredSize` — now sums `GetLineHeightForVisualLine` across all visual lines instead of `VisualLines.Num() * uniform LineHeight`.
- `SRichTextArea::OnPaint` — rendering loop converted to index-based; `CurrentLineHeight = GetLineHeightForVisualLine(LineIndex, Scale)` per line; `RunTabSpace` computed per run after typeface swap; `YOffset += CurrentLineHeight`; underline and strikethrough Y use `CurrentLineHeight`; cursor height uses `GetLineHeightForVisualLine` for the cursor's specific visual line.
- `SRichTextArea::GetCursorPosition` — `TabSpace` parameter removed (each run now computes its own tab space internally); inner segment loop replaced with full run-clip walk so each run's text is measured with its own `FSlateFontInfo` — fixes cursor X being wrong when the cursor is in a large-font run but the line starts with small-font text.
- `SRichTextArea::DrawHighlight` — removed `InFontInfo` and `LineHeight` parameters; start/end line indices now found via Y-accumulation walk (`StartPos.Y < AccumY + H`); `TopY` precomputed as sum of heights before `StartLineIndex`; non-terminal line widths measured via run-clip walk with per-run fonts.
- `SRichTextArea::HitTest` — target line now found via Y-accumulation walk instead of `LocalMousePos.Y / LineHeight` division.
- Bug fix: Y-walk condition was `AccumY >= Y` (fires one iteration too late — off-by-one). Corrected to `Y < AccumY + H` across `DrawHighlight` and `HitTest`.
- Bug fix: `GetCursorPosition` used `FindFontAtIndex` at segment start for the entire line, returning wrong X when cursor was in a later large-font run. Fixed by the run-clip walk above.
- Dead code removed: all `#if 0` blocks (`OnPaint` old tab-segment loop, old range-based render loop, old `GetCursorPosition` loop); unused `LineHeight` variable in `OnPaint`.

---

## 2026-05-18

**Implementation:**
- `SRichTextEditor::SetFontStyle(UFont*)` and `SetFontSize(int32)` added; `SetFontStyle` constructs a new `FSlateFontInfo` preserving current size/typeface, then calls `FormatToSelection` to apply to any active selection; `SetFontSize` updates `ActiveFormat.FontInfo.Size` and calls `FormatToSelection`
- `UEditableRichText` gains `SetFontStyle` and `SetFontSize` passthrough methods; include order fixed (`EditableRichText.h` first, then `Engine/Font.h`)
- `USessionNotesChannel` gains font toolbar: `FontStyleDropdown (UComboBoxString)`, `FontSizeText (UTextBlock)`, `FontSizeUpButton`/`FontSizeDownButton (UButton)` BindWidgets; `AvailableFonts (TArray<TObjectPtr<UFont>>)` config field; `OnFontStyleChanged`, `OnFontSizeUpClicked`, `OnFontSizeDownClicked` event handlers; `NativeConstruct` populates dropdown, initializes size display, binds all three
- Three composite UFont assets imported (`Content/Fonts/`): JetBrains Mono, Source Code Pro, DM Mono — each with Regular/Bold/Italic/Bold Italic typefaces
- Bug 2.10 fixed: `SRichTextArea::OnPaint` inner run-clip walk now updates `RunStart = RunEnd` before `continue` — previously, subsequent runs computed their end offset from the wrong base and never rendered
- Bug 2.11 fixed: Enter and Tab in `SRichTextEditor::OnKeyDown` now `return FReply::Handled()` early, bypassing the `bHandled` block that called `SyncActiveFormat()` and silently re-enabled toggled-off format flags
- `SRichTextArea::ComputeDesiredSize` now falls back to `Document->GetLines().Num()` before `VisualLines` is populated (cold start); previously returned incorrect height until first paint

---

## 2026-05-12

**Implementation:**
- `SRichTextArea::OnPaint` rendering loop replaced: outer loop now iterates `VisualLines`; inner run-clip walk accumulates `RunStart`/`RunEnd`, skips non-overlapping runs, clips overlapping runs with `Run.Text.Mid(ClipStart, ClipEnd - ClipStart)`, applies typeface swap, splits clip on `\t`, draws each tab segment via `DrawTextSegment`
- `GetCursorPosition` converted from `static` to `const` member; now walks `VisualLines` instead of `Document->GetLines()`; `CursorY` accumulates per visual line; tab segment walk uses `CachedText.Mid` and `VisualLine.StartIndex`; uses `<= EndIndex` (not `<`) so cursor-at-end-of-line stays on that line
- `HitTest` converted from `static` to `const` member; `TargetLine` now indexes into `VisualLines`; character walk uses `CachedText` with document indices from `StartIndex` to `EndIndex`; return value is `VisualLines[TargetLine].StartIndex + CharInLine`
- `OnUpOrDownPressed` updated: `Lines` parameter removed; uses `TextAreaRef->GetVisualLines()` and `TextAreaRef->GetCachedText()` directly; bounds check and character walk both use visual line data; result is `VisualLines[TargetLine].StartIndex + CharInLine`
- `GetVisualLines()` and `GetCachedText()` public const getters added to `SRichTextArea` for use by `SRichTextEditor`
- Unused `TArray<FString> Lines` variable removed from `SRichTextEditor::OnKeyDown`

---

## 2026-05-11

**Design:**
- Measurement Tools section filled in: tool set is D&D 5e-based for now (ruler, cone, sphere, line, cube AoE templates, grid-range highlighter candidate); two placement modes (grid-snap and free-float); Z-axis measurement deferred as open question
- Scheduling scope finalized: minimum only — meeting days, frequency, and session length as a public browser filter; no calendar integration, reminders, or RSVP planned
- Campaign Manager layout decided: left sidebar lists game systems the player is part of; main area shows a 3-per-row scrollable grid of campaigns for the selected system
- Open questions #5 (Scheduling) and #6 (Campaign Manager layout) moved to Resolved

---

## 2026-05-10

**Implementation:**
- `FVisualLine` struct added in `RichTextArea.h` — holds `StartIndex` and `EndIndex` (exclusive) as character indices into the full document text
- `SRichTextArea` gains three `mutable` private fields: `CachedWidth (float)`, `CachedText (FString)`, `VisualLines (TArray<FVisualLine>)`
- `RebuildVisualLines(const FSlateFontInfo&, float Scale, float TabSpace) const` added — walks `CachedText` char by char; `\n` hard-breaks, `\t` breaks on overflow (tab starts next line), `' '` breaks on overflow (space starts next line), default chars break on overflow using `LastSpaceIndex` backtracking (no prior space: whole word to next line; prior space: break at last space)
- `OnPaint` checks `CachedWidth < 0 || width changed || text changed` at top — updates cache and calls `RebuildVisualLines` on any mismatch
- `VisualLines` is built on each invalidation; rendering loop not yet updated to consume it

---

## 2026-05-09

**Implementation:**
- Toolbar fix: B/I/U/S checkboxes moved from `SRichTextEditor` to `USessionNotesChannel` as `BindWidget` UCheckBoxes; `SVerticalBox` wrapper removed from `SRichTextEditor`; `ChildSlot` now holds `SRichTextArea` directly
- `SRichTextEditor` broadcasts `OnFormatStateChanged (bool×4)` instead of calling `SetIsChecked` on internal checkboxes; `UEditableRichText` exposes `GetOnFormatStateChanged()`
- `USessionNotesChannel` gains `bIsSyncing` guard, four `OnXxxCheckStateChanged` handlers, and `OnFormatChanged` receiver; Blueprint `W_SessionNotesChannel` updated with four `UCheckBox` widgets above the scroll box
- Height fix: removing `SVerticalBox` resolved the `FillHeight(1.0f)` collapse — `UEditableRichText` now fills available height correctly
- Enter scroll fix: `OnKeyDown` tracks `bDocumentModified`; cursor-only moves no longer broadcast `OnDocumentChanged`
- Bug 2.6 fix: `OnUpOrDownPressed` now takes `float TabSpace`; character walk uses `TabSpace` for `\t` and `MeasureText` for others; midpoint is `LeftEdge + CharWidth / 2.f`

---

## 2026-05-08

**Implementation:**
- Notes save/load: `USessionUIComponent::Init` loads `USessionNotesSave`; creates default private tab on first join or calls `RestoreChannels` to rebuild saved tabs with stored content
- `UFunctionLibrary::LoadSessionNotesSave` and `GetNotesSaveSlotName` added
- Scroll chain removed: `Scroll(bool)` and `ScrollMultiplier` dropped from `UBaseChannel`, `UBaseChannelPanel`, `USessionChatComponent`; `IA_ScrollChat` removed from `ASessionController` and `IMC_Chat` — `UScrollBox` handles mouse-wheel natively
- `SRichTextArea::ComputeDesiredSize` fixed to return `LineHeight * LineCount` height (was zero, collapsing inside `UScrollBox`)
- `ChatBox.cpp` save bug fixed: `SaveGameToSlot` moved inside the `if (!Find)` guard
- `UMainScreenUIComponent` field layout fixed

---

## 2026-05-07

**Implementation:**
- `FNoteRecord` (USTRUCT) and `USessionNotesSave` (USaveGame, slot `"Notes_{PlayerID}_{SessionID}"`) added in `SaveLoad/SessionNotesSave.h/.cpp`
- `USessionNotesSave::SessionNoteRecords (TArray<FNoteRecord>)` — array order is tab order; `NoteTabOrder` dropped as redundant
- `UFunctionLibrary::GetNotesSaveSlotName(const FGuid& PlayerID, const FGuid& SessionID) → FString` implemented
- `USessionNotesComponent` added in `Components/` with `SharedNoteCache (TMap<FGuid, FNoteRecord>)` (in-memory relay, no UPROPERTY) and RPC stubs: `Server_PushNote`, `Multicast_ReceiveNote`, `Server_RequestNoteSync` — all bodies log "not yet implemented"
- `UMainScreenUIComponent.h` field layout fixed — Config region moved above function regions

**Design:** `NoteTabOrder` dropped — `SessionNoteRecords` array order is sufficient for stable tab ordering, consistent with how chat tabs work. Draggable tab reordering added to GDD Out of Scope as an explicitly deferred extra-credit feature.

---

## 2026-05-05

**Design:**
- Notes save/sync architecture settled: private notes (Word doc model) are creator-only, saved locally in `USessionNotesSave` (slot `"Notes_{PlayerID}_{SessionID}"`), no server involvement
- Shared notes (Google Docs model): all editors are peers; creator identity stored in `CreatorPlayerID` for a future "remove editor" feature only — creator holds no special runtime authority
- Conflict resolution: `LastEdited` timestamp wins
- In-session relay: `USessionNotesComponent` (planned, attached to `ASessionController`) maintains an in-memory cache; any editor pushes updates to the relay; relay multicasts to other online editors; cache lost if host drops but all online editors retain up-to-date local copies
- Real-time sync deferred; RPC stubs (`Server_PushNote`, `Multicast_ReceiveNote`, `Server_RequestNoteSync`) built as placeholders
- Default tab changed from one shared tab to one private tab per player on session join
- `FNoteRecord` struct planned: `NoteID`, `DisplayName`, `Content (FRichTextDocument)`, `LastEdited`, `CreatorPlayerID`, `EditorPlayerIDs`
- `NotesTabNames` and `NotesTabContent` marked for removal from `USessionSave`
- `UFunctionLibrary::GetNotesSaveSlotName(PlayerID, SessionID)` planned

---

## 2026-05-02

**Implementation:**
- `SRichTextArea::FindFontAtIndex(const FRichTextDocument&, int32 CharIndex) → FSlateFontInfo` added — walks runs using `CharCount + Run.Text.Len() > CharIndex` boundary (not `>=`, which returns the wrong run)
- `UFunctionLibrary::GetDocumentLineHeight(const FRichTextDocument&, float Scale, FSlateFontInfo* OutFontInfo) → float` added — single shared line-height + best-font computation; pointer (not ref) allows `nullptr` default
- `FRichTextDocument::GetLines() const → TArray<FString>` inline method added — replaces inline `ParseIntoArray` calls throughout
- `SRichTextArea::OnPaint`, `GetCursorPosition`, and `HitTest` updated to use `GetDocumentLineHeight` and `FindFontAtIndex` per segment
- **Critical ordering:** `SegmentOffset` update must happen before `SegCharCount` increment in `GetCursorPosition` — inverting them uses the next segment's font
- `HitTest` midpoint comparison `(LeftEdge + RightEdge) / 2 > X` now consistent with `OnUpOrDownPressed`
- Enter key fix: `RangeDelete()` now runs before `DrawSpecialCharacter('\n')` — was inserting newline before deleting selection
- `SplitRunAt(int32 RunIndex, int32 LocalOffset) → int32` added — copy-remove-insert pattern used by `FormatToSelection` and Ctrl+V paste
- `ApplyFormatShortcut(bool&, SCheckBox*, TFunction<void(FRichTextRun&)>) → FReply` added — toggle/apply/sync pattern shared by Ctrl+B/I/U/S
- Format-all pass run across all source files

---

## 2026-05-01

**Documentation:** Added `RichText/` to the source folder hierarchy in `ProjectIronTable_TechDoc.md` — the folder was missing from the tree, though the class sections were already present.

**Implementation:**
- `PreferredX (float)` field added — captured on first Up/Down press, reused across consecutive vertical moves, reset on all other keys and mouse clicks; fixes column drift (bug 2.9)
- Off-by-one in `OnUpOrDownPressed` fixed — midpoint comparison `(LeftEdge + RightEdge) / 2` instead of right-edge strict comparison
- Drag select added to `OnMouseMove` — anchor set on first character boundary cross; guard prevents anchor set on sub-character-boundary wobble
- `RangeDelete()` helper added — loops `OnBackspaceOrDeletePressed(SelectionMin)` N times; resets anchor; called by backspace, delete, typing, and paste when selection active
- `FormatToSelection(TFunction<void(FRichTextRun&)>)` helper added — splits runs at selection boundaries, applies lambda to covered runs, calls `PruneRuns()`
- Ctrl+A, Ctrl+C/X/V added to `OnKeyDown`; `OnKeyChar` guards `RangeDelete()` before insert when selection active
- Ctrl+B/I/U/S early-return pattern added — bypasses `SyncActiveFormat`; calls `SetIsChecked` explicitly inside `bIsSyncing` guard
- `CopiedRuns (TArray<FRichTextRun>)` field added

---

## 2026-04-30

**Implementation (session 2):** Dead code block removed from `GetSelectedRange`. Full changelogs removed from TechDoc and GDD — both docs now hold only a single `*Last updated*` one-liner; full history in `ProjectIronTable_Changelog.md`. `/update-docs` command updated to include changelog step and corrected memory path.

**Implementation (session 1):**
- `GetAreaCursorPosition` helper added — uses `TextAreaRef->GetTickSpaceGeometry()` for mouse position conversion, avoiding toolbar-height Y offset
- `GetSelectedRange()` added — walks runs from `SelectionMin` to `SelectionMax`, clips each run to selection boundaries, returns `TArray<FRichTextRun>`
- `HandleSelectionOnMove(bShiftDown)` added — captures anchor before cursor moves on Shift+Arrow; clears anchor on bare arrow key
- `SRichTextArea` gains `SelectionAnchor (const int32*)` via `SLATE_ARGUMENT`; `DrawHighlight` added for per-line selection rects in `OnPaint`
- `OnMouseButtonDown` updated: bare click resets `SelectionAnchor = -1`

---

## 2026-04-29

**Implementation:**
- `SelectionStart`/`SelectionEnd` replaced by `SelectionAnchor` (fixed-anchor selection model); `GetSelectionMin/Max()` inline helpers added in `.h`
- `PruneRuns()` extended to merge adjacent same-format runs after removing empties
- `SRichTextArea::HitTest()` static method added — converts local mouse position to a character index
- `OnMouseButtonDown` wired to set anchor + cursor via `HitTest`; `OnMouseMove` added to extend selection on drag
- New coding standard: trivial single-expression getters may be defined inline in `.h`

---

## 2026-04-27

**Implementation:**
- `SRichTextArea::OnPaint` refactored from line-driven to run-driven; typeface swapped per-run from bold/italic flags (`"Regular"`, `"Bold"`, `"Italic"`, `"BoldItalic"`)
- `DrawLine` helper added for underline and strikethrough; width trimmed via `TrimEnd()` to avoid extending over trailing whitespace
- `PruneRuns()` added — removes empty runs after splits/deletions; re-adds blank default run if document empties
- `bIsSyncing` guard added to prevent `SCheckBox::SetIsChecked` callback from overwriting `ActiveFormat` (bug 2.7)
- Format checkboxes set `.IsFocusable(false)` to prevent Space/Enter being consumed by the checkbox instead of the editor (bug 2.8)

---

## 2026-04-26

**Implementation (session 2):**
- Multi-run document model functional; `FRichTextDocument::GetFullText()` added — concatenates all runs
- `FindRunAtIndex`, `OnBackspaceOrDeletePressed`, and `SyncActiveFormat` private helpers added to `SRichTextEditor`
- `OnKeyChar` is now format-aware: splits run into Left/Middle/Right on `ActiveFormat` mismatch; Ctrl+B/I/U wired
- `OnPaint` and `GetCursorPosition` updated to use `GetFullText()` instead of `Runs[0].Text`
- Known bug 2.6: Up/Down into a line with two consecutive tabs snaps cursor to wrong character

**Implementation (session 1):** Tab key support added. `DrawSpecialCharacter` helper consolidates Enter/Tab insertion. `SRichTextArea` rendering and cursor positioning are now tab-aware via segment-walking inner loop. `MeasureText` and `DrawTextSegment` static helpers added.

---

## 2026-04-25

**Implementation:**
- `SRichTextArea` added — `SLeafWidget` handles multi-line text rendering (split on `\n`) and cursor drawing via `MakeLines`
- `SRichTextEditor` wired: Enter inserts `\n`; Up/Down implemented via `OnUpOrDownPressed` (measures target line char by char)
- `GetCursorPosition` static helper on `SRichTextArea` shared with `SRichTextEditor` for Up/Down navigation
- `CursorPosition` passed as `const int32*` so `SRichTextArea` always reflects live cursor state
- Gotcha: `FSlateFontMeasure::Measure` returns DPI-scaled values — divide by `InAllottedGeometry.Scale` for layout-space coordinates

---

## 2026-04-23

**Implementation (session 2):** `SRichTextEditor::Construct` implemented: toolbar (`SHorizontalBox` with four `SCheckBox` buttons wired via `MakeFormatCheckbox` helper) + `SRichTextArea` text area. Coding standards updated: class layout rule changed to fields-before-functions, private→protected→public within each group; `.cpp` function definitions must mirror `.h` declaration order.

**Implementation (session 1):** `URichTextEditorWidget` renamed to `UEditableRichText`. `SMultiLineEditableText` removed from `SRichTextEditor` — replaced with `SBox` placeholder. `OnKeyChar` implemented with run-walking insertion. `FormatsMatch` private helper added. Gotcha added: `SMultiLineEditableText` placed inside `SCompoundWidget` consumes all keyboard input.

**Design:** `URichTextEditorWidget` renamed to `UEditableRichText`. Layout rule updated in coding standards.

---

## 2026-04-22

**Implementation:**
- `RichText/` folder added with `FRichTextRun`, `FRichTextDocument` (structs), `SRichTextEditor` (Slate widget), `UEditableRichText` (UMG wrapper); `RichText` added to Build.cs
- `USessionSave` gains `NotesTabNames (TMap<FGuid, FString>)`
- Chat log persistence: `USessionSave` gains `FChatMessageRecord`, `FChatLogRecord`, `ChatLog`; `UChatBox` gains `FindOrCreateChannel`; log restored in `BeginPlay`; saved per message in `SendChatMessageOnServer`
- `bPendingRefocus` flag added to `UChatBox` to fix Enter double-fire bug

**Design:** Custom Slate rich-text editor chosen over WebBrowser+Quill — better UE integration, no browser overhead, TTRPG-specific extension potential. Notes formatting scope confirmed: bold, italic, underline, strikethrough, headers, bullet lists.

---

## 2026-04-20

**Implementation:** `UBaseChannelPanel` fully implemented — all method bodies written, template method pattern wired (`CreateTabLabel`, `SaveCreatedTab`, `OnChannelRenamed`, `OnChannelSwitched` virtual hooks). `UChatBox` reparented to `UBaseChannelPanel`: all base-owned members removed, four virtual hooks overridden. `USessionUIComponent` cast fixes. Gotcha: `CreateWidget<T>()` requires full type definition in `.cpp`.

**Design:** Notes channels confirmed to use GUID-based identity (not participant-list) — multiple tabs can share the same participants.

---

## 2026-04-19

**Implementation:**
- Base channel hierarchy added to `Utility/`: `UBaseChannel`, `UBaseChannelTab`, `UBaseChannelListEntry`, `UBaseChannelPanel`
- `UChatChannel` inherits `UBaseChannel`; `UChatTab` and `UChatChannelListEntry` reduced to empty typed subclasses; `USessionNotesTab` added as `UBaseChannelTab` subclass
- Gotchas: `BindWidget` in base class must be `protected`; delegate param types lock handler signatures; deleting a C++ class orphans dependent Blueprints

---

## 2026-04-18

**Implementation (session 2):** `USessionChatComponent` completed — dice-to-chat handlers moved from `#if 0` blocks in `USessionUIComponent` into `USessionChatComponent`. `USessionUIComponent` gains `GetDiceTray()` and `GetPlayerList()` getters. `BeginPlay` stubs removed. **Private Methods** region added to class layout standard.

**Implementation (session 1):**
- Component renames: `USessionHUDComponent` → `USessionUIComponent` + `USessionChatComponent`, `UMainScreenHUDComponent` → `UMainScreenUIComponent`, `UGameTypeButton` → `UGameTypeTab`, `UDiceSelectorManager` → `UDiceTray`
- `ASessionController` updated to create both components
- Gotcha: widget `Init` must go in `BeginPlay`, not `OnPossess` — `OnPossess` is server-only and remote clients never call it

---

## 2026-04-16

**Implementation:** Server RPC infrastructure: `ASessionController::Server_TravelToSession` added; `UCampaignManagerScreen::OnCampaignSelected` routes through RPC. `USessionNotesPanel` added (`SessionNotes/`): scrollable `UMultiLineEditableText` with auto-scroll. Source folder restructure: `Screens/` added; `DiceSelector`/`DiceSelectorManager` moved to `Dice/`; `SettingsSlider` moved to `Settings/`; `ContextMenu`/`ContextMenuButton` moved to `UI/`.

**Design:** Session notes foundation added as plain-text scrollable panel. Rich-text formatting deferred. Design decision: no separate Edit mode button — clicking activates editing, clicking away deactivates.

---

## 2026-04-15

**Implementation:** Code quality pass — `MacroLibrary.h` added (`GET_OUTER`, `GET_OWNING_PC`, `CHECK_IF_VALID`). `UFunctionLibrary` gains `MakeParticipantKey` and `GetEnumDisplayName<T>`. `CHECK_IF_VALID` applied across ~11 files (~33 sites). ~176 lines removed.

---

## 2026-04-14

**Implementation (session 2):** Chat tab rename wired end-to-end. `UChatBox::OnTabRightClickedHandler` spawns `W_ContextMenu`; `OnTabRenamedHandler` persists to `USessionSave::ChatTabNames`. Close button removed from `UChatTab` — channel close now in context menu. `UContextMenu` updated: bounds-check approach in `NativeOnMouseButtonDown`; `SetMenuPosition` added.

**Implementation (session 1):** `UFunctionLibrary` gains `GetSessionSaveSlotName`, `LoadSessionSave`, `GetLocalPlayerName`. `USessionSave` gains `ChatTabNames (TMap<FString, FString>)`. `UChatBox` Config gains `ContextMenuClass`. ~50 lines of inline boilerplate removed.

**Design:** Chat tab rename complete — right-click opens context menu with Rename and Close options. Rename is client-local, persisted to `USessionSave::ChatTabNames`.

---

## 2026-04-13

**Implementation:** Chat tab rename infrastructure. `UChatTab` gains `EditLabel`, `EnterRenameMode()`, `OnTabRightClicked` and `OnTabRenamed` delegates. `UContextMenu` and `UContextMenuButton` added to `Utility/` — generic floating context menu with auto-dismiss. Format-all pass complete across all 90+ source files. 15 recurring code patterns catalogued.

---

## 2026-04-12

**Implementation:** Chat log persistence implemented. `USessionSave` gains `FChatMessageRecord`, `FChatLogRecord`, `ChatLog`. `UChatChannel` gains `RestoreMessage`. `UChatBox` gains `FindOrCreateChannel`.

---

## 2026-04-11

**Implementation:** `USessionInstance` added. `ASessionGameMode` fully implemented: `InitGame`, `PostLogin`, `Logout` logic complete. `ASessionPlayerState` gains `SessionPlayerID` (Replicated FGuid). `ASessionGameState` gains full getter/setter API. `GameInstances/` folder added to source tree and Build.cs.

---

## 2026-04-10

**Implementation:** `USessionSave` implemented. `ASessionGameState` and `ASessionPlayerState` implemented with replication. `ASessionGameMode` stub added. Build.cs updated. Session slot-name model documented: `"Session_{SessionID}"` with `UCampaignManagerSave` as authoritative index.

**Design:** GM role finalized — multiple GMs supported per session, role is transferable, default GM = campaign creator. Session save/load fully designed: sessions stored as `"Session_{SessionID}"` save slots; `UCampaignManagerSave` is the campaign→session index. Server model confirmed as listen server.

---

## 2026-04-09

**Implementation:** `UBaseScreen` added as shared base for all main screen widgets. `UCampaignManagerScreen` and `USettingsScreen` refactored to inherit `UBaseScreen`. `UAssetLibraryScreen` and `UCampaignBrowserScreen` added as stubs. `UHomeScreen` updated: Play button replaced with Campaign Manager button. `UDelegateLibrary` added to `Utility/`. `UGameTypeButton` expanded with `SetTabColors`, `SetSelected`, `GetLabel`.

---

## 2026-04-08

**Design:** Environment system (Time of Day & Weather) fully designed — time of day slider, weather types and intensity, GM panel, replication model, planned C++ classes (`AEnvironmentManager`, `EWeatherType`, `UEnvironmentControlPanel`).

---

## 2026-04-03

**Design:** Map system redesigned as a 3D tile/prop builder. Combat Map and World/Region Map scale modes with location pin linking. Host/Server Owner roles clarified and separated. Custom content section rewritten with auto-distribution model and glTF format. Full session lifecycle designed. Full Fog of War design. Sound and Music added. Character Sheet, Inventory and Loot, Vision system, Measurement Tools all documented.

---

## 2026-04-02

**Implementation:** Settings screen save/load/defaults fully implemented. Panel layout reset button added.

---

## 2026-04-01

**Implementation:** Home screen implemented. Private dice roll initiation implemented (`@Names` in chat input before rolling). Panel layout persistence implemented. Dice collision SFX implemented.

**Design:** Home Screen and Asset Library sections added. Import flow defined. Camera settings system in progress.

---

## 2026-03-31

**Implementation:** Draggable and resizable panels implemented. Close/reopen private chat tabs implemented.

**Design:** Tab renaming confirmed as client-local.

---

## 2026-03-29

**Implementation:** Taskbar minimize system implemented (`UTaskbar`, `UTaskbarButton`).

---

## 2026-03-27

**Implementation:** Private messaging and tabbed chat fully implemented. Player list widget added.

**Design:** Dice roll routing added: rolls go to the active channel, not always broadcast. Private dice roll initiation added as open item.

---

## 2026-03-26

**Design:** Chat section updated with tabbed channel design: Server tab, private tabs with `@P1 +2` labels, `@Name` addressing, auto-switch on send, notification on receive, channels list for closed tabs.

---

## 2026-03-25

**Design:** Initial GDD created.
