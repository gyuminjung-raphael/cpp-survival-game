#include "Game.h"
#include "Map.h"
#include "UI.h"
#include "Player.h"
#include "Monster.h"
#include "Combat.h"
#include "status_ui.h"
#include "SaveLoad.h"
#include "GameState.h"
#include "InventoryUI.h"
#include "Crafting.h"
#include "Fishing.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>

// Decay step thresholds: every N steps a stat loses 1 point.
// Lower = faster decay. Scaled by difficulty in runGame().
static const int BASE_HUNGER_STEPS  = 25;
static const int BASE_THIRST_STEPS  = 15;
static const int BASE_FATIGUE_STEPS = 20;

// Map a 24-hour clock into the named period shown on the HUD.
static std::string getTimeOfDay(int hour) {
    if (hour >= 6 && hour < 12) return "Morning";
    if (hour >= 12 && hour < 18) return "Afternoon";
    if (hour >= 18 && hour < 22) return "Evening";
    return "Night";
}

// True during night hours (22:00 - 05:59); needs decay faster at night.
static bool isNightTime(int hour) {
    return (hour >= 22 || hour < 6);
}

// Advance the in-game clock by N hours and roll the day over at midnight.
static void advanceTime(Player& player, int hoursPassed = 1) {
    player.hour += hoursPassed;

    while (player.hour >= 24) {
        player.hour -= 24;
        player.day++;
    }
}

// Apply a fixed needs tick (used after long actions like a combat round).
static void applyNeedsDecay(Player& player) {
    player.applyNeedsLoss(1, 2, 1);
}

// True when the player is inside the village hut interior (safe to sleep).
// Hut interior: rows 21-23, cols 5-17 (0-indexed).
static bool isInsideVillageBuilding(int row, int col) {
    return row >= 21 && row <= 23 && col >= 5 && col <= 17;
}

// True when the player is inside the cliff-enclosed cave interior (safe to sleep).
// Bounds derived from the innermost ^ wall positions in map_cave.txt (rows 6-21).
static bool isInsideCaveArea(int row, int col) {
    if (row < 6 || row > 21) return false;
    // leftWall / rightWall: innermost ^ col for each row (index == row number)
    static const int leftWall[22]  = {0,0,0,0,0,0, 14,12,10,10,10, 9, 9, 9, 9, 9, 9,10,10,11,12,16};
    static const int rightWall[22] = {0,0,0,0,0,0, 33,34,35,35,35,35,35,35,35,35,35,35,35,34,33,27};
    return col > leftWall[row] && col < rightWall[row];
}

struct PendingRespawn {
    int   mapID;
    int   row, col;      // original tile position (erase target)
    char  obj;           // tile char to restore
    bool  samePosition;  // true = respawn at (row,col); false = random spot
    int   dueAbsHour;    // day*24+hour when respawn fires
};

// Each game tick: fire any pending respawns whose due-time has arrived,
// re-placing the original tile char at the original spot or a random one.
static void processPendingRespawns(std::vector<PendingRespawn>& pending,
                                   Map* currentMap, int currentMapID,
                                   int absHour) {
    for (auto it = pending.begin(); it != pending.end(); ) {
        if (it->mapID != currentMapID || absHour < it->dueAbsHour) {
            ++it; continue;
        }
        if (it->samePosition) {
            // Grass: restore in place only if tile is still empty
            if (currentMap->getTile(it->row, it->col) == ' ')
                currentMap->setTile(it->row, it->col, it->obj);
        } else if (it->obj == 'G') {
            // Debris: random open interior tile within village bounds (rows 2-29, cols 1-58)
            std::vector<std::pair<int,int>> cands;
            for (int r = 2; r <= 29; r++)
                for (int c = 1; c <= 58; c++) {
                    char t = currentMap->getTile(r, c);
                    if (t == ' ' || t == ',')
                        cands.push_back({r, c});
                }
            if (!cands.empty()) {
                auto& p = cands[rand() % (int)cands.size()];
                currentMap->setTile(p.first, p.second, it->obj);
            }
        } else if ((it->obj == 'M' || it->obj == 'H') && currentMapID == MAP_FOREST) {
            // Medical Herb respawn: random forest-only grass/open tile.
            // This works like trees/palms/rocks: the herb does not return to
            // the exact same cell; it reappears somewhere suitable on the map.
            std::vector<std::pair<int,int>> cands;
            int rows = currentMap->getRows(), cols = currentMap->getCols();
            for (int r = 1; r < rows - 1; r++)
                for (int c = 1; c < cols - 1; c++) {
                    char t = currentMap->getTile(r, c);
                    if (t == ' ' || t == ',')
                        cands.push_back({r, c});
                }
            if (!cands.empty()) {
                auto& p = cands[rand() % (int)cands.size()];
                currentMap->setTile(p.first, p.second, it->obj);
            }
        } else {
            // Tree / Palm / Rock: random open tile
            std::vector<std::pair<int,int>> cands;
            int rows = currentMap->getRows(), cols = currentMap->getCols();
            for (int r = 1; r < rows - 1; r++)
                for (int c = 1; c < cols - 1; c++) {
                    char t = currentMap->getTile(r, c);
                    if (t == ' ' || t == ',' || t == '.')
                        cands.push_back({r, c});
                }
            if (!cands.empty()) {
                auto& p = cands[rand() % (int)cands.size()];
                currentMap->setTile(p.first, p.second, it->obj);
            }
        }
        it = pending.erase(it);
    }
}

