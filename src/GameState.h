#ifndef GAMESTATE_H
#define GAMESTATE_H

enum GameScreen {
    SCREEN_WORLD,
    SCREEN_INVENTORY,
    SCREEN_CRAFTING,
    SCREEN_COOKING,
    SCREEN_PAUSE
};

#endif

// game.cpp, inventoryui.cpp will use this, and it's gonna be comlicated to use enum anywhere
