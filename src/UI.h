#ifndef UI_H
#define UI_H

#include <string>

enum Difficulty { DIFF_EASY = 1, DIFF_NORMAL, DIFF_HARD };

char getInput();
void clearScreen();

void displayTitleScreen();
void displayMainMenu();
char getValidMenuChoice();
void displayGameOver(bool playerWon);

Difficulty selectDifficulty();

// Layout helpers (used to center the whole UI on screen)
int getTerminalWidth();
int getTerminalHeight();
int displayWidthUtf8(const std::string& s);
std::string leftPad(int width);
std::string centerPad(int contentWidth);
void printCentered(const std::string& line);
void printBlockLine(const std::string& line, int blockWidth);
void printVerticalPadding(int contentHeight);

#endif
