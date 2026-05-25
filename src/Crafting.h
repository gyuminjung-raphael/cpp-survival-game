#ifndef CRAFTING_H
#define CRAFTING_H

#include "Inventory.h"
#include "GameState.h"
#include <string>
#include <vector>

struct Ingredient {
    std::string name;
    int quantity;
};

struct Recipe {
    std::string outputName;
    int outputQty;
    std::vector<Ingredient> ingredients;
};

std::vector<Recipe> getCraftingRecipes();
std::vector<Recipe> getCookingRecipes();

bool canCraft(const Recipe& recipe, const Inventory& inv);
bool tryCraft(const Recipe& recipe, Inventory& inv);

static const int RECIPES_PER_PAGE = 4;

void renderCraftingUI(const std::vector<Recipe>& recipes, const Inventory& inv,
                      int selectedIndex, int pageIndex,
                      const std::string& title, const std::string& craftMsg);

void handleCraftingInput(char input, const std::vector<Recipe>& recipes,
                         Inventory& inv, int& selectedIndex, int& pageIndex,
                         GameScreen& screen, GameScreen returnScreen,
                         std::string& craftMsg);

#endif
