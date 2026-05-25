#include "InventoryUI.h"
#include "Item.h"
#include "UI.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

// Print two side-by-side columns of text, used to lay out inventory + status panel.
static void printTwoColumns(const vector<string>& leftLines, const vector<string>& rightLines, int leftWidth = 36) {
    size_t totalLines = max(leftLines.size(), rightLines.size());
    const int gap = 4;
    const int rightWidth = 60;
    const int totalWidth = leftWidth + gap + rightWidth;
    string pad = leftPad(totalWidth);

    for (size_t i = 0; i < totalLines; i++) {
        string l = (i < leftLines.size()) ? leftLines[i] : "";
        string r = (i < rightLines.size()) ? rightLines[i] : "";

        cout << pad << std::left << setw(leftWidth) << l
             << string(gap, ' ') << r << '\n';
    }
}

// Draw the full inventory screen: item list, selected item info, drop prompt,
// raft-use hints, and the player status panel on the right.
void renderInventory(const Inventory& inv, int selectedIndex, bool dropMode, int dropAmount,
                     const PlayerStatus& status, int day, const string& timeOfDay,
                     const string& location, const string& equippedItem,
                     bool showRaftHint, const std::string& raftHintMessage) {
    vector<string> leftPanel;
    vector<string> rightPanel = buildStatusLines(status, day, timeOfDay, location);

    leftPanel.push_back("========== INVENTORY ==========");
    leftPanel.push_back("");

    if (inv.empty()) {
        leftPanel.push_back("Inventory is empty.");
    } else {
        for (int i = 0; i < inv.size(); i++) {
            string line;
            if (i == selectedIndex) line += "> ";
            else line += "  ";

            line += inv.getItem(i).name + " x" + to_string(inv.getItem(i).quantity);
            if (!equippedItem.empty() && inv.getItem(i).name == equippedItem)
                line += "  [Equipped]";
            leftPanel.push_back(line);
        }

        leftPanel.push_back("");
        leftPanel.push_back("------ ITEM INFO ------");
        leftPanel.push_back(inv.getItem(selectedIndex).name);
        leftPanel.push_back(inv.getItem(selectedIndex).description);

        if (inv.getItem(selectedIndex).name == "Raft") {
            leftPanel.push_back("");
            leftPanel.push_back("Raft can only be used at Shore,");
            leftPanel.push_back("while standing next to the sea.");
        }
    }

    leftPanel.push_back("");

    if (showRaftHint && !raftHintMessage.empty()) {
        leftPanel.push_back("------ NOTICE ------");
        leftPanel.push_back(raftHintMessage);
        leftPanel.push_back("");
    }

    if (dropMode && !inv.empty()) {
        leftPanel.push_back("------ DROP ITEM ------");
        leftPanel.push_back("Item   : " + inv.getItem(selectedIndex).name);
        leftPanel.push_back("Owned  : " + to_string(inv.getItem(selectedIndex).quantity));
        leftPanel.push_back("Drop   : " + to_string(dropAmount));
        leftPanel.push_back("");
        leftPanel.push_back("[W/S] Adjust amount");
        leftPanel.push_back("[D] Confirm");
        leftPanel.push_back("[X] Cancel");
    } else {
        leftPanel.push_back("[W/S] Select");
        leftPanel.push_back("[Enter] Use / Equip");
        leftPanel.push_back("[D] Drop");
        leftPanel.push_back("[I] Close");
    }

    printTwoColumns(leftPanel, rightPanel);
}

