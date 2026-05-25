#ifndef MAP_H
#define MAP_H

#include <string>
#include <vector>

enum MapID {
    MAP_BEACH = 0,
    MAP_CAVE,
    MAP_FOREST,
    MAP_VILLAGE,
    MAP_SHORE,
    MAP_COUNT
};

struct ExitPoint {
    int row, col;
    MapID destination;
    int destRow, destCol;
};

class Map {
public:
    Map();

    bool loadFromFile(const std::string& filename);
    void render(int playerRow, int playerCol, int viewH = 25, int viewW = 60) const;
    void renderEmoji(int playerRow, int playerCol, int viewH = 25, int viewW = 30) const;
    bool isWalkable(int row, int col) const;
    char getTile(int row, int col) const;
    void setTile(int row, int col, char c);
    void getSpawnPoint(int& row, int& col) const;
    int getRows() const;
    int getCols() const;

    void addExitPoint(const ExitPoint& ep);
    int checkExit(int row, int col, int& destRow, int& destCol) const;

private:
    int rows, cols;
    int spawnRow, spawnCol;
    std::vector<std::string> grid;
    std::vector<ExitPoint> exitPoints;
};

class MapManager {
public:
    MapManager();

    bool loadAllMaps(const std::string& dataDir);
    Map* getMap(MapID id);
    Map* getCurrentMap();
    void switchMap(MapID id);
    MapID getCurrentMapID() const;

private:
    Map maps[MAP_COUNT];
    MapID currentMap;
    void setupExitConnections();
};

const char* getMapName(MapID id);

#endif
