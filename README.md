# Desert Island Survival

A 2D text-based survival game built in C++11. The player wakes up stranded on a deserted island after a shipwreck with nothing but the will to survive. Manage hunger, thirst, and fatigue, fight off dangerous animals, gather resources, craft tools, and ultimately build a raft to escape.

---

## Team — Group 212

| Name | Student ID | GitHub |
|------|------------|--------|
| Yang Heemoon | 3036418162 | heemoon72 |
| Kim Jaehan | 3036294762 | JAEHAN KIM |
| Oh Philseung | 3036324957 | PhilseungOh |
| Jung Gyumin | 3036279059 | 정규민 Gyumin Jung |

---

## Game Description

You are a shipwreck survivor stranded on a remote island. The game is played entirely in the terminal as a top-down ASCII / emoji map. Every step you take costs hunger, thirst, and fatigue. Walking near bushes can trigger animal encounters; surviving combat earns you meat and hides. The island has five connected zones — Beach, Cave, Forest, Village, and Shore — each with unique resources and dangers.

Your goal is to gather enough materials to craft a complete **Raft** and escape. The game tracks an in-game clock (hours and days); needs decay faster at night, and dangerous predators become more common as days pass. Three difficulty modes (Easy, Normal, Hard) scale the speed of decay and the intensity of encounters.

## Game Play Video

https://youtu.be/GtZDGk9ylIg

### Zones

| # | Zone | Description |
|---|------|-------------|
| 1 | Beach | Starting area. Sandy shore with shipwreck debris, scorpions, and palm trees (Coconut source). |
| 2 | Cave | Rocky cliffs enclosing a cave interior with a campfire — the only shelter for the first two days. |
| 3 | Forest | Dense woods with fresh water, berries, Medical Herbs, and the widest variety of wildlife. |
| 4 | Village | Abandoned settlement with ruins, a shelter (safe to sleep), a campfire, and a crafting workshop. |
| 5 | Shore | Coastal dock where the final Raft is assembled and launched to win the game. |

Maps are loaded from plain-text files in `data/` and rendered either as ASCII art or as a Unicode emoji grid (toggled with **V** in-game).

---

## Features Implemented

### 1. Survival Stats & Day/Night Cycle
The player has four tracked stats: **HP**, **Hunger**, **Thirst**, and **Fatigue**. Each stat decays by 1 point every fixed number of movement steps (the threshold scales with difficulty). Needs decay **50% faster at night** (22:00–05:59). When Hunger or Thirst hits 0 while sleeping, the player loses 50 HP on waking. Fatigue is fully restored by sleeping in the cave or shelter; Hunger and Thirst drop slightly during sleep.

The in-game clock ticks through 24 hours — Morning, Afternoon, Evening, and Night — and is displayed on the HUD. Each action (combat, foraging, fishing) advances the clock by a set number of hours.

### 2. Five Connected Maps with Tile Interactions
Five 30×60 tile maps are linked by exit tiles (`E`). Each tile type carries specific game logic:

| Tile | Effect |
|------|--------|
| `,` Grass/Bush | Walkable; 5–20% chance of animal encounter per step (scales with day) |
| `T` Tree | Press **E** adjacent to chop; yields Wood (×2 with Axe) |
| `P` Palm | Press **E** adjacent to chop; yields Wood + 1 Coconut (×2 with Axe) |
| `^` Rock | Press **E** adjacent to mine; yields Stone, sometimes Flint (×2 with Pickaxe) |
| `C` Campfire | Press **E** adjacent to open the Cooking menu |
| `G` Debris | Press **E** adjacent to search; chance to find Murky Vial, Dried Entrails, or Resin |
| `M`/`H` Herb | Walk onto to auto-collect 1 Medical Herb; tile respawns after several hours |
| `~` Ocean | Impassable; equip Fishing Rod and stand adjacent to fish |
| `E` Exit | Walk onto to travel to the adjacent map |

