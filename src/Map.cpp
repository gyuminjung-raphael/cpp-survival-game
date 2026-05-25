#include "Map.h"
#include "UI.h"
#include <fstream>
#include <iostream>
#include <algorithm>

// Default constructor: zero out dimensions and spawn coordinates.
Map::Map() : rows(0), cols(0), spawnRow(0), spawnCol(0) {}

// Load a map from a text file: first line is "rows cols", then ASCII grid.
// Captures the '@' marker as the spawn point.
bool Map::loadFromFile(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cerr << "Error: cannot open " << filename << std::endl;
        return false;
    }

    fin >> rows >> cols;
    fin.ignore();

    grid.clear();
    grid.resize(rows);

    for (int r = 0; r < rows; r++) {
        std::getline(fin, grid[r]);
        if ((int)grid[r].size() < cols)
            grid[r].resize(cols, ' ');

        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == '@') {
                spawnRow = r;
                spawnCol = c;
                grid[r][c] = ' ';
            }
        }
    }

    fin.close();
    return true;
}

// Render the map as ASCII characters, centered on the player's view window.
void Map::render(int playerRow, int playerCol, int viewH, int viewW) const {
    int startR = playerRow - viewH / 2;
    int startC = playerCol - viewW / 2;

    if (startR < 0) startR = 0;
    if (startC < 0) startC = 0;
    if (startR + viewH > rows) startR = rows - viewH;
    if (startC + viewW > cols) startC = cols - viewW;
    if (startR < 0) startR = 0;
    if (startC < 0) startC = 0;

    int endR = std::min(startR + viewH, rows);
    int endC = std::min(startC + viewW, cols);

    int contentW = endC - startC;
    std::string pad = leftPad(contentW);
    // Render each tile as exactly one cell so the grid stays rectangular.
    // For edge exit arrows, we "extend" visually by overwriting the two
    // adjacent grass cells with '=' so the arrow reads as "<==" / "==>"
    // without changing the row width.
    for (int r = startR; r < endR; r++) {
        std::cout << pad;

        bool leftArrow  = (startC == 0)   && !grid[r].empty() &&
                          grid[r][0] == '<';
        bool rightArrow = (endC == cols)  && (int)grid[r].size() >= cols &&
                          grid[r][cols - 1] == '>';

        for (int c = startC; c < endC; c++) {
            if (r == playerRow && c == playerCol) {
                std::cout << '@';
                continue;
            }

            if (leftArrow && c >= 0 && c <= 2) {
                static const char LEFT[4] = "<==";
                std::cout << LEFT[c];
                continue;
            }
            if (rightArrow && c >= cols - 3 && c <= cols - 1) {
                static const char RIGHT[4] = "==>";
                std::cout << RIGHT[c - (cols - 3)];
                continue;
            }

            if (c < (int)grid[r].size())
                std::cout << grid[r][c];
            else
                std::cout << ' ';
        }
        std::cout << '\n';
    }
}

// Convert a single map tile character into its UTF-8 emoji representation.
static std::string tileToEmoji(char c) {
    switch (c) {
        case '~': return "\xF0\x9F\x8C\x8A"; // 🌊 water
        case '.': return "\xF0\x9F\x9F\xA8"; // 🟨 sand
        case ',': return "\xF0\x9F\x8C\xBF"; // 🌿 grass
        case ':': return "\xF0\x9F\x9F\xAB"; // 🟫 path
        case '#': return "\xF0\x9F\xA7\xB1"; // 🧱 wall
        case '+': return "\xF0\x9F\xA7\xB1"; // 🧱 building corner
        case '-': return "\xF0\x9F\xA7\xB1"; // 🧱 building wall (horizontal)
        case '|': return "\xF0\x9F\xA7\xB1"; // 🧱 building wall (vertical)
        case 'T': return "\xF0\x9F\x8C\xB3"; // 🌳 tree
        case 'P': return "\xF0\x9F\x8C\xB4"; // 🌴 palm
        // Use supplementary-plane emoji that are reliably East-Asian-Wide.
        // Avoid U+2728 ✨, U+25C0/U+25B6 ◀️/▶️, and U+1FAA8 🪨 because they
        // render at width 1 in several terminals, which breaks the grid.
        case '^': return "\xF0\x9F\x97\xBB"; // 🗻 cliff (Mount Fuji)
        case 'E': return "\xF0\x9F\x9A\xAA"; // 🚪 exit
        case '<': return "\xF0\x9F\x9A\xAA"; // 🚪 exit left
        case '>': return "\xF0\x9F\x9A\xAA"; // 🚪 exit right
        case '*': return "\xF0\x9F\x8C\x9F"; // 🌟 sparkle
        case 'M': return "\xF0\x9F\x8C\xB1"; // 🌱 medical herb
        case 'H': return "\xF0\x9F\x8D\x83"; // 🍃 herb
        case 'C': return "\xF0\x9F\x94\xA5"; // 🔥 campfire
        case 'G': return "\xF0\x9F\x92\x80"; // 💀 debris/rubble
        case ' ': return "  ";               // open ground (2 spaces)
        default: {
            std::string s;
            s += c;
            s += ' ';
            return s;
        }
    }
}

