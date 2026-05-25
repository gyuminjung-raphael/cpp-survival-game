#include "Combat.h"
#include "Item.h"
#include "UI.h"
#include "status_ui.h"
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <iomanip>

static const int BOX_WIDTH = 70;
static const int COMBAT_HUNGER_COST = 1;
static const int COMBAT_THIRST_COST = 2;
static const int COMBAT_FATIGUE_COST = 3;

// Left-side padding so the combat box appears horizontally centered.
static std::string boxLeftPad() { return leftPad(BOX_WIDTH + 2); }

// Draw the top "+----+" border of a combat-screen box.
static void drawBoxTop() {
    std::cout << boxLeftPad() << "+";
    for (int i = 0; i < BOX_WIDTH; i++) std::cout << "-";
    std::cout << "+\n";
}

// Bottom border is identical to the top.
static void drawBoxBottom() { drawBoxTop(); }

// Draw a single left-aligned text line inside a combat box.
static void drawBoxLine(const std::string& text) {
    int dw = displayWidthUtf8(text);
    int inner = BOX_WIDTH - 2;
    if (dw > inner) dw = inner;
    std::cout << boxLeftPad() << "| ";
    std::cout << text;
    for (int i = 0; i < inner - dw; i++) std::cout << ' ';
    std::cout << " |\n";
}

// Draw a horizontally centered text line inside a combat box.
static void drawBoxLineCentered(const std::string& text) {
    int dw = displayWidthUtf8(text);
    int inner = BOX_WIDTH - 2;
    if (dw > inner) {
        drawBoxLine(text);
        return;
    }
    int leftSp = (inner - dw) / 2;
    int rightSp = inner - dw - leftSp;
    std::cout << boxLeftPad() << "| ";
    for (int i = 0; i < leftSp; i++) std::cout << ' ';
    std::cout << text;
    for (int i = 0; i < rightSp; i++) std::cout << ' ';
    std::cout << " |\n";
}

// Draw an empty padding row inside a combat box.
static void drawBoxEmpty() { drawBoxLine(""); }

// Build a "[####----]" HP bar of a given character width.
static std::string makeHpBar(int hp, int maxHp, int width) {
    if (maxHp <= 0) maxHp = 1;
    int filled = hp * width / maxHp;
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;
    std::string bar = "[";
    for (int i = 0; i < width; i++) {
        bar += (i < filled) ? '#' : '-';
    }
    bar += "]";
    return bar;
}

// Penalty profile per monster: random HP loss range + random item-loss range.
struct FleePenalty {
    int hpMin, hpMax;
    int itemsMin, itemsMax;
    const char* description;
};

// Per-monster penalty profile applied when the player flees combat.
static FleePenalty getFleePenalty(MonsterType t) {
    switch (t) {
        case MON_BIRD:         return { 0,  0,  0, 0, "" };
        case MON_RABBIT:       return { 0,  0,  0, 0, "" };
        case MON_SCORPION:     return { 1,  5,  0, 0, "a few stings" };
        case MON_RATTLESNAKE:  return { 4,  8,  0, 1, "a venomous bite" };
        case MON_WILD_BOAR:    return { 8,  15, 0, 2, "a goring tusk swipe" };
        case MON_WOLF:         return { 10, 18, 1, 2, "snapping wolf jaws" };
        case MON_CROCODILE:    return { 18, 28, 1, 3, "a brutal death-roll" };
        case MON_GRIZZLY_BEAR: return { 22, 35, 2, 4, "a crushing claw swipe" };
        default:               return { 3,  7,  0, 1, "a parting blow" };
    }
}

// Inclusive random integer in [lo, hi]; returns lo if hi <= lo.
static int randInRange(int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + rand() % (hi - lo + 1);
}

