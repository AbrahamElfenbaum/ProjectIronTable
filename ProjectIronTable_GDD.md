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

### Server Owner

- The player who created the session
- Technical/administrative role: create or close the session, kick players, transfer server ownership
- Does **not** automatically run the game — the Server Owner and Host are separate roles and can be different people

### Host (Game Facilitator)

- The designated game facilitator — equivalent to GM in traditional TTRPGs, but the role name is "Host" at the system level to support GM-less games
- Can be any connected player, including the Server Owner; assigned by the Server Owner
- Full control over the live game state: bring maps into the session, advance turns, manage game flow
- Controls NPC/monster miniatures
- Can hide or reveal sections of the map (fog of war)
- Has access to Host-only information (monster HP, hidden notes)
- Can override or adjudicate dice rolls
- Manages session state (start, pause, save, end)
- Can grant or revoke specific permissions to/from individual players
- Can delegate specific powers to individual players (e.g., allow a player to move enemy tokens or bring in a map)

> **Design Note:** "Host" is the system-level role name. Games that use a traditional GM (D&D, Pathfinder, etc.) can label it "GM" in their UI theme. GM-less games (Warhammer Fantasy, some PbtA games) use the same permission system with whoever is facilitating that session assigned as Host.

### Game Master (GM)

- Full control over the map: place, move, and remove tiles, minis, and objects
- Controls NPC/monster miniatures
- Can hide or reveal sections of the map (fog of war)
- Has access to GM-only information (monster HP, hidden notes)
- Can override or adjudicate dice rolls
- Manages session state (start, pause, save, end)
- Can grant or revoke specific permissions to/from individual players (for gameplay or administrative reasons)

**GM Camera:** The GM has a separate God-view camera with all the functions of the player camera, plus the ability to see the full map including areas behind fog of war. However, the GM's vision is still subject to vision-blocking effects — if a Sphere of Darkness is placed on the map, the GM cannot see inside it unless one of their controlled NPCs has a way to perceive through it. Vision is entity-based, not role-based: what you see is determined by what your controlled characters can perceive.

> **Design Note:** Player vision and character vision are separate systems. The player can always see any part of the map that has been revealed to them — they are not restricted to their character's perspective. Line of sight and vision-blocking effects only affect what the character *mechanically* can do: attack, target spells, perceive hidden enemies, etc. A character with blocked line of sight may have disadvantage on attacks or be unable to target certain spells, but the player can still see the map.
>
> **Tracking through vision-blocking effects:** When a character is inside a vision-blocking effect (e.g. magical Darkness), the player can no longer see their mini clearly. The solution is to render a persistent outline of any controlled character that remains visible through occluding effects, so the player always knows where their character is.
>
> **Vision types** (darkvision, blindsight, tremorsense, truesight, etc.) pierce different effects and are defined per game system. Full implementation is deferred until the vision system is built.

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

**Hidden rolls:** Any user (GM or player) can make a roll private by typing `@Name` recipients in the chat input before rolling. The result is routed only to the private channel shared with those recipients. A "[rolled privately]" indicator visible to non-recipients is not yet implemented and may or may not be added.

**Advantage / Disadvantage:** The dice UI supports advantage and disadvantage for any die type, not just D20. Advantage rolls two dice of the selected type and takes the higher result; disadvantage takes the lower. This is a general mechanic and is not hardcoded to D20 — games that use D100 or other dice for decision-making are equally supported.

**Feel goal:** Rolls should look and feel satisfying — dice should tumble visibly, have weight, and settle naturally. Physics tuning (mass, damping, impulse) is a polish task.

### Chat

In-session text chat. Dice roll results are automatically posted to chat, attributed to the rolling player.

**Channels:** Chat is organized into tabs. The Server tab is always present and is the global broadcast channel. Private conversations each get their own tab, created automatically when a private message is first sent. Tab labels use the format `@P1 +2` (first recipient name, plus count of remaining). Users can rename tabs client-side; the internal channel identity is unchanged. Closing a tab hides it but does not destroy it — closed channels are accessible via a channels list button. An incoming message on a closed channel automatically reopens it.

**Addressing:** To send a private message, prefix each recipient's name with `@` (e.g. `@P1 @P2 message`). Everything after the last `@recipient` block is the message body. A UI picker will replace this syntax in a future pass.

**Private channel behavior:** When a sender creates a new private channel, they are automatically switched to that tab. All recipients see a notification indicator on the new tab. All participants (sender and all recipients) share the same channel and can send messages to it.

