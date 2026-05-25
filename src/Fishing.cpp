#include "Fishing.h"
#include "UI.h"
#include <iostream>
#include <string>
#include <cstdlib>

// Cross-platform helpers used by the fishing animation: sleep, non-blocking
// key check, and a blocking key read.
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
static void sleepMs(int ms) { Sleep(ms); }
static bool keyAvailable()  { return _kbhit() != 0; }
static int  readKey()       { return _getch(); }
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
static void sleepMs(int ms) { usleep(ms * 1000); }
static bool keyAvailable() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}
static int readKey() { return getchar(); }
#endif

// ── Bar layout (16 chars) ─────────────────────────────────
// Positions  0- 5: MISS (-)    6 chars
// Positions  6- 8: HIT  (=)    3 chars
// Positions  9-10: BEST (*)    2 chars
// Positions 11-12: HIT  (=)    2 chars
// Positions 13-15: MISS (-)    3 chars
static const int BAR_WIDTH = 16;

// Character used to draw a given column of the fishing bar.
static char zoneChar(int pos) {
    if (pos >= 9 && pos <= 10) return '*';
    if (pos >= 6 && pos <= 12) return '=';
    return '-';
}

// Build the display bar with the moving marker shown as '|'.
static std::string makeBar(int markerPos) {
    std::string bar = "[";
    for (int i = 0; i < BAR_WIDTH; i++)
        bar += (i == markerPos) ? '|' : zoneChar(i);
    bar += "]";
    return bar;
}

enum Zone { ZONE_MISS, ZONE_HIT, ZONE_BEST };

// Map a marker column to its zone (MISS / HIT / BEST).
static Zone getZone(int pos) {
    if (pos >= 9 && pos <= 10) return ZONE_BEST;
    if (pos >= 6 && pos <= 12) return ZONE_HIT;
    return ZONE_MISS;
}

// Roll loot for the given zone. Returns item name or "" for nothing.
// BEST: 70% fish, else 50% cloth scrap
// HIT:  50% fish, else 25% cloth scrap
// MISS:  0% fish,      5% cloth scrap
// Roll loot based on which zone the player stopped on; returns "" on a miss.
static std::string rollLoot(Zone zone) {
    int r = rand() % 100;
    if (zone == ZONE_BEST) {
        if (r < 70) return "Fish";
        if (r < 70 + 50) return "Cloth Scrap";
    } else if (zone == ZONE_HIT) {
        if (r < 50) return "Fish";
        if (r < 50 + 25) return "Cloth Scrap";
    } else {
        if (r < 5) return "Cloth Scrap";
    }
    return "";
}