// Redraw the entire combat scene: monster art + HP, message box, player HP,
// action menu, and the input hint line.
static void renderCombatScreen(const Player& player, const Monster& monster,
                               const std::string& message, int selectedAction,
                               bool poisoned) {
    clearScreen();

    std::vector<std::string> art = monster.getAsciiArt();
    // Monster box: top + empty + art + empty + monster-hp + bottom
    // Gap (1) + msg box (3) + gap (1) + player hp (1) + gap (1) + actions (1) + gap (1) + hint (1)
    int contentHeight = (int)art.size() + 5 + 1 + 3 + 1 + 1 + 1 + 1 + 1;
    printVerticalPadding(contentHeight);

    drawBoxTop();
    drawBoxEmpty();
    for (size_t i = 0; i < art.size(); i++) {
        drawBoxLineCentered(art[i]);
    }
    drawBoxEmpty();

    std::string monHp = monster.getName() + "  HP: " +
                        makeHpBar(monster.getHp(), monster.getMaxHp(), 25) + " " +
                        std::to_string(monster.getHp()) + "/" +
                        std::to_string(monster.getMaxHp());
    drawBoxLineCentered(monHp);

    drawBoxBottom();

    std::cout << "\n";
    drawBoxTop();
    drawBoxLineCentered(message);
    drawBoxBottom();

    std::cout << "\n";

    std::string playerHp = "YOUR HP: " +
                           makeHpBar(player.hp, player.maxHp, 25) + " " +
                           std::to_string(player.hp) + "/" +
                           std::to_string(player.maxHp);
    if (poisoned)
        playerHp += "  [POISONED!]";
    std::cout << leftPad(displayWidthUtf8(playerHp)) << playerHp << "\n\n";

    std::string actions[4] = {"FIGHT", "ACT", "ITEM", "FLEE"};
    std::string actionLine;
    for (int i = 0; i < 4; i++) {
        if (i == selectedAction)
            actionLine += "  [* " + actions[i] + " *]  ";
        else
            actionLine += "     " + actions[i] + "      ";
    }
    std::cout << leftPad(displayWidthUtf8(actionLine)) << actionLine << "\n\n";

    std::string hint = "[A/D] Select   [ENTER] Confirm";
    std::cout << leftPad(displayWidthUtf8(hint)) << hint << "\n";
}

// Show a transient combat message and wait for any key press.
static void showMessage(const Player& player, const Monster& monster,
                        const std::string& msg, bool poisoned) {
    renderCombatScreen(player, monster, msg, -1, poisoned);
    std::string hint = "Press any key to continue...";
    std::cout << "\n" << leftPad(displayWidthUtf8(hint)) << hint;
    getInput();
}

// Lets the player pick an HP-restoring item from inventory.
// Returns true if an item was actually used (player's turn ends).
// In-combat sub-menu: lets the player pick a healing item from inventory.
// Returns true if an item was used (consumes the player's turn).
static bool useHealingItemMenu(Player& player, Monster& monster, bool poisoned) {
    std::vector<int> invIdx;
    std::vector<const ItemDef*> defs;
    for (int i = 0; i < player.inventory.size(); i++) {
        const ItemDef* def = getItemDef(player.inventory.getItem(i).name);
        if (def && def->hpRestore > 0) {
            invIdx.push_back(i);
            defs.push_back(def);
        }
    }

    if (invIdx.empty()) {
        showMessage(player, monster,
                    "* You have no healing items!", poisoned);
        return false;
    }

    int selected = 0;
    while (true) {
        renderCombatScreen(player, monster,
                           "* Choose an item to use:", -1, poisoned);
        std::cout << "\n";
        for (size_t i = 0; i < invIdx.size(); i++) {
            const Item& item = player.inventory.getItem(invIdx[i]);
            std::string line = (i == (size_t)selected) ? "> " : "  ";
            line += item.name + " x" + std::to_string(item.quantity) +
                    "  (+" + std::to_string(defs[i]->hpRestore) + " HP)";
            std::cout << leftPad(displayWidthUtf8(line)) << line << "\n";
        }
        std::cout << "\n";
        std::string hint = "[W/S] Select   [ENTER] Use   [C] Cancel";
        std::cout << leftPad(displayWidthUtf8(hint)) << hint << "\n";

        char input = getInput();
        if (input == 'w' || input == 'W') {
            if (selected > 0) selected--;
        } else if (input == 's' || input == 'S') {
            if (selected + 1 < (int)invIdx.size()) selected++;
        } else if (input == 'c' || input == 'C') {
            return false;
        } else if (input == '\n' || input == '\r') {
            std::string name = player.inventory.getItem(invIdx[selected]).name;
            int heal = defs[selected]->hpRestore;
            int before = player.hp;
            player.hp += heal;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            int gained = player.hp - before;
            player.inventory.consumeItem(name, 1);
            showMessage(player, monster,
                        "* You used " + name + "! +" +
                        std::to_string(gained) + " HP.", poisoned);
            return true;
        }
    }
}