**Dice roll routing:** Dice roll results are posted to whichever channel the rolling player currently has active, not always broadcast to everyone. If the rolling player has a private channel open, the roll result goes only to that channel's participants. If the Server tab is active, the roll broadcasts to all.

**Private roll initiation:** To start a private roll without an existing private channel, the player types `@Name` tokens into the chat input field and then clicks Roll. The roll button parses the input: if recipients are found, it sends a message to those recipients first (`"Rolling..."` if no other text was typed, or the typed text as the body), creating the private channel and switching to it. The dice result then arrives on that channel automatically since it was just made active. The input field is cleared after the message is sent but persists through click-away so players don't lose their typed recipients by accidentally clicking the roll button area first.

**Visibility:** By default, messages and roll results are broadcast to all users via the Server tab. Private messages and private rolls are visible only to the participants of that channel.

**Persistence:** The full chat log (all messages and roll results) is saved as part of the session and restored on reload. Nothing is lost between sessions.

### Maps

The map builder is the primary feature that differentiates ProjectIronTable from other VTTs. Rather than uploading flat images, players build fully 3D environments using tiles and props in real time inside Unreal Engine.

**Map types:**
- **Combat Map** — tactical scale (~5ft per tile); individual character/creature tokens; grid always shown; close overhead camera
- **World/Region Map** — travel scale (miles per unit); party group tokens and named location pins; grid optional; wide survey camera

Combat maps and world maps are separate saved map files. A world map can have **location pins** that reference a saved combat map by ID — the Host can activate a pin during a session to transition to that combat map (e.g., party arrives at the dungeon entrance on the world map, Host clicks the pin, combat map loads).

**Tile system:**
- Tiles are 3D actors that snap to a square grid (hex support planned after square is stable)
- Tile sizes are variable: 1×1, 1×2, 2×2, etc. — size is defined per tile type in its data asset
- Full height/elevation support — tiles can be placed at height increments, enabling multi-floor dungeons, cliffs, raised platforms, and ramps
- Tile categories: ground, water, elevated terrain, walls, roads, etc.

**Prop system:**
- Props (trees, rocks, barrels, furniture, etc.) are free-floating — they snap to the surface beneath them but their X/Y position is unconstrained
- Props do not need to land at the center of a tile or stay within a single tile's footprint
- Props support free rotation and scale
- Suitable for decoration, scatter, and environmental storytelling

**Map builder mode:**
- Separate input mode from "play mode"
- In builder mode: place, move, rotate, and delete tiles and props; undo/redo
- In play mode: map is static; only tokens can be moved
- Anyone can build and save maps independently. Only the Host can bring a map into a live session.
- Collaborative real-time editing (multiple players building together) is planned for a later phase

**Grid:** Square grid for initial release; hex grid planned after square system is stable. Grid display is toggleable by the Host.

**Fog of war:** The map starts fully hidden. The GM reveals and re-hides areas at their discretion throughout the session. Reveal tools: brush (paint areas freehand) and region/tile selection (flip whole areas at once) — both available, GM chooses what fits the moment. The GM can re-fog any previously revealed area at any time.

**Fog aesthetic:** Default is an opaque fog texture. The GM can switch to pitch black or semi-transparent per session preference.

**Auto-reveal:** When enabled, the map automatically reveals areas within player token line of sight as they move. On by default; the GM can disable it per session. When disabled, all revealing is manual.

**Camera boundary:** Fog of war acts as a hard camera boundary for players — they cannot pan or fly their camera into unrevealed areas. This extends to other impassable barriers the GM defines: locked doors, walls, and any other boundary the GM does not want players to see past. Players are limited to viewing only what has been revealed to them, regardless of where they move their camera. Exact boundary types and GM controls to be expanded and refined when the system is built.

**Environment (Time of Day & Weather):** The GM can control the time of day and weather in the simulator at any time during a session. All changes replicate to all connected players.

**Time of day:**
- The GM sets the time of day via a slider (0–24 hour range). Changing the time rotates the sun, which adjusts sky color, shadow direction, and ambient light automatically via UE5's Sky Atmosphere system.
- There is no automatic time progression — time only moves when the GM changes it.

**Weather:**
- The GM selects a weather type and adjusts its intensity (0–100%).
- Weather types include: Clear, Overcast, Rain, Heavy Rain, Snow, Blizzard, Wind, Sandstorm, and others. Weather types are not locked to biomes — the GM can apply any weather type regardless of the map's setting.
- Intensity controls particle density, fog thickness, and wind strength.
- Weather is purely cosmetic — no mechanical effects are attached in the core system (game system plugins may add mechanics later).

