# ProjectIronTable — Changelog

> Most recent entries at top. For implementation detail see TechDoc; for design rationale see GDD.

---

## 2026-05-10

**Implementation:** Word wrap layout cache added to `SRichTextArea`. New `FVisualLine` struct (defined in `RichTextArea.h`) holds `StartIndex` and `EndIndex` — character indices into the full document text (not pixel positions); `EndIndex` is exclusive. `SRichTextArea` gains three `mutable` private fields: `CachedWidth (float, -1 until first paint)`, `CachedText (FString)`, and `VisualLines (TArray<FVisualLine>)`. `RebuildVisualLines(const FSlateFontInfo&, float Scale, float TabSpace) const` added — walks `CachedText` char by char: `\n` closes line; `\t` closes line if `LineWidth + TabSpace >= CachedWidth` (tab starts next line), else accumulates; `' '` closes line on overflow (space starts next line), else records `LastSpaceIndex`; default chars close line on overflow — if `LastSpaceIndex == -1` the word starts next line from `i`, if `LastSpaceIndex != -1` breaks at last space and measures new line width via `CachedText.Mid`; final line added after loop. `OnPaint` updated to check `CachedWidth < 0 || CachedWidth != AllottedGeometry.GetLocalSize().X || CachedText != Document->GetFullText()` at top — on any mismatch updates both cached values and calls `RebuildVisualLines`. `VisualLines` is now built on each invalidation but the rendering loop is not yet updated to consume it.

---

## 2026-05-09

