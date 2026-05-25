#include "Monster.h"
#include <cstdlib>
#include <ctime>

// Default constructor: a basic Scorpion (used as a placeholder).
Monster::Monster()
    : type(MON_SCORPION), name("Scorpion"),
      hp(20), maxHp(20), atk(7), def(3), expReward(10) {}

// Build a monster of the given type with its preset stats.
Monster::Monster(MonsterType t) : type(t) {
    switch (t) {
        case MON_SCORPION:
            name = "Scorpion";
            hp = 20; maxHp = 20; atk = 7; def = 4; expReward = 12;
            break;
        case MON_RATTLESNAKE:
            name = "Rattlesnake";
            hp = 22; maxHp = 22; atk = 9; def = 1; expReward = 18;
            break;
        case MON_WILD_BOAR:
            name = "Wild Boar";
            hp = 45; maxHp = 45; atk = 12; def = 6; expReward = 28;
            break;
        case MON_CROCODILE:
            name = "Crocodile";
            hp = 55; maxHp = 55; atk = 16; def = 10; expReward = 40;
            break;
        case MON_WOLF:
            name = "Gray Wolf";
            hp = 50; maxHp = 50; atk = 14; def = 5; expReward = 35;
            break;
        case MON_GRIZZLY_BEAR:
            name = "Grizzly Bear";
            hp = 80; maxHp = 80; atk = 20; def = 9; expReward = 55;
            break;
        case MON_BIRD:
            name = "Wild Bird";
            hp = 8; maxHp = 8; atk = 2; def = 0; expReward = 5;
            break;
        case MON_RABBIT:
            name = "Rabbit";
            hp = 10; maxHp = 10; atk = 1; def = 0; expReward = 5;
            break;
        default:
            name = "Scorpion";
            hp = 20; maxHp = 20; atk = 7; def = 3; expReward = 10;
            break;
    }
}

// True if this monster is small game (Bird/Rabbit) hunted for food.
bool isHuntableFood(MonsterType t) {
    return t == MON_BIRD || t == MON_RABBIT;
}

// Item name for the food drop produced by hunting small game (nullptr if none).
const char* foodDropFor(MonsterType t) {
    switch (t) {
        case MON_BIRD:   return "Raw Meat";
        case MON_RABBIT: return "Raw Meat";
        default:         return nullptr;
    }
}

// True if this monster is a dangerous predator rather than harmless game.
bool isDangerousAnimal(MonsterType t) {
    switch (t) {
        case MON_SCORPION:
        case MON_RATTLESNAKE:
        case MON_WILD_BOAR:
        case MON_CROCODILE:
        case MON_WOLF:
        case MON_GRIZZLY_BEAR:
            return true;
        default:
            return false;
    }
}

// Chance (0-100) that an encounter will roll a dangerous predator. Scales with day.
int dangerousAnimalChancePercent(int playerDay) {
    // Day 1 → 15%, then +8% per day passed, capped at 65%.
    int chance = 15 + (playerDay - 1) * 8;
    if (chance < 15) chance = 15;
    if (chance > 65) chance = 65;
    return chance;
}

// Chance (0-100) per bush step that an encounter is triggered. Scales with day.
int bushEncounterChancePercent(int playerDay) {
    // Day 1 → 5%, then +2% per day passed, capped at 20%.
    int chance = 5 + (playerDay - 1) * 2;
    if (chance < 5) chance = 5;
    if (chance > 20) chance = 20;
    return chance;
}

// Simple stat accessors.
std::string Monster::getName() const { return name; }
MonsterType Monster::getType() const { return type; }
int Monster::getHp() const { return hp; }
int Monster::getMaxHp() const { return maxHp; }
int Monster::getAtk() const { return atk; }
int Monster::getDef() const { return def; }
int Monster::getExpReward() const { return expReward; }
bool Monster::isAlive() const { return hp > 0; }

// Subtract damage from the monster's HP, clamped at zero.
void Monster::takeDamage(int dmg) {
    hp -= dmg;
    if (hp < 0) hp = 0;
}

// Roll the monster's attack damage with ±25% variance.
int Monster::attack() const {
    int variance = atk / 4;
    if (variance < 1) variance = 1;
    int dmg = atk + (rand() % (variance * 2 + 1)) - variance;
    if (dmg < 1) dmg = 1;
    return dmg;
}

