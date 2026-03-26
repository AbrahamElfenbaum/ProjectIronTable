# ProjectIronTable — Game Design Document

**Version:** 0.1 (Draft)
**Engine:** Unreal Engine 5.7
**Primary Game System:** D&D 5e / 2024 (expandable)
**Status:** Active Development

---

## Overview

ProjectIronTable is a virtual tabletop (VTT) simulator built in Unreal Engine. It lets a group of players run a tabletop RPG session online — rolling dice, moving miniatures on a shared map, tracking initiative and character stats, and communicating through an in-session chat. The GM runs the session and controls the environment; players control their own characters.

**Design goals:**
- Feel like a real tabletop, not a video game — physical dice rolls, a spatial map, visible miniatures
- Keep the interface clean and out of the way during play
- Support custom user-imported content (maps, minis, sounds) so groups aren't locked to built-in assets
- Start with D&D 5e and build the system layer to support other TTRPGs later

**Target audience:** TTRPG groups (typically 2–6 players) who want to play online without losing the tactile feel of a physical table.

**Platform:** PC (Windows). Other platforms TBD.

---

## Core Loop

A typical session:

1. GM creates or loads a session and selects a map
2. Players join the session
3. GM places miniatures and sets up the scene
4. Players and GM interact — moving minis, rolling dice, chatting, tracking turns
5. Results (dice rolls, chat, initiative events) are visible to all players in real time
6. Session ends; state can be saved for next time

Between sessions, players may update their character sheets. The GM may prep maps, encounters, and assets.

---

## Player Roles

### Game Master (GM)

- Hosts the session (listen server or dedicated — TBD)
- Full control over the map: place, move, and remove tiles, minis, and objects
- Controls NPC/monster miniatures
- Can hide or reveal sections of the map (fog of war)
- Has access to GM-only information (monster HP, hidden notes)
- Can override or adjudicate dice rolls
- Manages session state (start, pause, save, end)
- Can grant or revoke specific permissions to/from individual players (for gameplay or administrative reasons)

**GM Camera:** The GM has a separate God-view camera with all the functions of the player camera, plus the ability to see the full map including areas behind fog of war. However, the GM's vision is still subject to vision-blocking effects — if a Sphere of Darkness is placed on the map, the GM cannot see inside it unless one of their controlled NPCs has a way to perceive through it. Vision is entity-based, not role-based: what you see is determined by what your controlled characters can perceive.

> **Design Note (future):** Both players and the GM need a way to track characters they control when those characters are inside a vision-blocking effect. A likely solution is rendering an outline of controlled characters that remains visible through occluding effects. This needs to be addressed when the vision system is built.

### Player

- Joins an existing session
- Controls their own character's miniature
- Rolls dice for their character's actions
- Views the map from their character's perspective (fog of war and vision effects apply)
- Participates in chat
- May be granted additional permissions by the GM

**Permissions:** The GM can grant or revoke specific permissions to any player at any time. Permissions are enforced in code — players are technically blocked from actions they have not been granted. The specific permission types are TBD but the system must support fine-grained, per-player control.

---

## Features

### Dice System

Physics-based dice rolling using Unreal's physics simulation. Each die is a 3D actor that receives an impulse and settles via physics. Face value is read via normal dot product when the die comes to rest.

- Supported dice: D4, D6, D8, D10, D12, D20, D100
- Player selects dice type and count via the Dice Selector UI, then clicks Roll
- Results are broadcast to all players via the chat log by default
- Roll button is disabled while a roll is in progress and while no dice are selected
- If a die gets stuck and never settles, a failsafe timer destroys it and reports it as lost

**Hidden rolls:** Any user (GM or player) can choose to make a roll private. A private roll is visible only to the roller and any recipients they choose. The GM can roll on behalf of NPCs with results hidden from players, and may reveal those results at any time. Private rolls are indicated as hidden in the chat log to recipients who cannot see them (e.g. "[GM rolled privately]").

**Advantage / Disadvantage:** The dice UI supports advantage and disadvantage for any die type, not just D20. Advantage rolls two dice of the selected type and takes the higher result; disadvantage takes the lower. This is a general mechanic and is not hardcoded to D20 — games that use D100 or other dice for decision-making are equally supported.

**Feel goal:** Rolls should look and feel satisfying — dice should tumble visibly, have weight, and settle naturally. Physics tuning (mass, damping, impulse) is a polish task.

