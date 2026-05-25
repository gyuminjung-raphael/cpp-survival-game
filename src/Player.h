#ifndef PLAYER_H
#define PLAYER_H
#include "Inventory.h"

class Player {
public:
    Player();

    void setPosition(int r, int c);
    int getRow() const;
    int getCol() const;

    // Movement methods
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    // Reduces needs and applies HP loss only when hunger or thirst is 0
    int applyNeedsLoss(int hungerLoss, int thirstLoss, int fatigueLoss,
                       int depletedHpLoss = 3);

    // Stats
    int hp, maxHp;
    int hunger;
    int thirst;
    int fatigue;
    int day;
    int hour;

    Inventory inventory;
    std::string equippedItem; // "" = nothing equipped

private:
    int row, col;
};

#endif // PLAYER_H