// Multi-line Braille-art portrait of the monster, used in the combat scene.
std::vector<std::string> Monster::getAsciiArt() const {
    std::vector<std::string> art;
    switch (type) {
        case MON_SCORPION:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⣴⣶⣶⣶⡦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣧⣝⠛⠛⠋⢾⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣠⡙⠿⡟⠋⢀⣀⣀⣌⣽⣯⡷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⠁⠐⠉⠈⢻⣷⡻⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣩⣶⣧⡀⠀⠀⢀⣸⣿⠿⢛⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣿⣯⣷⣶⣦⣠⡾⢡⣾⢿⡿⣿⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢀⣠⡿⠻⣆⣘⢸⣿⣻⣾⣿⡷⣛⠵⢟⣱⡾⣻⣷⣿⢶⣤⣴⣤⠀⠀");
            art.push_back("⠀⠀⣸⠟⣡⣤⣤⣬⣿⣣⣅⣿⡿⣷⣿⣿⣿⣞⠋⣐⣻⠏⣜⠛⢿⣿⣽⣷⣄");
            art.push_back("⠀⢀⡇⢐⣿⢁⡴⠽⣷⣾⡿⠻⣷⣹⡛⠿⠿⣭⣜⡛⠃⠀⠹⠤⠀⠘⣿⣿⣿⣷");
            art.push_back("⠐⠉⠀⠈⣿⢸⣇⢰⡾⠿⠿⠛⠛⠋⡥⢾⠃⣍⠉⠿⠀⠀⠀⠀⣰⣿⠿⠟⢛⡋");
            art.push_back("⠀⠀⠀⠀⣺⢘⡏⢸⠟⠀⢐⣭⢻⠿⣿⠏⠁⠈⠀⠀⠀⠀⠀⠀⠿⠋⠀⢀⣾⠇");
            art.push_back("⠀⠀⠀⠈⠁⠀⣹⠹⣆⢠⣿⣿⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⠟⠁⠀");
            art.push_back("⠀⠀⠀⠀⠀⠘⠁⠀⡾⠸⣿⣟⣿⣷⣶⣶⣤⣀⣀⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⢸⣿⣿⣿⣿⡿⠟⠿⠛⠟⠉⠃⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠛⠛⠉⠺⠷⢦⠶⠷⠊⠀⠀⠀");
            break;
        case MON_RATTLESNAKE:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣄⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡖⣻⠉⢿⣿⠆⠈⠙⢶⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣷⡚⠒⠊⠙⠂⠀⠀⢆⣱⡘⡷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡟⠛⠳⣖⠒⠒⢙⡤⣿⣷⠃⢳⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠀⢻⠆⠤⠤⡗⣿⢻⣼⢀⢷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠇⠀⠸⣼⣏⡒⢲⠟⡟⣾⡾⣎⢾⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡾⠀⢸⡴⢻⠃⠀⡜⢸⣻⠴⠛⠁⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⣰⣰⣷⠏⠀⢰⠃⣿⣷⢳⣰⣤⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠹⣯⣿⣟⠢⢤⣇⣸⣿⡽⣧⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣶⣭⠓⠌⠉⡛⠉⣿⣼⣾⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠃⠀⠀⣰⠁⣼⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠃⠀⠀⠐⠁⣴⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠇⠀⠀⠀⠀⣼⠏⢰⢦⡀⠀⠀⠀⠀⠀⣀⡠⠤⠤⠤⠤⣀⡀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡿⠀⡀⠀⠀⣸⡟⠀⠈⢯⡓⠦⢤⡤⠴⠚⠁⠀⠀⠀⠀⠀⢘⠍⠳⡄⠀⠀⠀⠀");
            art.push_back("⠀⠀⢀⣠⠤⠖⠒⡒⠒⠢⢤⡗⢤⡉⢺⠒⣿⡃⣀⣀⣠⠽⠷⠒⠛⠉⠉⣉⣉⣛⣛⣛⣛⡉⠀⠀⣸⠀⠀⠀⠀");
            art.push_back("⢀⡴⠋⠀⠀⢠⠊⠀⠀⠀⢸⡇⢄⡈⠛⣏⣿⠉⠁⠀⢀⣠⠤⠖⠚⠉⠉⠀⠀⠀⠓⠦⣄⠉⠙⠚⠯⣄⡀⠀⠀");
            art.push_back("⡜⠀⠀⠀⠀⢸⣤⡶⠦⢤⣼⣇⠀⠈⢉⣧⢿⣧⠴⠚⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣷⠀⠀⠀⠀⠉⢦⠀");
            art.push_back("⣇⠀⠀⠀⠀⠈⠳⣄⣀⣀⣈⣿⠑⠢⠤⠼⡞⣿⡄⠀⠀⠀⠀⠀⢀⣀⣠⡤⠴⠶⠶⠒⠒⢿⡇⠀⠀⠀⠀⠸⡆");
            art.push_back("⠘⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢇⠈⠐⠂⠙⣖⠻⣤⣠⣤⡶⠞⠋⠉⠀⠀⠀⠀⠀⠀⢀⡼⠃⠀⠀⠀⠀⢸⠇");
            art.push_back("⠀⠈⠓⢦⣀⠀⠀⠀⠀⠀⠀⠀⠘⢧⡀⠀⠀⠈⠢⠀⠉⠓⠦⠤⢤⣀⣠⠤⠤⠤⠒⠚⠉⠀⠀⠀⠀⠀⣠⡟⠁");
            art.push_back("⠀⠀⠀⠀⠈⠙⠓⠲⠶⠶⠶⠶⠞⠛⠓⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⠟⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠒⠤⢄⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⣤⠴⠞⠋⠀⠀⠀⠀");
            break;
        case MON_WILD_BOAR:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣦⣼⣷⣦⣄⠀⢠⣶⠀⠀⠀⠀⢀⣠⠆⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣿⣿⣿⣿⣿⣿⠀⣿⣿⡆⢀⡀⠀⠛⠟⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣀⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⢀⣿⣿⣇⣸⣿⣿⣶⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢀⣠⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀");
            art.push_back("⠀⠀⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡁⠉⢻⣿⣿⣿⠀⠀⠀⠀");
            art.push_back("⠀⢀⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢹⣿⣿⣿⣿⠈⣿⣿⣿⣷⣾⣿⣿⣿⡄⠀⠀⠀");
            art.push_back("⠀⢸⣿⣿⣿⡏⣿⣿⣿⣿⣿⡅⠸⣿⣿⣿⣿⡇⠸⣿⣿⣿⣿⡿⠉⢿⣿⣦⡄⠀");
            art.push_back("⠀⢸⣿⣿⣿⠀⢻⣿⣿⣿⣿⣧⠀⢻⣿⣿⣿⣷⠀⠻⣿⣯⣭⣴⠆⢸⡿⠋⠀⠀");
            art.push_back("⠀⠀⠈⢿⣿⡇⠈⢻⣿⣿⣿⣿⣧⡀⠙⢿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢸⣿⣿⡄⠀⠙⠋⠉⠛⠋⠉⠀⠈⠻⣿⣇⠀⠀⣶⣶⡄⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢸⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣿⠀⠀⢸⣿⡇⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠛⠛⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠃⠀⠀⠛⠃⠀⠀⠀⠀⠀⠀");
            break;
        case MON_CROCODILE:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⡶⣿⣿⣿⡛⠛⠛⠷⣾⣦⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣋⣵⠟⣿⣙⣿⣟⡻⣦⣦⡌⠹⡿⣷⡄⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣼⢏⡽⠛⠁⠀⠀⠈⠙⢿⣿⣷⣄⢹⡀⣾⣆⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⡿⣷⠞⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣯⠀⢳⣸⣿⡄⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣶⠀⠀⠀⠀⢠⣾⣿⣿⡻⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡟⡇⠀⢺⣿⣿⡇⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣤⣀⣠⣶⣿⣿⣻⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⡿⢷⠋⠉⡽⢸⡿⣇⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣷⣿⣿⣟⡷⠞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⡟⣱⠛⠃⡴⢁⡾⣿⡏⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣤⠶⢟⣡⠿⠋⣡⠼⢧⣾⣿⣿⡇⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣴⢿⣿⣿⢷⡶⢾⣉⣧⣾⣉⣱⢎⣽⣿⣿⠁⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⡾⠛⣾⡾⢿⣩⠝⢻⣀⣴⡟⠻⣤⣿⢧⣾⣿⣿⡟⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⢾⣏⣉⡿⢾⣉⡤⢀⣩⡤⠿⣝⠋⠁⣴⠟⠓⣺⠛⢻⣿⠇⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⣁⣴⡟⢻⠓⢂⣼⠷⠾⡿⢧⡀⢈⣦⡞⠛⢦⣠⡟⠐⠀⢿⣝⣦⡄⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡤⣿⠉⡟⢛⡝⣯⡖⢛⡿⠆⢀⣁⣤⠼⢿⣇⠈⣶⣼⠟⢀⣷⣶⡀⠻⣽⣿⡀⠀⠀⠀");
            art.push_back("⠀⢀⡴⠒⠋⠉⠓⣄⡀⠀⠀⠀⣀⡤⢊⣩⣷⡿⢿⣭⣟⠷⠯⡿⠭⠉⣻⣴⢞⠻⣿⠁⢀⣀⣿⠷⠋⣠⣴⣿⣿⣿⣿⣷⣿⣿⣧⣀⡀⠀");
            art.push_back("⡔⣡⠞⠋⠀⠀⠛⠉⠙⠻⣿⣍⣁⠒⠉⠡⠤⠖⠿⡿⣿⣲⣬⣁⡤⠖⠋⠴⣅⡴⣾⡶⠿⢽⣷⠤⠚⣻⣿⣿⣿⠀⢈⣿⣋⣩⡀⠷⣿⣶");
            art.push_back("⠹⣧⣀⠀⠀⡀⠀⠀⠀⣀⡙⠐⠀⠀⠀⠀⠀⠀⠤⠐⠀⠀⠉⠁⠰⢶⣴⣦⣸⣶⠟⠠⠄⢷⣶⣶⣶⣿⡿⠋⠁⠀⠀⠉⠁⠀⠈⠑⠛⠛");
            art.push_back("⠀⠉⠛⠷⣶⡗⠾⠾⣿⠻⣧⣤⣴⣶⣶⣌⣁⣠⣴⡖⢶⡖⠀⢰⠀⠈⣿⣿⣇⣙⣄⢀⠀⠈⠛⡿⠛⠋⠻⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠈⠁⠀⠀⠁⢠⣿⣿⣿⠟⣿⣿⠿⠿⢿⣿⠚⢣⣴⠃⠀⣤⣈⣿⣿⣿⣿⣿⣿⣶⢋⣠⣤⣤⣦⣰⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⢀⣤⣿⣿⡴⢣⠞⠉⠀⠄⢀⡼⠋⢀⣠⠞⢰⣖⣿⡿⠿⠿⠛⠛⠉⣽⠟⢉⡁⠉⢹⣾⡟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢰⢦⣤⠶⣯⡽⢿⡛⠒⠁⠀⣀⡤⠶⠋⠀⢀⠞⣋⣴⣾⢃⠞⠁⠀⠀⠀⠀⠀⡿⠀⣀⣲⣤⣿⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⣴⡟⠊⠡⠾⡧⣤⣞⣱⣾⣗⢛⣃⣤⣽⡥⢴⣿⠿⠿⠛⠛⠁⠀⠀⠀⠀⠀⢀⡴⠗⠀⢹⡷⢾⡟⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⢻⡿⢷⡦⢼⡶⣏⣨⢿⣿⣿⣯⠴⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠿⣀⣠⡆⠈⢷⣴⣠⣾⣿⠆⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠈⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠁⠀⠈⢷⣴⠋⠈⢻⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            break;
        case MON_WOLF:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⠻⣥⠙⢦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡿⠀⡿⠻⣆⠙⠦⣤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢿⡄⠁⠀⠘⣆⡔⢶⣆⠉⢷⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡄⠀⠀⡿⢿⡀⠉⠀⠞⠹⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡿⡄⠀⡇⠘⣧⣀⣀⣀⠀⠻⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⠃⠁⢀⣠⠞⣹⢿⠻⡟⢿⣿⣯⢳⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⠃⠶⠒⠉⠁⣴⠇⢸⡇⡟⡷⢬⡙⠎⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣿⠇⢀⣠⣄⡀⠚⠁⠀⠈⠀⠀⣷⠀⠉⠛⠛⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⡼⣽⣿⣶⠋⢉⡿⠇⠀⠀⠀⠀⠀⣰⣿⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⢱⣿⣿⠇⠀⣠⣥⣤⡀⠀⠀⠀⢀⡟⣿⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⢿⣿⢀⣾⡟⠉⢹⡇⠀⠀⠀⢸⠁⡿⠙⣿⣷⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⢸⣇⣾⡟⠀⠸⡏⣄⡀⠀⠀⢹⢀⡇⢀⢘⢿⣮⡙⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣇⠀⡀⣧⠰⣿⣶⣄⠀⠀⠀⠘⣎⠳⣿⣿⣦⡀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⡿⣿⣆⠹⣿⡐⣾⣷⣹⣆⠀⠀⠀⠘⢷⣄⣻⣿⣿⣷⡄⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⢿⣿⣦⠽⣇⣹⣟⢿⠙⠁⠀⠀⠀⣤⠉⠻⣿⣿⣿⣿⣦⡀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠙⡟⠂⣿⢹⡿⣼⠇⠀⠀⣀⠀⣷⡀⠀⠈⠻⣿⣿⣿⣷⡀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⡆⢻⠀⠉⢸⡇⠈⣀⣠⣾⠇⠀⠻⣿⣦⣤⣴⣿⠿⣿⡿⣷⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡇⢸⡀⠀⢸⠁⣰⠛⣽⡧⠖⠻⢿⡆⠈⠉⠉⠀⠀⢻⣷⠹⠇");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠘⡇⠀⢸⢰⡏⢰⡟⠀⣀⣀⡼⠃⠀⢀⡆⠀⠀⠘⣿⡆⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣴⣿⣶⣷⣶⣾⣿⣧⣾⣤⣄⣀⣀⣤⣤⣶⡿⠀⠀⠀⢠⣿⡇⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣠⣴⣾⣿⣟⡛⠛⠛⠉⠉⠉⠉⢉⣭⣽⡿⠿⠿⠿⠛⠛⠛⠓⠲⠦⠄⣼⢻⡇⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠘⢉⣼⣿⣿⠿⠛⠛⠁⠀⠀⣠⠖⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⠁⣸⡇⠀");
            art.push_back("⠀⠀⠀⠀⠀⢀⣴⠿⠛⠁⢀⣀⣀⣀⣀⣀⣄⡀⠀⠀⠀⢦⣀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠇⣰⣿⠁⠀");
            art.push_back("⠀⠀⠀⢀⣴⣟⣥⣶⣾⣿⣿⣿⣿⣿⣿⣭⣤⣤⣤⣀⣀⡀⠈⠛⠶⢶⣶⣶⣶⣾⣿⣿⣿⠟⠁⠀⠀");
            art.push_back("⠀⢀⣴⡿⠟⠋⡽⠟⠉⠉⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠙⠛⠛⠛⠿⠿⠿⠿⠟⠛⠉⠁⠀⠀⠀⠀");
            art.push_back("⠐⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            break;
        case MON_GRIZZLY_BEAR:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⣄⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡴⠚⠉⠀⠀⠀⠀⠉⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⠤⠔⡲⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠢⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡴⠚⠁⠀⣠⢊⣀⡀⠀⠀⠀⢀⣀⠤⠤⠤⠤⣀⠤⠚⠉⠉⠉⢷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡾⠋⠀⠀⠀⠀⡟⠉⠀⠉⠙⠒⠲⠁⠀⠀⠀⠀⠀⠀⡴⠚⠉⠉⠂⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⢀⡴⠁⠀⠀⠀⠀⡜⠀⡇⢏⠓⠦⠤⡴⠃⠀⠀⠀⡆⠀⠀⠀⠙⠲⠤⣀⠀⠘⠿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⢀⡞⠀⠀⡴⠀⠀⡸⠁⠀⠸⡌⠁⣠⠞⠀⠀⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠑⢄⠀⢻⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⡾⠀⠀⢰⠁⠀⠠⠇⠀⠀⠀⣧⠈⠁⠀⠀⠀⠀⠀⠀⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡇⠱⡄⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⢸⠃⠀⠀⡜⠀⠀⠀⠀⠀⠀⠀⢿⠀⠀⠀⠀⠀⠀⠀⠀⠿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠹⡄⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⡿⠀⠀⢠⡇⠀⠀⠀⠀⠀⠀⠀⠘⠤⡀⠀⠀⠀⣠⣀⠀⠀⠀⢀⣤⣤⠀⠀⠀⠀⠘⠀⡜⠁⠀⠀⢳⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⢰⠇⠀⠀⠘⣇⠀⠀⢀⠀⠀⠀⠀⠀⠀⠱⡄⠀⠀⠙⠛⠁⠀⠀⠀⢩⠁⠀⠀⠀⢠⣮⠎⠀⠀⠀⠀⢸⡆⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⡾⠀⠀⠀⠀⢻⡄⠀⠸⡇⠀⠀⠀⠀⠀⠀⠹⣄⠀⠀⢿⠀⠀⠀⠀⠘⡄⠀⠀⡰⠋⠁⠀⠀⠀⠀⠀⠈⡇⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⢠⠇⠀⠀⠀⠀⠀⢳⡀⠀⣇⠀⠀⠀⠀⠀⠀⠀⠈⠑⢾⡞⠀⣀⣄⣀⡀⢹⢃⠜⠀⠀⠀⠀⠀⠀⠀⠀⠀⣷⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⣼⠀⠀⠀⠀⠀⠀⠀⠳⡄⢸⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣯⣠⣤⣤⢃⡾⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢹⡄⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢠⡇⠀⠀⠀⠀⠀⠀⠀⠀⠙⢌⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⠷⠶⠟⠋⠀⣠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣄⠀⠀⠀");
            art.push_back("⠀⠀⠀⢸⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡠⠤⠐⠁⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣆⠀⠀");
            art.push_back("⠀⠀⠀⣼⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠀⠀⠀⢠⠛⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⠀⠀");
            art.push_back("⠀⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡼⢧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⡎⠀⠙⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⡇⠀");
            art.push_back("⠀⠀⢰⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠏⠀⢸⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⠀⠀⢳⠀⠀⠈⠳⣄⡰⠀⠀⠀⠀⠀⠀⠀⢀⠇⠀");
            art.push_back("⠀⠀⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠞⠁⠀⠀⠀⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠇⠀⠀⠈⡆⠀⠀⢠⡞⠁⠀⠀⠀⠀⠀⠀⠀⡼⠀⠀");
            art.push_back("⠀⠀⣸⠀⠀⠀⠀⠀⠀⠀⠀⣠⠏⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢧⠀⠀⢰⠃⠀⣤⣏⠀⠀⠀⠀⠀⠀⠀⣠⡞⠁⠀⠀");
            art.push_back("⠀⠀⡿⠀⠀⠀⠀⠀⠀⢀⡴⠃⠀⠀⠀⠀⠀⠀⢸⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠃⠀⡇⠀⠀⢻⡿⣦⡀⠀⠀⠀⣠⠾⠋⠀⠀⠀⠀");
            art.push_back("⠀⢰⡇⠀⠀⠀⠀⠀⠀⡜⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣧⠀⠀⠀⠀⠀⠀⠀⠀⠸⠀⢸⠁⠀⠀⠀⠉⠉⠙⠓⠚⠋⠁⠀⠀⠀⠀⠀⠀");
            art.push_back("⠠⠼⠥⠤⠤⠤⠤⠤⠤⠧⠤⠤⠤⠀⠀⠀⠀⠀⠀⠀⠘⣧⠀⠀⠀⠀⠀⠀⠀⠀⣇⣸⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠉⠉⠉⠒⠒⠒⠒⠛⠛⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            break;
        case MON_BIRD:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⢠⣀⠀⠙⠳⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⢶⢄⠀⢹⡉⠂⡀⢳⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⣠⡀⠸⠢⣀⠙⢆⠀⢳⡀⠐⠈⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⣄⠈⢿⡢⣅⠈⢳⡈⠀⢀⣱⡈⠄⠸⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠳⡉⠲⣽⣿⣷⣆⡽⣦⠀⠀⢳⡈⡄⢳⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⢀⠈⣢⡀⠉⠻⣿⣷⣿⠷⡄⠈⢷⡰⡈⢧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠈⡳⣌⠙⠦⣀⠈⠹⣿⡆⢻⡄⡘⣿⣱⠸⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠻⡲⢽⣦⣈⢳⣦⣘⢻⣆⢿⡵⣚⣿⣧⢻⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠈⠳⢬⣛⡷⣾⣽⡏⠸⣟⣿⣽⣞⣿⣿⠿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⡻⣶⣭⣟⡷⣿⣿⣦⡬⢿⣿⣾⣻⡏⢠⢧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠠⣬⣻⣿⣿⣿⣧⡀⠙⢏⣿⣿⡿⠁⡀⢸⡆⠀⣤⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠈⣙⡻⢿⣿⣿⣟⣦⣄⠙⢿⣿⠀⠆⣼⣷⠰⢻⡅⠀⠀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠈⠑⠫⢥⣯⢿⡿⠾⢿⣿⣧⣿⣍⣺⣿⣻⡆⠟⢀⡴⠋⠁⠀⠀⠉⠓⢦⡀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠈⠛⢿⣿⣿⣿⣿⣾⣽⣿⣿⢶⣿⣳⣯⢿⣠⡏⢠⠀⠄⡀⠀⠀⢰⣤⣍⣹⠷⠶⣶⣢⣤⣤");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⣈⡻⠿⢿⣏⣉⣿⣿⣿⣿⣯⢷⣻⣿⣻⣜⡣⣍⠲⣠⢁⡦⠞⠉⠀⠀⠀⠀⠀⠀⠀⠈");
            art.push_back("⠀⠀⠀⠀⠀⠀⠈⣓⣤⣤⣷⣿⣿⢿⣿⣿⣿⣯⣿⣿⡷⣯⣷⡳⣎⡗⣦⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠐⠟⠛⠛⠋⣉⣥⣾⣠⣿⣿⣿⣿⣿⣿⣿⣽⣻⡷⣏⡾⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠰⠛⢋⣴⢿⣻⣿⣿⣿⣿⣿⣿⣟⣾⣟⣿⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠞⡡⢏⣷⣿⣿⣿⣿⣿⣿⣿⣛⡾⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⣠⠋⠠⢑⣿⠿⣿⣿⣿⠿⠿⠛⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠰⠋⢀⣱⣽⣿⠘⣷⡀⡠⢄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⢀⣴⣾⣿⣿⣿⣿⡆⠘⣇⠱⣄⠙⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⡏⣿⡇⠀⢸⣆⠹⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⢠⡟⣻⣿⣿⣿⣿⢹⡿⣷⢀⠈⢿⡄⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⢠⠏⣸⣿⡿⣿⡿⡟⢸⣟⣿⠀⢹⡄⢹⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⣿⣠⣿⡿⠁⣿⣿⠇⢸⣯⢿⢀⣏⢿⡀⢳⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠉⣿⠟⠁⠀⣿⡟⠀⠸⣾⣿⠀⠻⠘⢺⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠙⠃⠀⠀⠹⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            break;
        case MON_RABBIT:
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⣠⣤⣦⣤⣄⡀⠀⠀⠀⠀⢀⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⣰⠟⠙⠀⠀⠀⠈⢻⡆⠀⣴⠞⠋⠉⠉⠙⠳⣦⡀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⢸⡛⠂⠀⠀⠀⠀⠀⠈⣿⣾⠋⠀⠀⠀⠀⠀⠀⠈⣿⡄⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⣽⠁⠀⠀⠀⠀⠀⠀⠀⣽⢇⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⢰⣿⠄⠀⠀⠀⠀⠀⠀⠐⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⢺⡇⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⢨⡟⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⠇⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠈⣿⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⢠⡿⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⣿⡆⠀⠀⢀⣀⣀⡀⢸⣇⠀⠀⠀⠀⠀⠀⠀⢀⣾⠃⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⣘⡟⠰⠛⠛⠉⠙⠉⠈⠃⠀⠀⠀⠀⠀⠀⢰⣾⡟⠚⢶⣄⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⣤⡾⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⡁⠀⢀⡬⢹⡇⠀⠀⠀⠀");
            art.push_back("⠀⠀⣴⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣷⠀⠚⢷⣼⡷⠀⠀⠀⠀");
            art.push_back("⠀⣼⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢙⣷⠀⠀⠘⢿⣷⠀⠀⠀");
            art.push_back("⢸⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣇⠀⠀⠀⢹⣧⠀⠀");
            art.push_back("⣿⢣⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡏⣡⠀⠀⠀⠻⣧⠀");
            art.push_back("⣿⡾⡿⠖⠀⠀⠀⠀⠀⠀⠀⠀⢀⣶⣿⣤⠀⠀⠀⠀⠀⠀⠀⣼⡇⠃⠀⠀⠀⠀⢹⣇");
            art.push_back("⠹⣧⡀⠀⠀⠰⣦⣸⣶⠄⠀⠀⠸⡿⠿⠇⠀⠀⠀⠀⠀⠀⢢⡿⠅⠀⠀⠀⠀⠀⠀⣿");
            art.push_back("⠀⠈⠻⣦⣒⠸⠛⠻⠖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⠟⠁⠀⠀⠀⠀⣄⠀⠀⣾");
            art.push_back("⠀⠀⠀⠈⢙⣷⢶⣤⣀⣀⠀⠀⠀⠀⠀⠀⠀⣀⣤⡶⠟⠁⠀⠀⠀⠀⠀⣼⢏⣠⣾⠟");
            art.push_back("⠀⠀⠀⢀⣾⠃⠀⠀⠉⠛⠛⠻⠶⠶⠶⠶⠞⠋⠁⠀⠀⠀⠀⠀⠀⣰⡾⠛⠛⠉⠀⠀");
            art.push_back("⠀⠀⠀⠘⣿⠀⠀⠀⠀⠀⢲⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⣠⡾⠏⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠻⣧⡀⠀⠀⣡⣿⠛⠻⠶⣾⠀⠀⠀⠀⠀⠀⠈⢾⡟⠆⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠉⠛⠛⠛⠋⠁⠀⠀⠀⢿⣦⠀⠀⠀⠀⠀⣠⡾⠁⠀⠀⠀⠀⠀⠀⠀⠀");
            art.push_back("⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣶⣤⣀⣦⣴⡟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀");
            break;
        default:
            art.push_back("                          ???                           ");
            break;
    }
    return art;
}

// Intro line shown when the monster first appears in combat.
std::string Monster::getFlavorText() const {
    switch (type) {
        case MON_SCORPION:
            return "* A Scorpion raises its venomous tail, ready to strike!";
        case MON_RATTLESNAKE:
            return "* A Rattlesnake coils up, rattling a deadly warning!";
        case MON_WILD_BOAR:
            return "* A Wild Boar charges from the undergrowth, tusks bared!";
        case MON_CROCODILE:
            return "* A Crocodile lunges from the water, jaws wide open!";
        case MON_WOLF:
            return "* A Gray Wolf snarls, teeth dripping with saliva!";
        case MON_GRIZZLY_BEAR:
            return "* A Grizzly Bear towers above you, roaring in fury!";
        case MON_BIRD:
            return "* A Wild Bird flutters down -- looks like easy game.";
        case MON_RABBIT:
            return "* A Rabbit freezes in the grass, twitching its nose.";
        default:
            return "* Something dangerous appears!";
    }
}

// Phrase used when the monster strikes the player ("X gores you...").
std::string Monster::getAttackText() const {
    switch (type) {
        case MON_SCORPION:
            return " drives its stinger into your flesh!";
        case MON_RATTLESNAKE:
            return " strikes and sinks its fangs into you!";
        case MON_WILD_BOAR:
            return " gores you with its tusks!";
        case MON_CROCODILE:
            return " clamps its jaws and death-rolls!";
        case MON_WOLF:
            return " lunges for your throat!";
        case MON_GRIZZLY_BEAR:
            return " mauls you with crushing force!";
        case MON_BIRD:
            return " pecks at you for";
        case MON_RABBIT:
            return " kicks at you for";
        default:
            return " attacks!";
    }
}

// Random loot drop list for this monster (item name + quantity pairs).
std::vector<std::pair<std::string, int>> Monster::getDrops() const {
    std::vector<std::pair<std::string, int>> drops;

    switch (type) {
        case MON_SCORPION:
            // Venomous — flesh is not safely edible.
            if (rand() % 100 < 40)
                drops.push_back({"Venom", 1});
            break;

        case MON_RATTLESNAKE:
            drops.push_back({"Venom", 1});
            if (rand() % 100 < 25)
                drops.push_back({"Medical Herb", 1});
            break;

        case MON_WILD_BOAR:
            // Large animal — always drops good meat
            drops.push_back({"Raw Meat", 2 + (rand() % 100 < 40 ? 1 : 0)});
            break;

        case MON_CROCODILE:
            drops.push_back({"Raw Meat", 2});
            break;

        case MON_WOLF:
            drops.push_back({"Raw Meat", 1});
            break;

        case MON_GRIZZLY_BEAR:
            drops.push_back({"Raw Meat", 3});
            drops.push_back({"Bear Skin", 1});
            break;

        default:
            break;
    }

    return drops;
}

// Pick a random monster appropriate for the current map and survival day,
// using per-map safe/dangerous pools and the day-scaled danger probability.
Monster spawnRandomMonster(int mapId, int playerDay) {
    static bool seeded = false;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = true; }

    std::vector<MonsterType> safePool;
    std::vector<MonsterType> dangerPool;

    switch (mapId) {
        case 0: // Beach
            safePool.push_back(MON_BIRD);
            dangerPool.push_back(MON_SCORPION);
            if (playerDay >= 3) dangerPool.push_back(MON_RATTLESNAKE);
            break;
        case 1: // Cave
            safePool.push_back(MON_BIRD);
            dangerPool.push_back(MON_SCORPION);
            dangerPool.push_back(MON_RATTLESNAKE);
            break;
        case 2: // Forest
            safePool.push_back(MON_BIRD);
            safePool.push_back(MON_RABBIT);
            dangerPool.push_back(MON_WILD_BOAR);
            dangerPool.push_back(MON_RATTLESNAKE);
            if (playerDay >= 3) dangerPool.push_back(MON_GRIZZLY_BEAR);
            if (playerDay >= 5) dangerPool.push_back(MON_WOLF);
            break;
        case 3: // Village
            safePool.push_back(MON_RABBIT);
            dangerPool.push_back(MON_WILD_BOAR);
            if (playerDay >= 4) dangerPool.push_back(MON_WOLF);
            break;
        case 4: // Shore
            safePool.push_back(MON_BIRD);
            dangerPool.push_back(MON_CROCODILE);
            if (playerDay >= 3) dangerPool.push_back(MON_GRIZZLY_BEAR);
            if (playerDay >= 6) dangerPool.push_back(MON_WOLF);
            break;
        default:
            dangerPool.push_back(MON_SCORPION);
            break;
    }

    bool chooseDangerous;
    if (dangerPool.empty())      chooseDangerous = false;
    else if (safePool.empty())   chooseDangerous = true;
    else chooseDangerous = (rand() % 100) < dangerousAnimalChancePercent(playerDay);

    const std::vector<MonsterType>& pool = chooseDangerous ? dangerPool : safePool;
    int idx = rand() % (int)pool.size();
    return Monster(pool[idx]);
}
