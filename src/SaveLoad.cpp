#include "SaveLoad.h"
#include <fstream>
#include <sstream>

// Write the entire save record to disk in a simple line-per-field text format.
bool saveGame(const SaveData& data, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) return false;

    out << data.playerRow << " " << data.playerCol << "\n";
    out << data.currentMapID << "\n";
    out << data.hp << " " << data.maxHp << "\n";
    out << data.hunger << " " << data.thirst << " " << data.fatigue << "\n";
    out << data.day << " " << data.hour << "\n";
    out << data.moveCount << "\n";
    out << (data.emojiMode ? 1 : 0) << "\n";

    // Inventory: "<quantity> <name>" per line
    out << data.inventoryItems.size() << "\n";
    for (const auto& item : data.inventoryItems) {
        out << item.quantity << " " << item.name << "\n";
    }

    return out.good();
}

// Read one line and rebind the stringstream to it. Trailing tokens on
// a line are silently ignored by the caller, which lets the loader
// tolerate older save formats that wrote an extra trailing field
// (e.g. sanity, removed in commit 7ccab4a).
static bool readLine(std::ifstream& in, std::istringstream& out) {
    std::string line;
    if (!std::getline(in, line)) return false;
    out.clear();
    out.str(line);
    return true;
}

// Parse a save file back into a SaveData struct; returns false on any failure.
bool loadGame(SaveData& data, const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;

    std::istringstream ls;

    if (!readLine(in, ls) || !(ls >> data.playerRow >> data.playerCol)) return false;
    if (!readLine(in, ls) || !(ls >> data.currentMapID)) return false;
    if (!readLine(in, ls) || !(ls >> data.hp >> data.maxHp)) return false;
    if (!readLine(in, ls) || !(ls >> data.hunger >> data.thirst >> data.fatigue)) return false;
    if (!readLine(in, ls) || !(ls >> data.day >> data.hour)) return false;
    if (!readLine(in, ls) || !(ls >> data.moveCount)) return false;

    int emoji = 0;
    if (!readLine(in, ls) || !(ls >> emoji)) return false;
    data.emojiMode = (emoji != 0);

    if (data.currentMapID < 0 || data.currentMapID >= 5) return false;

    int numItems = 0;
    if (!readLine(in, ls) || !(ls >> numItems)) return false;

    data.inventoryItems.clear();
    for (int i = 0; i < numItems; i++) {
        std::string line;
        if (!std::getline(in, line)) break;
        std::istringstream ss(line);
        Item item;
        ss >> item.quantity;
        std::getline(ss >> std::ws, item.name);
        if (!item.name.empty())
            data.inventoryItems.push_back(item);
    }

    return true;
}

// True if the given save file exists and parses successfully.
bool hasSaveFile(const std::string& filename) {
    SaveData tmp;
    return loadGame(tmp, filename);
}