// Tile letters (T, P, C, E, H, M) are reused inside cosmetic ASCII labels
// ("CAVE", "SHELTER", "WORKSHOP", "CAMPFIRE", "O C E A N", ...). When such
// a letter sits inside a word we must render it as ASCII in emoji mode so
// we don't splash a 🌳 / 🔥 / 🚪 glyph in the middle of the text. The '*'
// sparkle is deliberately NOT treated as a label glyph — it represents the
// campfire flame inside "(*)" and should always render as 🌟.
// True if this character belongs to a cosmetic ASCII label (e.g. "CAVE").
static bool isLabelGlyph(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           c == '(' || c == ')';
}

// Render the map using emoji tiles instead of ASCII (alternate display mode).
void Map::renderEmoji(int playerRow, int playerCol, int viewH, int viewW) const {
    int startR = playerRow - viewH / 2;
    int startC = playerCol - viewW / 2;

    if (startR < 0) startR = 0;
    if (startC < 0) startC = 0;
    if (startR + viewH > rows) startR = rows - viewH;
    if (startC + viewW > cols) startC = cols - viewW;
    if (startR < 0) startR = 0;
    if (startC < 0) startC = 0;

    int endR = std::min(startR + viewH, rows);
    int endC = std::min(startC + viewW, cols);

    int contentW = (endC - startC) * 2; // each emoji takes 2 columns
    std::string pad = leftPad(contentW);
    for (int r = startR; r < endR; r++) {
        std::cout << pad;
        for (int c = startC; c < endC; c++) {
            if (r == playerRow && c == playerCol) {
                std::cout << "\xF0\x9F\xA7\x8D"; // 🧍 player
                continue;
            }
            if (c >= (int)grid[r].size()) {
                std::cout << "  ";
                continue;
            }

            char ch = grid[r][c];

            // If this cell is a label glyph AND another label glyph is
            // within ±2 columns on the same row, render it as ASCII so
            // adjacent words ("SHELTER") and space-separated words
            // ("O C E A N") stay readable. Standalone tile letters
            // (T/P/C/E/H/M trees, campfires, exits, herbs) have no
            // nearby label letters and still render as emoji.
            // Exception: '(C)' is the campfire marker — render the inner
            // C as 🔥 even though it sits between paren label glyphs.
            bool isCampfireC = (ch == 'C' && c - 1 >= 0 &&
                                c + 1 < (int)grid[r].size() &&
                                grid[r][c - 1] == '(' &&
                                grid[r][c + 1] == ')');
            if (isLabelGlyph(ch) && !isCampfireC) {
                bool inLabel = false;
                for (int dx = -2; dx <= 2 && !inLabel; dx++) {
                    if (dx == 0) continue;
                    int nc = c + dx;
                    if (nc < 0 || nc >= (int)grid[r].size()) continue;
                    if (isLabelGlyph(grid[r][nc])) inLabel = true;
                }
                if (inLabel) {
                    std::string s;
                    s += ch;
                    s += ' ';
                    std::cout << s;
                    continue;
                }
            }

            std::cout << tileToEmoji(ch);
        }
        std::cout << '\n';
    }
}

// Returns true if the given tile can be walked onto by the player.
bool Map::isWalkable(int row, int col) const {
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return false;

    char t = grid[row][col];
    // Walkable: space, dot, comma, colon, E (unwired exit),
    //           <, > (directional exits), M/H (herbs - pickable)
    return (t == ' ' || t == '.' || t == ',' || t == ':' || t == 'E' ||
            t == '<' || t == '>' || t == 'M' || t == 'H' || t == 'G');
}

// Read a tile; out-of-bounds reads return '#' (treated as wall).
char Map::getTile(int row, int col) const {
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        return '#';
    return grid[row][col];
}

// Overwrite a tile; auto-extends the row if the column is past current width.
void Map::setTile(int row, int col, char c) {
    if (row < 0 || row >= rows || col < 0 || col >= cols) return;
    if (col >= (int)grid[row].size()) grid[row].resize(col + 1, ' ');
    grid[row][col] = c;
}

