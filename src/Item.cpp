#include "Item.h"
#include <cstddef>

// ─────────────────────── Item Registry ──────────────────────
// Fields: name, type, description,
//         consumable, equippable,
//         hungerRestore, thirstRestore, hpRestore, fatigueRestore

static const ItemDef ITEM_REGISTRY[] = {

    // ── Raw Materials ────────────────────────────────────────
    {"Wood",          ITEM_MATERIAL, "Basic building material.",                          false, false, 0,  0,  0,  0},
    {"Stone",         ITEM_MATERIAL, "Used for crafting tools and weapons.",               false, false, 0,  0,  0,  0},
    {"Grass",         ITEM_MATERIAL, "Basic resource for crafting Vine.",                 false, false, 0,  0,  0,  0},
    {"Flint",         ITEM_MATERIAL, "Used to create fire-related tools.",                false, false, 0,  0,  0,  0},
    {"Rope",          ITEM_MATERIAL, "Used in tools and raft construction.",              false, false, 0,  0,  0,  0},
    {"Vine",          ITEM_MATERIAL, "A substitute for rope in early game.",              false, false, 0,  0,  0,  0},
    {"Cloth Scrap",   ITEM_MATERIAL, "Used for healing items and raft sail.",             false, false, 0,  0,  0,  0},
    {"Bait",          ITEM_MATERIAL, "Used for fishing. Each cast consumes 1.",           false, false, 0,  0,  0,  0},
    {"Venom",         ITEM_MATERIAL, "Toxic extract from a scorpion. Crafting ingredient.",false, false, 0,  0,  0,  0},
    {"Resin",         ITEM_MATERIAL, "Sticky resin from ruins. Required for raft crafting.",false, false, 0,  0,  0,  0},
    {"Dried Entrails",ITEM_MATERIAL, "Foul-smelling dried innards. Auto-used when fleeing to escape safely.",false, false, 0,  0,  0,  0},
    {"Bear Skin",     ITEM_MATERIAL, "Thick hide from a grizzly bear. Required for raft crafting.", false, false, 0, 0, 0, 0},

    // ── Processed Components ─────────────────────────────────
    {"Log",           ITEM_MATERIAL, "Processed wood. Core raft building block.",         false, false, 0,  0,  0,  0},
    {"Raft Base",     ITEM_MATERIAL, "The hull of your escape raft.",                     false, false, 0,  0,  0,  0},
    {"Sail",          ITEM_MATERIAL, "A cloth sail to catch the wind.",                   false, false, 0,  0,  0,  0},
    {"Oar",           ITEM_MATERIAL, "Wooden oar for steering the raft.",                 false, false, 0,  0,  0,  0},
    {"Reinforcement", ITEM_MATERIAL, "Stone and wood brace. Keeps the raft together.",    false, false, 0,  0,  0,  0},

    // ── Raw Food ─────────────────────────────────────────────
    {"Fish",          ITEM_FOOD,     "Raw fish. Hunger: +10. Best cooked first.",         true,  false, 10, 0,  0,  0},
    {"Raw Meat",      ITEM_FOOD,     "Raw meat. Hunger: +15. Cook for better effect.",    true,  false, 15, 0,  0,  0},
    {"Raw Poultry",   ITEM_FOOD,     "Raw bird meat. Hunger: +15. Cook before eating.",   true,  false, 15, 0,  0,  0},

    // ── Gathered Food / Drink ────────────────────────────────
    {"Berry",         ITEM_FOOD,     "Wild berries. Hunger: +1 / Thirst: +3.",            true,  false, 1,  3,  0,  0},
    {"Coconut",       ITEM_FOOD,     "Hunger: +2 / Thirst: +15. Early survival staple.",  true,  false, 2,  15, 0,  0},
    {"Medical Herb",  ITEM_FOOD,     "Natural remedy. Heal: +5 HP.",                      true,  false, 0,  0,  5,  0},
    {"Water",         ITEM_DRINK,    "Fresh water. Thirst: +35.",                         true,  false, 0,  35, 0,  0},

    // ── Cooked Food ──────────────────────────────────────────
    {"Cooked Fish",   ITEM_FOOD,     "Grilled fish. Hunger: +20.",                        true,  false, 20, 0,  0,  0},
    {"Cooked Meat",   ITEM_FOOD,     "Roasted meat. Hunger: +30.",                        true,  false, 30, 0,  0,  0},
    {"Cooked Poultry",ITEM_FOOD,     "Roasted bird. Hunger: +30.",                        true,  false, 30, 0,  0,  0},
    {"Soup",          ITEM_FOOD,     "Hunger: +20 / Thirst: +20 / Heal: +10 HP.",         true,  false, 20, 20, 10, 0},

    // ── Drinks ───────────────────────────────────────────────
    {"Coconut Water", ITEM_DRINK,    "Refreshing. Thirst: +25.",                          true,  false, 0,  25, 0,  0},
    {"Herbal Tea",    ITEM_DRINK,    "Thirst: +25 / Fatigue: +5.",                        true,  false, 0,  25, 0,  5},

    // ── Healing & Potions ─────────────────────────────────────
    {"Bandage",       ITEM_TOOL,     "Heal: +15 HP.",                                     true,  false, 0,  0,  15, 0},
    {"First Aid Kit", ITEM_TOOL,     "Heal: +50 HP. Rare and valuable.",                  true,  false, 0,  0,  50, 0},
    {"Potion",        ITEM_DRINK,    "Brewed from herb and scorpion venom. Restores fatigue by 20.", true, false, 0, 0, 0, 20},
    {"Murky Vial",    ITEM_DRINK,    "A sealed vial from the ruins. The contents look questionable. Thirst: +10 / Heal: +20 HP.", true, false, 0, 10, 20, 0},

    // ── Equippable Tools ─────────────────────────────────────
    {"Axe",           ITEM_TOOL,     "Doubles wood collected from trees and palms.",      false, true,  0,  0,  0,  0},
    {"Pickaxe",       ITEM_TOOL,     "Doubles stone collected; raises Flint drop to 30%.",false, true,  0,  0,  0,  0},
    {"Knife",         ITEM_TOOL,     "+5 bonus damage in combat.",                        false, true,  0,  0,  0,  0},
    {"Fishing Rod",   ITEM_TOOL,     "Enables fishing near ocean water.",                 false, true,  0,  0,  0,  0},
    {"Torch",         ITEM_TOOL,     "Provides light at night. Prevents monster encounters.", false, true, 0, 0, 0, 0},

    // ── Win Condition ─────────────────────────────────────────
    {"Raft",          ITEM_TOOL,     "Your ticket off the island.",                       false, false, 0,  0,  0,  0},
};

static const int REGISTRY_SIZE =
    (int)(sizeof(ITEM_REGISTRY) / sizeof(ITEM_REGISTRY[0]));

// Look up an item definition by name in the static registry; nullptr if not found.
const ItemDef* getItemDef(const std::string& name) {
    for (int i = 0; i < REGISTRY_SIZE; i++) {
        if (ITEM_REGISTRY[i].name == name)
            return &ITEM_REGISTRY[i];
    }
    return nullptr;
}

// Human-readable label for an ItemType enum value.
const char* itemTypeName(ItemType type) {
    switch (type) {
        case ITEM_MATERIAL: return "Material";
        case ITEM_FOOD:     return "Food";
        case ITEM_DRINK:    return "Drink";
        case ITEM_TOOL:     return "Tool";
        default:            return "?";
    }
}
