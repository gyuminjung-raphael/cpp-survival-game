#ifndef ITEM_H
#define ITEM_H

#include <string>

enum ItemType {
    ITEM_MATERIAL,
    ITEM_FOOD,
    ITEM_DRINK,
    ITEM_TOOL,
    ITEM_UNKNOWN
};

// Item struct lives in Inventory.h — registry only here.

// Static definition stored in the item registry
struct ItemDef {
    std::string name;
    ItemType    type;
    std::string description;
    bool        consumable;
    bool        equippable;
    int         hungerRestore;   // hunger points added (0 = none)
    int         thirstRestore;   // thirst points added
    int         hpRestore;       // HP restored
    int         fatigueRestore;  // fatigue points added
};

// Look up a definition by name; returns nullptr if unknown
const ItemDef* getItemDef(const std::string& name);

// Human-readable label for an ItemType
const char* itemTypeName(ItemType type);

#endif