**UI:** The GM controls time and weather from a dedicated Environment panel in the gameplay HUD — a draggable, resizable panel registered with the taskbar like all other panels. Visible to all players in the initial implementation; GM-only access will be added when the permission system is built.

**Implementation:** Driven by `AEnvironmentManager` (replicated level actor), `EWeatherType` (enum), and `UEnvironmentControlPanel` (widget). `AEnvironmentManager` controls a `ADirectionalLight` (sun), `ASkyLight`, `AExponentialHeightFog`, `AWindDirectionalSourceComponent`, and `ANiagaraActor` instances per weather type placed in the level.

### Sound and Music

Audio is GM-controlled during a session. All sound types are supported with no restrictions on content.

**Sound types:**
- **Music** — background tracks, heard by all players simultaneously
- **Ambient audio** — environmental loops (wind, rain, tavern noise, etc.), heard by all players simultaneously
- **Sound effects** — one-shot or looping SFX (doors, explosions, footsteps, etc.), proximity-based: a player only hears a sound effect if their miniature is close enough to its source on the map

**Map-baked audio:** When building a map, the creator can assign default music and ambient tracks to it. These play automatically when the map loads in a session. The GM can toggle them on or off at any time.

**Built-in library:** The game ships with a built-in asset library covering all categories — tiles, props, miniatures, sounds, and music — so all players have a usable starting point without importing anything. The exact contents depend on what can be sourced, but at minimum every category will have something. Players and GMs can expand on this with their own imported assets.

### Miniatures

3D representations of characters and creatures placed on the map grid.

- Characters: controlled by the owning player (or GM for NPCs)
- Movement snaps to grid
- Labels display name, HP, and status conditions
- User-importable meshes for custom minis

**Movement:**
- Movement is freeform within a radius — the player drags their mini anywhere within their remaining movement range; the system enforces the limit and prevents moving beyond it
- Base range is the character's movement speed (e.g. 30ft = 6 tiles on a standard combat map)
- Range shrinks as movement is spent; modifiers from climbing, jumping, difficult terrain, etc. are layered on top as those systems are built
- **Difficult terrain:** Costs double movement to enter. Whether this is enforced automatically or left to player honor is a GM setting per session.
- **Diagonal movement:** GM-controlled setting. Options include: every diagonal costs 5ft (D&D 5e standard), alternating 5ft/10ft (Pathfinder style), or others TBD.

**Scale:** Depends on map purpose. Combat maps follow the measurement rules of the active game system (e.g., 1 tile = 5 feet in D&D 5e). Non-combat maps — world maps, region maps, scene art — have no enforced scale; the GM and players define what distances mean in context.

**Default miniature:** A wooden artist's mannequin (the jointed human analog used for figure drawing reference). Natural wood tone. Animated if suitable animations can be sourced — at minimum a walk cycle and idle. Used as the default for all entity types (humanoid, creature, monster) regardless of size or type; the same model is scaled to fit. Easy to swap out if a better fit is found later.

### Shared Notes

Any user can create a notes document within the session. By default a note is private (visible only to its creator). The creator can share it with specific users or with the entire group, and can grant read-only or edit access.

- Multiple users can edit a shared document simultaneously, with changes visible in real time (collaborative editing, similar to Google Docs)
- Notes support basic rich-text formatting: headers, bullet points, bold, italic
- Notes are saved as part of the session and persist across reloads
- Notes are also accessible outside of an active session via the Campaign Manager — players can read and edit their notes between sessions without launching a game
- Typical uses: session recap, lore the party has discovered, quest tracking, GM prep notes shared selectively with players
- A user can have multiple notes documents open at once

> **Design Note:** Real-time collaborative editing requires an operational transform or CRDT-style conflict resolution strategy for simultaneous edits. This is a non-trivial networking problem — scope carefully when this feature is built.

### Character Creation

Players build their characters through a character creator before or between sessions.

**Structure:** The creator has two layers:
- **Root layer** — fields common to all characters regardless of game system (name, appearance, bio, portrait/artwork, etc.)
- **Game system layer** — fields specific to the active game system (stats, class, race, skills, etc.). What appears here is defined by the game system plugin.

Characters are created outside of an active session and stored in the player's campaign profile. A player can have multiple characters across different campaigns.

**Custom characters:** The same principle applies to custom character options — a player or GM can create custom races, classes, backgrounds, or other game-system-specific entries using the same layered structure.

### Inventory and Loot

Both players and the GM have an inventory. Items can be transferred between them during a session.