// Process a key press while the inventory screen is open: navigation, drop,
// use/equip, raft-launch (win condition), and exiting back to the world.
void handleInventoryInput(char input, Player& player, int& selectedIndex,
                          GameScreen& screen, bool& dropMode, int& dropAmount,
                          bool& running, bool& playerWon,
                          MapID currentMapId, bool nearWater,
                          bool& showRaftHint, std::string& raftHintMessage) {
    Inventory& inv = player.inventory;

    if (dropMode) {
        if (inv.empty()) {
            dropMode = false;
            dropAmount = 1;
            return;
        }

        int maxAmount = inv.getItem(selectedIndex).quantity;

        switch (input) {
            case 's': case 'S':
                if (dropAmount > 1) dropAmount--;
                break;

            case 'w': case 'W':
                if (dropAmount < maxAmount) dropAmount++;
                break;

            case 'd': case 'D':
                inv.removeQuantity(selectedIndex, dropAmount);

                if (!inv.empty() && selectedIndex >= inv.size()) {
                    selectedIndex = inv.size() - 1;
                }

                dropMode = false;
                dropAmount = 1;
                showRaftHint = false;
                raftHintMessage = "";
                break;

            case 'x': case 'X':
                dropMode = false;
                dropAmount = 1;
                showRaftHint = false;
                raftHintMessage = "";
                break;
        }

        return;
    }

    switch (input) {
        case 'w': case 'W':
            if (selectedIndex > 0) selectedIndex--;
            showRaftHint = false;
            raftHintMessage = "";
            break;

        case 's': case 'S':
            if (selectedIndex < inv.size() - 1) selectedIndex++;
            showRaftHint = false;
            raftHintMessage = "";
            break;

        case 'd': case 'D':
            if (!inv.empty()) {
                dropMode = true;
                dropAmount = 1;
                showRaftHint = false;
                raftHintMessage = "";
            }
            break;

        case '\n':
        case '\r':
            if (!inv.empty()) {
                const Item& item = inv.getItem(selectedIndex);

                // Win condition: Raft can only be used at Shore and next to water
                if (item.name == "Raft") {
                    if (currentMapId == MAP_SHORE && nearWater) {
                        inv.consumeItem("Raft", 1);
                        playerWon = true;
                        running = false;
                        screen = SCREEN_WORLD;
                        showRaftHint = false;
                        raftHintMessage = "";
                        return;
                    } else {
                        showRaftHint = true;

                        if (currentMapId == MAP_BEACH) {
                            raftHintMessage = "These waters are packed with reefs. You should find a safer shore.";
                        } else if (currentMapId == MAP_CAVE || currentMapId == MAP_FOREST) {
                            raftHintMessage = "You need to find the shore before using the Raft.";
                        } else if (currentMapId == MAP_VILLAGE) {
                            raftHintMessage = "This is not a good place to launch the Raft. Try another part of the shore.";
                        } else if (currentMapId == MAP_SHORE && !nearWater) {
                            raftHintMessage = "You need to stand right next to the sea to use the Raft.";
                        } else {
                            raftHintMessage = "You cannot use the Raft here.";
                        }

                        return;
                    }
                }

                const ItemDef* def = getItemDef(item.name);
                if (def && def->consumable) {
                    if (def->hpRestore > 0) {
                        player.hp = min(player.maxHp, player.hp + def->hpRestore);
                    }
                    if (def->hungerRestore > 0) {
                        player.hunger = min(100, player.hunger + def->hungerRestore);
                    }
                    if (def->thirstRestore > 0) {
                        player.thirst = min(100, player.thirst + def->thirstRestore);
                    }
                    if (def->fatigueRestore > 0) {
                        player.fatigue = min(100, player.fatigue + def->fatigueRestore);
                    }
                    inv.consumeItem(item.name, 1);
                    if (!inv.empty() && selectedIndex >= inv.size()) {
                        selectedIndex = inv.size() - 1;
                    }
                } else if (def && def->equippable) {
                    if (player.equippedItem == item.name)
                        player.equippedItem = "";
                    else
                        player.equippedItem = item.name;
                }

                showRaftHint = false;
                raftHintMessage = "";
            }
            break;

        case 'i': case 'I':
            screen = SCREEN_WORLD;
            showRaftHint = false;
            raftHintMessage = "";
            break;
    }
}