### 3. Combat System
Walking through or adjacent to bush (`,`) tiles can trigger a random monster encounter. The encounter chance starts at 5% on Day 1 and caps at 20% after Day 8. A second roll determines whether the spawn is a **dangerous predator** or **harmless game** — the predator chance starts at 20% and rises to 80% by Day 7.

**Monster roster:**

| Monster | Type | Notes |
|---------|------|-------|
| Scorpion | Dangerous | Can inflict poison |
| Rattlesnake | Dangerous | Fast; can poison |
| Wild Boar | Dangerous | High HP |
| Crocodile | Dangerous | Slow but heavy-hitting |
| Wolf | Dangerous | Day 4+ only |
| Grizzly Bear | Dangerous | Day 3+; drops Bear Skin (required raft material) |
| Wild Bird | Harmless | Drops Raw Meat |
| Rabbit | Harmless | Drops Raw Meat |

Combat is turn-based: **Attack**, **Use Item**, or **Flee**. Attack damage is randomised within a variance window; the Knife adds +5 to base attack. Fleeing always succeeds but costs random HP and may forfeit inventory items (mitigated by carrying **Dried Entrails**). Hard difficulty applies a damage multiplier. Winning combat drops loot which is automatically collected.

### 4. Inventory System
The inventory is a list of named items with quantities. It supports:
- **Add / consume / remove** items by name
- **Sort** alphabetically (triggered automatically when opened)
- **Equip** a single tool at a time (Axe, Pickaxe, Knife, Fishing Rod, Torch)
- **Drop** items with configurable amounts
- **Use** consumables (food, drinks, healing items) directly from the inventory screen

Items are backed by a **static registry** (`Item.cpp`) that stores every item's type, description, and stat-restore values, keeping game data separate from runtime inventory state.

### 5. Crafting & Cooking
Two separate recipe lists are accessible from distinct locations:

**Workshop Crafting** (press **E** near the workshop in the village):
- Processed materials: Vine, Rope, Log
- Tools: Axe, Pickaxe, Knife, Fishing Rod, Torch
- Healing: Bandage, First Aid Kit, Potion
- Raft components: Raft Base, Sail, Oar, Reinforcement
- Final goal: **Raft** (requires Raft Base ×10, Sail ×3, Oar ×2, Reinforcement ×5, Resin ×10, Bear Skin ×1)

**Campfire Cooking** (press **E** near any campfire `C`):
- Cooked Fish, Cooked Meat, Soup, Herbal Tea

Recipes are displayed in a paginated UI showing each ingredient and how many you currently have. Items marked `[craftable]` can be crafted immediately.

### 6. Fishing Minigame
With a **Fishing Rod** equipped and **Bait** in the inventory, press **E** adjacent to ocean water to fish. Each cast animates a marker bouncing across a 16-position bar divided into zones:

```
[------===**==---]
  MISS  HIT ** HIT
```

Press **Enter** to stop the marker. The zone hit determines loot:
- `**` BEST zone: 70% Fish, 20% Cloth Scrap
- `==` HIT zone: 50% Fish, 25% Cloth Scrap
- `--` MISS zone: 5% Cloth Scrap only

Each cast consumes 1 Bait. The session continues until bait runs out or the player presses **Q**.

### 7. Save & Load System
The game can be saved and loaded from `data/save.txt`. Saved data includes: player position and map, all four stats, current day/hour, move count, emoji/ASCII mode preference, and the full inventory. Load is available from the main menu; the file is written on manual save (press **S** in-game).

### 8. Three Difficulty Modes
Selected at the start of each new game:

| Difficulty | Needs Decay Rate | Encounter Chance |
|------------|-----------------|-----------------|
| Easy | ×1.5 slower | −10% bush encounter chance |
| Normal | Baseline | Baseline |
| Hard | ×0.6 faster | Monsters deal extra damage |

### 9. Dual Render Mode
Press **V** in-game to toggle between ASCII art and a Unicode emoji grid. In emoji mode every tile maps to a distinct emoji (🌊 water, 🌿 grass, 🌳 tree, 🌴 palm, 🔥 campfire, 💀 debris, 🧱 wall, etc.). The entire display is centred on-screen in both modes.

