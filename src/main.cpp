#include "Game.h"
#include "UI.h"
#include "SaveLoad.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

// Program entry point: ensures UTF-8 console output on Windows, shows the
// title screen, then loops the main menu until the user chooses Quit.
int main() {
#ifdef _WIN32
    // Force UTF-8 output so Unicode art (Braille, emoji) renders correctly
    // instead of being misread as  multibyte encodings.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    displayTitleScreen();

    bool running = true;
    while (running) {
        displayMainMenu();
        char choice = getValidMenuChoice();

        switch (choice) {
            case '1': {
                Difficulty diff = selectDifficulty();
                clearScreen();
                runGame(false, diff);
                break;
            }
            case '2':
                clearScreen();
                if (hasSaveFile()) {
                    runGame(true);
                } else {
                    printVerticalPadding(2);
                    printCentered("No save file found. Start a new game first.");
                    std::cout << '\n';
                    printCentered("Press Enter to return to menu...");
                    std::cin.ignore(10000, '\n');
                }
                break;
            case '3':
                clearScreen();
                std::cout << "  Thank you for playing!\n";
                running = false;
                break;
        }
    }

    return 0;
}
