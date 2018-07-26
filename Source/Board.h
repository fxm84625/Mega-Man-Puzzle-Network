#pragma once
#include <vector>
#include "Panel.h"
using namespace std;

class Board {
    int numItems;
    int numTraps;
    int numRocks;
    int numCracked;
    int numIce;
    int numPoison;
    int numHoly;

    void initLevel( int level, int gameDiff, int gain, int type, int subtype );
    void generateItems( int type, int subtype );
    void generateRocks( int type, int subtype );
    void generateFloor( int type, int subtype );
    void generateHolyPanels( vector<pair<int, int>>& coords );
    void generateCrackedPanels( vector<pair<int, int>>& coords );
    void generateIcePanels( vector<pair<int, int>>& coords );
    void generatePoisonPanels( vector<pair<int, int>>& coords );

public:
    Board();
    Panel map[6][6];

    void clear();
    void reset();
    void updatePrevEnergy( float elapsed );

    void generateTrainingLevel( int type = 1, bool reload = false );
    void generateBossLevel( int type = 0 );
    void generateLevel( int level, int lvlDiff, int gain, int type = -1, int subtype = -1 );

    void upgradeAnimationAll();
};
