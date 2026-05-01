# ProjectIronTable — Changelog

> Most recent entries at top. For implementation detail see TechDoc; for design rationale see GDD.

---

## 2026-05-01

**Implementation:** `SRichTextEditor` selection system complete. `PreferredX (float)` field added — captured on first Up/Down press, reused across consecutive vertical moves, reset on all other keys and mouse clicks; fixes column drift (bug 2.9). Off-by-one in `OnUpOrDownPressed` fixed via midpoint character comparison (LeftEdge + RightEdge) / 2 instead of right-edge strict comparison. Drag select implemented in `OnMouseMove` — anchor set on first character boundary cross while left button held; guard prevents anchor from being set on sub-character-boundary wobble. `RangeDelete()` helper added — loops `OnBackspaceOrDeletePressed(SelectionMin)` N times, resets anchor; called by backspace, delete, typing, and paste when a selection is active. `FormatToSelection(TFunction<void(FRichTextRun&)>)` helper added — splits runs at `SelectionMin` and `SelectionMax`, applies lambda to all covered runs, calls `PruneRuns()`; called by all four `Toggle*` methods and Ctrl+B/I/U/S in `OnKeyDown`. Ctrl+A (select all), Ctrl+C/X (fill `CopiedRuns` via `GetSelectedRange`), Ctrl+V (insert `CopiedRuns` at cursor, guarded `RangeDelete` if selection active) added to `OnKeyDown`. `OnKeyChar` guards `RangeDelete()` before insert when selection active. Ctrl+B/I/U/S early-return pattern added — bypasses `SyncActiveFormat` to prevent checkbox state being overwritten by the document run, then explicitly calls `SetIsChecked` inside `bIsSyncing` guard. `CopiedRuns (TArray<FRichTextRun>)` field added. `bVerticalMove` flag tracks whether the current key is a vertical move; `PreferredX` reset only when false.

---

## 2026-04-30

**Implementation (session 2):** Dead code block removed from `GetSelectedRange`. Full changelogs removed from TechDoc and GDD — both docs now hold only a single `*Last updated*` one-liner; full history in `ProjectIronTable_Changelog.md`. `/update-docs` command updated to include changelog step and corrected memory path.

**Implementation (session 1):** Selection system progress. `GetAreaCursorPosition` helper fixes toolbar-height Y offset in mouse hit-testing — uses `TextAreaRef->GetTickSpaceGeometry()` for both position conversion and scale. `GetSelectedRange()` added to `SRichTextEditor` — walks runs from `SelectionMin` to `SelectionMax`, clips each run's text to selection boundaries, returns `TArray<FRichTextRun>`. `HandleSelectionOnMove(bShiftDown)` added — captures anchor before cursor moves on Shift+Arrow, clears anchor on bare arrow key. Shift+Arrow keyboard selection wired in `OnKeyDown`. `SRichTextArea` gains `SelectionAnchor (const int32*)` pointer via `SLATE_ARGUMENT`. `DrawHighlight` added to `SRichTextArea` — draws per-line selection rects in `OnPaint` when selection is active. `OnMouseButtonDown` updated: bare click resets `SelectionAnchor = -1`, only updates `CursorPosition`.

---

## 2026-04-29

**Implementation:** `SelectionStart`/`SelectionEnd` replaced by `SelectionAnchor` (fixed-anchor selection model). `GetSelectionMin/Max()` inline helpers added in `.h`. `PruneRuns()` extended to merge adjacent same-format runs after removing empties. `SRichTextArea::HitTest()` static method added — converts local mouse position to character index. `OnMouseButtonDown` wired to set anchor+cursor via `HitTest`; `OnMouseMove` added to extend selection on drag. New coding standard: trivial single-expression getters may be defined inline in `.h`. Known issue added: cursor Y off-by-one due to toolbar geometry offset.

---

## 2026-04-27

**Implementation:** `SRichTextArea::OnPaint` refactored from line-driven to run-driven. Typeface name swapping for bold/italic (`"Regular"`, `"Bold"`, `"Italic"`, `"BoldItalic"`). `DrawLine` helper added for underline and strikethrough decorations; width trimmed via `TrimEnd()` to avoid extending over trailing whitespace. `PruneRuns()` added to `SRichTextEditor` — removes empty runs after splits/deletions, re-adds blank default run if document empties. `bIsSyncing` guard added to prevent `SCheckBox::SetIsChecked` feedback loop from overwriting `ActiveFormat`. Format checkboxes set `.IsFocusable(false)` to prevent Space/Enter being consumed. Bugs 2.7 and 2.8 resolved.

---

## 2026-04-26

**Implementation (session 2):** Multi-run document model fully functional. `FRichTextDocument::GetFullText()` added — concatenates all runs. `SRichTextEditor` gains `FindRunAtIndex`, `OnBackspaceOrDeletePressed`, and `SyncActiveFormat` private helpers. `OnKeyChar` is now format-aware: splits runs into Left/Middle/Right on `ActiveFormat` mismatch. Ctrl+B/I/U shortcuts wired. All cursor-moving operations call `SyncActiveFormat`. `OnPaint` and `GetCursorPosition` updated to use `GetFullText()` instead of `Runs[0].Text`. Known bug 2.6: Up/Down into a line with two consecutive tabs snaps to nearest visible character.

