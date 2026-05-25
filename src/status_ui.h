#ifndef STATUS_UI_H
#define STATUS_UI_H

#include <string>
#include <vector>
using namespace std;

struct PlayerStatus {
    int hp;
    int hunger;
    int thirst;
    int fatigue;
};

string makeBar(int value, int width = 20);
string formatClock(int day, int hour);
void displayTopBar(int day, const string& clockStr, const string& timeOfDay,
                   const string& location, const string& legend);
void displayStatBars(const PlayerStatus& player);
void displaySep();
void displayStatus(const PlayerStatus& player, int day, const string& timeOfDay, const string& location);
vector<string> buildStatusLines(const PlayerStatus& player, int day,
                                const string& timeOfDay, const string& location);

#endif