#include "status_ui.h"
#include "UI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace std;

// Clamp a stat value to the valid 0-100 range.
int clampStat(int value) {
    return max(0, min(100, value));
}

// Build a "[####----]" status bar of `width` cells from a 0-100 value.
string makeBar(int value, int width) {
    value = clampStat(value);
    int filled = value * width / 100;
    string bar = "[";
    bar += string(filled, '#');
    bar += string(width - filled, '-');
    bar += "]";
    return bar;
}

static const int STATUS_WIDTH = 60;
// Centering padding used by every status panel line.
static string statusPad() { return leftPad(STATUS_WIDTH); }

// Format the current in-game day and 24-hour clock as e.g. "DAY 3 08 AM".
string formatClock(int day, int hour) {
    int h12 = hour % 12;
    if (h12 == 0) h12 = 12;
    string ampm = (hour < 12) ? "AM" : "PM";
    ostringstream oss;
    oss << "DAY " << day << " "
        << setw(2) << setfill('0') << h12 << " " << ampm;
    return oss.str();
}

// Build the status panel as a list of pre-rendered text lines (so it can be
// embedded in a multi-column layout, e.g. inventory + status side by side).
vector<string> buildStatusLines(const PlayerStatus& player, int day,
                                const string& timeOfDay, const string& location) {
    vector<string> lines;
    lines.push_back("============================================================");
    lines.push_back(" Day " + to_string(day) + " | " + timeOfDay + " | " + location);
    lines.push_back("============================================================");
    lines.push_back(string("HP      ") + makeBar(player.hp)      + " " + to_string(player.hp));
    lines.push_back(string("Hunger  ") + makeBar(player.hunger)  + " " + to_string(player.hunger));
    lines.push_back(string("Thirst  ") + makeBar(player.thirst)  + " " + to_string(player.thirst));
    lines.push_back(string("Fatigue ") + makeBar(player.fatigue) + " " + to_string(player.fatigue));
    lines.push_back("============================================================");
    return lines;
}

// Render the world-screen top bar: separator, "Day | clock | location  legend"
// info row, then a closing separator.
void displayTopBar(int /*day*/, const string& clockStr, const string& timeOfDay,
                   const string& location, const string& legend) {
    string sep(STATUS_WIDTH, '=');
    cout << statusPad() << sep << '\n';

    ostringstream oss;
    oss << " " << clockStr << " | " << timeOfDay << " | " << location;
    if (!legend.empty()) oss << "   " << legend;
    string line = oss.str();
    int dw = displayWidthUtf8(line);
    if (dw < STATUS_WIDTH) line += string(STATUS_WIDTH - dw, ' ');
    cout << statusPad() << line << '\n';

    cout << statusPad() << sep << '\n';
}

// Print the four stat bars (HP / Hunger / Thirst / Fatigue). The caller is
// responsible for any surrounding separator lines.
void displayStatBars(const PlayerStatus& player) {
    auto statLine = [&](const string& label, int value) {
        ostringstream oss;
        oss << " " << left << setw(10) << label << " "
            << makeBar(value) << " " << value;
        string line = oss.str();
        int dw = displayWidthUtf8(line);
        if (dw < STATUS_WIDTH) line += string(STATUS_WIDTH - dw, ' ');
        cout << statusPad() << line << '\n';
    };

    statLine("HP",      player.hp);
    statLine("Hunger",  player.hunger);
    statLine("Thirst",  player.thirst);
    statLine("Fatigue", player.fatigue);
}

// Print one full-width "===" separator line.
void displaySep() {
    string sep(STATUS_WIDTH, '=');
    cout << statusPad() << sep << '\n';
}

// Render the full standalone status panel (header + stat bars).
void displayStatus(const PlayerStatus& player, int day, const string& timeOfDay, const string& location) {
    string sep(STATUS_WIDTH, '=');
    cout << statusPad() << sep << '\n';

    {
        ostringstream oss;
        oss << " Day " << day << " | " << timeOfDay << " | " << location;
        string line = oss.str();
        int dw = displayWidthUtf8(line);
        if (dw < STATUS_WIDTH) line += string(STATUS_WIDTH - dw, ' ');
        cout << statusPad() << line << '\n';
    }

    cout << statusPad() << sep << '\n';

    auto statLine = [&](const string& label, int value) {
        ostringstream oss;
        oss << " " << left << setw(10) << label << " "
            << makeBar(value) << " " << value;
        string line = oss.str();
        int dw = displayWidthUtf8(line);
        if (dw < STATUS_WIDTH) line += string(STATUS_WIDTH - dw, ' ');
        cout << statusPad() << line << '\n';
    };

    statLine("HP",      player.hp);
    statLine("Hunger",  player.hunger);
    statLine("Thirst",  player.thirst);
    statLine("Fatigue", player.fatigue);

    cout << statusPad() << sep << '\n';
}