### Chat

In-session text chat. Dice roll results are automatically posted to chat, attributed to the rolling player.

**Channels:** Chat is organized into tabs. The Server tab is always present and is the global broadcast channel. Private conversations each get their own tab, created automatically when a private message is first sent. Tab labels use the format `@P1 +2` (first recipient name, plus count of remaining). Users can rename tabs client-side; the internal channel identity is unchanged. Closing a tab hides it but does not destroy it — closed channels are accessible via a channels list button. An incoming message on a closed channel automatically reopens it.

**Addressing:** To send a private message, prefix each recipient's name with `@` (e.g. `@P1 @P2 message`). Everything after the last `@recipient` block is the message body. A UI picker will replace this syntax in a future pass.

**Private channel behavior:** When a sender creates a new private channel, they are automatically switched to that tab. All recipients see a notification indicator on the new tab. All participants (sender and all recipients) share the same channel and can send messages to it.

**Visibility:** By default, messages and roll results are broadcast to all users via the Server tab. Private messages are visible only to the participants of that channel.

**Persistence:** The full chat log (all messages and roll results) is saved as part of the session and restored on reload. Nothing is lost between sessions.

### Maps

Maps support two formats, and both can be used simultaneously:

- **Flat image:** A user-imported image (PNG, JPG, etc.) used as the map background — good for pre-drawn battle maps and scene art
- **Tile-based:** A grid of placeable tiles assembled in-session — good for procedural dungeon building and dynamic layouts

- Grid overlay (square and hex) toggled by the GM
- User-importable map images and tile sets
- Fog of war (GM toggles revealed areas per-tile or per-region)
- Lighting and atmosphere controls (time of day, ambient lighting)

### Miniatures

3D representations of characters and creatures placed on the map grid.

- Characters: controlled by the owning player (or GM for NPCs)
- Movement snaps to grid
- Labels display name, HP, and status conditions
- User-importable meshes for custom minis

**Scale:** Depends on map purpose. Combat maps follow the measurement rules of the active game system (e.g., 1 tile = 5 feet in D&D 5e). Non-combat maps — world maps, region maps, scene art — have no enforced scale; the GM and players define what distances mean in context.

**Default miniature:** Not yet decided. The game will ship with a set of built-in default options, and players can customize or replace the default with their own asset. Final default design will be determined as the project matures.

### Shared Notes

Any user can create a notes document within the session. By default a note is private (visible only to its creator). The creator can share it with specific users or with the entire group, and can grant read-only or edit access.

- Multiple users can edit a shared document simultaneously, with changes visible in real time (collaborative editing, similar to Google Docs)
- Notes are saved as part of the session and persist across reloads
- Typical uses: session recap, lore the party has discovered, quest tracking, GM prep notes shared selectively with players
- A user can have multiple notes documents open at once

> **Open Question:** Is there a text-only format, or should notes support basic formatting (headers, bullet points, bold/italic)?

> **Open Question:** Should notes be accessible outside of an active session (e.g., between sessions in a campaign view)?

> **Design Note:** Real-time collaborative editing requires an operational transform or CRDT-style conflict resolution strategy for simultaneous edits. This is a non-trivial networking problem — scope carefully when this feature is built.

### Initiative Tracker

Turn order panel visible to all players during combat. Tracks whose turn it is and counts down.

**Rolling initiative:** Players can roll initiative manually (via the dice UI) or automatically (pulled from character stats). Both options are supported — which is used depends on the situation and GM preference.

**Public tracker:** Shows the active turn order for all combatants currently in combat. Visible to all users at all times during a combat encounter.

**GM staging list:** The GM has a private list of combatants not yet in combat — hidden NPCs, reinforcements, ambush units, or any creature that may or may not enter the encounter. The GM can move a combatant from the staging list into the public tracker at any time, inserting them into the turn order at the appropriate position. Players do not see the staging list.

### Character Sheet

Tracks the player's character stats, skills, HP, spell slots, conditions, and inventory.

- D&D 5e stats: STR, DEX, CON, INT, WIS, CHA
- Proficiency bonus, saving throws, skill modifiers
- HP (current / max / temp), hit dice
- Spell slots by level
- Active conditions (Poisoned, Stunned, etc.)
- Attack rolls, saving throws, and skill checks pull from character sheet stats

**Visibility:** A character sheet is visible to its owner, the GM, and any other players the owner chooses to share it with. Sharing is controlled by the owning player.

