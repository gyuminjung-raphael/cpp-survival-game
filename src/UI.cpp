#include "UI.h"
#include <iostream>
#include <string>
#include <cctype>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
// Read one keypress without echo or Enter (Windows).
char getInput() {
    return _getch();
}
// Current terminal width in columns (Windows).
int getTerminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return 100;
}
// Current terminal height in rows (Windows).
int getTerminalHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return 30;
}
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
// Read one keypress without echo or Enter (POSIX).
char getInput() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
// Current terminal width in columns (POSIX).
int getTerminalWidth() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return w.ws_col;
    return 100;
}
// Current terminal height in rows (POSIX).
int getTerminalHeight() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0)
        return w.ws_row;
    return 30;
}
#endif

// Number of display columns occupied by a UTF-8 string (treats non-ASCII
// codepoints as 1 column; sufficient for the layout used by this game).
int displayWidthUtf8(const std::string& s) {
    int w = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = s[i];
        if (c < 0x80)        { w++; i += 1; }
        else if (c < 0xC0)   { i += 1; }
        else if (c < 0xE0)   { w++; i += 2; }
        else if (c < 0xF0)   { w++; i += 3; }
        else                  { w++; i += 4; }
    }
    return w;
}

// Whitespace prefix that horizontally centers `width` columns in the terminal.
std::string leftPad(int width) {
    int term = getTerminalWidth();
    int pad = (term - width) / 2;
    if (pad < 0) pad = 0;
    return std::string(pad, ' ');
}

// Alias for leftPad — kept for code that reads "center" intent more clearly.
std::string centerPad(int contentWidth) {
    return leftPad(contentWidth);
}

// Print a single line horizontally centered on the terminal.
void printCentered(const std::string& line) {
    std::cout << leftPad(displayWidthUtf8(line)) << line << '\n';
}

// Print a line left-aligned within a fixed-width block centered on screen.
void printBlockLine(const std::string& line, int blockWidth) {
    std::cout << leftPad(blockWidth) << line << '\n';
}

// Clear the terminal and move the cursor to the top-left.
void clearScreen() {
    std::cout << "\033[2J\033[H";
}

// Emit blank lines so a `contentHeight`-tall block ends up vertically centered.
void printVerticalPadding(int contentHeight) {
    int term = getTerminalHeight();
    int pad = (term - contentHeight) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) std::cout << '\n';
}

// Render the opening title splash and wait for Enter.
void displayTitleScreen() {
    clearScreen();
    printVerticalPadding(12);
    printCentered("===============================================");
    printCentered("|                                             |");
    printCentered("|        ISLAND ESCAPE: An RPG Adventure      |");
    printCentered("|                                             |");
    printCentered("|      University Group Project - COMP2113    |");
    printCentered("|                                             |");
    printCentered("===============================================");
    std::cout << '\n';
    printCentered("You awake on an uncharted island...");
    printCentered("Can you survive and escape?");
    std::cout << '\n';
    printCentered("Press Enter to continue...");
    std::cin.ignore(10000, '\n');
}

// Render the main menu (New / Load / Quit).
void displayMainMenu() {
    clearScreen();
    printVerticalPadding(8);
    printCentered("========== MAIN MENU ==========");
    printCentered("1. Start New Game             ");
    printCentered("2. Load Saved Game            ");
    printCentered("3. Quit                       ");
    printCentered("===============================");
    std::cout << '\n';
    std::cout << leftPad(30) << "Enter your choice (1-3): ";
}

// Re-prompt until the user enters '1', '2', or '3'.
char getValidMenuChoice() {
    std::string input;
    char choice;

    while (true) {
        std::getline(std::cin, input);

        if (input.length() != 1 || !std::isdigit(input[0])) {
            std::cout << leftPad(40) << "Invalid input. Please enter 1, 2, or 3: ";
            continue;
        }

        choice = input[0];

        if (choice >= '1' && choice <= '3') {
            return choice;
        } else {
            std::cout << leftPad(40) << "Invalid choice. Please enter 1, 2, or 3: ";
        }
    }
}

// Render the difficulty menu and return the player's chosen level.
Difficulty selectDifficulty() {
    clearScreen();
    printVerticalPadding(8);
    printCentered("========== SELECT DIFFICULTY ==========");
    printCentered("1. Easy    - Relaxed survival          ");
    printCentered("2. Normal  - Balanced challenge        ");
    printCentered("3. Hard    - Harsh island, unforgiving ");
    printCentered("=======================================");
    std::cout << '\n';
    std::cout << leftPad(30) << "Enter your choice (1-3): ";

    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input.length() == 1 && input[0] >= '1' && input[0] <= '3')
            return (Difficulty)(input[0] - '0');
        std::cout << leftPad(40) << "Invalid choice. Please enter 1, 2, or 3: ";
    }
}

// Render the win or loss banner at end of game.
void displayGameOver(bool playerWon) {
    clearScreen();
    printVerticalPadding(4);
    if (playerWon) {
        printCentered("========================================");
        printCentered("|    CONGRATULATIONS! YOU ESCAPED!    |");
        printCentered("========================================");
    } else {
        printCentered("========================================");
        printCentered("|         GAME OVER - YOU LOST         |");
        printCentered("========================================");
    }
    std::cout << '\n';
}