// Clear the tile at (ar,ac) now and schedule it to respawn after delayHours.
static void queueRespawn(std::vector<PendingRespawn>& pending,
                         Map* map, int mapID,
                         int ar, int ac, char obj,
                         bool samePosition, int currentAbsHour, int delayHours) {
    map->setTile(ar, ac, ' ');
    PendingRespawn pr;
    pr.mapID        = mapID;
    pr.row          = ar;
    pr.col          = ac;
    pr.obj          = obj;
    pr.samePosition = samePosition;
    pr.dueAbsHour   = currentAbsHour + delayHours;
    pending.push_back(pr);
}

// Look at the four neighbours of (row,col); return the first interactable
// tile char found (T/P/^/C/G), or '\0' if none.
static char getAdjacentInteractable(const Map* map, int row, int col) {
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    for (int i = 0; i < 4; i++) {
        char t = map->getTile(row + dr[i], col + dc[i]);
        if (t == 'T' || t == 'P' || t == '^' || t == 'C' || t == 'G')
            return t;
    }
    return '\0';
}

// True if any of the four neighbours of (row,col) is a water tile ('~').
static bool isAdjacentToWater(const Map* map, int row, int col) {
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    for (int i = 0; i < 4; i++) {
        if (map->getTile(row + dr[i], col + dc[i]) == '~')
            return true;
    }
    return false;
}

// Render the in-game help / survival guide screen (controls, legend, tips).
static void displayHelpScreen() {
    clearScreen();

    const int W = 68;
    printVerticalPadding(36);

    printBlockLine("================================================================", W);
    printBlockLine("                       SURVIVAL GUIDE                          ", W);
    printBlockLine("================================================================", W);
    std::cout << '\n';

    // ── Section 1: Controls ───────────────────────────────────────
    printBlockLine("[ CONTROLS ]", W);
    printBlockLine("----------------------------------------------------------------", W);
    printBlockLine("  WASD     Move around the island", W);
    printBlockLine("  E        Interact with resources, campfires, or water", W);
    printBlockLine("  I        Open inventory — use or equip items", W);
    printBlockLine("  C        Open crafting menu", W);
    printBlockLine("  Z        Sleep (inside cave or village hut) to restore Fatigue", W);
    printBlockLine("  V        Toggle emoji / ASCII display mode", W);
    printBlockLine("  O        Save game", W);
    printBlockLine("  Q        Quit game", W);
    std::cout << '\n';

    // ── Section 2: Map Legend (two columns) ──────────────────────
    printBlockLine("[ MAP LEGEND ]", W);
    printBlockLine("----------------------------------------------------------------", W);
    {
        auto row = [&](const std::string& l, const std::string& r) {
            std::string line = "  " + l;
            line += std::string(32 - (int)l.size(), ' ');
            line += r;
            printBlockLine(line, W);
        };
        row("@   Player",        "C   Campfire");
        row("~   Water",         "M   Medical Herb");
        row("T   Tree",          "G   Debris / Ruins");
        row("P   Palm",          "E   Exit");
        row("^   Rock / Cliff",  ".   Sand");
        row(",   Grass",         "#   Wall");
    }
    std::cout << '\n';

    // ── Section 3: Survival ───────────────────────────────────────
    printBlockLine("[ SURVIVAL ]", W);
    printBlockLine("----------------------------------------------------------------", W);
    printBlockLine("  Keep Hunger, Thirst, and Fatigue above 0.", W);
    printBlockLine("  If Hunger or Thirst hits 0, HP starts dropping.", W);
    printBlockLine("  Needs decay faster at night — avoid wandering after dark.", W);
    printBlockLine("  Bushy grass areas can trigger monster encounters as days pass.", W);
    printBlockLine("  The Village unlocks on Day 3.", W);
    printBlockLine("  To escape, use the Raft at the Shore next to the sea.", W);
    printBlockLine("  Bears are STRONG — prepare well before engaging!", W);
    std::cout << '\n';

    // ── Section 4: Gathering & Food ──────────────────────────────
    printBlockLine("[ GATHERING & FOOD ]", W);
    printBlockLine("----------------------------------------------------------------", W);
    printBlockLine("  Trees and Palms give Wood.  Palms also drop Coconuts.", W);
    printBlockLine("  Rocks give Stone and sometimes Flint.  Cave walls can't be mined.", W);
    printBlockLine("  Forage Grass tiles (E) to get Grass, with a 5% chance for Medical Herb.", W);
    printBlockLine("  Walk through grass for a small chance to find Berries.", W);
    printBlockLine("  Campfires let you cook raw food into stronger food.", W);
    printBlockLine("  Soup restores Hunger +20, Thirst +20, and HP +10.", W);
    printBlockLine("  Medical Herb heals +5 HP when used from inventory.", W);
    std::cout << '\n';

    // ── Section 5: Tools & Fishing ───────────────────────────────
    printBlockLine("[ TOOLS & FISHING ]", W);
    printBlockLine("----------------------------------------------------------------", W);
    printBlockLine("  Axe       Doubles Wood from trees and palms.", W);
    printBlockLine("  Pickaxe   Doubles Stone and raises Flint drop chance to 30%.", W);
    printBlockLine("  Knife     Grants +5 bonus damage in combat.", W);
    printBlockLine("  Torch     Prevents monster encounters while equipped.", W);
    printBlockLine("  Fishing Rod + Bait needed to fish near water (E key).", W);
    printBlockLine("  Bait is crafted from Berries (1 Berry = 2 Bait).", W);
    std::cout << '\n';

    printBlockLine("================================================================", W);
    printBlockLine("  Press any key to return...", W);
    printBlockLine("================================================================", W);
    getInput();
}