**Sending items:** Any user can click an item in their inventory and choose "Send to Player." If the item is stackable (quantity > 1), they are prompted for how many to send. The recipient receives the item in their inventory.

**GM item list:** In addition to their personal inventory, the GM has access to a master item list — a full catalog of items they can grant to players without needing to hold the item in their own inventory first. This covers cases like rewarding loot that the GM hasn't pre-stocked.

**Custom items:** Any user can create a custom item. The creation flow follows the same layered structure as character creation — a root layer for universal item properties (name, description, icon, quantity) and a game system layer for system-specific fields (weight, value, damage, rarity, etc.).

### Measurement Tools

In scope and universal across all game systems. Many of the same measurement tools appear in different TTRPGs, so these are part of the core layer rather than any specific game system plugin.

Specific tools (ruler, AoE templates — cone, sphere, line, cube, etc.) and their exact behavior to be designed when this system is built.

### Session Save and Load

**What is saved:** The full session state — map layout, token positions, fog of war, initiative tracker, chat log, character sheets, notes, and inventory. Everything is saved together as a single snapshot.

**Save slot:** Each campaign has one rolling save slot. There are no multiple save states — each save overwrites the previous.

**Manual save:** The GM can save at any time. The GM can grant save permission to other players.

**Autosave:** Autosave runs on a configurable interval. Can be toggled on or off by the GM. Default state (on or off) TBD.

**Save on session close:** The session automatically saves when it ends. The GM can disable this if needed.

**Storage:** Save data should be stored efficiently — exact format and compression strategy to be determined during implementation.

### Entity Management Panel

The GM has a dedicated panel for managing all entities under their control. This includes:
- **NPCs and monsters** — creatures the GM controls directly
- **Player-summoned creatures** — familiars, animal companions, summoned monsters, or any creature a player's character controls
- **Controllable items** — animated objects or other items that act as independent entities on the map

Players with summoned creatures or companions also have access to this panel for their own controlled entities, separate from their character sheet.

The panel provides a centralized view for tracking HP, conditions, and turn order across multiple controlled entities without hunting for them on the map.

### Combat

**Starting and ending combat:** The GM starts and ends combat manually. There is no automatic detection of combat state — too many variables (ambushes, social encounters that turn violent, partial retreats) make automation impractical. Combat is a narrative state that the GM controls. In GM-less games or PvP scenarios, whoever holds the Host role takes this responsibility.

**During combat:** The map is static — only tokens can be moved. The initiative tracker becomes active. The GM retains full map control.

### Initiative Tracker

Turn order panel visible to all players during combat. Tracks whose turn it is and counts down.

**Rolling initiative:** Players can roll initiative manually (via the dice UI) or automatically (pulled from character stats). Both options are supported — which is used depends on the situation and GM preference.

**Public tracker:** Shows the active turn order for all combatants currently in combat. Visible to all users at all times during a combat encounter.

**GM staging list:** The GM has a private list of combatants not yet in combat — hidden NPCs, reinforcements, ambush units, or any creature that may or may not enter the encounter. The GM can move a combatant from the staging list into the public tracker at any time, inserting them into the turn order at the appropriate position. Players do not see the staging list.

**Round tracking:** The tracker counts rounds automatically. This is primarily useful for duration-based effects (e.g. "lasts 2 rounds", concentration spells, condition timers).

**Turn advancement:** Turns do not end automatically — the active player or GM manually clicks "End Turn." No timers.

**Skipping and delaying:** A combatant can request to skip or delay their turn. The GM approves the request. Once approved, the combatant's slot moves or is passed as appropriate.

**Removing combatants:** The GM can remove any combatant from the tracker at any time ("Remove from Combat"). This is separate from removing the miniature from the map — the GM decides independently whether the mini stays on the map or is removed. A creature that fled, surrendered, or died may still have its mini present for narrative purposes.

### Character Sheet

Tracks the player's character stats, skills, HP, spell slots, conditions, and inventory.

- D&D 5e stats: STR, DEX, CON, INT, WIS, CHA
- Proficiency bonus, saving throws, skill modifiers
- HP (current / max / temp), hit dice
- Spell slots by level
- Active conditions (Poisoned, Stunned, etc.)
- Attack rolls, saving throws, and skill checks pull from character sheet stats

**Visibility:** A character sheet is visible to its owner, the GM, and any other players the owner chooses to share it with. Sharing is controlled by the owning player. The GM can always view the full sheet of any player in their session.