// ── Main fishing session ──────────────────────────────────
// Run the fishing minigame loop: cast, animate the marker, evaluate the
// stop position, award loot, and repeat until bait runs out or player quits.
void startFishing(Player& player) {
    int totalCaught = 0;

    // No bait → cannot fish at all.
    if (!player.inventory.hasItem("Bait", 1)) {
        clearScreen();
        printVerticalPadding(5);
        printCentered("========== FISHING ==========");
        std::cout << '\n';
        printCentered("You have no Bait! (Craft from Berry)");
        std::cout << '\n';
        printCentered("[Any key] Continue");
        std::cout.flush();
        getInput();
        return;
    }

    while (player.inventory.hasItem("Bait", 1)) {
        int baitLeft = 0;
        for (int i = 0; i < player.inventory.size(); i++)
            if (player.inventory.getItem(i).name == "Bait")
                baitLeft = player.inventory.getItem(i).quantity;

        // ── Waiting screen ──────────────────────────────────
        clearScreen();
        printVerticalPadding(12);
        printCentered("========== FISHING ==========");
        std::cout << '\n';
        printCentered("Bait remaining : " + std::to_string(baitLeft));
        printCentered("Fish caught    : " + std::to_string(totalCaught));
        std::cout << '\n';
        printCentered("Zone guide:");
        printCentered("[------===**==---]");
        printCentered(" MISS  HIT ** HIT");
        std::cout << '\n';
        printCentered("[ENTER] Cast   [Q] Leave");
        std::cout.flush();

        char start = getInput();
        if (start == 'q' || start == 'Q') break;

        // Consume 1 bait for this cast.
        player.inventory.consumeItem("Bait", 1);
        baitLeft--;

        // ── Animation phase ─────────────────────────────────
        int  markerPos  = 0;
        int  direction  = 1;
        int  frameMs    = 70;
        int  frameTick  = 0;
        bool stopped    = false;

        while (!stopped) {
            clearScreen();
            printVerticalPadding(10);
            printCentered("========== FISHING ==========");
            std::cout << '\n';
            printCentered("Bait remaining : " + std::to_string(baitLeft));
            printCentered("Fish caught    : " + std::to_string(totalCaught));
            std::cout << '\n';
            printCentered(makeBar(markerPos));
            printCentered("     MISS      HIT ** HIT  MISS");
            std::cout << '\n';
            printCentered("[ENTER] Stop!");
            std::cout.flush();

            sleepMs(frameMs);

            if (keyAvailable()) {
                int c = readKey();
                if (c == '\r' || c == '\n' || c == ' ') {
                    stopped = true;
                } else if (c == 'q' || c == 'Q') {
                    clearScreen();
                    printVerticalPadding(5);
                    printCentered("You reel in your line.");
                    std::cout << '\n';
                    printCentered("Total fish caught: " + std::to_string(totalCaught));
                    std::cout << '\n';
                    printCentered("[Any key] Continue");
                    getInput();
                    return;
                }
            }

            if (!stopped) {
                markerPos += direction;
                if (markerPos >= BAR_WIDTH - 1) { markerPos = BAR_WIDTH - 1; direction = -1; }
                if (markerPos <= 0)             { markerPos = 0;              direction =  1; }

                frameTick++;
                if (frameTick % 15 == 0 && frameMs > 35)
                    frameMs -= 5;
            }
        }

        // ── Evaluate zone & roll loot ────────────────────────
        Zone zone = getZone(markerPos);
        std::string loot = rollLoot(zone);
        std::string resultLine;

        if (loot == "Fish") {
            player.inventory.addItem("Fish", 1);
            totalCaught++;
            if (zone == ZONE_BEST)
                resultLine = "** PERFECT! ** You caught a fish!";
            else
                resultLine = "Nice catch!  You got 1 Fish.";
        } else if (loot == "Cloth Scrap") {
            player.inventory.addItem("Cloth Scrap", 1);
            resultLine = "Something got caught... 1 Cloth Scrap.";
        } else {
            if (zone == ZONE_MISS)
                resultLine = "Missed...  The fish got away.";
            else
                resultLine = "So close! The fish slipped off the hook.";
        }

        // ── Result screen ────────────────────────────────────
        clearScreen();
        printVerticalPadding(10);
        printCentered("========== FISHING ==========");
        std::cout << '\n';
        printCentered(makeBar(markerPos));
        printCentered("     MISS      HIT ** HIT  MISS");
        std::cout << '\n';
        printCentered(resultLine);
        std::cout << '\n';

        if (baitLeft > 0)
            printCentered("Bait left: " + std::to_string(baitLeft) +
                          "   [Any key] Next cast   [Q] Leave");
        else
            printCentered("Out of bait.   [Any key] Done");
        std::cout.flush();

        char cont = getInput();
        if ((cont == 'q' || cont == 'Q') && baitLeft > 0) break;
    }

    // ── End summary ──────────────────────────────────────────
    clearScreen();
    printVerticalPadding(8);
    printCentered("========== FISHING ==========");
    std::cout << '\n';
    printCentered("You reel in your line.");
    std::cout << '\n';
    printCentered("Total fish caught this session: " + std::to_string(totalCaught));
    std::cout << '\n';
    printCentered("[Any key] Continue");
    std::cout.flush();
    getInput();
}