// Main gameplay loop. Loads maps, restores state from save (if requested),
// then drives the world / inventory / crafting / cooking screens until the
// player wins, dies, or quits.
void runGame(bool loadFromSave, Difficulty diff) {
    srand((unsigned)time(NULL)); // seed RNG once per game session

    MapManager mgr;

    if (!mgr.loadAllMaps("data")) {
        std::cerr << "Failed to load maps." << std::endl;
        return;
    }

    SaveData saveData;
    bool loaded = loadFromSave && loadGame(saveData);

    MapID startMap = loaded ? (MapID)saveData.currentMapID : MAP_BEACH;
    mgr.switchMap(startMap);
    Map* currentMap = mgr.getCurrentMap();

    Player player;
    int playerRow, playerCol;
    int moveCount = 0;

    if (loaded) {
        playerRow      = saveData.playerRow;
        playerCol      = saveData.playerCol;
        player.hp      = saveData.hp;
        player.maxHp   = saveData.maxHp;
        player.hunger  = saveData.hunger;
        player.thirst  = saveData.thirst;
        player.fatigue = saveData.fatigue;
        player.day     = saveData.day;
        player.hour    = saveData.hour;
        moveCount      = saveData.moveCount;
        for (const auto& item : saveData.inventoryItems)
            player.inventory.addItem(item.name, item.quantity);
    } else {
        currentMap->getSpawnPoint(playerRow, playerCol);
    }
    player.setPosition(playerRow, playerCol);

    bool running   = true;
    bool playerWon = false;
    bool emojiMode = loaded ? saveData.emojiMode : false;
    bool showSaveMsg = false;

    bool showRaftHint = false;
    std::string raftHintMessage = "";

    // Village (map 4) unlocks on day 3; skip popup if already past day 3
    bool villageUnlocked  = (player.day >= 3);
    bool shownDay3Message = (player.day >= 3);

    GameScreen currentScreen = SCREEN_WORLD;
    int selectedIndex = 0;      // inventory selection
    int selectedCraftIndex = 0; // selection within current crafting page
    int craftPageIndex = 0;     // current crafting/cooking page

    std::string interactMsg = ""; // one-frame message shown on world screen
    std::string craftMsg    = ""; // persists in crafting/cooking screen

    std::vector<Recipe> craftingRecipes = getCraftingRecipes();
    std::vector<Recipe> cookingRecipes  = getCookingRecipes();

    std::vector<PendingRespawn> pendingRespawns;

    bool dropMode = false;
    int dropAmount = 1;

    // Real-time clock: 1 real minute = 1 in-game hour
    auto lastHourTick = std::chrono::steady_clock::now();

    // Decay thresholds scaled by difficulty:
    //   Easy  → ×1.5 (steps needed, so slower decay)
    //   Normal → ×1.0
    //   Hard  → ×0.6 (fewer steps needed, so faster decay)
    int hungerSteps, thirstSteps, fatigueSteps;
    if (diff == DIFF_EASY) {
        hungerSteps  = BASE_HUNGER_STEPS  * 3 / 2;
        thirstSteps  = BASE_THIRST_STEPS  * 3 / 2;
        fatigueSteps = BASE_FATIGUE_STEPS * 3 / 2;
    } else if (diff == DIFF_HARD) {
        hungerSteps  = BASE_HUNGER_STEPS  * 6 / 10;
        thirstSteps  = BASE_THIRST_STEPS  * 6 / 10;
        fatigueSteps = BASE_FATIGUE_STEPS * 6 / 10;
    } else {
        hungerSteps  = BASE_HUNGER_STEPS;
        thirstSteps  = BASE_THIRST_STEPS;
        fatigueSteps = BASE_FATIGUE_STEPS;
    }

    int stepCount = 0;

    while (running) {
        // ── Advance in-game time (30 sec real = 1 hour game) ────────
        auto now = std::chrono::steady_clock::now();
        int elapsedHours = (int)std::chrono::duration_cast<std::chrono::seconds>(
                               now - lastHourTick).count() / 30;
        if (elapsedHours > 0) {
            lastHourTick += std::chrono::seconds(elapsedHours * 30);
            player.hour  += elapsedHours;
            while (player.hour >= 24) {
                player.hour -= 24;
                player.day++;
            }
        }

        // ── Deferred respawns ────────────────────────────────────────
        processPendingRespawns(pendingRespawns, currentMap,
                               mgr.getCurrentMapID(),
                               player.day * 24 + player.hour);

        // ── Lose check ──────────────────────────────────────────────
        if (player.hp <= 0) {
            running = false;
            playerWon = false;
            break;
        }

        // ── Day 3: unlock village ────────────────────────────────────
        if (player.day >= 3 && !villageUnlocked)
            villageUnlocked = true;

        // ── Day 3: one-time message ──────────────────────────────────
        if (player.day >= 3 && !shownDay3Message) {
            shownDay3Message = true;
            clearScreen();
            printVerticalPadding(9);
            printCentered("====================================================");
            printCentered("               ** DAY 3 ARRIVED **                 ");
            printCentered("====================================================");
            std::cout << '\n';
            printCentered("You spot signs of civilization in the distance.");
            printCentered("The Village is now accessible!");
            printCentered("Head there - survivors may be waiting.");
            std::cout << '\n';
            printCentered("Press any key to continue...");
            getInput();
        }

        clearScreen();

        // ── WORLD ──────────────────────────────────────────────────────
        if (currentScreen == SCREEN_WORLD) {
            // topbar(3) + blank(1) + map(25) + sep(1) + msg(1) + sep(1) + bars(4) + blank(2) + controls(1) = 39
            printVerticalPadding(39);

            // Build legend for top bar
            std::string legend;
            {
                MapID mid = mgr.getCurrentMapID();
                if (mid > 0)
                    legend += std::string("< ") + getMapName((MapID)(mid - 1));
                if (mid > 0 && mid < MAP_COUNT - 1)
                    legend += "   ";
                if (mid < MAP_COUNT - 1)
                    legend += std::string(getMapName((MapID)(mid + 1))) + " >";
            }

            // ── Top bar: === / Day | clock | time-of-day | location  legend / ===
            displayTopBar(player.day, formatClock(player.day, player.hour),
                          getTimeOfDay(player.hour),
                          getMapName(mgr.getCurrentMapID()), legend);
            std::cout << '\n';

            // ── Map ──────────────────────────────────────────────────
            if (emojiMode)
                currentMap->renderEmoji(playerRow, playerCol);
            else
                currentMap->render(playerRow, playerCol);

            // ── Message box: === / messages / === ────────────────────
            displaySep();

            if (showSaveMsg) {
                printCentered("[Game saved!]");
                showSaveMsg = false;
            }
            if (!interactMsg.empty()) {
                printCentered(interactMsg);
                interactMsg = "";
            }
            if (!player.equippedItem.empty())
                printCentered(std::string("Equipped: ") + player.equippedItem);

            char nearby = getAdjacentInteractable(currentMap, playerRow, playerCol);
            if (nearby == 'C')
                printCentered("[Campfire nearby - press E to cook]");
            else if (nearby != '\0')
                printCentered("[Press E to interact]");

            if (player.equippedItem == "Fishing Rod") {
                const int fdr[] = {-1, 1, 0, 0};
                const int fdc[] = {0, 0, -1, 1};
                for (int fi = 0; fi < 4; fi++) {
                    if (currentMap->getTile(playerRow + fdr[fi], playerCol + fdc[fi]) == '~') {
                        printCentered("[Fishing Rod - press E to fish]");
                        break;
                    }
                }
            }

            displaySep();

            // ── Stat bars ────────────────────────────────────────────
            PlayerStatus stats = {player.hp, player.hunger, player.thirst, player.fatigue};
            displayStatBars(stats);

            std::cout << '\n';
            std::string controls = "[WASD] Move  [E] Interact  [C] Craft  [I] Inventory  [H] Help  [V] Emoji  [O] Save  [Q] Quit";
            bool insideCave = (mgr.getCurrentMapID() == MAP_CAVE &&
                               isInsideCaveArea(playerRow, playerCol));
            bool insideVillageHut = (mgr.getCurrentMapID() == MAP_VILLAGE &&
                                     isInsideVillageBuilding(playerRow, playerCol));
            if (insideCave || insideVillageHut)
                controls += "  [Z] Sleep";
            printCentered(controls);
        }
        // ── INVENTORY ──────────────────────────────────────────────────
        else if (currentScreen == SCREEN_INVENTORY) {
            printVerticalPadding(12);
            PlayerStatus invStats = {player.hp, player.hunger, player.thirst, player.fatigue};

            renderInventory(player.inventory, selectedIndex, dropMode, dropAmount,
                            invStats, player.day, getTimeOfDay(player.hour),
                            getMapName(mgr.getCurrentMapID()), player.equippedItem,
                            showRaftHint, raftHintMessage);
        }
        // ── CRAFTING ───────────────────────────────────────────────────
        else if (currentScreen == SCREEN_CRAFTING) {
            renderCraftingUI(craftingRecipes, player.inventory, selectedCraftIndex, craftPageIndex, "CRAFTING", craftMsg);
        }
        // ── COOKING ────────────────────────────────────────────────────
        else if (currentScreen == SCREEN_COOKING) {
            renderCraftingUI(cookingRecipes, player.inventory, selectedCraftIndex, craftPageIndex, "COOKING", craftMsg);
        }

        char input = getInput();

        // ── WORLD INPUT ────────────────────────────────────────────────
        if (currentScreen == SCREEN_WORLD) {
            int nr = playerRow, nc = playerCol;

            switch (input) {
                case 'w': case 'W': nr--; break;
                case 's': case 'S': nr++; break;
                case 'a': case 'A': nc--; break;
                case 'd': case 'D': nc++; break;

                case 'e': case 'E': {
                    bool interacted = false;

                    char ownTile = currentMap->getTile(playerRow, playerCol);
                    if (!interacted && ownTile == ',') {
                        player.inventory.addItem("Grass", 1);
                        if (rand() % 100 < 5) {
                            player.inventory.addItem("Medical Herb", 1);
                            interactMsg = "[Foraged - 1 Grass + 1 Medical Herb!]";
                        } else {
                            interactMsg = "[Foraged - got 1 Grass]";
                        }
                        queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                     playerRow, playerCol, ',',
                                     true, player.day * 24 + player.hour, 3);
                        interacted = true;
                    }

                    if (!interacted) {
                        const int dr[] = {-1, 1, 0, 0};
                        const int dc[] = {0, 0, -1, 1};
                        for (int i = 0; i < 4 && !interacted; i++) {
                            int ar = playerRow + dr[i];
                            int ac = playerCol + dc[i];
                            char tile = currentMap->getTile(ar, ac);
                            switch (tile) {
                                case 'T': {
                                    bool hasAxe = (player.equippedItem == "Axe");
                                    int wood = hasAxe ? 2 : 1;
                                    player.inventory.addItem("Wood", wood);
                                    interactMsg = hasAxe
                                        ? "[Axe - chopped tree: 2 Wood]"
                                        : "[Chopped tree - 1 Wood]";
                                    queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                                 ar, ac, 'T',
                                                 false, player.day * 24 + player.hour, 12);
                                    interacted = true;
                                    break;
                                }
                                case 'P': {
                                    bool hasAxe = (player.equippedItem == "Axe");
                                    int wood = hasAxe ? 2 : 1;
                                    player.inventory.addItem("Wood", wood);
                                    player.inventory.addItem("Coconut", 1);
                                    interactMsg = hasAxe
                                        ? "[Axe - palm: 2 Wood, 1 Coconut]"
                                        : "[Palm - 1 Wood, 1 Coconut]";
                                    queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                                 ar, ac, 'P',
                                                 false, player.day * 24 + player.hour, 12);
                                    interacted = true;
                                    break;
                                }
                                case '^': {
                                    if (mgr.getCurrentMapID() == MAP_CAVE) {
                                        interactMsg = "[These cave walls cannot be mined]";
                                        interacted = true;
                                        break;
                                    }
                                    bool hasPick = (player.equippedItem == "Pickaxe");
                                    int stone = hasPick ? 2 : 1;
                                    int flintChance = hasPick ? 30 : 15;
                                    player.inventory.addItem("Stone", stone);
                                    if (rand() % 100 < flintChance) {
                                        player.inventory.addItem("Flint", 1);
                                        interactMsg = "[Mined - " + std::to_string(stone) + " Stone + 1 Flint]";
                                    } else {
                                        interactMsg = "[Mined - " + std::to_string(stone) + " Stone]";
                                    }
                                    queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                                 ar, ac, '^',
                                                 false, player.day * 24 + player.hour, 15);
                                    interacted = true;
                                    break;
                                }
                                case 'G': {
                                    int roll = rand() % 100;
                                    if (roll < 60 || roll >= 95) {
                                        interactMsg = "[You rummage through the debris... nothing useful.]";
                                    } else if (roll < 70) {
                                        player.inventory.addItem("Murky Vial", 1);
                                        interactMsg = "[Found a Murky Vial!]";
                                    } else if (roll < 85) {
                                        player.inventory.addItem("Dried Entrails", 1);
                                        interactMsg = "[Found Dried Entrails — odd, but might be useful.]";
                                    } else {
                                        player.inventory.addItem("Resin", 1);
                                        interactMsg = "[Found 1 Resin in the debris.]";
                                    }
                                    queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                                 ar, ac, 'G', false,
                                                 player.day * 24 + player.hour, 12);
                                    currentMap->setTile(ar, ac, ' ');
                                    interacted = true;
                                    break;
                                }
                                case 'C':
                                    currentScreen = SCREEN_COOKING;
                                    selectedCraftIndex = 0;
                                    craftPageIndex = 0;
                                    craftMsg = "";
                                    interacted = true;
                                    break;
                                case '~':
                                    if (player.equippedItem == "Fishing Rod") {
                                        startFishing(player);
                                        interacted = true;
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                    }

                    if (!interacted) {
                        interactMsg = "[Nothing to interact with nearby]";
                    } else {
                        // Physical work costs energy
                        player.applyNeedsLoss(1, 1, 2); // hunger -1, thirst -1, fatigue -2
                    }
                    continue;
                }

                case 'c': case 'C':
                    currentScreen = SCREEN_CRAFTING;
                    selectedCraftIndex = 0;
                    craftPageIndex = 0;
                    craftMsg = "";
                    continue;

                case 'v': case 'V':
                    emojiMode = !emojiMode;
                    continue;

                case 'h': case 'H':
                    displayHelpScreen();
                    continue;

                case 'i': case 'I':
                    currentScreen = SCREEN_INVENTORY;
                    selectedIndex = 0;
                    dropMode = false;
                    dropAmount = 1;
                    showRaftHint = false;
                    raftHintMessage = "";
                    continue;

                // ── Sleep (cave interior or village hut) ──────────
                case 'z': case 'Z': {
                    bool canSleep = (mgr.getCurrentMapID() == MAP_CAVE &&
                                     isInsideCaveArea(playerRow, playerCol))
                                 || (mgr.getCurrentMapID() == MAP_VILLAGE &&
                                     isInsideVillageBuilding(playerRow, playerCol));
                    if (!canSleep) {
                        clearScreen();
                        printVerticalPadding(9);
                        printCentered("============================================");
                        printCentered("  Find shelter first — cave interior or the village hut.");
                        printCentered("============================================");
                        std::cout << '\n';
                        printCentered("Press any key...");
                        getInput();
                    } else {
                        bool starvedOrDehydrated = (player.hunger == 0 || player.thirst == 0);
                        player.hour += 12;
                        while (player.hour >= 24) {
                            player.hour -= 24;
                            player.day++;
                        }
                        player.fatigue = 100;
                        player.thirst  = std::max(0, player.thirst - 10);
                        player.hunger  = std::max(0, player.hunger  - 15);
                        if (starvedOrDehydrated)
                            player.hp = std::max(0, player.hp - 50);
                        clearScreen();
                        printVerticalPadding(9);
                        printCentered("============================================");
                        printCentered("                ** RESTING **              ");
                        printCentered("============================================");
                        std::cout << '\n';
                        printCentered("You rest in the cave...");
                        if (starvedOrDehydrated) {
                            printCentered("You wake up weak — your body suffered overnight.");
                            printCentered("HP -50!");
                        } else {
                            printCentered("You wake up feeling refreshed.");
                        }
                        printCentered(formatClock(player.day, player.hour));
                        std::cout << '\n';
                        printCentered("Press any key to continue...");
                        getInput();
                    }
                    continue;
                }

                case 'o': case 'O': {
                    SaveData sd;
                    sd.playerRow    = playerRow;
                    sd.playerCol    = playerCol;
                    sd.currentMapID = (int)mgr.getCurrentMapID();
                    sd.hp           = player.hp;
                    sd.maxHp        = player.maxHp;
                    sd.hunger       = player.hunger;
                    sd.thirst       = player.thirst;
                    sd.fatigue      = player.fatigue;
                    sd.day          = player.day;
                    sd.hour         = player.hour;
                    sd.moveCount    = moveCount;
                    sd.emojiMode    = emojiMode;
                    for (int i = 0; i < player.inventory.size(); i++)
                        sd.inventoryItems.push_back(player.inventory.getItem(i));
                    showSaveMsg = saveGame(sd);
                    continue;
                }

                case 'q': case 'Q': {
                    int quitSel = 0; // 0 = Save & Quit, 1 = Quit
                    while (true) {
                        clearScreen();
                        printVerticalPadding(9);
                        printCentered("============================================");
                        printCentered("       Warning: Unsaved progress will be lost!");
                        printCentered("============================================");
                        std::cout << '\n';
                        printCentered(
                            std::string(quitSel == 0 ? "[ > Save and Quit ]" : "[   Save and Quit ]")
                            + "      "
                            + std::string(quitSel == 1 ? "[ > Quit without Saving ]" : "[   Quit without Saving ]")
                        );
                        std::cout << '\n';
                        printCentered("[A/D] Select   [Enter] Confirm");
                        char qc = getInput();
                        if (qc == 'a' || qc == 'A') quitSel = 0;
                        else if (qc == 'd' || qc == 'D') quitSel = 1;
                        else if (qc == '\r' || qc == '\n') {
                            if (quitSel == 0) {
                                SaveData sd;
                                sd.playerRow    = playerRow;
                                sd.playerCol    = playerCol;
                                sd.currentMapID = (int)mgr.getCurrentMapID();
                                sd.hp           = player.hp;
                                sd.maxHp        = player.maxHp;
                                sd.hunger       = player.hunger;
                                sd.thirst       = player.thirst;
                                sd.fatigue      = player.fatigue;
                                sd.day          = player.day;
                                sd.hour         = player.hour;
                                sd.moveCount    = moveCount;
                                sd.emojiMode    = emojiMode;
                                for (int i = 0; i < player.inventory.size(); i++)
                                    sd.inventoryItems.push_back(player.inventory.getItem(i));
                                saveGame(sd);
                            }
                            running = false;
                            break;
                        }
                    }
                    continue;
                }

                default:
                    continue;
            }

            if (currentMap->isWalkable(nr, nc)) {
                playerRow = nr;
                playerCol = nc;
                player.setPosition(playerRow, playerCol);

                // Per-step needs decay — thresholds scaled by difficulty,
                // and ~60% faster at night.
                stepCount++;
                {
                    bool nightNow = isNightTime(player.hour);
                    int hTick = nightNow ? hungerSteps  * 3 / 5 : hungerSteps;
                    int tTick = nightNow ? thirstSteps  * 2 / 3 : thirstSteps;
                    int fTick = nightNow ? fatigueSteps * 3 / 5 : fatigueSteps;
                    if (hTick < 1) hTick = 1;
                    if (tTick < 1) tTick = 1;
                    if (fTick < 1) fTick = 1;
                    int hLoss = (stepCount % hTick == 0) ? 1 : 0;
                    int tLoss = (stepCount % tTick == 0) ? 1 : 0;
                    int fLoss = (stepCount % fTick == 0) ? 1 : 0;
                    if (hLoss || tLoss || fLoss)
                        player.applyNeedsLoss(hLoss, tLoss, fLoss);
                }

                char steppedTile = currentMap->getTile(playerRow, playerCol);
                if (steppedTile == 'M' || steppedTile == 'H') {
                    player.inventory.addItem("Medical Herb", 1);
                    interactMsg = "[Picked up 1 Medical Herb]";

                    // Replace with whichever terrain the herb was growing
                    // on (inferred from adjacent tiles). Default to grass
                    // if no clear neighbor.
                    char replacement = ',';
                    const int ndr[] = {-1, 1, 0, 0};
                    const int ndc[] = {0, 0, -1, 1};
                    for (int ni = 0; ni < 4; ni++) {
                        char n = currentMap->getTile(playerRow + ndr[ni],
                                                     playerCol + ndc[ni]);
                        if (n == '~' || n == '.' || n == ',' || n == ':' ||
                            n == ' ') {
                            replacement = n;
                            break;
                        }
                    }
                    // Forest herbs respawn after the same 12-hour cooldown used
                    // by trees and palms, but at a random suitable forest tile.
                    if (mgr.getCurrentMapID() == MAP_FOREST) {
                        queueRespawn(pendingRespawns, currentMap, mgr.getCurrentMapID(),
                                     playerRow, playerCol, steppedTile,
                                     false, player.day * 24 + player.hour, 12);
                    }
                    currentMap->setTile(playerRow, playerCol, replacement);
                } else if (steppedTile == ',' && rand() % 100 < 5) {
                    // 5% chance to forage berries while walking through grass
                    player.inventory.addItem("Berry", 1);
                    interactMsg = "[You found a berry in the grass! +1 Berry]";
                }

                char here = currentMap->getTile(playerRow, playerCol);
                if (here == 'E' || here == '<' || here == '>' ||
                    here == '^' || here == 'v') {
                    int dr, dc;
                    int dest = currentMap->checkExit(playerRow, playerCol, dr, dc);

                    if (dest == MAP_VILLAGE && !villageUnlocked) {
                        interactMsg = "[The village path is blocked — unlocks on Day 3.]";
                    } else if (dest >= 0 && dest < MAP_COUNT) {
                        mgr.switchMap((MapID)dest);
                        currentMap = mgr.getCurrentMap();
                        playerRow = dr;
                        playerCol = dc;
                        player.setPosition(playerRow, playerCol);
                    }
                }

                // Random encounter when moving through / around bush (',') tiles.
                bool nearBush = (here == ',');
                if (!nearBush) {
                    const int bdr[] = {-1, 1, 0, 0};
                    const int bdc[] = {0, 0, -1, 1};
                    for (int bi = 0; bi < 4; bi++) {
                        if (currentMap->getTile(playerRow + bdr[bi],
                                                playerCol + bdc[bi]) == ',') {
                            nearBush = true;
                            break;
                        }
                    }
                }

                // Encounter chance offset: Easy -10%. Hard keeps Normal's
                // spawn rate but hits harder (see Combat.cpp difficulty mults).
                int encounterOffset = (diff == DIFF_EASY) ? -10 : 0;
                int encounterChance = bushEncounterChancePercent(player.day) + encounterOffset;
                if (encounterChance < 1) encounterChance = 1;
                // Village is a safe zone — no random monster encounters.
                bool encountersDisabled = (mgr.getCurrentMapID() == MAP_VILLAGE);
                if (nearBush && !encountersDisabled &&
                    player.equippedItem != "Torch" &&
                    (rand() % 100) < encounterChance) {
                    Monster mon = spawnRandomMonster(
                        (int)mgr.getCurrentMapID(), player.day);
                    CombatResult result = startCombat(player, mon, diff);

                    advanceTime(player, 1);
                    applyNeedsDecay(player);

                    if (result == COMBAT_LOSE || player.hp <= 0) {
                        displayGameOver(false);
                        std::string hint = "Press any key to return to menu...";
                        std::cout << leftPad(displayWidthUtf8(hint)) << hint;
                        getInput();
                        running = false;
                    }
                }
            }
        }
        // ── INVENTORY INPUT ────────────────────────────────────────────
        else if (currentScreen == SCREEN_INVENTORY) {
            handleInventoryInput(
                input,
                player,
                selectedIndex,
                currentScreen,
                dropMode,
                dropAmount,
                running,
                playerWon,
                mgr.getCurrentMapID(),
                isAdjacentToWater(currentMap, playerRow, playerCol),
                showRaftHint,
                raftHintMessage
            );
        }
        // ── CRAFTING INPUT ─────────────────────────────────────────────
        else if (currentScreen == SCREEN_CRAFTING) {
            handleCraftingInput(input, craftingRecipes, player.inventory,
                                selectedCraftIndex, craftPageIndex, currentScreen, SCREEN_WORLD, craftMsg);
        }
        // ── COOKING INPUT ──────────────────────────────────────────────
        else if (currentScreen == SCREEN_COOKING) {
            handleCraftingInput(input, cookingRecipes, player.inventory,
                                selectedCraftIndex, craftPageIndex, currentScreen, SCREEN_WORLD, craftMsg);
        }
    }

    // ── End screen ──────────────────────────────────────────────────
    if (player.hp <= 0 || playerWon) {
        displayGameOver(playerWon);
        std::string hint = "Press Enter to return to menu...";
        std::cout << leftPad(displayWidthUtf8(hint)) << hint;
        std::cin.ignore(10000, '\n');
    } else {
        clearScreen();
        printCentered("Bye!");
    }
}