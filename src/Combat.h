#ifndef COMBAT_H
#define COMBAT_H

#include "Player.h"
#include "Monster.h"
#include "UI.h"

enum CombatResult {
    COMBAT_WIN,
    COMBAT_FLEE,
    COMBAT_LOSE
};

CombatResult startCombat(Player& player, Monster& monster, Difficulty diff = DIFF_NORMAL);

#endif
