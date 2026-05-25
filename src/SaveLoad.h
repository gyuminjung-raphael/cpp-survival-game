#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <string>
#include <vector>
#include "Inventory.h"

struct SaveData {
    int playerRow, playerCol;
    int currentMapID;
    int hp, maxHp;
    int hunger, thirst, fatigue;
    int day, hour;
    int moveCount;
    bool emojiMode;
    std::vector<Item> inventoryItems;
};

bool saveGame(const SaveData& data, const std::string& filename = "data/save.txt");
bool loadGame(SaveData& data, const std::string& filename = "data/save.txt");
bool hasSaveFile(const std::string& filename = "data/save.txt");

#endif // SAVELOAD_H