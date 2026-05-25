#include "Crafting.h"
#include "UI.h"
#include <iostream>

// ─────────────────────── Recipe Lists ───────────────────────

// Static list of all workshop crafting recipes (tools, raft parts, etc.).
std::vector<Recipe> getCraftingRecipes() {
    return {
        // ── Processed Materials ───────────────────────────────
        {"Vine",          1, {{"Grass",      3}}},
        {"Rope",          1, {{"Vine",       3}}},
        {"Log",           1, {{"Wood",       2}}},
        {"Bait",          2, {{"Berry",      1}}},

        // ── Tools ─────────────────────────────────────────────
        {"Axe",           1, {{"Stone", 2}, {"Wood", 2}}},
        {"Pickaxe",       1, {{"Stone", 2}, {"Wood", 2}}},
        {"Knife",         1, {{"Stone", 1}, {"Wood", 1}}},
        {"Fishing Rod",   1, {{"Vine",  3}, {"Wood", 2}}},
        {"Torch",         1, {{"Flint", 1}, {"Wood", 1}}},

        // ── Healing ───────────────────────────────────────────
        {"Bandage",       1, {{"Cloth Scrap", 2}}},
        {"First Aid Kit", 1, {{"Bandage", 2}, {"Medical Herb", 2}}},
        {"Potion",        1, {{"Venom", 1}, {"Medical Herb", 1}}},

        // ── Raft Parts ────────────────────────────────────────
        {"Raft Base",     1, {{"Log",        5}, {"Rope",        2}}},
        {"Sail",          1, {{"Cloth Scrap",3}, {"Rope",        1}}},
        {"Oar",           1, {{"Wood",       2}, {"Rope",        1}}},
        {"Reinforcement", 1, {{"Stone",      2}, {"Wood",        1}}},

        // Final escape raft
        {"Raft",          1, {{"Raft Base",  10}, {"Sail",       3},
                              {"Oar",        2}, {"Reinforcement", 5},
                              {"Resin",      10}, {"Bear Skin",  1}}},
    };
}

// Static list of all campfire cooking recipes.
std::vector<Recipe> getCookingRecipes() {
    return {
        {"Cooked Fish",    1, {{"Fish",  1}, {"Flint", 1}}},
        {"Cooked Meat",    1, {{"Raw Meat", 1}, {"Flint", 1}}},
        {"Soup",           1, {{"Coconut", 1}, {"Raw Meat", 1}, {"Medical Herb", 1}, {"Flint", 1}}},
        {"Herbal Tea",     1, {{"Coconut", 1}, {"Medical Herb", 2}, {"Flint", 1}}},
    };
}

// ─────────────────────── Craft Logic ────────────────────────

// True if the inventory has every ingredient required by the recipe.
bool canCraft(const Recipe& recipe, const Inventory& inv) {
    for (const auto& ing : recipe.ingredients) {
        if (!inv.hasItem(ing.name, ing.quantity)) return false;
    }
    return true;
}

// Consume ingredients and produce the output item. Returns false if not craftable.
bool tryCraft(const Recipe& recipe, Inventory& inv) {
    if (!canCraft(recipe, inv)) return false;

    for (const auto& ing : recipe.ingredients) {
        inv.consumeItem(ing.name, ing.quantity);
    }

    inv.addItem(recipe.outputName, recipe.outputQty);
    return true;
}

// ─────────────────────── Crafting UI ────────────────────────