**Implementation:** Toolbar fix (#1): Formatting toolbar (B/I/U/S checkboxes) removed from `SRichTextEditor::Construct`; `SVerticalBox` wrapper removed; `ChildSlot` now holds `SRichTextArea` directly. `SRichTextEditor` broadcasts `OnFormatStateChanged (FOnFormatStateChanged, FourParams: bool×4)` instead of calling `SetIsChecked` on internal checkboxes. `UEditableRichText` gained `GetOnFormatStateChanged() → FOnFormatStateChanged&`. `USessionNotesChannel` gained four `BindWidget` UCheckBox fields (`BoldCheckBox`, `ItalicCheckBox`, `UnderlineCheckBox`, `StrikethroughCheckBox`), four UFUNCTION `OnXxxCheckStateChanged(bool)` handlers (each guards with `bIsSyncing`), `OnFormatChanged(bool, bool, bool, bool)` handler (sets `bIsSyncing` around `SetIsChecked` calls), and `bIsSyncing (bool)` state field; `NativeConstruct` binds all delegates. Blueprint `W_SessionNotesChannel` updated with four `UCheckBox` widgets above the scroll box. Height fix (#2): Removing `SVerticalBox` from `SRichTextEditor` resolved the `FillHeight(1.0f)` collapse — `UEditableRichText` now fills the available height in the scroll box slot correctly. Fix #3 (Enter key scroll bug): `OnKeyDown` now tracks a local `bDocumentModified` flag; set to `true` in Enter, Tab, Backspace (normal path), Delete (normal path), Ctrl+V, Ctrl+X paths; selection-delete early-return branches in Backspace/Delete broadcast `OnDocumentChanged` directly before returning; cursor-only moves do not broadcast. Bug 2.6 (consecutive tabs Up/Down snap): `OnUpOrDownPressed` extended with `float TabSpace`; character walk replaced from cumulative-prefix `Measure` to per-character (`\t` → `TabSpace`, others → `MeasureText`); midpoint is `LeftEdge + CharWidth / 2.f`; `OnKeyDown` passes `TabSpace` at both call sites.

---

## 2026-05-08

**Implementation:** Notes system save/load complete. `USessionUIComponent::Init` now loads `USessionNotesSave` via `UFunctionLibrary::LoadSessionNotesSave`; creates a default private tab if no save exists, or calls `RestoreChannels(USessionNotesSave*)` to rebuild all saved tabs with stored content. `UFunctionLibrary::LoadSessionNotesSave` and `GetNotesSaveSlotName` added. Scroll chain removed: `Scroll(bool bUp)` and `ScrollMultiplier` dropped from `UBaseChannel`; `Scroll(bool bUp)` dropped from `UBaseChannelPanel`; `ScrollChat(bool)` dropped from `USessionChatComponent`; `IA_ScrollChat` input action removed from `ASessionController` and `IMC_Chat` — `UScrollBox` handles mouse-wheel scroll natively. `SRichTextArea::ComputeDesiredSize` fixed to return `LineHeight * LineCount` height (was returning zero, causing the text area to collapse inside a `UScrollBox`). `ChatBox.cpp` save bug fixed: `SaveGameToSlot` was outside the `if (!Find)` guard; moved inside. `UMainScreenUIComponent` field layout fixed.

---

## 2026-05-07

**Implementation:** Notes system — steps 1–4 complete. `FNoteRecord` (USTRUCT) and `USessionNotesSave` (USaveGame, slot `"Notes_{PlayerID}_{SessionID}"`) implemented in `SaveLoad/SessionNotesSave.h/.cpp`. `USessionNotesSave::SessionNoteRecords (TArray<FNoteRecord>)` — array position determines tab order; `NoteTabOrder` dropped as redundant. `UFunctionLibrary::GetNotesSaveSlotName(const FGuid& PlayerID, const FGuid& SessionID) → FString` implemented. `USessionNotesComponent` implemented in `Components/` — holds `TMap<FGuid, FNoteRecord> SharedNoteCache` (in-memory relay, no UPROPERTY); RPC stubs: `Server_PushNote(FNoteRecord)` (Server, Reliable), `Multicast_ReceiveNote(FNoteRecord)` (NetMulticast, Reliable), `Server_RequestNoteSync()` (Server, Reliable); all bodies log "not yet implemented". `UMainScreenUIComponent.h` field layout fixed — Config region was below function regions; moved above Event Handlers and Private Methods.

**Design:** `NoteTabOrder` dropped — `SessionNoteRecords` array order is sufficient for stable tab ordering, consistent with how chat tabs work. Draggable tab reordering added to GDD Out of Scope as an explicitly deferred extra-credit feature.

---

## 2026-05-05

**Design:** Notes save/sync architecture settled. Private notes (Word doc model): creator-only, saved locally in `USessionNotesSave` (slot `"Notes_{PlayerID}_{SessionID}"`), no server involvement. Shared notes (Google Docs model): all editors are peers — creator has no special runtime authority; creator identity stored as `CreatorPlayerID` for a future "remove editor" feature only. Conflict resolution: `LastEdited` timestamp wins. In-session relay: `USessionNotesComponent` (planned, attached to `ASessionController`) maintains an in-memory cache of shared notes during a session; any editor pushes updates to the relay; relay multicasts to other online editors; cache lost when host drops but all online editors retain up-to-date local copies. Real-time sync deferred; RPC stubs (`Server_PushNote`, `Multicast_ReceiveNote`, `Server_RequestNoteSync`) built now. Default tab changes from one shared tab to one private tab per player on session join. `FNoteRecord` struct planned: `NoteID`, `DisplayName`, `Content (FRichTextDocument)`, `LastEdited`, `CreatorPlayerID`, `EditorPlayerIDs`. `NotesTabNames` and `NotesTabContent` will be removed from `USessionSave`. `UFunctionLibrary::GetNotesSaveSlotName(PlayerID, SessionID)` planned.

---

## 2026-05-02

**Implementation:** Per-run font throughout the RichText system. `FindFontAtIndex(const FRichTextDocument&, int32 CharIndex) → FSlateFontInfo` added as a static helper on `SRichTextArea` — walks runs using `CharCount + Run.Text.Len() > CharIndex` boundary check (not `>=`, which returns the wrong run). `UFunctionLibrary::GetDocumentLineHeight(const FRichTextDocument&, float Scale, FSlateFontInfo* OutFontInfo = nullptr) → float` added — single shared implementation for line-height + best-font computation; uses `FSlateFontInfo*` (not reference) so `nullptr` default is valid. `FRichTextDocument::GetLines() const → TArray<FString>` inline method added — replaces inline `ParseIntoArray` calls throughout the system. `SRichTextArea::OnPaint`, `GetCursorPosition`, and `HitTest` updated to use `GetDocumentLineHeight` and `FindFontAtIndex` per segment. Critical ordering in `GetCursorPosition`: `SegmentOffset` update (calls `MeasureText` with `FindFontAtIndex`) must happen before `SegCharCount` increment — inverting them causes the wrong run's font. `HitTest` now uses `LineStartDocIndex` accumulated before the character walk; midpoint landing `(LeftEdge + RightEdge) / 2 > X` consistent with `OnUpOrDownPressed`. `SRichTextEditor::OnKeyDown` and `OnUpOrDownPressed` updated to use `GetDocumentLineHeight`. Enter key ordering bug fixed: `RangeDelete()` now runs before `DrawSpecialCharacter('\n')` — was inserting newline before deleting selection. `SplitRunAt(int32 RunIndex, int32 LocalOffset) → int32` added — copy-remove-insert-return-right-index helper; used by `FormatToSelection` and Ctrl+V paste. `ApplyFormatShortcut(bool& Flag, TSharedPtr<SCheckBox>& Checkbox, TFunction<void(FRichTextRun&)> Apply) → FReply` added — toggle/apply/sync/return pattern shared by all four Ctrl+B/I/U/S blocks. Full format-all pass run across all source files.

---

## 2026-05-01

**Documentation:** Added `RichText/` to the source folder hierarchy in `ProjectIronTable_TechDoc.md` — the folder was missing from the tree, though the class sections were already present.

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