### 10. Win & Lose Conditions
- **Win**: Complete the Raft and carry it to the dock on the Shore map.
- **Lose**: HP drops to 0 from combat, starvation, or dehydration.

---

## Coding Requirements

### Requirement 1 — Generation of Random Events

`rand()` (seeded once with `srand(time(NULL))` inside `Monster.cpp`) drives every probabilistic event in the game:

- **Bush encounter check** — every step near grass rolls `rand() % 100 < encounterChance`. The chance starts at 5% (Day 1) and ramps up by 2% per day, capped at 20% after Day 8.
- **Dangerous vs. harmless spawn split** — a second roll against `dangerousAnimalChancePercent(day)` (20%→80%) decides which monster pool to draw from. Each monster is then selected by `rand() % pool.size()`.
- **Combat damage variance** — every attack uses `baseAtk + (rand() % (variance*2+1)) - variance` so no two hits deal identical damage.
- **Flee penalty rolls** — `randInRange(hpMin, hpMax)` and `randInRange(itemsMin, itemsMax)` determine HP lost and items forfeited on escape; a random inventory index is chosen for each forfeited item.
- **Monster loot tables** — conditional `rand() % 100 < threshold` rolls per loot entry (e.g., 40% extra Raw Meat from Wild Boar, 25% Flint from rocks).
- **Debris search** — `rand() % 100` against tiered thresholds selects between nothing, Murky Vial, Dried Entrails, or Resin.
- **Berry foraging** — 5% chance per step on grass: `rand() % 100 < 5`.
- **Fishing loot** — zone-specific `rand()` rolls award Fish or Cloth Scrap depending on where the player stopped the bar.
- **Respawn placement** — when a collected tile respawns at a new random position, the code rejection-samples random (row, col) pairs until it finds an open, walkable cell.

**Files**: `Monster.cpp`, `Combat.cpp`, `Fishing.cpp`, `Game.cpp`

---

### Requirement 2 — Data Structures from the STL

The project uses `std::vector` and `std::string` as the primary STL containers throughout:

| Container | Location | Purpose |
|-----------|----------|---------|
| `std::vector<Item>` | `Inventory` (private field) | Runtime item list; grows/shrinks as items are picked up or consumed |
| `std::vector<Item>` | `SaveData` | Inventory snapshot serialised to and from the save file |
| `std::vector<std::string>` | `Map::grid` | Each row of the loaded map stored as one string |
| `std::vector<ExitPoint>` | `Map::exitPoints` | All exit tiles registered during map loading |
| `std::vector<Recipe>` | `getCraftingRecipes()`, `getCookingRecipes()` | Full recipe tables built and returned at runtime |
| `std::vector<Ingredient>` | `Recipe::ingredients` | Per-recipe ingredient list (nested inside `Recipe`) |
| `std::vector<std::pair<std::string,int>>` | `Monster::getDrops()` | Loot table as name–quantity pairs |
| `std::vector<PendingRespawn>` | `Game.cpp` (local) | Queue of tiles scheduled for delayed re-placement |
| `std::vector<std::string>` | `Monster::getAsciiArt()` | Multi-line ASCII art returned per monster type |
| `std::string` | Throughout | Item names, map rows, UI messages, file paths |

Supporting structs (`Item`, `ItemDef`, `Recipe`, `Ingredient`, `ExitPoint`, `PendingRespawn`, `SaveData`, `PlayerStatus`) work alongside these containers to group related fields.

**Files**: `Inventory.h`, `Map.h`, `Crafting.h`, `Monster.h`, `SaveLoad.h`, `Game.cpp`

---

### Requirement 3 — Dynamic Memory Management

Dynamic allocation is managed implicitly through STL containers whose internal buffers are heap-allocated and automatically resized via RAII:

- **`Map::grid`** (`std::vector<std::string>`) — allocated as the map file is read. Each of the 30 rows is pushed into the vector and then resized to exactly `cols` characters with `grid[r].resize(cols, ' ')` to pad short lines.
- **`Inventory::items`** (`std::vector<Item>`) — starts empty on game start; grows via `push_back` each time an item is collected, and shrinks via `erase` when an item's quantity reaches zero. The inventory can hold an arbitrary number of distinct item stacks.
- **`SaveData::inventoryItems`** (`std::vector<Item>`) — rebuilt from scratch on every load by parsing the save file line by line and calling `push_back` for each entry.
- **`pendingRespawns`** (`std::vector<PendingRespawn>` in `Game.cpp`) — grows each time the player interacts with a respawnable tile (herb, debris, monster) and is pruned each tick as entries come due. Its size is unbounded, allowing many pending respawns simultaneously.
- **Recipe vectors** — `getCraftingRecipes()` and `getCookingRecipes()` construct and return `std::vector<Recipe>` objects (each containing a nested `std::vector<Ingredient>`) by value; Return Value Optimisation (RVO) eliminates unnecessary copies.

No raw `new` or `delete` calls are used; all heap lifetimes are managed by their owning containers.

**Files**: `Map.cpp`, `Inventory.cpp`, `SaveLoad.cpp`, `Game.cpp`, `Crafting.cpp`

---

### Requirement 4 — File Input / Output

**Map loading** (`Map.cpp` — `Map::load()`):

```
data/map_beach.txt
data/map_cave.txt
data/map_forest.txt
data/map_village.txt
data/map_shore.txt
```

- Opens each file with `std::ifstream`.
- First line parsed for map dimensions (`rows cols`).
- Remaining lines read with `std::getline` into `Map::grid`; rows shorter than `cols` are padded.
- `@` spawn markers are recorded (stored as `spawnRow`/`spawnCol`) then replaced with a blank tile so they don't appear during play.
- Exit tiles (`E`, `<`, `>`, `^`, `v`) are registered into `Map::exitPoints` while scanning the grid.

**Save / Load** (`SaveLoad.cpp`):

- `saveGame()` writes a compact line-per-field plain-text record to `data/save.txt` using `std::ofstream`. Fields written in order: position, map ID, HP/maxHp, hunger/thirst/fatigue, day/hour, move count, emoji-mode flag, item count, and one `"<qty> <name>"` line per inventory item.
- `loadGame()` reads the file with `std::ifstream` and a custom `readLine()` helper that binds each line to a fresh `std::istringstream` for safe per-field token extraction without cross-line bleed.
- `hasSaveFile()` performs a lightweight parse attempt and returns a `bool` — used by the main menu to decide whether to offer the **Continue** option.

**Files**: `Map.cpp`, `SaveLoad.cpp`, `SaveLoad.h`

---

### Requirement 5 — Program Code in Multiple Files

The project is split across **14 compilation units**, each paired with a header that exposes only its public interface. All headers use `#ifndef` include guards.

| File pair | Responsibility |
|-----------|---------------|
| `main.cpp` | Entry point; initialises the UTF-8 console (Windows), runs the title screen and main-menu loop |
| `Game.h / Game.cpp` | Core game loop: map rendering, player input, per-step tick, win/lose detection |
| `Map.h / Map.cpp` | Map loading, ASCII & emoji rendering, walkability checks, exit detection, `MapManager` |
| `Player.h / Player.cpp` | Player stats (HP, hunger, thirst, fatigue), position, movement, needs-decay |
| `Inventory.h / Inventory.cpp` | Item storage: add, consume, remove, sort, equip operations |
| `Item.h / Item.cpp` | Static item registry (`ItemDef` array) and name-based lookup |
| `Crafting.h / Crafting.cpp` | Recipe lists, craft-eligibility check, craft execution, paginated crafting/cooking UI |
| `Combat.h / Combat.cpp` | Turn-based combat loop, flee mechanic, poison, difficulty scaling |
| `Monster.h / Monster.cpp` | Monster stats, ASCII art, loot tables, map-aware spawn-pool selection |
| `Fishing.h / Fishing.cpp` | Timing-based fishing minigame with animated bar and zone-based loot |
| `SaveLoad.h / SaveLoad.cpp` | Game-state serialisation to and from `data/save.txt` |
| `InventoryUI.h / InventoryUI.cpp` | Inventory screen rendering and input handling |
| `UI.h / UI.cpp` | Shared layout helpers: centering, padding, title/menu screens, difficulty picker |
| `status_ui.h / status_ui.cpp` | HUD top bar: clock, time-of-day, location label, stat display |