**Implementation (session 1):** Tab key support added. `DrawSpecialCharacter` helper consolidates Enter/Tab insertion. `SRichTextArea` rendering and cursor positioning are now tab-aware via segment-walking inner loop. `MeasureText` and `DrawTextSegment` static helpers added.

---

## 2026-04-25

**Implementation:** `SRichTextArea` added — `SLeafWidget` handles all text rendering (multi-line via `\n` split) and cursor drawing (`MakeLines`). `SRichTextEditor` fully wired: Enter inserts `\n`; Up/Down implemented via `OnUpOrDownPressed` (measures target line character by character). `GetCursorPosition` static helper on `SRichTextArea` shared with `SRichTextEditor` for Up/Down navigation. `CursorPosition` passed as `const int32*` pointer so `SRichTextArea` always reflects live state. Gotcha: `FSlateFontMeasure::Measure` returns DPI-scaled values — divide by `InAllottedGeometry.Scale` to get layout-space coordinates.

---

## 2026-04-23

**Implementation (session 2):** `SRichTextEditor::Construct` implemented: toolbar (`SHorizontalBox` with four `SCheckBox` buttons wired via `MakeFormatCheckbox` helper) + `SRichTextArea` text area. Coding standards updated: class layout rule changed to fields-before-functions, private→protected→public within each group; `.cpp` function definitions must mirror `.h` declaration order.

**Implementation (session 1):** `URichTextEditorWidget` renamed to `UEditableRichText`. `SMultiLineEditableText` removed from `SRichTextEditor` — replaced with `SBox` placeholder. `OnKeyChar` implemented with run-walking insertion. `FormatsMatch` private helper added. Gotcha added: `SMultiLineEditableText` placed inside `SCompoundWidget` consumes all keyboard input.

**Design:** `URichTextEditorWidget` renamed to `UEditableRichText`. Layout rule updated in coding standards.

---

## 2026-04-22

**Implementation:** Rich-text editor foundation built. `RichText/` folder added with `FRichTextRun`, `FRichTextDocument` (structs), `SRichTextEditor` (Slate widget), `UEditableRichText` (UMG wrapper). `USessionSave` gains `NotesTabNames (TMap<FGuid, FString>)`. `RichText` added to Build.cs `PublicIncludePaths`. Chat log persistence implemented: `USessionSave` gains `FChatMessageRecord`, `FChatLogRecord`, `ChatLog (TMap<FString, FChatLogRecord>)`; `UChatBox` gains `FindOrCreateChannel`; `USessionHUDComponent::BeginPlay` restores log on load; `SendChatMessageOnServer` saves each message. `bPendingRefocus` flag added to `UChatBox` to fix Enter double-fire bug.

**Design:** Custom Slate rich-text editor chosen over WebBrowser+Quill — better UE integration, no browser overhead, TTRPG-specific extension potential. Notes formatting scope confirmed: bold, italic, underline, strikethrough, headers, bullet lists.

---

## 2026-04-20

**Implementation:** `UBaseChannelPanel` fully implemented — all method bodies written, template method pattern wired (`CreateTabLabel`, `SaveCreatedTab`, `OnChannelRenamed`, `OnChannelSwitched` virtual hooks). `UChatBox` reparented to `UBaseChannelPanel`: all base-owned members removed, four virtual hooks overridden. `USessionUIComponent` cast fixes. Gotcha: `CreateWidget<T>()` requires full type definition in `.cpp`.

**Design:** Notes channels confirmed to use GUID-based identity (not participant-list) — multiple tabs can share the same participants.

---

## 2026-04-19

**Implementation:** Base channel class hierarchy added to `Utility/`: `UBaseChannel`, `UBaseChannelTab`, `UBaseChannelListEntry`, `UBaseChannelPanel`. `UChatChannel` inherits `UBaseChannel`. `UChatTab` and `UChatChannelListEntry` reduced to empty typed subclasses. `USessionNotesTab` added as `UBaseChannelTab` subclass. Gotchas: `BindWidget` in base must be `protected`; delegate param types lock handler signatures; deleting a C++ class orphans Blueprints.

---

## 2026-04-18

**Implementation (session 2):** `USessionChatComponent` completed — dice-to-chat handlers moved from `#if 0` blocks in `USessionUIComponent` into `USessionChatComponent`. `USessionUIComponent` gains `GetDiceTray()` and `GetPlayerList()` getters. `BeginPlay` stubs removed. **Private Methods** region added to class layout standard.

**Implementation (session 1):** Component rename/split: `USessionHUDComponent` → `USessionUIComponent` + `USessionChatComponent`. `UMainScreenHUDComponent` → `UMainScreenUIComponent`. `UGameTypeButton` → `UGameTypeTab`. `UDiceSelectorManager` → `UDiceTray`. `ASessionController` creates both components. Gotcha: widget `Init` must go in `BeginPlay`, not `OnPossess` — `OnPossess` is server-only.

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
