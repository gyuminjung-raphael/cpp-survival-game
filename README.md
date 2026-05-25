# Island Survival — C++ Text-Based Survival Game

A C++11 terminal-based survival game developed as a 4-person university team project.  
The game features multi-map exploration, survival stats, combat, inventory management, crafting, fishing, and save/load functionality.

---

## Team

| Name | GitHub |
|------|--------|
| Yang Heemoon | heemoon72 |
| Kim Jaehan | JAEHAN KIM |
| Oh Philseung | PhilseungOh |
| Jung Gyumin | gyuminjung-raphael |

---

## Team Collaboration

The project was developed by a 4-person team through regular meetings, shared debugging sessions, and Git/GitHub-based collaboration.  
All members contributed to feature development, testing, debugging, and final gameplay refinement.

## My Contributions

- Led the implementation of the inventory system, including inventory UI, screen switching, item dropping, and quantity handling.
- Built and finalized the status UI components for displaying player stats and game state.
- Developed time and movement-based survival mechanics, including night-time stat reduction and movement-count stat decay.
- Improved UI alignment, keyboard controls, help menus, and emoji spacing for better terminal gameplay.
- Contributed to crafting/material structure updates and environmental mechanics such as medical herb regeneration.
- Reviewed gameplay, fixed bugs, and filmed the gameplay demo video.

## Game Description

You are a shipwreck survivor stranded on a remote island. The game is played entirely in the terminal as a top-down ASCII / emoji map. Every step you take costs hunger, thirst, and fatigue. Walking near bushes can trigger animal encounters; surviving combat earns you meat and hides. The island has five connected zones — Beach, Cave, Forest, Village, and Shore — each with unique resources and dangers.

Your goal is to gather enough materials to craft a complete **Raft** and escape. The game tracks an in-game clock (hours and days); needs decay faster at night, and dangerous predators become more common as days pass. Three difficulty modes (Easy, Normal, Hard) scale the speed of decay and the intensity of encounters.

## Gameplay Video

https://youtu.be/GtZDGk9ylIg

## Technical Highlights

- Modular C++11 architecture with 14 source modules and separate header files.
- File I/O for map loading and save/load functionality.
- STL containers such as `std::vector` and `std::string` for inventory, maps, recipes, and game state.
- Random event generation for combat encounters, loot drops, fishing outcomes, and resource respawns.
- Makefile-based compilation workflow.
- No third-party libraries required.
  
### Zones

| # | Zone | Description |
|---|------|-------------|
| 1 | Beach | Starting area. Sandy shore with shipwreck debris, scorpions, and palm trees (Coconut source). |
| 2 | Cave | Rocky cliffs enclosing a cave interior with a campfire — the only shelter for the first two days. |
| 3 | Forest | Dense woods with fresh water, berries, Medical Herbs, and the widest variety of wildlife. |
| 4 | Village | Abandoned settlement with ruins, a shelter (safe to sleep), and a campfire. |
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

**Workshop Crafting** (press **C** to open the crafting menu):
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
The game can be saved and loaded from `data/save.txt`. Saved data includes: player position and map, all four stats, current day/hour, move count, emoji/ASCII mode preference, and the full inventory. Load is available from the main menu; the file is written on manual save (press **O** in-game).

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


## Compilation & Execution

### Requirements

- `g++` with C++11 support
- `make`
- A terminal that supports ANSI escape codes (macOS Terminal, any Linux terminal emulator, or Windows Terminal)

### Build

```bash
git clone <repo-url>
cd cpp-survival-game
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
8. Press **C** to open the crafting menu.
9. Press **V** to toggle between ASCII and emoji display.
10. Press **Z** inside the cave or village shelter to sleep and restore Fatigue.
11. Press **O** to save your progress at any time; **Q** to quit.
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
| O | Save game |
| H | Help / Survival Guide |
| Q | Quit |

---

## File Structure

```
cpp-survival-game/
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
