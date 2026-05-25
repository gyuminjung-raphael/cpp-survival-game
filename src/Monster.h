#ifndef MONSTER_H
#define MONSTER_H

#include <string>
#include <vector>
#include <utility>

enum MonsterType {
    MON_SCORPION = 0,
    MON_RATTLESNAKE,
    MON_WILD_BOAR,
    MON_CROCODILE,
    MON_WOLF,
    MON_GRIZZLY_BEAR,
    MON_BIRD,
    MON_RABBIT,
    MON_COUNT
};

bool isHuntableFood(MonsterType t);
const char* foodDropFor(MonsterType t);
bool isDangerousAnimal(MonsterType t);

// Percentage (0–100) that a spawn roll chooses a dangerous animal
// instead of a harmless one. Ramps up as days pass.
int dangerousAnimalChancePercent(int playerDay);

// Percentage (0–100) per step that walking through a bush tile
// provokes a random encounter. Ramps up as days pass.
int bushEncounterChancePercent(int playerDay);

class Monster {
public:
    Monster();
    Monster(MonsterType type);

    std::string getName() const;
    MonsterType getType() const;
    int getHp() const;
    int getMaxHp() const;
    int getAtk() const;
    int getDef() const;
    int getExpReward() const;
    bool isAlive() const;

    void takeDamage(int dmg);
    int attack() const;

    std::vector<std::string> getAsciiArt() const;
    std::string getFlavorText() const;
    std::string getAttackText() const;
    std::vector<std::pair<std::string, int>> getDrops() const;

private:
    MonsterType type;
    std::string name;
    int hp, maxHp;
    int atk, def;
    int expReward;
};

Monster spawnRandomMonster(int mapId, int playerDay);

#endif