// Tick down hunger/thirst/fatigue from the strain of combat; signals death
// by exertion (returns true) if the player's HP runs out as a result.
static bool applyCombatExertion(Player& player, const Monster& monster,
                                bool poisoned) {
    int hpLoss = player.applyNeedsLoss(COMBAT_HUNGER_COST,
                                       COMBAT_THIRST_COST,
                                       COMBAT_FATIGUE_COST);
    if (hpLoss > 0) {
        showMessage(player, monster,
                    "* The fight drains you! -" +
                    std::to_string(hpLoss) + " HP!",
                    poisoned);
    }
    if (player.hp <= 0) {
        showMessage(player, monster,
                    "* You collapse from exhaustion...",
                    poisoned);
        return true;
    }
    return false;
}

// diffMult: player attack multiplier per difficulty (×/100)
// Player damage multiplier for the chosen difficulty (×/100).
static int diffPlayerMult(Difficulty diff) {
    switch (diff) {
        case DIFF_EASY:   return 120; // +20%
        case DIFF_HARD:   return 80;  // -20%
        default:          return 100; // Normal: no change
    }
}

// diffMult: monster attack multiplier per difficulty (×/100)
// Monster damage multiplier for the chosen difficulty (×/100).
static int diffMonsterMult(Difficulty diff) {
    switch (diff) {
        case DIFF_EASY:   return 80;  // -20%
        case DIFF_HARD:   return 150; // +50% — Hard hits harder, not more often
        default:          return 100;
    }
}

// Roll the player's attack damage: base + knife bonus + variance, then
// scaled by fatigue and difficulty, and reduced by monster defence.
static int playerAttackDamage(const Player& player, const Monster& monster, Difficulty diff) {
    int baseAtk = 10 + (player.hp / 10);
    if (player.equippedItem == "Knife") baseAtk += 5;
    int variance = baseAtk / 4;
    if (variance < 1) variance = 1;
    int dmg = baseAtk + (rand() % (variance * 2 + 1)) - variance;
    dmg -= monster.getDef();
    if (dmg < 1) dmg = 1;

    // Fatigue scales damage: full fatigue (100) = 100%, zero fatigue = 50%
    int fatigueMult = 50 + player.fatigue / 2;
    dmg = dmg * fatigueMult / 100;
    if (dmg < 1) dmg = 1;

    // Difficulty scales player damage
    dmg = dmg * diffPlayerMult(diff) / 100;
    if (dmg < 1) dmg = 1;

    return dmg;
}

// Apply flee penalty: 100% successful escape. Rolls random HP/item loss from
// the monster's danger profile and actually mutates player state.
// Returns true if the flee damage was lethal.
// Run the FLEE outcome: roll random HP and item-loss, mutate player state,
// and return true if the parting blow killed the player.
static bool applyFleePenalty(Player& player, Monster& monster,
                             bool poisoned) {
    FleePenalty pen = getFleePenalty(monster.getType());

    showMessage(player, monster,
                "* You turn and bolt for safety!", poisoned);

    int hpLoss    = randInRange(pen.hpMin,    pen.hpMax);
    int itemsLost = randInRange(pen.itemsMin, pen.itemsMax);

    if (hpLoss > 0) {
        player.hp -= hpLoss;
        if (player.hp < 0) player.hp = 0;
        std::string msg = "* " + monster.getName() + " catches you with " +
                          pen.description + "! -" +
                          std::to_string(hpLoss) + " HP!";
        showMessage(player, monster, msg, poisoned);
        if (player.hp <= 0) {
            showMessage(player, monster,
                        "* You didn't make it away...", poisoned);
            return true;
        }
    }

    if (itemsLost > 0 && player.inventory.size() > 0) {
        int dropped = 0;
        while (itemsLost > 0 && player.inventory.size() > 0) {
            int idx = rand() % player.inventory.size();
            std::string name = player.inventory.getItem(idx).name;
            player.inventory.consumeItem(name, 1);
            itemsLost--;
            dropped++;
        }
        if (dropped > 0) {
            std::string msg = "* You dropped " + std::to_string(dropped) +
                              " item(s) in panic!";
            showMessage(player, monster, msg, poisoned);
        }
    }

    showMessage(player, monster,
                "* You escaped with your life.", poisoned);
    return false;
}