The `Makefile` compiles each `.cpp` to its own `.o` independently, so only modified files are recompiled on subsequent builds.

---

## Non-Standard Libraries

**None.** The game uses only the ISO C++11 standard library.

Two platform system headers are conditionally included for low-level terminal I/O in the fishing minigame and title screen:

| Header | Platform | Used for |
|--------|----------|---------|
| `<windows.h>`, `<conio.h>` | Windows | `_kbhit()` / `_getch()` for non-blocking key reads; `SetConsoleOutputCP(CP_UTF8)` for emoji display |
| `<unistd.h>`, `<termios.h>`, `<sys/select.h>` | macOS / Linux | `usleep()` for frame timing; `select()` for non-blocking stdin polling |

These are OS system headers included with the compiler toolchain — no third-party installation is required.

---

## Compilation & Execution

### Requirements

- `g++` with C++11 support
- `make`
- A terminal that supports ANSI escape codes (macOS Terminal, any Linux terminal emulator, or Windows Terminal)

### Build

```bash
git clone <repo-url>
cd Group_212
make
```

Compiles all 14 source files and links them into the `game` executable. Object files are placed in `src/`.

### Run

```bash
./game
```

### Clean

```bash
make clean
```

Removes all `.o` files and the `game` executable.

---

### Quick Start (Step-by-Step)

1. Run `./game` — the title screen appears.
2. Select **1 New Game** from the main menu.
3. Choose a difficulty: **1 Easy**, **2 Normal**, or **3 Hard**.
4. You begin on the **Beach** (Zone 1). Your character is `@`.
5. Move with **W A S D**.
6. Press **E** to interact with adjacent tiles (chop trees, open campfire, search debris).
7. Press **I** to open your inventory — use food, equip tools, or drop excess items.
8. Press **C** near the village workshop to craft tools and raft components.
9. Press **V** to toggle between ASCII and emoji display.
10. Press **Z** inside the cave or village shelter to sleep and restore Fatigue.
11. Press **S** to save your progress at any time; **Q** to quit.
12. Build all Raft components, assemble the Raft at the Shore dock, and escape to win!

### Controls Reference

| Key | Action |
|-----|--------|
| W / A / S / D | Move up / left / down / right |
| E | Interact with adjacent tile |
| I | Open inventory |
| C | Open crafting workshop |
| Z | Sleep (cave interior or village shelter only) |
| V | Toggle emoji / ASCII render mode |
| S | Save game |
| H | Help / Survival Guide |
| Q | Quit |

---

## File Structure

```
Group_212/
├── Makefile
├── README.md
├── data/
│   ├── map_beach.txt       Beach map (30×60)
│   ├── map_cave.txt        Cave map (30×60)
│   ├── map_forest.txt      Forest map (30×60)
│   ├── map_village.txt     Village map (30×60)
│   ├── map_shore.txt       Shore map (30×60)
│   └── save.txt            Save file (created on first save)
└── src/
    ├── main.cpp
    ├── Game.h / Game.cpp
    ├── Map.h / Map.cpp
    ├── Player.h / Player.cpp
    ├── Inventory.h / Inventory.cpp
    ├── Item.h / Item.cpp
    ├── Crafting.h / Crafting.cpp
    ├── Combat.h / Combat.cpp
    ├── Monster.h / Monster.cpp
    ├── Fishing.h / Fishing.cpp
    ├── SaveLoad.h / SaveLoad.cpp
    ├── InventoryUI.h / InventoryUI.cpp
    ├── UI.h / UI.cpp
    ├── status_ui.h / status_ui.cpp
    └── GameState.h
```