// Draw the crafting/cooking screen: paged list of recipes with ingredient
// availability, craft message banner, and the input-key legend.
void renderCraftingUI(const std::vector<Recipe>& recipes, const Inventory& inv,
                      int selectedIndex, int pageIndex,
                      const std::string& title, const std::string& craftMsg) {
    int total      = (int)recipes.size();
    int totalPages = (total + RECIPES_PER_PAGE - 1) / RECIPES_PER_PAGE;
    if (totalPages < 1) totalPages = 1;

    int pageStart = pageIndex * RECIPES_PER_PAGE;
    int pageEnd   = pageStart + RECIPES_PER_PAGE;
    if (pageEnd > total) pageEnd = total;

    // Estimate content height for vertical centering.
    int bodyLines = 0;
    if (recipes.empty()) {
        bodyLines = 1;
    } else {
        for (int i = pageStart; i < pageEnd; i++) {
            bodyLines += 1 + (int)recipes[i].ingredients.size() + 1;
        }
    }

    int contentHeight = 2 + bodyLines + (craftMsg.empty() ? 0 : 2) + 1;
    printVerticalPadding(contentHeight);

    // This keeps the whole crafting block around the center,
    // but aligns all recipe text to the same left edge.
    const int PANEL_WIDTH = 64;
    std::string pad = leftPad(PANEL_WIDTH);

    std::cout << pad << "========== " << title << " ==========   Page "
              << (pageIndex + 1) << "/" << totalPages << '\n';
    std::cout << '\n';

    if (recipes.empty()) {
        std::cout << pad << "No recipes available." << '\n';
        std::cout << '\n';
    } else {
        for (int i = pageStart; i < pageEnd; i++) {
            const Recipe& r = recipes[i];
            bool craftable = canCraft(r, inv);
            int localIdx = i - pageStart;

            std::string head = (localIdx == selectedIndex) ? "> " : "  ";
            head += std::to_string(i + 1) + ". " + r.outputName;
            if (r.outputQty > 1) head += " x" + std::to_string(r.outputQty);
            head += craftable ? "   [craftable]" : "   [need more]";

            std::cout << pad << head << '\n';

            for (const auto& ing : r.ingredients) {
                int have = 0;

                for (int k = 0; k < inv.size(); k++) {
                    if (inv.getItem(k).name == ing.name) {
                        have = inv.getItem(k).quantity;
                        break;
                    }
                }

                std::string ingredientLine = "     - " + ing.name
                                           + " x" + std::to_string(ing.quantity)
                                           + "  (have: " + std::to_string(have) + ")";

                std::cout << pad << ingredientLine << '\n';
            }

            std::cout << '\n';
        }
    }

    if (!craftMsg.empty()) {
        std::cout << pad << ">> " << craftMsg << '\n';
        std::cout << '\n';
    }

    std::cout << pad << "[W/S] Select  [A/D] Page  [Enter] Craft  [X] Close" << '\n';
}

// Handle a key press on the crafting screen: navigation, paging, craft, exit.
void handleCraftingInput(char input, const std::vector<Recipe>& recipes,
                         Inventory& inv, int& selectedIndex, int& pageIndex,
                         GameScreen& screen, GameScreen returnScreen,
                         std::string& craftMsg) {
    int total      = (int)recipes.size();
    int totalPages = (total + RECIPES_PER_PAGE - 1) / RECIPES_PER_PAGE;
    if (totalPages < 1) totalPages = 1;

    int pageStart   = pageIndex * RECIPES_PER_PAGE;
    int pageEnd     = pageStart + RECIPES_PER_PAGE;
    if (pageEnd > total) pageEnd = total;

    int itemsOnPage = pageEnd - pageStart;

    switch (input) {
        case 'w': case 'W':
            if (selectedIndex > 0) {
                selectedIndex--;
                craftMsg = "";
            }
            break;

        case 's': case 'S':
            if (selectedIndex < itemsOnPage - 1) {
                selectedIndex++;
                craftMsg = "";
            }
            break;

        case 'a': case 'A':
            if (pageIndex > 0) {
                pageIndex--;
                selectedIndex = 0;
                craftMsg = "";
            }
            break;

        case 'd': case 'D':
            if (pageIndex < totalPages - 1) {
                pageIndex++;
                selectedIndex = 0;
                craftMsg = "";
            }
            break;

        case '\r': case '\n': {
            int recipeIdx = pageIndex * RECIPES_PER_PAGE + selectedIndex;

            if (recipeIdx < total) {
                if (tryCraft(recipes[recipeIdx], inv)) {
                    craftMsg = "Crafted: " + recipes[recipeIdx].outputName + "!";
                } else {
                    craftMsg = "Not enough materials!";
                }
            }

            break;
        }

        case 'x': case 'X':
            screen = returnScreen;
            craftMsg = "";
            break;
    }
}