**GM editing:** The GM does not directly edit player character sheets. Stats, HP, spell slots, inventory, proficiencies, and all identity/build fields are the player's domain. Changes to those values come through game events — conditions applied via the game system, items granted through a loot flow, etc. — not direct GM sheet edits.

The GM can:
- Award XP (if the campaign uses XP)
- Signal a level-up — notify a player that they have leveled up or set their target level directly. The player then handles the leveling flow themselves (choosing new features, spells, HP, etc.). What that flow looks like is game-system-specific.

### Player Profiles

Every user has a profile visible to others when they send a join request or interact in the public browser.

**Profile fields:**
- **Username** — required, always visible
- **Bio** — optional short description the player writes themselves (e.g. playstyle, availability, what they're looking for in a game)
- **Games played** — a list of game systems the player has experience with. Populated automatically from their campaign history in the app, and manually self-reportable for real-life experience outside the app. Both sources appear on the profile.

When a join request is sent, the GM and current players see the requester's profile alongside their chat introduction. No formal application form — the profile provides context and the chat does the rest.

### Custom Content

The game ships with a built-in library of assets across all categories (tiles, props, miniatures, sounds, music) so players have a usable starting point immediately. Players and GMs can import their own assets to expand on this — groups are never locked to built-in content.

**Supported import types:**
- Tiles: custom 3D tile meshes for map building
- Props: trees, rocks, decorations, structures
- Miniatures: character and creature models
- Dice: custom mesh and face value definitions
- Sounds: custom SFX or ambient tracks

**Import format:** glTF (with Draco compression) is the target format for 3D mesh imports. It is compact, widely supported by tools like Blender, and has runtime loading support in UE5. Specific implementation is deferred until the mesh import phase.

**Asset distribution in sessions:**
- Assets are stored **locally** on each player's machine — there is no central server
- When the Host brings a map into a live session, any assets that connected clients are missing are automatically transferred from the Host's machine to those clients before the map renders
- All clients see the map in full — no placeholders for missing assets
- Assets are cached locally after the first transfer; re-download is not needed in future sessions
- A progress indicator shows what is syncing during map load

**Manual asset sharing:**
- Outside of sessions, players can share assets peer-to-peer for their own libraries (e.g., a player sees a tile someone else used and wants a copy for their own maps)
- Sharing is opt-in — the asset owner approves before transfer

**Future store:** Asset metadata includes a `bShareable` flag and a license field to support a future paid asset store without reworking the sharing system.

**Asset library:** Imported assets belong to the user, not the session. They are stored on the owner's local machine (with cloud storage as a future option). Assets persist across sessions, campaigns, and game systems — a custom mesh imported once is available in any future game the user plays.

---

## Home Screen

The home screen is the first scene the player sees when launching the game. It is separate from the gameplay scene and has its own game mode, player controller, and HUD component.

**Options (rough):**
- Play — opens the Campaign Manager
- Library
- Settings
- Quit

The home screen controller handles UI-only input — no camera pawn or game input is needed here.

---

## Campaign Manager

The Campaign Manager is the primary hub between the home screen and an active session. It lists all campaigns and games the player is a part of, and serves as the out-of-session home for character sheets, notes, scheduling, and campaign info.

**Access:** Reached via the Play button on the home screen.

**Layout (TBD — two candidates):**
- **Grid:** Columns represent game systems (D&D, Pathfinder, Warhammer, etc.); rows are campaigns within each system. Columns are collapsible.
- **Tab list:** A sidebar (left or right) with one tab per game system, displayed 3 per row (count subject to change). Clicking a tab shows a scrollable list of campaigns for that system.

Final layout to be decided during UI design pass.

**Campaign creation:**
- Any player can create a campaign at any time and invite whoever they want
- **Private:** Invite-only. Players must be invited to join.
- **Public:** Discoverable via the public browser. Players can apply; the Host approves and space must be available.

**Campaign browser (public games):**
A dedicated browsing screen for finding public campaigns. Filterable by:
- Name
- Game system
- Tags (player-defined labels describing tone, theme, style — e.g., "horror", "beginner-friendly", "roleplay-heavy")
- One-shot vs. multi-session
- Meeting days / frequency / estimated session length
- Additional filters TBD

Direct links and invite codes are also supported as an alternative to browsing.

**Campaign card content (varies by game system):**

For D&D 5e, a campaign card includes:
- Player's character sheet
- Notes the player has created or has been given access to
- Party member list — each entry shows character name (primary), player screen name or real name (subtitle), class(es) and level
- Character artwork (optional — displayed if the player has provided it)
- Next scheduled session (if scheduling is set up for this campaign)

Other game systems will define their own card contents appropriate to their ruleset.

---

## Scheduling

Sessions can have a schedule attached to them. Scheduling is in scope; the full feature set is TBD.

At minimum, a campaign can record when it meets (days of the week, frequency, typical session length). This data surfaces on the campaign card in the Campaign Manager and is used as a filter in the public campaign browser.

A more extensive scheduling system (calendar integration, session reminders, RSVP/attendance tracking) may be added — scope to be determined.

---

## Asset Library

Accessible from the home screen via a Library button. A file explorer-style view of all locally imported assets, organized by category folder (Sounds, Maps, etc.).

**Import flow:**
1. User clicks Import (or drags a file onto a folder)
2. If using the Import button: a panel asks what type of asset they are importing (Sound, Map, etc.)
3. The selected type determines the accepted file extensions and the destination folder
4. User either browses via a file dialog or drag-and-drops a file from an open file explorer window
5. File is validated against accepted extensions, copied to the destination folder, and registered

**Drag-and-drop shortcut:** If the user is already browsing the library and navigates into a specific folder (e.g. Sounds), they can drag a file directly onto that folder view. The destination is implicit — the importer validates the file type against that folder's accepted extensions and rejects if it doesn't match.

**Supported formats (current):**
- Images: PNG, JPG — for maps and tokens
- Audio: WAV — for dice sounds, ambience, music
- Meshes: deferred — requires a runtime mesh loader plugin; to be decided when miniature/dice import is built

**File dialog:** Windows native (`GetOpenFileName`), wrapped in `#if PLATFORM_WINDOWS` for future portability.

**Asset storage:** All imported assets are stored on the local machine. They persist across sessions, campaigns, and game systems. Cloud storage is a future option.

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

**Panel layout:** All UI panels are draggable, resizable, and toggleable (collapsible/hideable). Players have complete control over how their HUD is arranged. Panel layout persists per user across sessions — position, size, and collapsed state are all saved locally and restored on next launch. A Reset Layout button on the far right of the taskbar restores all panels to their default positions and sizes without affecting visibility.

**Notifications:** When something occurs in a collapsed or hidden panel — a new chat message, a dice result, an initiative change — the player receives a visible notification on or near that panel so nothing is missed. The specific notification style (badge count, flash, icon) is TBD.

---

## Multiplayer

The session runs as a networked game. Chat and dice rolls are already replicated. All other game state (map, minis, initiative) will also be replicated.

**Player count:** Default maximum of 8 users per session (GM included). This cap can be removed, but the option is off by default. No hard limit is enforced beyond what the host machine can handle — if the game grows and this becomes an issue it will be revisited.

**Server Owner vs Host:** The Server Owner and Host are separate roles. The Server Owner is whoever created/controls the server. The Host is the designated game facilitator and can be any player — the Server Owner assigns the Host role. The Host runs the game; the Server Owner manages the technical session. Both can be the same person or different people.

**Host disconnect:** The Host should have control over what happens to the session if they disconnect — whether intentionally or accidentally. At minimum, certain actions should be locked when no Host is present (e.g., moving NPCs, editing stats). The full disconnect policy will be defined as the session management system is built.

**Server model:** Listen server — the Server Owner hosts the session from their own machine. Acceptable tradeoff at this scale (2–8 players, non-persistent sessions). Code is structured so a future switch to dedicated server is seamless: all authoritative state lives server-side, and the Server Owner role is flag-based rather than tied to `IsLocalController()`.

**Session discovery:** Players find sessions via the Campaign Manager public browser (filterable) or via direct invite link/code.

**Joining a session:**
- **Invite code** — bypasses approval entirely. Player joins immediately.
- **Public browser** — sends a join request to the session. A notification appears in chat for all current players and the GM: *"[Username] wants to join."* The requesting player gets temporary chat access so they can introduce themselves and the group can talk to them before a decision is made. The GM approves or declines — no formal application process is imposed. Groups handle their own vetting however they see fit.

**Pre-session lobby:** When a player joins a session before the GM has started it, they land in a waiting room. The lobby shows who is connected and who hasn't joined yet, has chat available before the game starts, and lets players access their character sheet while waiting. The GM (Host) sees everyone's connection status and launches the session when ready.

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

### System Architecture (High Level)

The codebase is divided into two layers:

**Core layer** — game-agnostic. Handles the map, tiles, props, miniatures, dice physics, chat, session management, the Campaign Manager, asset library, and notes. This layer has no knowledge of any specific ruleset.

**Game system plugin layer** — sits on top of the core. Defines everything ruleset-specific: character sheet fields and layout, stat calculations, conditions and their effects, action economy, spell/ability systems, XP and leveling, campaign card content, and leveling flow. Each supported TTRPG is implemented as a separate plugin. Adding a new system means building a new plugin, not changing the core.

**Campaign locking:** A campaign is locked to one game system at creation. Mixing systems within a single campaign is not supported.

**Rule variants:** Within a known system, the GM can configure which rule variants the campaign follows — alternate diagonal movement rulings, optional rules, common house rules, etc. These are configuration knobs on top of a system plugin, not a separate system.

**Custom rulesets:** Full user-created game systems are a potential future addition but are explicitly out of scope for now. The plugin architecture does not need to account for this yet — it is noted only so the design is not closed off entirely.

---

## Out of Scope

The following are explicitly not planned for the current development arc:

- A built-in video/voice chat system — scope is undecided; see Open Questions
- An AI game master or procedural content generation
- Mobile or web platforms (PC-first)
- A marketplace or community asset library
- Real-time physics on miniatures (minis snap to grid, no physics simulation)

---

## Open Questions (Master List)

Resolved questions are struck through and kept for reference. Genuinely open items are listed at the top.

### Still Open

1. **Voice / video chat** — Undecided between three options: (a) built-in voice/video using a real-time comms SDK or UE plugin, (b) Discord integration (SDK or Rich Presence), (c) out of scope — players use a separate tool. All three are viable; scope and complexity differ significantly. To be decided before the UI/Polish phase at latest.
2. ~~**Server model** — Leaning listen server; pending technical validation in UE5.~~ — **Resolved:** Listen server confirmed. See Resolved item 26.
3. **Host disconnect policy** — Certain actions lock when no Host is present. Full policy TBD when session management is built.
4. **GM/Host permission types** — Fine-grained per-player permissions are enforced in code. Specific permission list TBD when the system is built.
5. **Host delegation permission types** — Delegation is supported. Specific delegatable actions TBD when built.
6. **Scheduling extensiveness** — Minimum: meeting days/frequency/session length. Calendar integration, reminders, RSVP tracking possible. Scope TBD.
7. **Campaign Manager layout** — Two candidates (collapsible grid vs. tab sidebar). TBD during UI design pass.
8. **Notification style** — Badge count, flash, icon, or other. TBD during UI design pass.

### Resolved

1. ~~Does the GM share the player camera view, or have a separate God-view camera?~~ — **Resolved:** GM has a separate God-view camera with full map visibility, but vision-blocking effects still apply based on what controlled NPCs can perceive.
2. ~~Are GM permissions enforced in code, or trust-based initially?~~ — **Resolved:** Enforced in code.
3. ~~Can the GM roll dice for NPCs with results hidden from players?~~ — **Resolved:** Yes. Any user can make a roll private. The GM can reveal hidden rolls at any time.
4. ~~Should advantage/disadvantage be built into the dice UI?~~ — **Resolved:** Yes, applies to any die type.
5. ~~Does the GM need a private whisper channel in chat?~~ — **Resolved:** Any user can send private messages or rolls. Not GM-exclusive.
6. ~~Does the chat log persist across saves?~~ — **Resolved:** Yes.
7. ~~What is the base map format?~~ — **Resolved:** 3D tile/prop builder (primary). Two map types: Combat Map and World/Region Map. World maps link to combat maps via location pins.
8. ~~How does miniature scale map to in-world units?~~ — **Resolved:** Combat maps follow game system rules (1 tile = 5ft in D&D 5e). Non-combat maps have no enforced scale.
9. ~~What is the default miniature?~~ — **Resolved:** Wooden artist's mannequin, natural wood tone, animated if possible. Used for all entity types, scaled as needed.
10. ~~Does initiative roll automatically or always manual?~~ — **Resolved:** Both supported.
11. ~~Is the initiative tracker always visible or GM-toggled?~~ — **Resolved:** Always visible. GM has a private staging list for combatants not yet in combat.
12. ~~Is the character sheet visible only to the owner and GM?~~ — **Resolved:** Visible to owner, GM, and anyone the owner shares it with.
13. ~~Can the GM directly edit a player's character sheet?~~ — **Resolved:** No direct edits to stats, HP, or build fields. GM can award XP and signal a level-up. All other changes come through game events.
14. ~~What mesh formats are supported for custom imports?~~ — **Resolved:** glTF with Draco compression. Implementation deferred until mesh import phase.
15. ~~Does the asset library persist between sessions?~~ — **Resolved:** Yes, stored locally on the owner's machine.
16. ~~Is spell management in scope for D&D 5e first pass?~~ — **Resolved:** Yes.
17. ~~Should conditions be auto-applied or always manual?~~ — **Resolved:** Auto-applied, with a GM toggle per table preference.
18. ~~Maximum player count per session?~~ — **Resolved:** Default max 8, removable cap, off by default.
19. ~~What happens when the GM disconnects?~~ — **Partially resolved:** Certain actions lock when no Host is present. Full policy TBD when session management is built. (See open question 2.)
20. ~~Lobby/matchmaking or direct IP?~~ — **Resolved:** Public browser with filters + direct invite link/code. No separate lobby needed.
21. ~~Should notes support rich-text formatting?~~ — **Resolved:** Yes. Headers, bullets, bold, italic.
22. ~~Should notes be accessible outside of an active session?~~ — **Resolved:** Yes, via Campaign Manager.
23. ~~Dark/neutral UI theme or fantasy/themed aesthetic?~~ — **Resolved:** No fixed default. Theme is decoupled from game system and customizable per player.
24. ~~Are UI panels fixed layout or draggable/resizable?~~ — **Resolved:** Fully draggable, resizable, and toggleable. Layout persists per user.
25. ~~Should the Host be able to delegate specific powers to players?~~ — **Resolved:** Yes. Specific delegatable actions TBD when built. (See open question 4.)
26. ~~Listen server or dedicated server?~~ — **Resolved:** Listen server. Server Owner hosts from their own machine. Code is structured for a seamless future switch to dedicated: authoritative state server-side, Server Owner role checked via flag not `IsLocalController()`.

---

*Last updated: 2026-04-10* — Server model confirmed as listen server (open question #2 resolved). Multiplayer section updated. Phase 2 session management identified as next focus.

*2026-04-09* — No design changes. Implementation session: home screen navigation fully wired to Campaign Manager, Campaign Browser (stub), and Asset Library (stub) screens. `UBaseScreen` introduced as shared base class. Play button removed in favour of Campaign Manager button.

*2026-04-09* — No design changes. Implementation session: `UCampaignManagerScreen` tab-switching and visual selection complete; `UDelegateLibrary` introduced for shared delegate types.

*2026-04-08* — Environment system (Time of Day & Weather) fully designed. Voice/video chat moved from Out of Scope to Open Questions (undecided between built-in, Discord integration, or out of scope). Open question list renumbered. Expanded the Maps "Lighting and atmosphere" stub into a complete feature section covering time of day, weather types, intensity, GM panel, replication model, and planned C++ classes.

*2026-04-03* — Map system redesigned as a 3D tile/prop builder (primary differentiator). Added Combat Map and World/Region Map scale modes with location pin linking. Host/Server Owner roles clarified and separated. Custom content section rewritten with auto-distribution model (no placeholders), glTF format decision, and manual peer-to-peer sharing. Host delegation added as a resolved design question. Player Roles section restructured with Server Owner and Host as distinct entries.

*2026-04-03 (continued)* — Shared Notes updated: rich-text formatting confirmed, out-of-session access confirmed via Campaign Manager. Home Screen Play button now leads to Campaign Manager. Campaign Manager section added: layout TBD (grid with collapsible columns or tab sidebar), campaign creation with private/public modes, public browser with filters, campaign card contents defined for D&D 5e. Scheduling added as in-scope with extensiveness TBD.

*2026-04-03 (continued)* — Character Sheet GM edit limits fully resolved. Miniatures: default mini (wooden artist's mannequin), movement rules, difficult terrain, diagonal ruling all added. Multiplayer: server model (leaning listen), session discovery, join flow (invite code vs. public request with temporary chat), pre-session lobby all documented. Game Systems architecture defined: core layer + game system plugin layer, rule variants, campaign locking, custom rulesets deferred. Combat section added: GM-driven start/end. Initiative Tracker expanded: round tracking, manual turn advancement, skip/delay, removing combatants. Fog of War fully designed including camera boundary. Sound and Music added: all types, proximity-based SFX, map-baked audio, built-in library. Player Profiles added. Character Creation added: root + game system layer. Inventory and Loot added: send flow, GM item list, custom items. Measurement Tools noted as in scope. Entity Management Panel added. Session Save/Load fully designed. Vision system documented: player vs. character vision separation, outline tracking for vision-blocking effects. Out of Scope updated: campaign management and notes removed (now in scope).

---

Copyright 2026 Abraham Elfenbaum. All Rights Reserved.
