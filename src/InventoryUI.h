#ifndef INVENTORY_UI_H
#define INVENTORY_UI_H

#include "Inventory.h"
#include "GameState.h"
#include "status_ui.h"
#include "Player.h"
#include "Map.h"
#include <string>

void renderInventory(const Inventory& inv, int selectedIndex, bool dropMode, int dropAmount,
                     const PlayerStatus& status, int day, const std::string& timeOfDay,
                     const std::string& location, const std::string& equippedItem,
                     bool showRaftHint, const std::string& raftHintMessage);

void handleInventoryInput(char input, Player& player, int& selectedIndex,
                          GameScreen& screen, bool& dropMode, int& dropAmount,
                          bool& running, bool& playerWon,
                          MapID currentMapId, bool nearWater,
                          bool& showRaftHint, std::string& raftHintMessage);

#endif