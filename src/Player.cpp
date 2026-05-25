#include "Player.h"

// Default constructor: full stats, day 1 starting at 8:00, no item equipped.
Player::Player()
    : hp(100), maxHp(100),
      hunger(100), thirst(100), fatigue(100),
      day(1), hour(8),
      equippedItem(""),
      row(0), col(0) {}

// Place the player at the given grid coordinates.
void Player::setPosition(int r, int c) { row = r; col = c; }
// Current grid coordinates of the player.
int Player::getRow() const { return row; }
int Player::getCol() const { return col; }

// One-tile movement helpers; bounds and walkability checks are done by the caller.
void Player::moveUp()    { row--; }
void Player::moveDown()  { row++; }
void Player::moveLeft()  { col--; }
void Player::moveRight() { col++; }

// Decrease hunger/thirst/fatigue by the given amounts; if hunger or thirst hits
// zero the player also loses HP. Returns the HP lost on this tick (0 if none).
int Player::applyNeedsLoss(int hungerLoss, int thirstLoss, int fatigueLoss,
                           int depletedHpLoss) {
    hunger -= hungerLoss;
    if (hunger < 0) hunger = 0;

    thirst -= thirstLoss;
    if (thirst < 0) thirst = 0;

    fatigue -= fatigueLoss;
    if (fatigue < 0) fatigue = 0;

    // HP loss only if hunger or thirst is depleted
    if (hunger == 0 || thirst == 0) {
        hp -= depletedHpLoss;
        if (hp < 0) hp = 0;
        return depletedHpLoss;
    }

    return 0;
}