**GM editing:** The GM can edit a player's character sheet to some extent, but with limits. What those limits are is not yet decided — this will be defined when the character sheet system is built.

### Custom Content

Players and GMs can import their own assets to use in a session.

- Maps: image files (PNG, JPG) used as map backgrounds
- Miniatures: custom meshes (format TBD)
- Dice: custom mesh and face value definitions
- Sounds: custom SFX or ambient tracks
- Asset sharing: a player can optionally share their custom assets with others in the session

> **Open Question (technical):** What 3D mesh formats will be supported for custom mini import? (FBX, OBJ, glTF, etc.) — to be decided when the import pipeline is built.

**Asset library:** Imported assets belong to the user, not the session. They are stored on the owner's local machine (with cloud storage as a future option). Assets persist across sessions, campaigns, and game systems — a custom mini or map imported once is available in any future game the user plays.

---

## UI / UX

**Philosophy:** The UI should feel like the edge of a physical table — present when needed, invisible when not. During active play, the map and minis should dominate the screen. Panels slide in and out rather than covering the view.

**Camera:** Top-down / isometric perspective. Players can pan (middle mouse), zoom (scroll wheel), and sprint-move (shift). Camera pitch is clamped; pan reset returns to default angle.

**HUD layout (rough):**
- Bottom center or bottom left: Dice Selector + Roll Button
- Right side: Chat log (collapsible)
- Top right or floating panel: Initiative tracker (visible during combat)
- Bottom right or docked panel: Character sheet (player's own)
- GM-only: Map controls, fog of war brush, NPC management panel

**Theming:** There is no single fixed default theme. The UI theme can change based on the game being played, and individual players can customize their own theme to suit their preferences. This implies a theming/skin system that is decoupled from game system — switching from D&D 5e to another system could offer a matching default theme, but players are never locked into it.

**Panel layout:** All UI panels are draggable, resizable, and toggleable (collapsible/hideable). Players have complete control over how their HUD is arranged. Panel layout should persist per user across sessions.

**Notifications:** When something occurs in a collapsed or hidden panel — a new chat message, a dice result, an initiative change — the player receives a visible notification on or near that panel so nothing is missed. The specific notification style (badge count, flash, icon) is TBD.

---

## Multiplayer

The session runs as a networked game. Chat and dice rolls are already replicated. All other game state (map, minis, initiative) will also be replicated.

**Player count:** Default maximum of 8 users per session (GM included). This cap can be removed, but the option is off by default. No hard limit is enforced beyond what the host machine can handle — if the game grows and this becomes an issue it will be revisited.

**GM vs host:** The GM role and the session host role are separate. The GM does not have to be the one hosting the server. This means a dedicated machine or another player can host while someone else runs the game as GM.

**GM disconnect:** The GM should have control over what happens to the session if they disconnect — whether intentionally or accidentally. At minimum, certain actions should be locked when no GM is present (e.g., moving NPCs, editing stats). The full disconnect policy will be defined as the session management system is built.

> **Open Question (technical):** Listen server or dedicated server? Needs research — to be decided before multiplayer architecture is finalized.

> **Open Question (technical):** How do players find and join a session? Direct IP, lobby system, friend invite? Needs research — to be decided alongside the server question.

---

## Game Systems

### D&D 5e / 2024 (Primary)

The first fully implemented ruleset. Covers:
- Ability scores and modifiers
- Proficiency bonus
- Skill checks, saving throws, attack rolls
- Initiative and turn order
- Action economy (action, bonus action, reaction, movement)
- Spell slots
- Conditions (Poisoned, Stunned, Blinded, etc.)
- Monster stat blocks

**Spell management:** In scope for the D&D 5e first pass. Spell management is complex and often confusing for players — the goal is to reduce that confusion with clear UI and automation where possible. This includes choosing spells, tracking spell slots, concentration, components, and spell effects.

**Condition tracking:** Conditions can be applied automatically or manually depending on the condition and the situation. When a game event occurs that would trigger a condition (e.g., a failed save against Poison), the system should apply it automatically. Auto-apply can be toggled — some tables prefer manual control over all conditions.

### Additional Systems (Future)

The game system layer should be architected to support other TTRPGs beyond D&D 5e. Specific systems are TBD but the design goal is that adding a new ruleset does not require core engine changes.

---

## Out of Scope

The following are explicitly not planned for the current development arc:

- A built-in video/voice chat system — players are expected to use a separate tool (Discord, etc.)
- A campaign management tool or session notes system (may be revisited)
- An AI game master or procedural content generation
- Mobile or web platforms (PC-first)
- A marketplace or community asset library
- Real-time physics on miniatures (minis snap to grid, no physics simulation)

---

## Open Questions (Master List)

A consolidated list of unresolved design decisions:

1. ~~Does the GM share the player camera view, or have a separate God-view camera?~~ — **Resolved:** GM has a separate God-view camera with full map visibility, but vision-blocking effects (e.g., Sphere of Darkness) still apply based on what controlled NPCs can perceive.
2. ~~Are GM permissions enforced in code, or trust-based initially?~~ — **Resolved:** Enforced in code. GM can grant/revoke per-player permissions. Specific permission types TBD.
3. ~~Can the GM roll dice for NPCs with results hidden from players?~~ — **Resolved:** Yes. Any user can make a roll private, visible only to chosen recipients. The GM can reveal hidden rolls at any time.
4. ~~Should advantage/disadvantage be built into the dice UI?~~ — **Resolved:** Yes, and it applies to any die type, not just D20.
5. ~~Does the GM need a private whisper channel in chat?~~ — **Resolved:** Any user can send private messages or private rolls to any other user(s). Not GM-exclusive.
6. ~~Does the chat log persist across saves?~~ — **Resolved:** Yes. All messages and roll results are saved and restored on reload.
7. ~~What is the base map format — flat image, tile-based, or both?~~ — **Resolved:** Both. Flat image and tile-based formats are each supported and can be used simultaneously.
8. ~~How does miniature scale map to in-world units?~~ — **Resolved:** Combat maps follow game system rules (e.g., 1 tile = 5 feet in D&D 5e). Non-combat maps have no enforced scale.
9. What is the default miniature when no custom mesh is provided? — **Deferred:** Game will ship with built-in defaults and customization options. Final design TBD as project matures.
10. ~~Does initiative roll automatically from stats, or always manual?~~ — **Resolved:** Both. Manual and automatic rolling are supported depending on the situation.
11. ~~Is the initiative tracker always visible or GM-toggled?~~ — **Resolved:** Always visible to all players. GM has a private staging list for combatants not yet in combat.
12. ~~Is the character sheet visible only to the owner and GM, or all players?~~ — **Resolved:** Visible to owner, GM, and anyone the owner chooses to share it with.
13. ~~Can the GM directly edit a player's character sheet?~~ — **Partially resolved:** Yes, to some extent, but with limits. Specific limits TBD when character sheet is built.
14. What mesh formats are supported for custom mini import? — **Deferred (technical):** To be decided when the import pipeline is built.
15. ~~Does imported asset library persist between sessions?~~ — **Resolved:** Assets are stored on the owner's local machine and persist across all sessions, campaigns, and games. Cloud storage is a future option.
16. ~~Is spell management in scope for the D&D 5e first pass?~~ — **Resolved:** Yes. Full spell management (slots, concentration, components) is in scope.
17. ~~Should conditions be auto-applied or always manual?~~ — **Resolved:** Auto-applied when a triggering event occurs. Auto-apply is toggleable per table preference.
18. Listen server or dedicated server? — **Deferred (technical):** Needs research before multiplayer architecture is finalized.
19. ~~Maximum player count per session?~~ — **Resolved:** Default max of 8 (GM included), removable cap, off by default. No hard engine limit.
20. ~~What happens when the GM disconnects?~~ — **Partially resolved:** GM and host are separate roles. Certain actions lock when no GM is present. Full policy TBD.
21. Lobby/matchmaking or direct IP? — **Deferred (technical):** To be decided alongside question 18.
22. ~~Dark/neutral UI theme or fantasy/themed aesthetic?~~ — **Resolved:** No fixed default. Theme is decoupled from game system and fully customizable per player. Game system may suggest a matching default theme.
23. ~~Are UI panels fixed layout or draggable/resizable?~~ — **Resolved:** Fully draggable, resizable, and toggleable. Player controls their own layout. Collapsed panels show notifications when activity occurs inside them.

---

*Last updated: 2026-03-26* — Chat section updated with tabbed channel design: Server tab, private tabs with `@P1 +2` labels, `@Name` addressing syntax, auto-switch on send, notification on receive, channels list for reopening closed tabs.

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