// Output the spawn coordinates captured during loadFromFile.
void Map::getSpawnPoint(int& row, int& col) const {
    row = spawnRow;
    col = spawnCol;
}

// Map dimensions in tiles.
int Map::getRows() const { return rows; }
int Map::getCols() const { return cols; }

// Register an exit tile that links this map to another.
void Map::addExitPoint(const ExitPoint& ep) {
    exitPoints.push_back(ep);
}

// If (row,col) is an exit, write destination coords and return destination MapID; else -1.
int Map::checkExit(int row, int col, int& destRow, int& destCol) const {
    for (size_t i = 0; i < exitPoints.size(); i++) {
        if (exitPoints[i].row == row && exitPoints[i].col == col) {
            destRow = exitPoints[i].destRow;
            destCol = exitPoints[i].destCol;
            return (int)exitPoints[i].destination;
        }
    }
    return -1;
}

// ─────────────────────── MapManager ──────────────────────────

// Default constructor: starts the player on the Beach map.
MapManager::MapManager() : currentMap(MAP_BEACH) {}

// Load every map file from dataDir and wire up exit transitions between them.
bool MapManager::loadAllMaps(const std::string& dataDir) {
    const std::string names[MAP_COUNT] = {
        "map_beach", "map_cave", "map_forest", "map_village", "map_shore"
    };

    for (int i = 0; i < MAP_COUNT; i++) {
        if (!maps[i].loadFromFile(dataDir + "/" + names[i] + ".txt")) {
            return false;
        }
    }

    setupExitConnections();
    return true;
}

// Convert raw 'E' tiles into directional '<' / '>' exits and link them
// to neighbours in the chain Beach <-> Cave <-> Forest <-> Village <-> Shore.
void MapManager::setupExitConnections() {
    // Scan each map for 'E' tiles and wire them based on position
    // Layout: Beach <-> Cave <-> Forest <-> Village <-> Shore
    MapID chain[MAP_COUNT] = {
        MAP_BEACH, MAP_CAVE, MAP_FOREST, MAP_VILLAGE, MAP_SHORE
    };

    for (int ci = 0; ci < MAP_COUNT; ci++) {
        MapID mid = chain[ci];
        Map& map = maps[mid];

        for (int r = 0; r < map.getRows(); r++) {
            for (int c = 0; c < map.getCols(); c++) {
                if (map.getTile(r, c) != 'E') continue;

                ExitPoint ep;
                ep.row = r;
                ep.col = c;

                bool left = (c <= 1);
                bool right = (c >= map.getCols() - 2);

                if (left && ci > 0) {
                    MapID prev = chain[ci - 1];
                    ep.destination = prev;
                    ep.destCol = maps[prev].getCols() - 3;
                    ep.destRow = r;
                    if (ep.destRow >= maps[prev].getRows())
                        ep.destRow = maps[prev].getRows() / 2;
                    map.addExitPoint(ep);
                    map.setTile(r, c, '<');
                }
                if (right && ci < MAP_COUNT - 1) {
                    MapID next = chain[ci + 1];
                    ep.destination = next;
                    ep.destCol = 2;
                    ep.destRow = r;
                    if (ep.destRow >= maps[next].getRows())
                        ep.destRow = maps[next].getRows() / 2;
                    map.addExitPoint(ep);
                    map.setTile(r, c, '>');
                }
            }
        }
    }
}

// Get a pointer to a specific map by id, or nullptr if id is out of range.
Map* MapManager::getMap(MapID id) {
    if (id >= 0 && id < MAP_COUNT) return &maps[id];
    return nullptr;
}

// Pointer to whichever map the player is currently in.
Map* MapManager::getCurrentMap() { return &maps[currentMap]; }

// Change the active map (used when the player walks through an exit).
void MapManager::switchMap(MapID id) {
    if (id >= 0 && id < MAP_COUNT) currentMap = id;
}

// Id of the map the player is currently in.
MapID MapManager::getCurrentMapID() const { return currentMap; }

// ─────────────────────── Map Names ──────────────────────────

static const char* MAP_NAMES[] = {
    "Beach \xF0\x9F\x8F\x96",    // 🏖
    "Cave \xF0\x9F\xA6\x87",     // 🦇
    "Forest \xF0\x9F\x8C\xB2",   // 🌲
    "Village \xF0\x9F\x8F\xA0",  // 🏠
    "Shore \xE2\x9B\xB5"         // ⛵
};

// Human-readable display name (with an emoji icon) for a given map id.
const char* getMapName(MapID id) {
    if (id >= 0 && id < MAP_COUNT) return MAP_NAMES[id];
    return "Unknown";
}