// Main combat loop. Alternates player and monster turns until one side
// drops or the player flees. Returns COMBAT_WIN / COMBAT_FLEE / COMBAT_LOSE.
CombatResult startCombat(Player& player, Monster& monster, Difficulty diff) {
    int selectedAction = 0;
    std::string message = monster.getFlavorText();
    bool playerTurn = true;
    bool turnStart  = true; // true only on the first iteration of a given turn
    bool poisoned = false;
    int poisonTurns = 0;

    while (monster.isAlive() && player.hp > 0) {
        if (playerTurn) {
            // Poison ticks once per player turn — NOT on every A/D keypress.
            if (turnStart && poisoned && poisonTurns > 0) {
                int poisonDmg = 3;
                player.hp -= poisonDmg;
                if (player.hp < 0) player.hp = 0;
                poisonTurns--;
                message = "* Poison courses through you! -" +
                          std::to_string(poisonDmg) + " HP!";
                if (poisonTurns <= 0) {
                    poisoned = false;
                    message += " The poison wore off.";
                }
                showMessage(player, monster, message, poisoned);
                if (player.hp <= 0) {
                    showMessage(player, monster,
                                "* You succumbed to the poison...", poisoned);
                    return COMBAT_LOSE;
                }
                message = "* What will you do?";
            }
            turnStart = false;

            renderCombatScreen(player, monster, message, selectedAction, poisoned);

            char input = getInput();

            switch (input) {
                case 'a': case 'A':
                    selectedAction = (selectedAction + 3) % 4;
                    break;
                case 'd': case 'D':
                    selectedAction = (selectedAction + 1) % 4;
                    break;
                case '\n': case '\r': {
                    if (selectedAction == 0) {
                        int dmg = playerAttackDamage(player, monster, diff);
                        monster.takeDamage(dmg);

                        // Melee retaliation: a dangerous beast (atk >= 3)
                        // often grazes you in the exchange.
                        int scratch = 0;
                        if (monster.getAtk() >= 3 && rand() % 100 < 60) {
                            int scratchMax = monster.getAtk() / 3;
                            if (scratchMax < 1) scratchMax = 1;
                            scratch = 1 + rand() % scratchMax;
                            player.hp -= scratch;
                            if (player.hp < 0) player.hp = 0;
                        }

                        if (scratch > 0) {
                            message = "* You attack! " +
                                      std::to_string(dmg) +
                                      " damage! ( -" +
                                      std::to_string(scratch) +
                                      " HP scratch )";
                        } else {
                            message = "* You attack! " +
                                      std::to_string(dmg) + " damage!";
                        }

                        if (player.hp <= 0) {
                            showMessage(player, monster, message, poisoned);
                            showMessage(player, monster,
                                        "* You fell in the exchange...",
                                        poisoned);
                            return COMBAT_LOSE;
                        }

                        if (applyCombatExertion(player, monster, poisoned))
                            return COMBAT_LOSE;
                        if (!monster.isAlive()) {
                            showMessage(player, monster,
                                        "* You defeated " +
                                        monster.getName() + "!", poisoned);
                            if (isHuntableFood(monster.getType())) {
                                const char* drop = foodDropFor(monster.getType());
                                if (drop) {
                                    player.inventory.addItem(drop, 1);
                                    showMessage(player, monster,
                                                std::string("* You obtained ") +
                                                drop + " x1!", poisoned);
                                }
                            }
                            // General loot drops (Meat, Cloth Scrap, etc.)
                            std::vector<std::pair<std::string,int>> drops =
                                monster.getDrops();
                            if (!drops.empty()) {
                                std::string dropMsg = "* Loot: ";
                                for (size_t di = 0; di < drops.size(); di++) {
                                    if (di > 0) dropMsg += ", ";
                                    dropMsg += drops[di].first +
                                               " x" + std::to_string(drops[di].second);
                                    player.inventory.addItem(drops[di].first,
                                                             drops[di].second);
                                }
                                showMessage(player, monster, dropMsg, poisoned);
                            }
                            return COMBAT_WIN;
                        }
                        playerTurn = false;
                    } else if (selectedAction == 1) {
                        int roll = rand() % 3;
                        bool monsterStunned = false;
                        if (roll == 0) {
                            message = "* You study " +
                                      monster.getName() + "'s movements...";
                            showMessage(player, monster, message, poisoned);
                            message = "* ATK: " +
                                      std::to_string(monster.getAtk()) +
                                      "  DEF: " +
                                      std::to_string(monster.getDef());
                        } else if (roll == 1) {
                            message = "* You try to intimidate " +
                                      monster.getName() + "!";
                            showMessage(player, monster, message, poisoned);
                            if (rand() % 3 == 0) {
                                message = "* " + monster.getName() +
                                          " flinches! It skips its turn!";
                                showMessage(player, monster, message, poisoned);
                                monsterStunned = true;
                            } else {
                                message = "* But it didn't work...";
                            }
                        } else {
                            message = "* You observe " +
                                      monster.getName() + " carefully.";
                            showMessage(player, monster, message, poisoned);
                            message = "* It has " +
                                      std::to_string(monster.getHp()) +
                                      " HP remaining.";
                        }
                        if (applyCombatExertion(player, monster, poisoned))
                            return COMBAT_LOSE;
                        if (monsterStunned) {
                            message = "* What will you do?";
                            continue;
                        }
                        playerTurn = false;
                    } else if (selectedAction == 2) {
                        if (useHealingItemMenu(player, monster, poisoned)) {
                            if (player.hp <= 0) return COMBAT_LOSE;
                            message = "* What will you do?";
                            playerTurn = false;
                        } else {
                            message = "* What will you do?";
                            continue;
                        }
                    } else if (selectedAction == 3) {
                        if (player.inventory.hasItem("Dried Entrails", 1)) {
                            player.inventory.consumeItem("Dried Entrails", 1);
                            clearScreen();
                            printVerticalPadding(9);
                            printCentered("============================================");
                            printCentered("             ** ESCAPED SAFELY **          ");
                            printCentered("============================================");
                            std::cout << '\n';
                            printCentered("You threw the dried entrails at the beast.");
                            printCentered("It stopped to investigate — you slipped away.");
                            std::cout << '\n';
                            printCentered("Press any key to continue...");
                            getInput();
                            return COMBAT_FLEE;
                        }
                        if (applyCombatExertion(player, monster, poisoned))
                            return COMBAT_LOSE;
                        // 100% flee success, but the monster lands a parting
                        // blow (HP loss) and the player may drop items.
                        if (applyFleePenalty(player, monster, poisoned))
                            return COMBAT_LOSE;
                        return COMBAT_FLEE;
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            int dmg = monster.attack();
            dmg = dmg * diffMonsterMult(diff) / 100;
            if (dmg < 1) dmg = 1;
            player.hp -= dmg;
            if (player.hp < 0) player.hp = 0;

            message = "* " + monster.getName() +
                      monster.getAttackText() + " " +
                      std::to_string(dmg) + " damage!";
            showMessage(player, monster, message, poisoned);

            if (monster.getType() == MON_RATTLESNAKE &&
                !poisoned && rand() % 2 == 0) {
                poisoned = true;
                poisonTurns = 3;
                message = "* You got poisoned! Venom seeps into your body!";
                showMessage(player, monster, message, poisoned);
            }

            if (player.hp <= 0) {
                showMessage(player, monster,
                            "* You have been defeated...", poisoned);
                return COMBAT_LOSE;
            }

            message = "* What will you do?";
            playerTurn = true;
            turnStart  = true;
        }
    }

    return COMBAT_LOSE;
}
