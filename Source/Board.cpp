#include "Board.h"

const int energyGainAmt = 100;			    // How much Energy the player gains when moving to a Pick-Up
const int itemWorth = energyGainAmt / 50;	// Item "Worth" is used to generate Levels
const float itemUpgradeTime = 0.32;

enum { ROOM, PLUS, PATH, CROSS, GALLERY, SCATTER, LAYER };

Board::Board()
  : numItems(0),
    numTraps(0),
    numRocks(0),
    numCracked(0),
    numIce(0),
    numPoison(0),
    numHoly(0)
{}

void Board::clear() {
    for( int i = 0; i < 6; i++ ) {
        for( int j = 0; j < 6; j++ ) {
            map[i][j].reset();
        }
    }
}
void Board::reset() {
    numItems = 0;
    numTraps = 0;
    numRocks = 0;
    numCracked = 0;
    numIce = 0;
    numPoison = 0;
    numHoly = 0;

    clear();
}

void Board::generateTrainingLevel( int type, bool reload ) {
    if( !reload ) clear();
    switch( type ) {
    default:
    case 1:     // Energy
        if( !reload ) {
            for( int i = 0; i < 3; i++ ) {
                map[i][3].rockHP = 3;       map[i][3].rockType = 1; }
            for( int i = 0; i < 2; i++ ) {
                map[i+4][4].rockHP = 5;     map[i+4][5].rockHP = 5; }
        }
        for( int i = 0; i < 4; i++ ) {
            map[4][i].state = -3;       map[5][i].state = -3; }
        map[0][5].item = -1;    map[1][5].item = 1;     map[2][5].item = 2;
        map[0][4].item = 2;     map[1][4].item = -1;    map[2][4].item = 1;
        map[0][3].item = 1;     map[1][3].item = 2;     map[2][3].item = -1;
        break;
    case 2:     // Attacking Vertically
        for( int i = 0; i < 4; i++ ) {
            map[2][i+2].state = -3;
            map[3][i+2].state = -3;
            map[5][i].state = -3; }
        map[0][2].state = -3;   map[0][3].state = -3;
        map[0][5].item = 2;     map[1][5].item = 2;
        map[2][1].item = 2;     map[3][1].item = 2;
        map[2][0].item = 2;     map[3][0].item = 2;
        if( !reload ) {
            map[1][3].rockHP = 3;   map[4][3].rockHP = 3; }
        break;
    case 3:     // Step-Swords
        for( int i = 0; i < 3; i++ ) {
            if( !reload ) map[2][i+3].rockHP = 3;
            map[2][i].state = -3;   map[3][i].state = -3;
            map[4][i].state = -3;   map[5][i].state = -3;
            map[4][i+3].state = -3; }
        for( int i = 0; i < 2; i++ ) {
            map[1][i+3].item = 2;
            map[3][i+3].item = 1;
            map[5][i+3].item = 2; }
        break;
    case 4:     // Cracked Panels
        for( int i = 0; i < 3; i++ ) {
            map[i][1].state = -3;       map[i][2].state = -3;
            map[2][i+3].state = -1; }
        for( int i = 0; i < 2; i++ ) {
            map[0][i+3].item = 1;       map[i][5].item = 2;     map[3][i+3].item = 1; }
        for( int i = 0; i < 5; i++ ) {
            map[4][i].state = -3;       map[5][i].state = -3; }
        if( !reload ) map[4][5].rockHP = 5;
        break;
    case 5:     // Ice and Poison Panels
        for( int i = 0; i < 2; i++ ) {
            map[i][1].state = -3;       map[i][2].state = -3;
            map[i+3][4].state = -3;     map[i+3][5].state = -3; }
        for( int i = 0; i < 3; i++ ) {
            map[3][i].state = -3;   map[4][i].state = -3;   map[5][i].state = -3;
            map[i][4].state = 1; }
        map[0][5].state = 2;    map[1][5].state = 1;    map[2][5].state = 2;
        map[0][3].state = 2;                            map[2][3].state = 1;

        map[0][5].item = 1;     map[1][5].item = 1;
        map[0][4].item = 1;     map[1][4].item = 2;     map[1][3].item = 2;

        if( !reload ) {
            map[1][3].rockHP = 3;   map[1][3].rockType = 1; }
        break;
    case 6:     // Holy
        for( int i = 0; i < 5; i++ ) {
            map[i][5].state = -3;
            map[2][i].state = 2;    map[3][i].state = 3;
            if( !reload ) map[4][i].rockHP = 5;
        }
        for( int i = 0; i < 2; i++ ) {
            map[i][2].state = -3;
            map[0][i+3].item = 2;
            map[1][i].item = 1;     map[1][i+3].item = 1; }
        break;
    case 7:     // Random Training
        if( rand() % 2 ) {
            for( int i = 0; i < 5; i++ ) {
                map[0][i+1].item = 1;   map[i+1][0].item = 2; } }
        else {
            for( int i = 0; i < 5; i++ ) {
                map[0][i+1].item = 2;   map[i+1][0].item = 1; } }
        for( int x = 1; x <= 5; x++ ) {
            for( int y = 1; y <= 5; y++ ) {
                map[x][y].state = rand() % 5 - 1;
                map[x][y].item = rand() % 4 - 1;
                if( map[x][y].item == 0 )
                    map[x][y].rockHP = 5;
                else {
                    map[x][y].rockHP = 3;   map[x][y].rockType = 1; }
                }
        }
        break;
    }
}
void Board::generateBossLevel( int type ) {
    clear();

    switch( type ) {
    default:
    case 0:
        for( int i = 0; i < 3; i++ ) { map[i][5].state = -3;    map[5][i].state = -3; }
        for( int i = 0; i < 2; i++ ) { map[i][4].state = -3;    map[4][i].state = -3; }
        map[0][3].state = -3;    map[3][0].state = -3;
        break;
    case 1:
        for( int i = 0; i < 5; i++ ) { map[i][5].state = -3;    map[i + 1][0].state = -3; }
        break;
    case 2:
        map[1][1].state = -3;    map[1][4].state = -3;    map[4][1].state = -3;    map[4][4].state = -3;
        break;
    case 3:
        for( int i = 0; i < 2; i++ ) { map[1][i + 3].state = -3;    map[4][i + 1].state = -3; }
        break;
    case 4:
        for( int i = 0; i < 2; i++ ) { map[1][i + 1].state = -3;    map[4][i + 3].state = -3; }
        break;
    case 5:
        for( int i = 0; i < 2; i++ ) { map[i + 1][4].state = -3;    map[i + 3][1].state = -3; }
        break;
    case 6:
        for( int i = 0; i < 2; i++ ) { map[i + 1][1].state = -3;    map[i + 3][4].state = -3; }
        break;
    case 7:
        for( int i = 0; i < 2; i++ ) { map[2][i + 2].state = -3;    map[3][i + 2].state = -3; }
        break;
    case 8:
        for( int i = 0; i < 2; i++ ) { map[1][i + 2].state = -3;    map[4][i + 2].state = -3; }
        break;
    case 9:
        for( int i = 0; i < 2; i++ ) { map[i + 2][1].state = -3;    map[i + 2][4].state = -3; }
        break;
    }
}

void Board::generateLevel( int level, int lvlDiff, int gain, int type, int subtype ) {
    clear();
    if( level == 1 ) {
        type = 0; subtype = 0;
    }
    else if( type == -1 || subtype == -1 ) {
        type = rand() % 7;
        subtype = ( type <= 2 ? rand() % 7 : rand() % 8 );
    }
    initLevel( level, lvlDiff, gain, type, subtype );
    generateItems( type, subtype );
    generateRocks( type, subtype );
    generateFloor( type, subtype );
}
void Board::initLevel( int level, int lvlDiff, int gain, int type, int subtype ) {
    switch( type ) {
    default: case 0:    // Room type Levels
        switch( subtype ) {
        default:
        case ROOM: {
            map[2][2].rockHP = 1;	map[2][5].rockHP = 1;	map[4][2].rockHP = 1;							//	] =O=O=[
            map[2][3].rockHP = 1;	map[4][0].rockHP = 1;	map[4][3].rockHP = 1;							//	] =O=  [
            map[2][4].rockHP = 1;	map[4][1].rockHP = 1;	map[4][5].rockHP = 1;							//	] =O=O=[
            map[0][1].state = -3;	map[1][1].state = -3;	map[2][1].state = -3;							//	] =O=O=[
            map[4][4].state = -3;	map[5][4].state = -3;													//	]   =O=[
            map[0][2].state = -3;	map[0][3].state = -3;	map[0][4].state = -3;	map[0][5].state = -3;	//	]====O=[

            // Amount of Rock Obstacles and Energy Item Resources
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            // Amount of Special Panels
            numCracked = rand() % 3 + lvlDiff;	            // 0-2	// 1-3	// 2-4
            if     ( lvlDiff == 1 ) numIce = rand() % 2;    // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numIce = 1;
            numPoison = 0;
            numHoly = rand() % 2;                           // 0-1
            break; }
        case PLUS: {
            for( int i = 0; i < 3; i++ ) {										//	]OO==O=[
			    map[0][i + 3].rockHP = 1;	map[1][i + 3].rockHP = 1;			//	]OO==  [
			    map[4][i].rockHP = 1;		map[5][i].rockHP = 1; }				//	]OO==  [
		    for( int i = 0; i < 2; i++ ) {										//	]  ==OO[
			    map[0][i + 1].state = -3;	map[1][i + 1].state = -3;			//	]  ==OO[
			    map[4][i + 3].state = -3;	map[5][i + 3].state = -3; }			//	]====OO[
		    map[4][5].rockHP = 1;

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 13 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	                // 0-2	// 1-3	// 2-4
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            if     ( lvlDiff == 1 ) numPoison = rand() % 2;     // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numPoison = 1;
            numHoly = 1;
            break; }
        case PATH: {
            map[0][2].state = -3;	map[2][0].state = -3;
		    map[3][5].state = -3;	map[5][3].state = -3;							//	]=== ==[
		    map[2][2].rockHP = 1;	map[3][3].rockHP = 1;							//	]=OO===[
		    for(int i = 0; i < 2; i++) {											//	]=OOO= [
			    map[1][i + 3].rockHP = 1;		map[2][i + 3].rockHP = 1;			//	] =OOO=[
			    map[3][i + 1].rockHP = 1;		map[4][i + 1].rockHP = 1; }			//	]===OO=[
                                                                                    //	]== ===[
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	                // 0-2	// 1-3	// 2-4
            numIce = rand() % 2 - 1 + lvlDiff;                  // 0    // 0-1  // 1-2
            if     ( lvlDiff == 1 ) numPoison = rand() % 2;     // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numPoison = 1;
            numHoly = 1;
            break; }
        case CROSS: {
            for( int i = 0; i < 2; i++ ) {																	//	]=    =[
                map[0][2 + i].rockHP = 1;		map[2][2 + i].rockHP = 1;									//	]=O==O=[
                map[3][2 + i].rockHP = 1;		map[5][2 + i].rockHP = 1; }									//	]O=OO=O[
            for( int i = 0; i < 4; i++ ) {																	//	]O=OO=O[
                map[i + 1][0].state = -3;		map[i + 1][5].state = -3; }									//	]=O==O=[
            map[1][1].rockHP = 1;	map[1][4].rockHP = 1;	map[4][1].rockHP = 1;	map[4][4].rockHP = 1;	//	]=    =[

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case GALLERY: {
            map[0][3].rockHP = 1;	map[1][2].rockHP = 1;	map[1][3].rockHP = 1;	map[2][1].rockHP = 1;	//	]==OO==[
            map[2][2].rockHP = 1;	map[2][4].rockHP = 1;	map[2][5].rockHP = 1;	map[3][0].rockHP = 1;	//	]==O=O=[
            map[3][1].rockHP = 1;	map[3][5].rockHP = 1;	map[4][2].rockHP = 1;	map[4][4].rockHP = 1;	//	]OO===O[
            map[5][2].rockHP = 1;	map[5][3].rockHP = 1;                                                   //	]=OO=OO[
                                                                                                            //	]==OO==[
                                                                                                            //	]===O==[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 14 + numRocks + gain;
            numCracked = rand() % 4 + 1 + lvlDiff * 2;      // 1-4	// 3-6	// 5-8
            numIce = rand() % 2 + lvlDiff;                  // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;           // 0-1  // 0-1  // 1-2
            numHoly = 1;
            break; }
        case SCATTER: {
            map[0][2].state = -3;    map[2][0].state = -3;            //	]== ===[
            map[2][5].state = -3;    map[5][2].state = -3;            //	]=O=O==[
            for( int i = 0; i < 3; i++ ) map[i+1][3-i].rockHP = 1;    //	]=OO=O=[
            for( int i = 0; i < 4; i++ ) map[i+1][4-i].rockHP = 1;    //	] =OO= [
            map[3][4].rockHP = 1;    map[4][3].rockHP = 1;            //	]===OO=[
                                                                      //	]== ===[

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	                // 0-2	// 1-3	// 2-4
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case LAYER: {
            for( int i = 0; i < 5; i++ ) {                            //	]     =[
                map[i][5].state = -3;    map[i+1][0].state = -3; }    //	]OO=OO=[
            for( int i = 0; i < 2; i++ ) {                            //	]OO=O==[
                map[i][3].rockHP = 1;    map[i][4].rockHP = 1;        //	]==O=OO[
                map[i+1][1].rockHP = 1;  map[i+3][4].rockHP = 1;      //	]=OO=OO[
                map[i+4][1].rockHP = 1;  map[i+4][2].rockHP = 1; }    //	]=     [
            map[2][2].rockHP = 1;    map[3][3].rockHP = 1;

            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 14 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        }
        break;
    case 1:             // Plus type Levels
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 3; i++ ) {										// ] ==O==[
                map[0][i + 3].state = -3;		map[i + 3][0].state = -3;		// ] =OOO=[
                map[i + 2][2].rockHP = 1;		map[2][i + 2].rockHP = 1;		// ]=OO=OO[
                map[i + 2][4].rockHP = 1;		map[4][i + 2].rockHP = 1; }		// ]==OOO=[
            map[1][3].rockHP = 1;		map[3][5].rockHP = 1;					// ]===O==[
            map[3][1].rockHP = 1;		map[5][3].rockHP = 1;					// ]====  [
            map[0][3].state = 0;		map[3][0].state = 0;

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 1: {
            for( int i = 0; i < 3; i++ ) {									// ] ===O=[
                map[0][i + 3].state = -3;	map[i + 3][0].state = -3;		// ] =OOOO[
                map[i + 2][2].rockHP = 1;	map[2][i + 2].rockHP = 1;		// ] =O=O=[
                map[i + 2][4].rockHP = 1;	map[4][i + 2].rockHP = 1; }		// ]=OOOO=[
            map[1][2].rockHP = 1;	map[4][5].rockHP = 1;					// ]==O===[
            map[2][1].rockHP = 1;	map[5][4].rockHP = 1;					// ]===   [

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 2: {
            map[4][4].rockHP = 1;
            for( int i = 0; i < 3; i++ ) {									// ] =O===[
                map[0][i + 3].state = -3;	map[i + 3][0].state = -3;		// ] OOOO=[
                map[i + 3][2].rockHP = 1;	map[2][i + 3].rockHP = 1;		// ] =O=O=[
                map[i + 1][4].rockHP = 1;	map[4][i + 1].rockHP = 1; }		// ]===OOO[
                                                                            // ]====O=[
                                                                            // ]===   [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 11 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 -1 + lvlDiff;           // 0    // 0-1  // 1-2
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 3: {
            map[4][4].state = -3;
            for( int i = 0; i < 3; i++ ) {									// ] O=O==[
                map[0][i + 3].state = -3;	map[i + 3][0].state = -3;		// ] O=O =[
                map[i + 3][1].rockHP = 1;	map[1][i + 3].rockHP = 1;		// ] O=OOO[
                map[i + 3][3].rockHP = 1;	map[3][i + 3].rockHP = 1; }		// ]======[
                                                                            // ]===OOO[
                                                                            // ]===   [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 11 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;		            // 0-2	// 1-3	// 2-4
            numIce = rand() % 2 - 1 + lvlDiff;                  // 0    // 0-1  // 1-2
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );      // 0    // 0-1
            numHoly = 1;
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {											// ] OOO==[
                map[0][i + 3].state = -3;	map[i + 3][0].state = -3;				// ] O=O==[
                map[i + 3][1].rockHP = 1;	map[1][i + 3].rockHP = 1;				// ] O=OOO[
                map[i + 3][3].rockHP = 1;	map[3][i + 3].rockHP = 1; }				// ]==O==O[
            map[2][2].rockHP = 1;	map[2][5].rockHP = 1;	map[5][2].rockHP = 1;	// ]===OOO[
                                                                                    // ]===   [
            numRocks = 5 + level / 5 + lvlDiff * 2;									
            numItems = 14 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;		            // 0-2	// 1-3	// 2-4
            numIce = rand() % 2 - 1 + lvlDiff;                  // 0    // 0-1  // 1-2
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );      // 0    // 0-1
            numHoly = 1;
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {
                map[i][5].state = -3;        map[5][i].state = -3;          // ]  O=O=[
                map[2][i + 1].rockHP = 1;	map[i + 1][2].rockHP = 1;		// ]=OO=OO[
                map[2][i + 4].rockHP = 1;    map[i + 4][2].rockHP = 1;      // ]===O==[
                map[4][i + 1].rockHP = 1;    map[i + 1][4].rockHP = 1;      // ]=OO=OO[
                map[4][i + 4].rockHP = 1;	map[i + 4][4].rockHP = 1; }	    // ]= O=O [
            map[3][3].rockHP = 1;	map[1][1].state = -3;					// ]===== [

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 13 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                                              // ]  =O==[
                map[i+1][3].rockHP = 1;  map[3][i+1].rockHP = 1; }                      // ] ==O =[
            map[0][4].state = -3;    map[0][5].state = -3;    map[1][5].state = -3;     // ]=OOOOO[
            map[2][2].state = -3;    map[4][4].state = -3;                              // ]== O==[
            map[4][0].state = -3;    map[5][0].state = -3;    map[5][1].state = -3;     // ]===O= [
                                                                                        // ]====  [
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        }
        break;
    case 2:             // Path type Levels
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 2; i++ ) {                              //	]=OO===[
                map[i+1][2].state = -3;    map[i+3][3].state = -3; }    //	]=OO===[
            for( int i = 0; i < 3; i++ ) {                              //	]=OO  =[
                map[1][i+3].rockHP = 1;    map[3][i].rockHP = 1;        //	]=  OO=[
                map[2][i+3].rockHP = 1;    map[4][i].rockHP = 1; }      //	]===OO=[
                                									    //	]===OO=[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 1: {
            for( int i = 0; i < 2; i++ ) {                          //	]=OO ==[
                map[1][i+4].rockHP = 1;  map[2][i+4].rockHP = 1;    //	]=OOO==[
                map[3][i+3].rockHP = 1;  map[2][i+1].rockHP = 1;    //	]== O= [
                map[3][i].rockHP = 1;    map[4][i].rockHP = 1; }    //	] =O ==[
            map[0][2].state = -3;    map[2][0].state = -3;          //	]==OOO=[
            map[2][3].state = -3;    map[3][2].state = -3;          //	]== OO=[
            map[3][5].state = -3;    map[5][3].state = -3;

            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 2: {
            for( int i = 0; i < 2; i++ ) {                                                          //	] =OO==[
                map[i][1].state = -3;    map[i + 2][4].rockHP = 1;    map[i + 4][1].state = -3;     //	]==OO==[
                map[i][2].rockHP = 1;    map[i + 2][5].rockHP = 1;    map[i + 4][2].rockHP = 1;     //	]OO==OO[
                map[i][3].rockHP = 1;    map[i + 4][0].state = -3;    map[i + 4][3].rockHP = 1; }   //	]OO==OO[
            map[0][5].state = -3;                                                                   //	]  ==  [
                                                                                                    //	]====  [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {														//	]    ==[
                map[i][3].rockHP = 1;	map[i + 2][1].rockHP = 1;	map[5][i].state = -3;		//	]OO==OO[
                map[i][4].rockHP = 1;	map[i + 2][2].rockHP = 1;	map[i + 4][3].rockHP = 1;	//	]OO==OO[
                map[i][5].state = -3;	map[i + 2][5].state = -3;	map[i + 4][4].rockHP = 1; }	//	]==OO==[
            map[0][1].state = -3;																//	] =OO= [
												                                                //	]===== [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {														    //	]  =OO=[
			    map[1][i + 2].rockHP = 1;	map[3][i].rockHP = 1;	map[3][i + 4].rockHP = 1;	    //	]===OO=[
			    map[2][i + 2].rockHP = 1;	map[4][i].rockHP = 1;	map[4][i + 4].rockHP = 1;	    //	]=OO= =[
			    map[1][i].state = -3;	    map[4][i + 2].state = -3;    map[i][5].state = -3; }    //	]=OO= =[
		                    																        //	]= =OO=[
															                                        //	]= =OO=[
            numRocks = 4 + level / 5 + lvlDiff * 2;
		    numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {														    //	]=OO= =[
			    map[1][i].rockHP = 1;	map[1][i + 4].rockHP = 1;	map[3][i + 2].rockHP = 1;	    //	]=OO= =[
			    map[2][i].rockHP = 1;	map[2][i + 4].rockHP = 1;	map[4][i + 2].rockHP = 1;	    //	]= =OO=[
			    map[1][i + 2].state = -3;	map[4][i].state = -3;    map[4][i + 4].state = -3; }    //	]= =OO=[
		                    																        //	]=OO= =[
															                                        //	]=OO= =[
            numRocks = 4 + level / 5 + lvlDiff * 2;
		    numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 6: {
            for( int i = 0; i < 2; i++ ) {                                                      //  ]  =O==[
                map[i][5].state = -3;    map[4+i][0].state = -3;  map[i+2][3-i].state = -3; }   //  ]=O=O =[
            for( int i = 0; i < 3; i++ ) {                                                      //  ]=O OO=[
                map[1][i+2].rockHP = 1;      map[2][i].rockHP = 1;                              //  ]=OO O=[
                map[3][i+3].rockHP = 1;      map[4][i+1].rockHP = 1; }                          //  ]= O=O=[
            map[1][1].state = -3;    map[4][4].state = -3;                                      //  ]==O=  [

            numRocks = 4 + level / 5 * lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 4 + 1 + lvlDiff * 2;      // 1-4	// 3-6	// 5-8
            numIce = rand() % 2 + lvlDiff;                  // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;           // 0-1  // 0-1  // 1-2
            numHoly = 1;
            break; }
        }
        break;
    case 3:             // Crossed type Levels
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 5; i++ ) {                          //  ]O===O=[
                map[i][i+1].rockHP = 1;  map[i][5-i].rockHP = 1;    //  ]=O=O= [
                map[5][i].state = -3; }                             //  ]==O== [
            for( int i = 0; i < 3; i++ ) map[i+2][0].state = -3;    //  ]=O=O= [
                                                                    //  ]O===O [
                                                                    //  ]==    [
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 1: {
            for( int i = 0; i < 5; i++ ) {                              //  ] O===O[
                map[i+1][i+1].rockHP = 1;    map[i+1][5-i].rockHP = 1;  //  ] =O=O=[
                map[i+1][0].state = -3; }                               //  ] ==O==[
            for( int i = 0; i < 3; i++ ) map[0][i+3].state = -3;        //  ]==O=O=[
                                                                        //  ]=O===O[
                                                                        //  ]=     [
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 2: {                                                   //  ]=== ==[
            for( int i = 0; i < 2; i++ ) {                          //  ]=OO=OO[
                map[i+1][2].rockHP = 1;  map[i+1][4].rockHP = 1;    //  ] ==O==[
                map[3][i].state = -3;                               //  ]=OO=OO[
                map[i+4][2].rockHP = 1;  map[i+4][4].rockHP = 1; }  //  ]=== ==[
            map[3][3].rockHP = 1;                                   //  ]=== ==[
            map[0][3].state = -3;    map[3][5].state = -3;

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	            // 0-2	// 1-3	// 2-4
            if( lvlDiff == 1 ) numIce = rand() % 2;         // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numIce = 1;
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );  // 0    // 0-1
            numHoly = 1;
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                          //  ]==O=O=[
                map[i][3].state = -3;                               //  ]==O=O=[
                map[2][i+1].rockHP = 1;  map[2][i+4].rockHP = 1;    //  ]  =O= [
                map[4][i+1].rockHP = 1;  map[4][i+4].rockHP = 1; }  //  ]==O=O=[
            map[3][3].rockHP = 1;                                   //  ]==O=O=[
            map[3][0].state = -3;    map[5][3].state = -3;          //  ]=== ==[

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	            // 0-2	// 1-3	// 2-4
            if( lvlDiff == 1 ) numIce = rand() % 2;         // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numIce = 1;
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );  // 0    // 0-1
            numHoly = 1;
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                                                      // ]== ===[
                map[i][1].rockHP = 1;   map[2][i + 2].rockHP = 1;   map[i + 3][1].rockHP = 1;   // ]OO=OO [
                map[i][4].rockHP = 1;                               map[i + 3][4].rockHP = 1; } // ]==O== [
            for( int i = 0; i < 5; i++ ) { map[5][i].state = -3; }                              // ]==O== [
            map[2][5].state = -3;                                                               // ]OO=OO [
                                                                                                // ]===== [
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                                                          // ]  == =[
                map[1][i].rockHP = 1;    map[i + 2][2].rockHP = 1;    map[1][i + 3].rockHP = 1;     // ]=O==O=[
                map[4][i].rockHP = 1;                                map[4][i + 3].rockHP = 1; }    // ]=O==O=[
            map[0][5].state = -3;    map[1][5].state = -3;    map[4][5].state = -3;                 // ]==OO= [
            map[5][2].state = -3;                                                                   // ]=O==O=[
                                                                                                    // ]=O==O=[

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                                          // ] ==O=O[
                map[i + 1][i + 1].rockHP = 1;    map[3 - i][i + 1].rockHP = 1;      // ] = =O=[
                map[i + 3][i + 3].rockHP = 1;    map[5 - i][i + 3].rockHP = 1; }    // ]=O=O=O[
            map[0][4].state = -3;    map[0][5].state = -3;    map[2][4].state = -3; // ]==O= =[
            map[4][0].state = -3;    map[4][2].state = -3;    map[5][0].state = -3; // ]=O=O==[
                                                                                    // ]====  [

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 -1 + lvlDiff;        // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 7: {
            for( int i = 0; i < 3; i++ ) {                                              // ] O=O==[
                map[i + 1][i + 3].rockHP = 1;    map[5 - i][i + 1].rockHP = 1;          // ]==O= =[
                map[i + 3][i + 1].rockHP = 1;    map[3 - i][i + 3].rockHP = 1;}         // ]=O=O=O[
            map[1][1].state = -3;    map[2][1].state = -3;                              // ]=  =O=[
            map[1][2].state = -3;    map[2][2].state = -3;                              // ]=  O=O[
            map[0][5].state = -3;    map[4][4].state = -3;    map[5][0].state = -3;     // ]===== [

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 9 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	    // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 - 1 + lvlDiff;          // 0    // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        }
        break;
    case 4:             // Gallery type Levels
        switch( subtype ) {
        default: case 0: {
            for( int i = 1; i < 5; i++ ) {						//	]======[
                map[i][1].rockHP = 1;	map[i][2].rockHP = 1;	//	]=OOOO=[
                map[i][3].rockHP = 1;	map[i][4].rockHP = 1; }	//	]=OOOO=[
                                                                //	]=OOOO=[
                                                                //	]=OOOO=[
                                                                //	]======[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 1: {
            for( int i = 1; i < 5; i++ ) {						// ]======[
                map[0][i].rockHP = 1;	map[1][i].rockHP = 1;	// ]OO==OO[
                map[4][i].rockHP = 1;	map[5][i].rockHP = 1; }	// ]OO==OO[
                                                                // ]OO==OO[
                                                                // ]OO==OO[
                                                                // ]======[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 2: {
            for( int i = 1; i < 5; i++ ) {						// ]=OOOO=[
                map[i][0].rockHP = 1;	map[i][3].rockHP = 1;	// ]======[
                map[i][2].rockHP = 1;	map[i][5].rockHP = 1;}	// ]=OOOO=[
                                                                // ]=OOOO=[
                                                                // ]======[
                                                                // ]=OOOO=[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 3: {
            for( int i = 1; i < 5; i++ ) {                      // ]======[
                map[0][i].rockHP = 1;	map[3][i].rockHP = 1;    // ]O=OO=O[
                map[2][i].rockHP = 1;	map[5][i].rockHP = 1; }  // ]O=OO=O[
                                                                // ]O=OO=O[
                                                                // ]O=OO=O[
                                                                // ]======[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {                              // ]=OOO==[
                map[0][i + 2].rockHP = 1;	map[i + 1][5].rockHP = 1;   // ]O=====[
                map[i + 2][0].rockHP = 1;	map[5][i + 1].rockHP = 1; } // ]O=OO=O[
            map[2][2].rockHP = 1;    map[3][2].rockHP = 1;              // ]O=OO=O[
            map[2][3].rockHP = 1;    map[3][3].rockHP = 1;              // ]=====O[
                                                                        // ]==OOO=[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                              // ]OO====[
                map[0][i + 4].rockHP = 1;    map[1][i + 4].rockHP = 1;  // ]OO=OO=[
                map[1][i + 1].rockHP = 1;    map[2][i + 1].rockHP = 1;  // ]===OO=[
                map[3][i + 3].rockHP = 1;    map[4][i + 3].rockHP = 1;  // ]=OO===[
                map[4][i].rockHP = 1;        map[5][i].rockHP = 1; }    // ]=OO=OO[
                                                                        // ]====OO[
            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 16 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                                          // ]  ====[
                map[i+1][0].rockHP = 1;  map[5][i].rockHP = 1; }                    // ] O=O=O[
            for( int i = 0; i < 3; i++ ) {                                          // ]===O=O[
                map[i+1][2].rockHP = 1;  map[3][i+2].rockHP = 1; }                  // ]=OOO=O[
            map[1][4].rockHP = 1;                                                   // ]=====O[
            map[0][4].state = -3;    map[0][5].state = -3;    map[1][5].state = -3; // ]=OOOOO[

            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 15 + numRocks + gain;
            numCracked = rand() % 3 + 1 + lvlDiff * 2;	// 1-3	// 3-5	// 5-7
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        case 7: {
            for( int i = 0; i < 5; i++ ) {                                          // ]OOOOO=[
                map[0][i+1].rockHP = 1;  map[i][5].rockHP = 1; }                    // ]O=====[
            for( int i = 0; i < 3; i++ ) {                                          // ]O=OOO=[
                map[2][i+1].rockHP = 1;  map[i+2][3].rockHP = 1; }                  // ]O=O===[
            map[4][1].rockHP = 1;                                                   // ]O=O=O [
            map[4][0].state = -3;    map[5][0].state = -3;    map[5][1].state = -3; // ]====  [

            numRocks = 5 + level / 5 + lvlDiff * 2;
            numItems = 15 + numRocks + gain;
            numCracked = rand() % 3 + 1 + lvlDiff * 2;	// 1-3	// 3-5	// 5-7
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = 0;
            numHoly = 0;
            break; }
        }
        break;
    case 5:             // Scattered type Levels
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 6; i += 2 ) {	//	]==O=O=[
                map[2][i + 1].rockHP = 1;		//	]===O=O[
                map[3][i].rockHP = 1;			//	]==O=O=[
                map[4][i + 1].rockHP = 1;		//	]===O=O[
                map[5][i].rockHP = 1; }			//	]==O=O=[
                                                //	]===O=O[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + lvlDiff * 2;		// 0-4	// 2-6	// 4-8
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 1: {
            for( int i = 0; i <= 4; i += 2 ) {
                map[i][3].rockHP = 1;
                map[i][5].rockHP = 1;
                map[i+1][2].rockHP = 1;
                map[i+1][4].rockHP = 1; }
            //	]O=O=O=[
            //	]=O=O=O[
            //	]O=O=O=[
            //	]=O=O=O[
            //	]======[
            //	]======[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + lvlDiff * 2;		// 0-4	// 2-6	// 4-8
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                  // ] =O===[
                map[i][3-i].rockHP = 1;                     // ]=O=O==[
                map[i+1][4-i].rockHP = 1;                   // ]O=O=O=[
                map[i+2][5-i].rockHP = 1; }                 // ]=O=O=O[
            map[0][5].state = -3;    map[5][0].state = -3;  // ]==O=O=[
                                                            // ]===O= [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + 2 + lvlDiff * 2;	// 2-6	// 4-8	// 6-10
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = rand() % 2 + 1;                   // 1-2
            break; }
        case 3: {
            for( int i = 1; i <= 5; i += 2 ) {
                map[i][2].rockHP = 1;    map[i][4].rockHP = 1;      // ] =O=O=[
                map[2][i].rockHP = 1;    map[4][i].rockHP = 1; }    // ] O=O=O[
            map[0][4].state = -3;    map[0][5].state = -3;          // ]==O=O=[
            map[4][0].state = -3;    map[5][0].state = -3;          // ]=O=O=O[
                                                                    // ]==O=O=[
                                                                    // ]====  [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + lvlDiff * 2;		// 0-4	// 2-6	// 4-8
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 4: {
            for( int i = 0; i < 5; i++ ) { map[i + 1][5 - i].rockHP = 1; }      // ]=O=O==[
            for( int i = 0; i < 3; i++ ) { map[i + 3][5 - i].rockHP = 1; }      // ]O=O=O=[
            for( int i = 0; i < 2; i++ ) {                                      // ]=O=O=O[
                map[i][4 - i].rockHP = 1;    map[i + 3][1 - i].rockHP = 1; }    // ] ===O=[
            map[0][2].state = -3;    map[2][0].state = -3;                      // ]===O=O[
                                                                                // ]== =O=[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + lvlDiff * 2;		// 0-4	// 2-6	// 4-8
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 5: {
            for( int i = 0; i < 6; i++ ) map[i][5-i].rockHP = 1;        // ]O=  ==[
            for( int i = 0; i < 2; i++ ) {                              // ]=O=O==[
                map[i+1][2-i].rockHP = 1;    map[i+3][4-i].rockHP = 1;  // ] =O=O [
                map[0][i+2].state = -3;      map[i+2][0].state = -3;    // ] O=O= [
                map[i+2][5].state = -3;      map[5][i+2].state = -3; }  // ]==O=O=[
                                                                        // ]==  =O[

            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;	            // 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + ( lvlDiff == 2 ? 1 : 0 );     // 0-1  // 0-1  // 1-2
            numPoison = rand() % 2 - 1 + lvlDiff;               // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                                  // ]=O=O==[
                map[i+1][3-i].rockHP = 1;    map[i+3][5-i].rockHP = 1; }    // ]==O=O=[
            for( int i = 0; i < 5; i++ ) map[i+1][5-i].rockHP = 1;          // ] O=O=O[
            map[0][3].state = -3;    map[3][0].state = -3;                  // ]==O=O=[
                                                                            // ]===O=O[
                                                                            // ]=== ==[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 11 + numRocks + gain;
            numCracked = rand() % 5 + lvlDiff * 2;		// 0-4	// 2-6	// 4-8
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 7: {
            for( int i = 0; i < 6; i++ ) map[i][5-i].rockHP = 1;            //	]O= =O=[
            for( int i = 0; i < 2; i++ ) {                                  //	]=O=O=O[
                map[i+3][i+4].rockHP = 1;    map[i+4][i+3].rockHP = 1; }    //	]O=O=O=[
            map[0][3].rockHP = 1;    map[3][0].rockHP = 1;                  //	]===O= [
            map[1][1].state = -3;                                           //	]= ==O=[
            map[2][5].state = -3;    map[5][2].state = -3;                  //	]===O=O[

            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 5 + 2 + lvlDiff * 2;	// 2-6	// 4-8	// 6-10
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = rand() % 2 + 1;                   // 1-2
            break; }
        }
        break;
    case 6:             // Layered type Levels
        switch( subtype ) {
        default: case 0: {
            map[1][0].state = -3;    map[2][0].state = -3;    map[4][0].state = -3; //	]= OO==[
            map[1][5].state = -3;    map[3][0].state = -3;    map[4][1].state = -3; //	]==OO==[
            for( int yPos = 1; yPos < 6; yPos++ ) {									//	]==OO==[
                map[2][yPos].rockHP = 1;		map[3][yPos].rockHP = 1; }			//	]==OO==[
                                                                                    //	]==OO =[
                                                                                    //	]=    =[
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 1: {
            map[5][0].state = -3;										//	]    ==[
            for( int i = 0; i < 4; i++ ) {								//	]=OO=O=[
                map[1][i + 1].rockHP = 1;	map[4][i + 1].rockHP = 1;	//	]=OO=O=[
                map[2][i + 1].rockHP = 1;								//	]=OO=O=[
                map[i][5].state = -3;	map[i + 1][0].state = -3; }  	//	]=OO=O=[
                                                                        //	]=     [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 2: {
            map[5][0].state = -3;										//	]    ==[
            for( int i = 0; i < 4; i++ ) {								//	]=O=OO=[
                map[1][i + 1].rockHP = 1;	map[4][i + 1].rockHP = 1;	//	]=O=OO=[
                map[3][i + 1].rockHP = 1;								//	]=O=OO=[
                map[i][5].state = -3;	map[i + 1][0].state = -3; }  	//	]=O=OO=[
                                                                        //	]=     [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                                                              //	]=OO===[
                map[i + 1][5].rockHP = 1;    map[i + 2][3].rockHP = 1;    map[i + 4][2].rockHP = 1;     //	]=OO===[
                map[i + 1][4].rockHP = 1;    map[i + 2][2].rockHP = 1;    map[i + 4][1].rockHP = 1; }   //	]==OO==[
            map[0][2].state = -3;    map[2][0].state = -3;                                              //	] =OOOO[
                                                                                                        //	]====OO[
                                                                                                        //	]== ===[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                                                          // ]=== ==[
                map[i][3].rockHP = 1;    map[i + 2][3].rockHP = 1;    map[i + 3][1].rockHP = 1;     // ]OO====[
                map[i][4].rockHP = 1;    map[i + 2][2].rockHP = 1;    map[i + 3][0].rockHP = 1; }   // ]OOOO= [
            map[3][5].state = -3;    map[5][3].state = -3;                                          // ]==OO==[
                                                                                                    // ]===OO=[
                                                                                                    // ]===OO=[
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 5: {
            for( int i = 0; i < 4; i++ ) {                              // ]    ==[
                map[2][i + 1].rockHP = 1;    map[i + 1][0].state = -3;  // ]==OOO=[
                map[3][i + 1].rockHP = 1;    map[i][5].state = -3;      // ]==OOO=[
                map[4][i + 1].rockHP = 1; }                             // ]==OOO=[
            map[5][0].state = -3;                                       // ]==OOO=[
                                                                        // ]=     [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff * 2;		// 0-2	// 2-4	// 4-6
            numIce = rand() % 2 + lvlDiff;              // 0-1  // 1-2  // 2-3
            numPoison = rand() % 2 - 1 + lvlDiff;       // 0    // 0-1  // 1-2
            numHoly = 1;
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                              // ]   ===[
                map[i+1][3-i].rockHP = 1;    map[i+1][4-i].rockHP = 1;  // ] OO===[
                map[i+2][4-i].rockHP = 1;                               // ]=OOO==[
                map[i][5].state = -3;    map[5][i].state = -3; }        // ]==OOO [
            map[0][4].state = -3;    map[4][0].state = -3;              // ]===OO [
                                                                        // ]====  [
            numRocks = 3 + level / 5 + lvlDiff * 2;
            numItems = 10 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	            // 0-2	// 1-3	// 2-4
            if( lvlDiff == 1 ) numIce = rand() % 2;         // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numIce = 1;
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );  // 0    // 0-1
            numHoly = 1;
            break; }
        case 7: {
            for( int i = 0; i < 3; i++ ) {                          // ]    ==[
                map[i][4].rockHP = 1;    map[i+1][3].rockHP = 1;    // ]OOO== [
                map[i+2][2].rockHP = 1;  map[i+3][1].rockHP = 1; }  // ]=OOO==[
            for( int i = 0; i < 4; i++ ) {                          // ]==OOO=[
                map[i][5].state = -3;    map[i+2][0].state = -3; }  // ] ==OOO[
                                                                    // ]==    [
            numRocks = 4 + level / 5 + lvlDiff * 2;
            numItems = 12 + numRocks + gain;
            numCracked = rand() % 3 + lvlDiff;	            // 0-2	// 1-3	// 2-4
            if( lvlDiff == 1 ) numIce = rand() % 2;         // 0    // 0-1  // 1
            else if( lvlDiff == 2 ) numIce = 1;
            numPoison = ( lvlDiff == 0 ? 0 : rand() % 2 );  // 0    // 0-1
            numHoly = 1;
            break; }
        }
        break;
    }

    int minItems = ( level < 50 ? 3 * itemWorth : 4 * itemWorth );
    if( numItems < minItems ) numItems = minItems;

    numTraps = rand() % 2 + lvlDiff + ( level >= 50 ? 1 : 0 );
}

void Board::generateItems( int type, int subtype ) {
    // Randomly places Items onto the Board, based on level templates
    if( numItems <= 0 && numTraps <= 0 ) return;

    vector< pair<int, int> > coords;
    switch( type ) {
    default: case ROOM:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 4; i++ ) {                  //	] +====[
                coords.push_back( make_pair( 1, i+2 ) );    //	] +==  [
                coords.push_back( make_pair( 5, i ) ); }    //	] +===+[
                                                            //	] +===+[
                                                            //	]   ==+[
                                                            //	]=====+[
            break; }
        case 1: {
            for( int i = 0; i < 3; i++ ) {                  //	]++====[
                coords.push_back( make_pair( 0, i+3 ) );    //	]++==  [
                coords.push_back( make_pair( 1, i+3 ) );    //	]++==  [
                coords.push_back( make_pair( 4, i ) );      //	]  ==++[
                coords.push_back( make_pair( 5, i ) ); }    //	]  ==++[
                                                            //	]====++[
            break; }
        case 2: {
            for( int i = 0; i < 2; i++ ) {                  //	]++= ==[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++====[
                coords.push_back( make_pair( 1, i+4 ) );    //	]===== [
                coords.push_back( make_pair( 4, i ) );      //	] =====[
                coords.push_back( make_pair( 5, i ) ); }    //	]====++[
                                                            //	]== =++[
            break; }
        case 3: {
            for( int i = 0; i < 3; i++ ) {                  //	]+    =[
                coords.push_back( make_pair( 0, i+3 ) );    //	]+=====[
                coords.push_back( make_pair( 5, i ) ); }    //	]+=++==[
            for( int i = 0; i < 2; i++ ) {                  //	]==++=+[
                coords.push_back( make_pair( 2, i+2 ) );    //	]=====+[
                coords.push_back( make_pair( 3, i+2 ) ); }  //	]=    +[
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                  //	]++====[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++====[
                coords.push_back( make_pair( 1, i+4 ) );    //	]======[
                coords.push_back( make_pair( 4, i ) );      //	]======[
                coords.push_back( make_pair( 5, i ) ); }    //	]====++[
                                                            //	]====++[
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  //	]++ ===[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++====[
                coords.push_back( make_pair( 1, i+4 ) );    //	]======[
                coords.push_back( make_pair( 4, i ) );      //	] ==== [
                coords.push_back( make_pair( 5, i ) ); }    //	]====++[
                                                            //	]== =++[
            break; }
        case 6: {
            for( int i = 0; i < 2; i++ ) {                  //	]     =[
                coords.push_back( make_pair( i, 4 ) );      //	]++=++=[
                coords.push_back( make_pair( i+1, 1 ) );    //	]+=====[
                coords.push_back( make_pair( i+3, 4 ) );    //	]=====+[
                coords.push_back( make_pair( i+4, 1 ) ); }  //	]=++=++[
            coords.push_back( make_pair( 0, 3 ) );          //	]=     [
            coords.push_back( make_pair( 5, 2 ) );
            break; }
        }
        break;
    case PLUS:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 3; i++ ) {                  //	] =====[
                coords.push_back( make_pair( i+2, 2 ) );    //	] =+++=[
                coords.push_back( make_pair( i+2, 4 ) );    //	]==+=+=[
                coords.push_back( make_pair( 2, i+2 ) );    //	]==+++=[
                coords.push_back( make_pair( 4, i+2 ) ); }  //	]======[
                                                            //	]====  [
            break; }
        case 1: {
            for( int i = 0; i < 3; i++ ) {                  //	] =====[
                coords.push_back( make_pair( i+2, 2 ) );    //	] =+++=[
                coords.push_back( make_pair( i+2, 4 ) );    //	] =+=+=[
                coords.push_back( make_pair( 2, i+2 ) );    //	]==+++=[
                coords.push_back( make_pair( 4, i+2 ) ); }  //	]======[
                                                            //	]===   [
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                  //	] =+===[
                coords.push_back( make_pair( i+1, 4 ) );    //	] ++++=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //	] ===+=[
            coords.push_back( make_pair( 2, 5 ) );          //	]====++[
            coords.push_back( make_pair( 5, 2 ) );          //	]====+=[
                                                            //	]===   [
            break; }
        case 3: {
            for( int i = 0; i < 3; i++ ) {                  //  ] +=+==[
                coords.push_back( make_pair( 1, i+3 ) );    //  ] +=+==[
                coords.push_back( make_pair( 3, i+3 ) );    //  ] +=+++[
                coords.push_back( make_pair( i+3, 1 ) );    //  ]======[
                coords.push_back( make_pair( i+3, 3 ) ); }  //  ]===+++[
                                                            //  ]===   [
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {                  //  ] +++==[
                coords.push_back( make_pair( 1, i+3 ) );    //  ] +=+==[
                coords.push_back( make_pair( 3, i+3 ) );    //  ] +=+++[
                coords.push_back( make_pair( i+3, 1 ) );    //  ]=====+[
                coords.push_back( make_pair( i+3, 3 ) ); }  //  ]===+++[
            coords.push_back( make_pair( 2, 5 ) );          //  ]===   [
            coords.push_back( make_pair( 5, 2 ) );
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  // ]  +===[
                coords.push_back( make_pair( i+1, 2 ) );    // ]=++=+=[
                coords.push_back( make_pair( i+1, 4 ) );    // ]======[
                coords.push_back( make_pair( i+4, 2 ) ); }  // ]=++=++[
            coords.push_back( make_pair( 2, 1 ) );          // ]= +=+ [
            coords.push_back( make_pair( 2, 5 ) );          // ]===== [
            coords.push_back( make_pair( 4, 1 ) );
            coords.push_back( make_pair( 4, 4 ) );
            break; }
        case 6: {
            for( int i = 0; i < 4; i++ ) {                  //	]  =+==[
                coords.push_back( make_pair( i+2, 3 ) );    //	] ==+ =[
                coords.push_back( make_pair( 3, i+2 ) ); }  //	]==++++[
                                                            //	]== +==[
                                                            //	]===== [
                                                            //	]====  [
            break; }
        }
        break;
    case PATH:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 3; i++ ) {                  //	]=++===[
                coords.push_back( make_pair( 1, i+3 ) );    //	]=++===[
                coords.push_back( make_pair( 2, i+3 ) );    //	]=++  =[
                coords.push_back( make_pair( 3, i ) );      //	]=  ++=[
                coords.push_back( make_pair( 4, i ) ); }    //	]===++=[
                                                            //	]===++=[
            break; }
        case 1: {
            for( int i = 0; i < 2; i++ ) {                  //	]=++ ==[
                coords.push_back( make_pair( 2, i+4 ) );    //	]==+===[
                coords.push_back( make_pair( i+2, i+2 ) );  //	]== += [
                coords.push_back( make_pair( 3, i ) ); }    //	] =+ ==[
            coords.push_back( make_pair( 1, 5 ) );          //	]===+==[
            coords.push_back( make_pair( 4, 0 ) );          //	]== ++=[
            break; }
        case 2: {
            for( int i = 0; i < 2; i++ ) {                  //	] =+===[
                coords.push_back( make_pair( 0, i+2 ) );    //	]==+===[
                coords.push_back( make_pair( 1, i+2 ) );    //	]++===+[
                coords.push_back( make_pair( 2, i+4 ) );    //	]++===+[
                coords.push_back( make_pair( 5, i+2 ) ); }  //	]  ==  [
                                                            //	]====  [
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                  //	]    ==[
                coords.push_back( make_pair( 0, i+3 ) );    //	]++===+[
                coords.push_back( make_pair( 1, i+3 ) );    //	]++===+[
                coords.push_back( make_pair( i+2, 1 ) );    //	]======[
                coords.push_back( make_pair( 5, i+3 ) ); }  //	] =++= [
                                                            //	]====  [
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                  //	]  ==+=[
                coords.push_back( make_pair( 2, i+2 ) );    //	]====+=[
                coords.push_back( make_pair( 3, i ) );      //	]==+= =[
                coords.push_back( make_pair( 4, i ) );      //	]==+= =[
                coords.push_back( make_pair( 4, i+4 ) ); }  //	]= =++=[
                                                            //	]= =++=[
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  //	]=++= =[
                coords.push_back( make_pair( 1, i ) );      //	]=++= =[
                coords.push_back( make_pair( 1, i+4 ) );    //	]= == =[
                coords.push_back( make_pair( 2, i ) );      //	]= == =[
                coords.push_back( make_pair( 2, i+4 ) ); }  //	]=++= =[
                                                            //	]=++= =[
            break; }
        case 6: {
            for( int i = 0; i < 2; i++ ) {                  //	]  ====[
                coords.push_back( make_pair( 1, i+2 ) );    //	]===+ =[
                coords.push_back( make_pair( 2, i+1 ) );    //	]=+ ++=[
                coords.push_back( make_pair( 3, i+3 ) );    //	]=++ +=[
                coords.push_back( make_pair( 4, i+2 ) ); }  //	]= +===[
                                                            //	]====  [
            break; }
        }
        break;
    case CROSS:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 4; i++ ) {                  //	]+=====[
                coords.push_back( make_pair( i, i+1 ) );    //	]=+=+= [
                coords.push_back( make_pair( i, 5-i ) ); }  //	]==+== [
            coords.push_back( make_pair( 4, 1 ) );          //	]=+=+= [
                                                            //	]+===+ [
                                                            //	]=     [
            break; }
        case 1: {
            for( int i = 0; i < 4; i++ ) {                      //	] +====[
                coords.push_back( make_pair( i+1, i+1 ) );      //	] =+=+=[
                coords.push_back( make_pair( i+1, 5-i ) ); }    //	] ==+==[
            coords.push_back( make_pair( 5, 1 ) );              //	]==+=+=[
                                                                //	]=+===+[
                                                                //	]=     [
            break; }
        case 2: {
            for( int i = 0; i < 2; i++ ) {                  //	]++= ==[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++====[
                coords.push_back( make_pair( 1, i+4 ) );    //	] =====[
                coords.push_back( make_pair( i+4, 0 ) );    //	]=====+[
                coords.push_back( make_pair( 5, i+1 ) ); }  //	]=== =+[
                                                            //	]=== ++[
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                  //	]+++===[
                coords.push_back( make_pair( 0, i+4 ) );    //	]+=====[
                coords.push_back( make_pair( i+1, 5 ) );    //	]  === [
                coords.push_back( make_pair( 4, i ) );      //	]======[
                coords.push_back( make_pair( 5, i ) ); }    //	]====++[
                                                            //	]=== ++[
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                  //	]++ ===[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++=== [
                coords.push_back( make_pair( 1, i+4 ) );    //	]===== [
                coords.push_back( make_pair( i, 1 ) );      //	]===== [
                coords.push_back( make_pair( i+3, 1 ) ); }  //	]++=++ [
                                                            //	]===== [
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  //	]======[
                coords.push_back( make_pair( 1, i ) );      //	]=+====[
                coords.push_back( make_pair( 1, i+3 ) );    //	]=+====[
                coords.push_back( make_pair( 4, i ) );      //	]===== [
                coords.push_back( make_pair( 5, i ) ); }    //	]=+==++[
                                                            //	]=+==++[
            break; }
        case 6: {
            for( int i = 0; i < 4; i++ ) {                      //	] ==+==[
                coords.push_back( make_pair( i+1, i+1 ) ); }    //	] = =+=[
            for( int i = 0; i <= 2; i +=2 ) {                   //	]=+=+=+[
                coords.push_back( make_pair( i+1, i+3 ) );      //	]==+= =[
                coords.push_back( make_pair( i+3, i+1 ) ); }    //	]=+=+==[
                                                                //	]====  [
            break; }
        case 7: {
            for( int i = 0; i < 5; i++ ) {                      //	] +====[
                coords.push_back( make_pair( i+1, 5-i ) ); }    //	]==+= =[
            coords.push_back( make_pair( 1, 3 ) );              //	]=+=+==[
            coords.push_back( make_pair( 3, 1 ) );              //	]=  =+=[
                                                                //	]=  +=+[
                                                                //	]===== [
            break; }
        }
        break;
    case GALLERY:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 4; i++ ) {                  //	]======[
                coords.push_back( make_pair( 1, i+1 ) );    //	]=++++=[
                coords.push_back( make_pair( 2, i+1 ) );    //	]=++++=[
                coords.push_back( make_pair( 3, i+1 ) );    //	]=++++=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //	]=++++=[
                                                            //	]======[
            break; }
        case 1: {
            for( int i = 0; i < 4; i++ ) {                  //	]======[
                coords.push_back( make_pair( 0, i+1 ) );    //	]++==++[
                coords.push_back( make_pair( 1, i+1 ) );    //	]++==++[
                coords.push_back( make_pair( 4, i+1 ) );    //	]++==++[
                coords.push_back( make_pair( 5, i+1 ) ); }  //	]++==++[
                                                            //	]======[
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                  //	]=++++=[
                coords.push_back( make_pair( i+1, 0 ) );    //	]======[
                coords.push_back( make_pair( i+1, 2 ) );    //	]=++++=[
                coords.push_back( make_pair( i+1, 3 ) );    //	]=++++=[
                coords.push_back( make_pair( i+1, 5 ) ); }  //	]======[
                                                            //	]=++++=[
            break; }
        case 3: {
            for( int i = 0; i < 4; i++ ) {                  //	]======[
                coords.push_back( make_pair( 0, i+1 ) );    //	]+=++=+[
                coords.push_back( make_pair( 2, i+1 ) );    //	]+=++=+[
                coords.push_back( make_pair( 3, i+1 ) );    //	]+=++=+[
                coords.push_back( make_pair( 5, i+1 ) ); }  //	]+=++=+[
                                                            //	]======[
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {                  //	]=+++==[
                coords.push_back( make_pair( 0, i+2 ) );    //	]+=====[
                coords.push_back( make_pair( i+1, 5 ) );    //	]+=++=+[
                coords.push_back( make_pair( i+2, 0 ) );    //	]+=++=+[
                coords.push_back( make_pair( 5, i+1 ) ); }  //	]=====+[
            for( int i = 0; i < 2; i++ ) {                  //	]==+++=[
                coords.push_back( make_pair( 2, i+2 ) );
                coords.push_back( make_pair( 3, i+2 ) ); }
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  //	]++====[
                coords.push_back( make_pair( 0, i+4 ) );    //	]++=++=[
                coords.push_back( make_pair( 1, i+4 ) );    //	]===++=[
                coords.push_back( make_pair( 1, i+1 ) );    //	]=++===[
                coords.push_back( make_pair( 2, i+1 ) );    //	]=++=++[
                coords.push_back( make_pair( 3, i+3 ) );    //	]====++[
                coords.push_back( make_pair( 4, i+3 ) );
                coords.push_back( make_pair( 4, i ) );
                coords.push_back( make_pair( 5, i ) ); }
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                  //	]  ====[
                coords.push_back( make_pair( i+1, 0 ) );    //	] +=+=+[
                coords.push_back( make_pair( 5, i ) ); }    //	]===+=+[
            for( int i = 0; i < 3; i++ ) {                  //	]=+++=+[
                coords.push_back( make_pair( i+1, 2 ) );    //	]=====+[
                coords.push_back( make_pair( 3, i+2 ) ); }  //	]=+++++[
            coords.push_back( make_pair( 1, 4 ) );
            break; }
        case 7: {
            for( int i = 0; i < 5; i++ ) {                  //	]+++++=[
                coords.push_back( make_pair( 0, i+1 ) );    //	]+=====[
                coords.push_back( make_pair( i, 5 ) ); }    //	]+=+++=[
            for( int i = 0; i < 3; i++ ) {                  //	]+=+===[
                coords.push_back( make_pair( 2, i+1 ) );    //	]+=+=+ [
                coords.push_back( make_pair( i+2, 3 ) ); }  //	]====  [
            coords.push_back( make_pair( 4, 1 ) );
            break; }
        }
        break;
    case SCATTER:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 4; i++ ) {                      //	]==+===[
                coords.push_back( make_pair( i+2, 5-i ) );      //	]===+==[
                coords.push_back( make_pair( i+2, 3-i ) ); }    //	]==+=+=[
            coords.push_back( make_pair( 2, 1 ) );              //	]===+=+[
            coords.push_back( make_pair( 3, 0 ) );              //	]==+=+=[
                                                                //	]===+=+[
            break; }
        case 1: {
            for( int i = 0; i <= 4; i += 2 ) {              //	]+=+===[
                coords.push_back( make_pair( i, 3 ) );      //	]=+=+==[
                coords.push_back( make_pair( i+1, 2 ) ); }  //	]+=+=+=[
            for( int i = 0; i <= 2; i += 2 ) {              //	]=+=+=+[
                coords.push_back( make_pair( i, 5 ) );      //	]======[
                coords.push_back( make_pair( i+1, 4 ) ); }  //	]======[
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                      //	] =====[
                coords.push_back( make_pair( i, 3-i ) );        //	]=+====[
                coords.push_back( make_pair( i+1, 4-i ) ); }    //	]+=+===[
                                                                //	]=+=+==[
                                                                //	]==+=+=[
                                                                //	]===+= [
            break; }
        case 3: {
            for( int i = 0; i < 4; i++ ) {                      //	] =+===[
                coords.push_back( make_pair( i+2, 5-i ) );      //	] +=+==[
                coords.push_back( make_pair( i+1, 4-i ) ); }    //	]==+=+=[
            coords.push_back( make_pair( 2, 1 ) );              //	]=+=+=+[
            coords.push_back( make_pair( 1, 2 ) );              //	]==+=+=[
                                                                //	]====  [
            break; }
        case 4: {
            for( int i = 0; i < 5; i++ ) {                      //	]=+====[
                coords.push_back( make_pair( i+1, 5-i ) ); }    //	]+=+===[
            for( int i = 0; i < 2; i++ ) {                      //	]=+=+==[
                coords.push_back( make_pair( i, 4-i ) );        //	] ===+=[
                coords.push_back( make_pair( i+3, 1-i ) ); }    //	]===+=+[
                                                                //	]== =+=[
            break; }
        case 5: {
            for( int i = 0; i < 6; i++ ) {                      //	]+=+ ==[
                coords.push_back( make_pair( i, 5-i ) ); }      //	]=+====[
            for( int i = 0; i <= 2; i += 2 ) {                  //	]+=+== [
                coords.push_back( make_pair( i+1, i+2 ) );      //	] ==+=+[
                coords.push_back( make_pair( i+2, i+1 ) ); }    //	]====+=[
                                                                //	]== +=+[
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                      //	]=+====[
                coords.push_back( make_pair( i+1, 5-i ) ); }    //	]==+===[
            for( int i = 0; i < 3; i++ ) {                      //	] +=+==[
                coords.push_back( make_pair( i+1, 3-i ) ); }    //	]==+=+=[
                                                                //	]===+=+[
                                                                //	]=== ==[
            break; }
        case 7: {
            for( int i = 0; i < 6; i++ ) {                      //	]+== ==[
                coords.push_back( make_pair( i, 5-i ) ); }      //	]=+=+==[
            coords.push_back( make_pair( 0, 3 ) );              //	]+=+=+ [
            coords.push_back( make_pair( 3, 0 ) );              //	]===+==[
            coords.push_back( make_pair( 3, 4 ) );              //	]= ==+=[
            coords.push_back( make_pair( 4, 3 ) );              //	]===+=+[
            break; }
        }
        break;
    case LAYER:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 5; i++ ) {                  //	]= ++==[
                coords.push_back( make_pair( 2, i+1 ) );    //	]==++==[
                coords.push_back( make_pair( 3, i+1 ) ); }  //	]==++==[
                                                            //	]==++==[
                                                            //	]==++ =[
                                                            //	]=    =[
            break; }
        case 1: {
            for( int i = 0; i < 4; i++ ) {                  //	]    ==[
                coords.push_back( make_pair( 2, i+1 ) );    //	]==+=+=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //	]==+=+=[
                                                            //	]==+=+=[
                                                            //	]==+=+=[
                                                            //	]=     [
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                  //	]    ==[
                coords.push_back( make_pair( 3, i+1 ) );    //	]===++=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //	]===++=[
                                                            //	]===++=[
                                                            //	]===++=[
                                                            //	]=     [
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                  //	]=+====[
                coords.push_back( make_pair( 1, i+4 ) );    //	]=+====[
                coords.push_back( make_pair( 2, i+2 ) );    //	]==+===[
                coords.push_back( make_pair( i+4, 1 ) ); }  //	] =++==[
            coords.push_back( make_pair( 3, 2 ) );          //	]====++[
                                                            //	]== ===[
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                  //	]=== ==[
                coords.push_back( make_pair( i, 4 ) );      //	]++====[
                coords.push_back( make_pair( i+2, 3 ) );    //	]==++= [
                coords.push_back( make_pair( 4, i ) ); }    //	]===+==[
            coords.push_back( make_pair( 3, 2 ) );          //	]====+=[
                                                            //	]====+=[
            break; }
        case 5: {
            for( int i = 0; i < 4; i++ ) {                  //	]    ==[
                coords.push_back( make_pair( 3, i+1 ) );    //	]===++=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //	]===++=[
                                                            //	]===++=[
                                                            //	]===++=[
                                                            //	]=     [
            break; }
        case 6: {
            for( int i = 0; i < 2; i++ ) {                  //	]   ===[
                coords.push_back( make_pair( i+1, 4 ) );    //	] ++===[
                coords.push_back( make_pair( i+2, 3 ) );    //	]==++==[
                coords.push_back( make_pair( i+3, 2 ) ); }  //	]===++ [
            coords.push_back( make_pair( 4, 1 ) );          //	]====+ [
                                                            //	]====  [
            break; }
        case 7: {
            for( int i = 0; i < 2; i++ ) {                  //	]    ==[
                coords.push_back( make_pair( i, 4 ) );      //	]++=== [
                coords.push_back( make_pair( i+1, 3 ) );    //	]=++===[
                coords.push_back( make_pair( i+3, 2 ) );    //	]===++=[
                coords.push_back( make_pair( i+4, 1 ) ); }  //	] ===++[
                                                            //	]==    [
            break; }
        }
        break;
    }

    // Add Energy item Pick-up Resources
    for( int i = 0; i < numItems; i += itemWorth ) {
        if( !coords.empty() ) {
            int randPlace = rand() % coords.size();
            int xPos = coords[randPlace].first;
            int yPos = coords[randPlace].second;
            if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 && map[xPos][yPos].state <= 1 && map[xPos][yPos].rockHP <= 3 ) {
                map[xPos][yPos].item++;
                if( map[xPos][yPos].rockHP > 0 ) { map[xPos][yPos].rockType = 1; }
                if( map[xPos][yPos].item >= 2 ) { coords.erase( coords.begin() + randPlace ); }
            }
            else { coords.erase( coords.begin() + randPlace ); }
        }
    }

    // Remove locations where there is an item, so that traps can be correctly placed
    for( int i = 0; i < coords.size(); i++ ) {
        int xPos = coords[i].first;
        int yPos = coords[i].second;
        if( map[xPos][yPos].item >= 1 ) { coords.erase( coords.begin() + i ); i--; }
    }

    // Add Damaging Energy traps
    for( int i = 0; i < numTraps; i++ ) {
        if( !coords.empty() ) {
            int randPlace = rand() % coords.size();
            int xPos = coords[randPlace].first;
            int yPos = coords[randPlace].second;
            if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 && map[xPos][yPos].state <= 1 && map[xPos][yPos].rockHP <= 3 && map[xPos][yPos].item == 0 ) {
                map[xPos][yPos].item = -1;
                if( map[xPos][yPos].rockHP > 0 ) { map[xPos][yPos].rockType = 1; }
            }
            coords.erase( coords.begin() + randPlace );
        }
    }
}
void Board::generateRocks( int type, int subtype ) {
    // Randomly Generate Rocks based on level templates
    if( numRocks <= 0 ) return;

    vector<pair<int, int>> coords;

    switch( type ) {
    default: case ROOM:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 4; i++ ) {                  //	] =O=O=[
                coords.push_back( make_pair( 2, i+2 ) );    //	] =O=  [
                coords.push_back( make_pair( 4, i ) ); }    //	] =O=O=[
            coords.push_back( make_pair( 4, 5 ) );          //	] =O=O=[
                                                            //	]   =O=[
                                                            //	]====O=[
            break; }
        case 1: {
            for( int i = 0; i < 3; i++ ) {					//	]OO==O=[
			    coords.push_back( make_pair( 0, i + 3 ) );  //	]OO==  [
                coords.push_back( make_pair( 1, i + 3 ) );	//	]OO==  [
			    coords.push_back( make_pair( 4, i ) );      //	]  ==OO[
                coords.push_back( make_pair( 5, i ) ); }	//	]  ==OO[
            coords.push_back( make_pair( 4, 5 ) );          //	]====OO[
            break; }
        case 2: {
		    coords.push_back( make_pair( 2, 2 ) );              //	]=== ==[
            coords.push_back( make_pair( 3, 3 ) );	            //	]=OO===[
		    for(int i = 0; i < 2; i++) {					    //	]=OOO= [
			    coords.push_back( make_pair( 1, i + 3 ) );      //	] =OOO=[
                coords.push_back( make_pair( 2, i + 3 ) );		//	]===OO=[
			    coords.push_back( make_pair( 3, i + 1 ) );      //	]== ===[
                coords.push_back( make_pair( 4, i + 1 ) ); }
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {						//	]=    =[
                coords.push_back( make_pair( 0, 2 + i ) );      //	]=O==O=[
                coords.push_back( make_pair( 2, 2 + i ) );		//	]O=OO=O[
                coords.push_back( make_pair( 3, 2 + i ) );      //	]O=OO=O[
                coords.push_back( make_pair( 5, 2 + i ) ); }	//	]=O==O=[
            coords.push_back( make_pair( 1, 1 ) );              //	]=    =[
            coords.push_back( make_pair( 1, 4 ) );
            coords.push_back( make_pair( 4, 1 ) );
            coords.push_back( make_pair( 4, 4 ) );
            break; }
        case 4: {
            coords.push_back( make_pair( 0, 3 ) );	coords.push_back( make_pair( 1, 2 ) );  //	]==OO==[
            coords.push_back( make_pair( 1, 3 ) );	coords.push_back( make_pair( 2, 1 ) );	//	]==O=O=[
            coords.push_back( make_pair( 2, 2 ) );	coords.push_back( make_pair( 2, 4 ) );  //	]OO===O[
            coords.push_back( make_pair( 2, 5 ) );	coords.push_back( make_pair( 3, 0 ) );	//	]=OO=OO[
            coords.push_back( make_pair( 3, 1 ) );	coords.push_back( make_pair( 3, 5 ) );  //	]==OO==[
            coords.push_back( make_pair( 4, 2 ) );	coords.push_back( make_pair( 4, 4 ) );	//	]===O==[
            coords.push_back( make_pair( 5, 2 ) );	coords.push_back( make_pair( 5, 3 ) );
            break; }
        case 5: {
            for( int i = 0; i < 3; i++ ) {                      //	]== ===[
                coords.push_back( make_pair( i+1, 3-i ) ); }    //	]=O=O==[
            for( int i = 0; i < 4; i++ ) {                      //	]=OO=O=[
                coords.push_back( make_pair( i+1, 4-i ) ); }    //	] =OO= [
            coords.push_back( make_pair( 3, 4 ) );              //	]===OO=[
            coords.push_back( make_pair( 4, 3 ) );              //	]== ===[
        break; }
        case 6: {
            for( int i = 0; i < 2; i++ ) {                                                              //	]     =[
                coords.push_back( make_pair( i, 3 ) );    coords.push_back( make_pair( i, 4 ) );        //	]OO=OO=[
                coords.push_back( make_pair( i+1, 1 ) );  coords.push_back( make_pair( i+3, 4 ) );      //	]OO=O==[
                coords.push_back( make_pair( i+4, 1 ) );  coords.push_back( make_pair( i+4, 2 ) ); }    //	]==O=OO[
            coords.push_back( make_pair( 2, 2 ) );    coords.push_back( make_pair( 3, 3 ) );            //	]=OO=OO[
                                                                                                        //	]=     [
            break; }
        }
        break;
    case PLUS:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 3; i++ ) {										                                // ] ==O==[
                coords.push_back( make_pair( i + 2, 2 ) );		coords.push_back( make_pair( 2, i + 2 ) );		// ] =OOO=[
                coords.push_back( make_pair( i + 2, 4 ) );		coords.push_back( make_pair( 4, i + 2 ) ); }	// ]=OO=OO[
            coords.push_back( make_pair( 1, 3 ) );		coords.push_back( make_pair( 3, 5 ) );					// ]==OOO=[
            coords.push_back( make_pair( 3, 1 ) );		coords.push_back( make_pair( 5, 3 ) );					// ]===O==[
                                                                                                                // ]====  [
            break; }
        case 1: {
            for( int i = 0; i < 3; i++ ) {									                                // ] ===O=[
                coords.push_back( make_pair( i + 2, 2 ) );	coords.push_back( make_pair( 2, i + 2 ) );		// ] =OOOO[
                coords.push_back( make_pair( i + 2, 4 ) );	coords.push_back( make_pair( 4, i + 2 ) ); }	// ] =O=O=[
            coords.push_back( make_pair( 1, 2 ) );	coords.push_back( make_pair( 4, 5 ) );					// ]=OOOO=[
            coords.push_back( make_pair( 2, 1 ) );	coords.push_back( make_pair( 5, 4 ) );					// ]==O===[
                                                                                                            // ]===   [
            break; }
        case 2: {
            coords.push_back( make_pair( 4, 4 ) );
            for( int i = 0; i < 3; i++ ) {									                                // ] =O===[
                coords.push_back( make_pair( i + 3, 2 ) );	coords.push_back( make_pair( 2, i + 3 ) );		// ] OOOO=[
                coords.push_back( make_pair( i + 1, 4 ) );	coords.push_back( make_pair( 4, i + 1 ) ); }	// ] =O=O=[
                                                                                                            // ]===OOO[
                                                                                                            // ]====O=[
                                                                                                            // ]===   [
            break; }
        case 3: {
            for( int i = 0; i < 3; i++ ) {						// ] O=O==[
                coords.push_back( make_pair( i + 3, 1 ) );      // ] O=O =[
                coords.push_back( make_pair( 1, i + 3 ) );		// ] O=OOO[
                coords.push_back( make_pair( i + 3, 3 ) );      // ]======[
                coords.push_back( make_pair( 3, i + 3 ) ); }	// ]===OOO[
                                                                // ]===   [
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {						// ] OOO==[
                coords.push_back( make_pair( i + 3, 1 ) );      // ] O=O==[
                coords.push_back( make_pair( 1, i + 3 ) );		// ] O=OOO[
                coords.push_back( make_pair( i + 3, 3 ) );      // ]==O==O[
                coords.push_back( make_pair( 3, i + 3 ) ); }	// ]===OOO[
            coords.push_back( make_pair( 2, 2 ) );              // ]===   [
            coords.push_back( make_pair( 2, 5 ) );
            coords.push_back( make_pair( 5, 2 ) );
                                                                                    
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                                                                  // ]  O=O=[
                coords.push_back( make_pair( 2, i + 1 ) );	coords.push_back( make_pair( i + 1, 2 ) );		// ]=OO=OO[
                coords.push_back( make_pair( 2, i + 4 ) );  coords.push_back( make_pair( i + 4, 2 ) );      // ]===O==[
                coords.push_back( make_pair( 4, i + 1 ) );  coords.push_back( make_pair( i + 1, 4 ) );      // ]=OO=OO[
                coords.push_back( make_pair( 4, i + 4 ) );	coords.push_back( make_pair( i + 4, 4 ) ); }	// ]= O=O [
            coords.push_back( make_pair( 3, 3 ) );				                                            // ]===== [
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                  // ]  =O==[
                coords.push_back( make_pair( i+1, 3 ) );    // ] ==O =[
                coords.push_back( make_pair( 3, i+1 ) ); }  // ]=OOOOO[
                                                            // ]== O==[
                                                            // ]===O= [
                                                            // ]====  [
            break; }
        }
        break;
    case PATH:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 3; i++ ) {                  //	]=OO===[
                coords.push_back( make_pair( 1, i+3 ) );    //	]=OO===[
                coords.push_back( make_pair( 3, i ) );      //	]=OO  =[
                coords.push_back( make_pair( 2, i+3 ) );    //	]=  OO=[
                coords.push_back( make_pair( 4, i ) ); }    //	]===OO=[
                                							//	]===OO=[
            break; }
        case 1: {
            for( int i = 0; i < 2; i++ ) {                  //	]=OO ==[
                coords.push_back( make_pair( 1, i+4 ) );    //	]=OOO==[
                coords.push_back( make_pair( 2, i+4 ) );    //	]== O= [
                coords.push_back( make_pair( 3, i+3 ) );    //	] =O ==[
                coords.push_back( make_pair( 2, i+1 ) );    //	]==OOO=[
                coords.push_back( make_pair( 3, i ) );      //	]== OO=[
                coords.push_back( make_pair( 4, i ) ); }
            break; }
        case 2: {
            for( int i = 0; i < 2; i++ ) {                      //	] =OO==[
                coords.push_back( make_pair( i + 2, 4 ) );      //	]==OO==[
                coords.push_back( make_pair( i, 2 ) );          //	]OO==OO[
                coords.push_back( make_pair( i + 2, 5 ) );      //	]OO==OO[
                coords.push_back( make_pair( i + 4, 2 ) );      //	]  ==  [
                coords.push_back( make_pair( i, 3 ) );          //	]====  [
                coords.push_back( make_pair( i + 4, 3 ) ); }
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {					    //	]    ==[
                coords.push_back( make_pair( i, 3 ) );          //	]OO==OO[
                coords.push_back( make_pair( i + 2, 1 ) );		//	]OO==OO[
                coords.push_back( make_pair( i, 4 ) );          //	]==OO==[
                coords.push_back( make_pair( i + 2, 2 ) );      //	] =OO= [
                coords.push_back( make_pair( i + 4, 3 ) );	    //	]===== [
                coords.push_back( make_pair( i + 4, 4 ) ); }
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {					    //	]  =OO=[
			    coords.push_back( make_pair( 1, i + 2 ) );      //	]===OO=[
                coords.push_back( make_pair( 3, i ) );          //	]=OO= =[
                coords.push_back( make_pair( 3, i + 4 ) );	    //	]=OO= =[
			    coords.push_back( make_pair( 2, i + 2 ) );      //	]= =OO=[
                coords.push_back( make_pair( 4, i ) );          //	]= =OO=[
                coords.push_back( make_pair( 4, i + 4 ) ); }
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {					    //	]=OO= =[
                coords.push_back( make_pair( 1, i ) );          //	]=OO= =[
                coords.push_back( make_pair( 1, i + 4 ) );      //	]= =OO=[
                coords.push_back( make_pair( 3, i + 2 ) );	    //	]= =OO=[
                coords.push_back( make_pair( 2, i ) );          //	]=OO= =[
                coords.push_back( make_pair( 2, i + 4 ) );      //	]=OO= =[
                coords.push_back( make_pair( 4, i + 2 ) ); }
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                  //  ]  =O==[
                coords.push_back( make_pair( 1, i+2 ) );    //  ]=O=O =[
                coords.push_back( make_pair( 2, i ) );      //  ]=O OO=[
                coords.push_back( make_pair( 3, i+3 ) );    //  ]=OO O=[
                coords.push_back( make_pair( 4, i+1 ) ); }  //  ]= O=O=[
                                                            //  ]==O=  [
            break; }
        }
        break;
    case CROSS:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 5; i++ ) {                  //  ]O===O=[
                coords.push_back( make_pair( i, i+1 ) );    //  ]=O=O= [
                coords.push_back( make_pair( i, 5-i ) ); }  //  ]==O== [
                                                            //  ]=O=O= [
                                                            //  ]O===O [
                                                            //  ]==    [
            break; }
        case 1: {
            for( int i = 0; i < 5; i++ ) {                      //  ] O===O[
                coords.push_back( make_pair( i+1, i+1 ) );      //  ] =O=O=[
                coords.push_back( make_pair( i+1, 5-i ) ); }    //  ] ==O==[
                                                                //  ]==O=O=[
                                                                //  ]=O===O[
                                                                //  ]=     [
            break; }
        case 2: {                                                   
            for( int i = 0; i < 2; i++ ) {                  //  ]=== ==[
                coords.push_back( make_pair( i+1, 2 ) );    //  ]=OO=OO[
                coords.push_back( make_pair( i+1, 4 ) );    //  ] ==O==[
                coords.push_back( make_pair( i+4, 2 ) );    //  ]=OO=OO[
                coords.push_back( make_pair( i+4, 4 ) ); }  //  ]=== ==[
            coords.push_back( make_pair( 3, 3 ) );          //  ]=== ==[
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                  //  ]==O=O=[
                coords.push_back( make_pair( 2, i+1 ) );    //  ]==O=O=[
                coords.push_back( make_pair( 2, i+4 ) );    //  ]  =O= [
                coords.push_back( make_pair( 4, i+1 ) );    //  ]==O=O=[
                coords.push_back( make_pair( 4, i+4 ) ); }  //  ]==O=O=[
            coords.push_back( make_pair( 3, 3 ) );          //  ]=== ==[
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                      // ]== ===[
                coords.push_back( make_pair( i, 1 ) );          // ]OO=OO [
                coords.push_back( make_pair( 2, i + 2 ) );      // ]==O== [
                coords.push_back( make_pair( i + 3, 1 ) );      // ]==O== [
                coords.push_back( make_pair( i, 4 ) );          // ]OO=OO [
                coords.push_back( make_pair( i + 3, 4 ) ); }    // ]===== [
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                      // ]  == =[
                coords.push_back( make_pair( 1, i ) );          // ]=O==O=[
                coords.push_back( make_pair( i + 2, 2 ) );      // ]=O==O=[
                coords.push_back( make_pair( 1, i + 3 ) );      // ]==OO= [
                coords.push_back( make_pair( 4, i ) );          // ]=O==O=[
                coords.push_back( make_pair( 4, i + 3 ) ); }    // ]=O==O=[
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                          // ] ==O=O[
                coords.push_back( make_pair( i + 1, i + 1 ) );      // ] = =O=[
                coords.push_back( make_pair( 3 - i, i + 1 ) );      // ]=O=O=O[
                coords.push_back( make_pair( i + 3, i + 3 ) );      // ]==O= =[
                coords.push_back( make_pair( 5 - i, i + 3 ) ); }    // ]=O=O==[
                                                                    // ]====  [
            break; }
        case 7: {
            for( int i = 0; i < 3; i++ ) {                          // ] O=O==[
                coords.push_back( make_pair( i + 1, i + 3 ) );      // ]==O= =[
                coords.push_back( make_pair( 5 - i, i + 1 ) );      // ]=O=O=O[
                coords.push_back( make_pair( i + 3, i + 1 ) );      // ]=  =O=[
                coords.push_back( make_pair( 3 - i, i + 3 ) ); }    // ]=  O=O[
                                                                    // ]===== [
            break; }
        }
        break;
    case GALLERY:
        switch( subtype ) {
        default: case 0: {
            for( int i = 1; i < 5; i++ ) {				    //	]======[
                coords.push_back( make_pair( i, 1 ) );      //	]=OOOO=[
                coords.push_back( make_pair( i, 2 ) );	    //	]=OOOO=[
                coords.push_back( make_pair( i, 3 ) );      //	]=OOOO=[
                coords.push_back( make_pair( i, 4 ) ); }	//	]=OOOO=[
                                                            //	]======[
            break; }
        case 1: {
            for( int i = 1; i < 5; i++ ) {					// ]======[
                coords.push_back( make_pair( 0, i ) );      // ]OO==OO[
                coords.push_back( make_pair( 1, i ) );	    // ]OO==OO[
                coords.push_back( make_pair( 4, i ) );      // ]OO==OO[
                coords.push_back( make_pair( 5, i ) ); }	// ]OO==OO[
                                                            // ]======[
            break; }
        case 2: {
            for( int i = 1; i < 5; i++ ) {					// ]=OOOO=[
                coords.push_back( make_pair( i, 0 ) );      // ]======[
                coords.push_back( make_pair( i, 3 ) );	    // ]=OOOO=[
                coords.push_back( make_pair( i, 2 ) );      // ]=OOOO=[
                coords.push_back( make_pair( i, 5 ) ); }	// ]======[
                                                            // ]=OOOO=[
            break; }
        case 3: {
            for( int i = 1; i < 5; i++ ) {                  // ]======[
                coords.push_back( make_pair( 0, i ) );      // ]O=OO=O[
                coords.push_back( make_pair( 3, i ) );      // ]O=OO=O[
                coords.push_back( make_pair( 2, i ) );      // ]O=OO=O[
                coords.push_back( make_pair( 5, i ) ); }    // ]O=OO=O[
                                                            // ]======[
            break; }
        case 4: {
            for( int i = 0; i < 3; i++ ) {                      // ]=OOO==[
                coords.push_back( make_pair( 0, i + 2 ) );      // ]O=====[
                coords.push_back( make_pair( i + 1, 5 ) );      // ]O=OO=O[
                coords.push_back( make_pair( i + 2, 0 ) );      // ]O=OO=O[
                coords.push_back( make_pair( 5, i + 1 ) ); }    // ]=====O[
            coords.push_back( make_pair( 2, 2 ) );              // ]==OOO=[
            coords.push_back( make_pair( 3, 2 ) );
            coords.push_back( make_pair( 2, 3 ) );
            coords.push_back( make_pair( 3, 3 ) );
            break; }
        case 5: {
            for( int i = 0; i < 2; i++ ) {                  // ]OO====[
                coords.push_back( make_pair( 0, i + 4 ) );  // ]OO=OO=[
                coords.push_back( make_pair( 1, i + 4 ) );  // ]===OO=[
                coords.push_back( make_pair( 1, i + 1 ) );  // ]=OO===[
                coords.push_back( make_pair( 2, i + 1 ) );  // ]=OO=OO[
                coords.push_back( make_pair( 3, i + 3 ) );  // ]====OO[
                coords.push_back( make_pair( 4, i + 3 ) );
                coords.push_back( make_pair( 4, i ) );
                coords.push_back( make_pair( 5, i ) ); }
            break; }
        case 6: {
            for( int i = 0; i < 5; i++ ) {                  // ]  ====[
                coords.push_back( make_pair( i+1, 0 ) );    // ] O=O=O[
                coords.push_back( make_pair( 5, i ) ); }    // ]===O=O[
            for( int i = 0; i < 3; i++ ) {                  // ]=OOO=O[
                coords.push_back( make_pair( i+1, 2 ) );    // ]=====O[
                coords.push_back( make_pair( 3, i+2 ) ); }  // ]=OOOOO[
            coords.push_back( make_pair( 1, 4 ) );
            break; }
        case 7: {
            for( int i = 0; i < 5; i++ ) {                  // ]OOOOO=[
                coords.push_back( make_pair( 0, i+1 ) );    // ]O=====[
                coords.push_back( make_pair( i, 5 ) ); }    // ]O=OOO=[
            for( int i = 0; i < 3; i++ ) {                  // ]O=O===[
                coords.push_back( make_pair( 2, i+1 ) );    // ]O=O=O [
                coords.push_back( make_pair( i+2, 3 ) ); }  // ]====  [
            coords.push_back( make_pair( 4, 1 ) );
            break; }
        }
        break;
    case SCATTER:
        switch( subtype ) {
        default: case 0: {
            for( int i = 0; i < 6; i += 2 ) {	            //	]==O=O=[
                coords.push_back( make_pair( 2, i + 1 ) );	//	]===O=O[
                coords.push_back( make_pair( 3, i ) );		//	]==O=O=[
                coords.push_back( make_pair( 4, i + 1 ) );	//	]===O=O[
                coords.push_back( make_pair( 5, i ) ); }	//	]==O=O=[
                                                            //	]===O=O[
            break; }
        case 1: {
            //	]O=O=O=[
            //	]=O=O=O[
            //	]O=O=O=[
            //	]=O=O=O[
            //	]======[
            //	]======[
            for( int i = 0; i <= 4; i += 2 ) {
                coords.push_back( make_pair( i, 3 ) );
                coords.push_back( make_pair( i, 5 ) );
                coords.push_back( make_pair( i+1, 2 ) );
                coords.push_back( make_pair( i+1, 4 ) ); }
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {                      // ] =O===[
                coords.push_back( make_pair( i, 3-i ) );        // ]=O=O==[
                coords.push_back( make_pair( i+1, 4-i ) );      // ]O=O=O=[
                coords.push_back( make_pair( i+2, 5-i ) ); }    // ]=O=O=O[
                                                                // ]==O=O=[
                                                                // ]===O= [
            break; }
        case 3: {
            for( int i = 1; i <= 5; i += 2 ) {              // ] =O=O=[
                coords.push_back( make_pair( i, 2 ) );      // ] O=O=O[
                coords.push_back( make_pair( i, 4 ) );      // ]==O=O=[
                coords.push_back( make_pair( 2, i ) );      // ]=O=O=O[
                coords.push_back( make_pair( 4, i ) ); }    // ]==O=O=[
                                                            // ]====  [
            break; }
        case 4: {
            for( int i = 0; i < 5; i++ ) coords.push_back( make_pair( i + 1, 5 - i ) ); // ]=O=O==[
            for( int i = 0; i < 3; i++ ) coords.push_back( make_pair( i + 3, 5 - i ) ); // ]O=O=O=[
            for( int i = 0; i < 2; i++ ) {                                              // ]=O=O=O[
                coords.push_back( make_pair( i, 4 - i ) );                              // ] ===O=[
                coords.push_back( make_pair( i + 3, 1 - i ) ); }                        // ]===O=O[
                                                                                        // ]== =O=[
            break; }
        case 5: {
            for( int i = 0; i < 6; i++ ) coords.push_back( make_pair( i, 5-i ) );   // ]O=  ==[
            for( int i = 0; i < 2; i++ ) {                                          // ]=O=O==[
                coords.push_back( make_pair( i+1, 2-i ) );                          // ] =O=O [
                coords.push_back( make_pair( i+3, 4-i ) ); }                        // ] O=O= [
                                                                                    // ]==O=O=[
                                                                                    // ]==  =O[
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                      // ]=O=O==[
                coords.push_back( make_pair( i+1, 3-i ) );      // ]==O=O=[
                coords.push_back( make_pair( i+3, 5-i ) ); }    // ] O=O=O[
            for( int i = 0; i < 5; i++ ) {                      // ]==O=O=[
                coords.push_back( make_pair( i+1, 5-i ) ); }    // ]===O=O[
                                                                // ]=== ==[
            break; }
        case 7: {
            for( int i = 0; i < 6; i++ ) {
                coords.push_back( make_pair( i, 5-i ) ); }      //	]O= =O=[
            for( int i = 0; i < 2; i++ ) {                      //	]=O=O=O[
                coords.push_back( make_pair( i+3, i+4 ) );      //	]O=O=O=[
                coords.push_back( make_pair( i+4, i+3 ) ); }    //	]===O= [
            coords.push_back( make_pair( 0, 3 ) );              //	]= ==O=[
            coords.push_back( make_pair( 3, 0 ) );              //	]===O=O[
            break; }
        }
        break;
    case LAYER:
        switch( subtype ) {
        default: case 0: {
            for( int yPos = 1; yPos < 6; yPos++ ) {			//	]= OO==[
                coords.push_back( make_pair( 2, yPos ) );   //	]==OO==[
                coords.push_back( make_pair( 3, yPos ) ); }	//	]==OO==[
                                                            //	]==OO==[
                                                            //	]==OO =[
                                                            //	]=    =[
            break; }
        case 1: {
            for( int i = 0; i < 4; i++ ) {						//	]    ==[
                coords.push_back( make_pair( 1, i + 1 ) );      //	]=OO=O=[
                coords.push_back( make_pair( 4, i + 1 ) );		//	]=OO=O=[
                coords.push_back( make_pair( 2, i + 1 ) ); }	//	]=OO=O=[
                                                                //	]=OO=O=[
                                                                //	]=     [
            break; }
        case 2: {
            for( int i = 0; i < 4; i++ ) {					    //	]    ==[
                coords.push_back( make_pair( 1, i + 1 ) );      //	]=O=OO=[
                coords.push_back( make_pair( 4, i + 1 ) );		//	]=O=OO=[
                coords.push_back( make_pair( 3, i + 1 ) ); }	//	]=O=OO=[
                                                                //	]=O=OO=[
                                                                //	]=     [
            break; }
        case 3: {
            for( int i = 0; i < 2; i++ ) {                      //	]=OO===[
                coords.push_back( make_pair( i + 1, 5 ) );      //	]=OO===[
                coords.push_back( make_pair( i + 2, 3 ) );      //	]==OO==[
                coords.push_back( make_pair( i + 4, 2 ) );      //	] =OOOO[
                coords.push_back( make_pair( i + 1, 4 ) );      //	]====OO[
                coords.push_back( make_pair( i + 2, 2 ) );      //	]== ===[
                coords.push_back( make_pair( i + 4, 1 ) ); }
            break; }
        case 4: {
            for( int i = 0; i < 2; i++ ) {                      // ]=== ==[
                coords.push_back( make_pair( i, 3 ) );          // ]OO====[
                coords.push_back( make_pair( i + 2, 3 ) );      // ]OOOO= [
                coords.push_back( make_pair( i + 3, 1 ) );      // ]==OO==[
                coords.push_back( make_pair( i, 4 ) );          // ]===OO=[
                coords.push_back( make_pair( i + 2, 2 ) );      // ]===OO=[
                coords.push_back( make_pair( i + 3, 0 ) ); }                          
            break; }
        case 5: {
            for( int i = 0; i < 4; i++ ) {                      // ]    ==[
                coords.push_back( make_pair( 2, i + 1 ) );      // ]==OOO=[
                coords.push_back( make_pair( 3, i + 1 ) );      // ]==OOO=[
                coords.push_back( make_pair( 4, i + 1 ) ); }    // ]==OOO=[
                                                                // ]==OOO=[
                                                                // ]=     [
            break; }
        case 6: {
            for( int i = 0; i < 3; i++ ) {                      // ]   ===[
                coords.push_back( make_pair( i+1, 3-i ) );      // ] OO===[
                coords.push_back( make_pair( i+1, 4-i ) );      // ]=OOO==[
                coords.push_back( make_pair( i+2, 4-i ) ); }    // ]==OOO [
                                                                // ]===OO [
                                                                // ]====  [
            break; }
        case 7: {
            for( int i = 0; i < 3; i++ ) {                  // ]    ==[
                coords.push_back( make_pair( i, 4 ) );      // ]OOO== [
                coords.push_back( make_pair( i+1, 3 ) );    // ]=OOO==[
                coords.push_back( make_pair( i+2, 2 ) );    // ]==OOO=[
                coords.push_back( make_pair( i+3, 1 ) ); }  // ] ==OOO[
                                                            // ]==    [
            break; }
        }
        break;
    }

    while ( numRocks > 0 && !coords.empty() ) {
        int randPlace = rand() % coords.size();
        int xPos = coords[randPlace].first;
        int yPos = coords[randPlace].second;
        if ( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 ) {
            if ( map[xPos][yPos].item != 0 ) {
                map[xPos][yPos].rockType = 1;
                if ( map[xPos][yPos].rockHP >= 3 ) { coords.erase( coords.begin() + randPlace ); }
                else {
                    map[xPos][yPos].rockHP++;
                    numRocks--;
                }
            }
            else {
                if ( map[xPos][yPos].rockHP > 3 ) { map[xPos][yPos].rockType = 0; }
                if ( map[xPos][yPos].rockHP >= 5 ) { coords.erase( coords.begin() + randPlace ); }
                else {
                    map[xPos][yPos].rockHP++;
                    numRocks--;
                }
            }
        }
        else { coords.erase( coords.begin() + randPlace ); }
    }
}
void Board::generateFloor( int type, int subtype ) {
    if( numCracked <= 0 && numIce <= 0 && numPoison <= 0 && numHoly <= 0 ) return;

    vector<pair<int, int>> coords;
    switch( type ) {
    default: case ROOM:
        switch( subtype ) {
        default: case 0: {
            //	] ==X==[
            //	] ==X  [
            //	] ==X==[
            //	] ==X==[
            //	]   X==[
            //	]===X==[
            for( int i = 0; i < 6; i++ ) {
                coords.push_back( make_pair( 3, i ) );
            }
            break; }
        case 1: {
            //	]==X===[
            //	]==X=  [
            //	]==X=  [
            //	]  =X==[
            //	]  =X==[
            //	]===X==[
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( 2, i+3 ) );
                coords.push_back( make_pair( 3, i ) ); }
            break; }
        case 2: {
            //	]X== ==[
            //	]===X==[
            //	]====X [
            //	] X====[
            //	]==X===[
            //	]== ==X[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 2-i ) );
                coords.push_back( make_pair( i+3, 4-i ) ); }
            coords.push_back( make_pair( 0, 5 ) );
            coords.push_back( make_pair( 5, 0 ) );
            break; }
        case 3: {
            //	]=    =[
            //	]==XX==[
            //	]=X==X=[
            //	]=X==X=[
            //	]==XX==[
            //	]=    =[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 1, i+2 ) );
                coords.push_back( make_pair( i+2, 1 ) );
                coords.push_back( make_pair( i+2, 4 ) );
                coords.push_back( make_pair( 4, i+2 ) );
            }
            break; }
        case 4: {
            //	]====X=[
            //	]=X=X=X[
            //	]==XXX=[
            //	]X==X==[
            //	]====X=[
            //	]==X===[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 4-i ) );
                coords.push_back( make_pair( i+3, i+4 ) );
                coords.push_back( make_pair( i+3, 2-i ) );
                coords.push_back( make_pair( i+4, i+3 ) ); }
            coords.push_back( make_pair( 0, 2 ) );
            coords.push_back( make_pair( 2, 0 ) );
            coords.push_back( make_pair( 3, 3 ) );
            break; }
        case 5: {
            //	]X= ===[
            //	]==X===[
            //	]===X==[
            //	] X==X [
            //	]==X===[
            //	]== ==X[
            for( int i = 0; i < 3; i++ ) {
                   coords.push_back( make_pair( i+2, 4-i ) ); }  
            for( int i = 0; i < 2; i++ ) {
                   coords.push_back( make_pair( i+1, 2-i ) ); } 
            coords.push_back( make_pair( 0, 5 ) );
            coords.push_back( make_pair( 5, 0 ) );
        break; }
        case 6: {
            //	]     =[
            //	]==X===[
            //	]==X=XX[
            //	]XX=X==[
            //	]===X==[
            //	]=     [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, 2 ) );
                coords.push_back( make_pair( 2, i+3 ) );
                coords.push_back( make_pair( 3, i+1 ) );
                coords.push_back( make_pair( i+4, 3 ) ); }
            break; }
        }
        break;
    case PLUS:
        switch( subtype ) {
        default: case 0: {
            // ] =X=X=[
            // ] X===X[
            // ]===X==[
            // ]=X===X[
            // ]====  [                                                                          
            // ]==X=X=[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 2-i ) );
                coords.push_back( make_pair( i+1, i+4 ) );
                coords.push_back( make_pair( i+4, i+1 ) );
                coords.push_back( make_pair( i+4, 5-i ) ); }
            break; }
        case 1: {
            // ] =XX==[
            // ] X====[
            // ] X=X=X[
            // ]=====X[
            // ]===XX=[
            // ]===   [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 1, i+3 ) );
                coords.push_back( make_pair( i+2, 5 ) );
                coords.push_back( make_pair( i+3, 1 ) );
                coords.push_back( make_pair( 5, i+2 ) ); }
            coords.push_back( make_pair( 3, 3 ) );
            break; }
        case 2: {
            // ] ==XX=[
            // ] ====X[
            // ] X=X=X[
            // ]==X===[
            // ]===X==[
            // ]===   [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+2, i+2 ) );
                coords.push_back( make_pair( i+3, 5 ) );
                coords.push_back( make_pair( 5, i+3 ) ); }
            coords.push_back( make_pair( 1, 3 ) );
            coords.push_back( make_pair( 3, 1 ) );
            break; }
        case 3: {
            // ] =X===[
            // ] =X= =[
            // ] =X===[
            // ]==XXXX[
            // ]======[
            // ]===   [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 2, i+2 ) );
                coords.push_back( make_pair( i+2, 2 ) ); }
            break; }
        case 4: {
            // ] =====[
            // ] =X=X=[
            // ] =X===[
            // ]=X=XX=[
            // ]==X===[
            // ]===   [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 2-i ) );
                coords.push_back( make_pair( 2, i+3 ) );
                coords.push_back( make_pair( i+3, 2 ) ); }
            coords.push_back( make_pair( 4, 4 ) );
            break; }
        case 5: {
            // ]  =X==[
            // ]===X==[
            // ]XXX=XX[
            // ]===X==[
            // ]= =X= [
            // ]===X= [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 3 ) );
                coords.push_back( make_pair( 3, i+1 ) );
                coords.push_back( make_pair( 3, i+4 ) );
                coords.push_back( make_pair( i+4, 3 ) ); }
            coords.push_back( make_pair( 0, 3 ) );
            coords.push_back( make_pair( 3, 0 ) );
            break; }
        case 6: {
            // ]  X=X=[
            // ] XX= X[
            // ]======[
            // ]== =XX[
            // ]====X [
            // ]====  [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 4 ) );
                coords.push_back( make_pair( 4, i+1 ) );
                coords.push_back( make_pair( 4+i, 5-i ) ); }
            coords.push_back( make_pair( 2, 5 ) );
            coords.push_back( make_pair( 5, 2 ) );
            break; }
        }
        break;
    case PATH:
        switch( subtype ) {
        default: case 0: {
            //	]X==X==[
            //	]X==X==[
            //	]X==  =[
            //	]=  ==X[
            //	]==X==X[
            //	]==X==X[
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( 0, i+3 ) );
                coords.push_back( make_pair( 5, i ) ); }
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 2, i ) );
                coords.push_back( make_pair( 3, i+4 ) ); }
            break; }
        case 1: {
            //	]X== ==[
            //	]X===X=[
            //	]=X =X [
            //	] X= X=[
            //	]=X===X[
            //	]== ==X[
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( 1, i+1 ) );
                coords.push_back( make_pair( 4, i+2 ) ); }
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 0, i+4 ) );
                coords.push_back( make_pair( 5, i ) ); }
            break; }
        case 2: {
            //	] X==X=[
            //	]XX==XX[
            //	]==XX==[
            //	]==XX==[
            //	]  ==  [
            //	]====  [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, 4 ) );
                coords.push_back( make_pair( i+2, 2 ) );
                coords.push_back( make_pair( i+2, 3 ) );
                coords.push_back( make_pair( i+4, 4 ) ); }
            coords.push_back( make_pair( 1, 5 ) );
            coords.push_back( make_pair( 4, 5 ) );
            break; }
        case 3: {
            //	]    ==[
            //	]==XX==[
            //	]==XX==[
            //	]XX==XX[
            //	] X==X [
            //	]===== [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, 2 ) );
                coords.push_back( make_pair( i+2, 3 ) );
                coords.push_back( make_pair( i+2, 4 ) );
                coords.push_back( make_pair( i+4, 2 ) ); }
            coords.push_back( make_pair( 1, 1 ) );
            coords.push_back( make_pair( 4, 1 ) );
            break; }
        case 4: {
            //	]  X===[
            //	]=XX===[
            //	]===X =[
            //	]===X =[
            //	]= X===[
            //	]= X===[
            for( int i = 0; i < 2; i++ ) {
			    coords.push_back( make_pair( 2, i ) );
                coords.push_back( make_pair( 2, i+4 ) );
                coords.push_back( make_pair( 3, i+2 ) ); }
			coords.push_back( make_pair( 1, 4 ) );
            break; }
        case 5: {
            //	]===X =[
            //	]X==X =[
            //	]X X===[
            //	]X X===[
            //	]X==X =[
            //	]===X =[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 0, i+1 ) );
                coords.push_back( make_pair( 0, i+3 ) );
                coords.push_back( make_pair( 2, i+2 ) );
                coords.push_back( make_pair( 3, i ) );
                coords.push_back( make_pair( 3, i+4 ) ); }
            break; }
        case 6: {
            //  ]  X=X=[
            //  ]X=X= =[
            //  ]X= ==X[
            //  ]X== =X[
            //  ]= =X=X[
            //  ]=X=X  [
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( 0, i+2 ) );
                coords.push_back( make_pair( 5, i+1 ) ); }
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 2, i+4 ) );
                coords.push_back( make_pair( 3, i ) ); }
            coords.push_back( make_pair( 1, 0 ) );
            coords.push_back( make_pair( 4, 5 ) );
            break; }
        }
        break;
    case CROSS:
        switch( subtype ) {
        default: case 0: {
            //  ]==X===[
            //  ]==X===[
            //  ]XX=XX [
            //  ]==X== [
            //  ]==X== [
            //  ]==    [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, 3 ) );
                coords.push_back( make_pair( 2, i+1 ) );
                coords.push_back( make_pair( 2, i+4 ) );
                coords.push_back( make_pair( i+3, 3 ) ); }
            break; }
        case 1: {
            //  ] ==X==[
            //  ] ==X==[
            //  ] XX=XX[
            //  ]===X==[
            //  ]===X==[
            //  ]=     [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 3 ) );
                coords.push_back( make_pair( 3, i+1 ) );
                coords.push_back( make_pair( 3, i+4 ) );
                coords.push_back( make_pair( i+4, 3 ) ); }
            break; }
        case 2: {
            //  ]=== ==[
            //  ]===X==[
            //  ] XX=XX[
            //  ]===X==[
            //  ]=== ==[
            //  ]=== ==[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 3 ) );
                coords.push_back( make_pair( i+4, 3 ) ); }
            coords.push_back( make_pair( 3, 2 ) );
            coords.push_back( make_pair( 3, 4 ) );
            break; }
        case 3: {
            //  ]===X==[
            //  ]===X==[
            //  ]  X=X [
            //  ]===X==[
            //  ]===X==[
            //  ]=== ==[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 3, i+1 ) );
                coords.push_back( make_pair( 3, i+4 ) ); }
            coords.push_back( make_pair( 2, 3 ) );
            coords.push_back( make_pair( 4, 3 ) );
            break; }
        case 4: {
            // ]== ===[
            // ]==X== [
            // ]XX=XX [
            // ]XX=XX [
            // ]==X== [
            // ]===== [
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, 2 ) );
                coords.push_back( make_pair( i, 3 ) );
                coords.push_back( make_pair( i+3, 2 ) );
                coords.push_back( make_pair( i+3, 3 ) ); }
            coords.push_back( make_pair( 2, 1 ) );
            coords.push_back( make_pair( 2, 4 ) );
            break; }
        case 5: {
            // ]  == =[
            // ]==XX==[
            // ]==XX==[
            // ]=X==X [
            // ]==XX==[
            // ]==XX==[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 2, i ) );
                coords.push_back( make_pair( 2, i+3 ) );
                coords.push_back( make_pair( 3, i ) );
                coords.push_back( make_pair( 3, i+3 ) ); }
            coords.push_back( make_pair( 1, 2 ) );
            coords.push_back( make_pair( 4, 2 ) );
            break; }
        case 6: {
            // ] =X=X=[
            // ] = X=X[
            // ]==X=X=[
            // ]=X=X =[
            // ]==X=X=[
            // ]====  [
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( i+2, 3-i ) );
                coords.push_back( make_pair( i+2, 5-i ) ); }
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, 2-i ) );
                coords.push_back( make_pair( i+4, 5-i ) ); }
            break; }
        case 7: {
            // ] =X===[
            // ]=X=X =[
            // ]==X=X=[
            // ]=  X=X[
            // ]=  =X=[
            // ]===== [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i+1, 4-i ) );
                coords.push_back( make_pair( i+2, 5-i ) );
            }
            break; }
        }
        break;
    case GALLERY:
        switch( subtype ) {
        default: case 0: {
            //	]======[
            //	]X====X[
            //	]X====X[
            //	]X====X[
            //	]X====X[
            //	]======[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 5, i+1 ) );
                coords.push_back( make_pair( 0, i+1 ) ); }
            break; }
        case 1: {
            // ]======[
            // ]==XX==[
            // ]==XX==[
            // ]==XX==[
            // ]==XX==[
            // ]======[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 2, i+1 ) );
                coords.push_back( make_pair( 3, i+1 ) ); }
            break; }
        case 2: {
            // ]======[
            // ]=XXXX=[
            // ]======[
            // ]======[
            // ]=XXXX=[
            // ]======[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i+1, 1 ) );
                coords.push_back( make_pair( i+1, 4 ) ); }
            break; }
        case 3: {
            // ]======[
            // ]=X==X=[
            // ]=X==X=[
            // ]=X==X=[
            // ]=X==X=[
            // ]======[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 1, i+1 ) );
                coords.push_back( make_pair( 4, i+1 ) ); }
            break; }
        case 4: {
            // ]======[
            // ]==XX==[
            // ]=X==X=[
            // ]=X==X=[
            // ]==XX==[
            // ]======[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 1, i+2 ) );
                coords.push_back( make_pair( i+2, 1 ) );
                coords.push_back( make_pair( i+2, 4 ) );
                coords.push_back( make_pair( 4, i+2 ) ); }
            break; }
        case 5: {
            // ]==X===[
            // ]==X===[
            // ]=XX===[
            // ]===XX=[
            // ]===X==[
            // ]===X==[
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( 2, i+3 ) );
                coords.push_back( make_pair( 3, i ) ); }
            coords.push_back( make_pair( 1, 3 ) );
            coords.push_back( make_pair( 4, 2 ) );
            break; }
        case 6: {
            // ]  ====[
            // ] =X=X=[
            // ]=XX=X=[
            // ]====X=[
            // ]=XXXX=[
            // ]======[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i+1, 1 ) );
                coords.push_back( make_pair( 4, i+1 ) ); }
            coords.push_back( make_pair( 1, 3 ) );
            coords.push_back( make_pair( 2, 3 ) );
            coords.push_back( make_pair( 2, 4 ) );
            break; }
        case 7: {
            // ]======[
            // ]=XXXX=[
            // ]=X====[
            // ]=X=XX=[
            // ]=X=X= [
            // ]====  [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 1, i+1 ) );
                coords.push_back( make_pair( i+1, 4 ) ); }
            coords.push_back( make_pair( 3, 1 ) );
            coords.push_back( make_pair( 3, 2 ) );
            coords.push_back( make_pair( 4, 2 ) );
            break; }
        }
        break;
    case SCATTER:
        switch( subtype ) {
        default: case 0: {
            //	]===X==[
            //	]==X=X=[
            //	]===X=X[
            //	]==X=X=[
            //	]===X=X[
            //	]==X=X=[
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( i+2, 2-i ) );
                coords.push_back( make_pair( i+2, 4-i ) );
                coords.push_back( make_pair( i+3, 5-i ) ); }
            coords.push_back( make_pair( 2, 0 ) );
            coords.push_back( make_pair( 5, 1 ) );
            break; }
        case 1: {
            //	]=X=X==[
            //	]X=X=X=[
            //	]=X=X=X[
            //	]X=X=X=[
            //	]======[
            //	]======[
            for( int i = 0; i < 4; i += 2 ) {
                coords.push_back( make_pair( i, 2 ) );
                coords.push_back( make_pair( i, 4 ) );
                coords.push_back( make_pair( i+1, 3 ) ); }
            coords.push_back( make_pair( 1, 5 ) );
            coords.push_back( make_pair( 3, 5 ) );
            break; }
        case 2: {
            // ] X=X==[
            // ]X=X=X=[
            // ]=X=X=X[
            // ]X=X=X=[
            // ]=X=X=X[
            // ]==X=X [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i, i+2 ) );
                coords.push_back( make_pair( i+1, i+1 ) );
                coords.push_back( make_pair( i+2, i ) ); }
            for( int i = 0; i< 2; i++ ) {
                coords.push_back( make_pair( i, i+4 ) );
                coords.push_back( make_pair( i+4, i ) ); }
            break; }
        case 3: {
            // ] X=X==[
            // ] =X=X=[
            // ]=X=X=X[
            // ]==X=X=[
            // ]===X=X[
            // ]====  [
            for( int i = 0; i < 5; i++ ) coords.push_back( make_pair( i+1, 5-i ) );
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( i+1, 3-i ) );
                coords.push_back( make_pair( i+3, 5-i ) ); }
            break; }
        case 4: {
            // ]X=X=X=[
            // ]=X=X=X[
            // ]==X=X=[
            // ] ==X=X[
            // ]====X=[
            // ]== ==X[
            for( int i = 0; i < 6; i++ ) coords.push_back( make_pair( i, 5-i ) );
            for( int i = 0; i < 4; i++ ) coords.push_back( make_pair( i+2, 5-i ) );
            for( int i = 0; i < 2; i++ ) coords.push_back( make_pair( i+4, 5-i ) );
            break; }
        case 5: {
            // ]==  ==[
            // ]==X=X=[
            // ] X=X= [
            // ] =X=X [
            // ]=X=X==[
            // ]==  ==[
            for( int i = 0; i < 4; i++ ) coords.push_back( make_pair( i+1, i+1 ) );
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, i+3 ) );
                coords.push_back( make_pair( i+3, i+1 ) ); }
            break; }
        case 6: {
            // ]==X=X=[
            // ]=X=X=X[
            // ] =X=X=[
            // ]=X=X=X[
            // ]==X=X=[
            // ]=== ==[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i+1, i+2 ) );
                coords.push_back( make_pair( i+2, i+1 ) ); }
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+1, i+4 ) );
                coords.push_back( make_pair( i+4, i+1 ) ); }
            break; }
        case 7: {
            //	]=X X==[
            //	]X=X=X=[
            //	]=X=X=X[
            //	]X=X=X [
            //	]= =X=X[
            //	]==X=X=[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( i, i+2 ) );
                coords.push_back( make_pair( i+2, i ) ); }
            for( int i = 0; i < 3; i++ ) coords.push_back( make_pair( i+2, i+2 ) );
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i, i+4 ) );
                coords.push_back( make_pair( i+4, i ) ); }
            break; }
        }
        break;
    case LAYER:
        switch( subtype ) {
        default: case 0: {
            //	]= ==X=[
            //	]=X==X=[
            //	]=X==X=[
            //	]=X==X=[
            //	]=X== =[
            //	]=    =[
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 1, i+1 ) );
                coords.push_back( make_pair( 4, i+2 ) ); }
            break; }
        case 1: {
            //	]    ==[
            //	]===X=X[
            //	]===X=X[
            //	]===X=X[
            //	]===X=X[
            //	]=     [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 3, i+1 ) );
                coords.push_back( make_pair( 5, i+1 ) ); }
            break; }
        case 2: {
            //	]    ==[
            //	]==X==X[
            //	]==X==X[
            //	]==X==X[
            //	]==X==X[
            //	]=     [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 2, i+1 ) );
                coords.push_back( make_pair( 5, i+1 ) ); }
            break; }
        case 3: {
            //	]======[
            //	]X==X==[
            //	]=X==X=[
            //	] X====[
            //	]==XX==[
            //	]== =X=[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( 1, i+2 ) );
                coords.push_back( make_pair( i+2, 1 ) );
                coords.push_back( make_pair( i+3, 4-i ) ); }
            coords.push_back( make_pair( 0, 4 ) );
            coords.push_back( make_pair( 4, 0 ) );
            break; }
        case 4: {
            // ]=X= ==[
            // ]==XX==[
            // ]====X [
            // ]=X==X=[
            // ]==X==X[
            // ]======[
            for( int i = 0; i < 2; i++ ) {
                coords.push_back( make_pair( i+2, 4 ) );
                coords.push_back( make_pair( 4, i+2 ) );
                coords.push_back( make_pair( i+1, 2-i ) ); }
            coords.push_back( make_pair( 1, 5 ) );
            coords.push_back( make_pair( 5, 1 ) );
            break; }
        case 5: {
            // ]    ==[
            // ]=X===X[
            // ]=X===X[
            // ]=X===X[
            // ]=X===X[
            // ]=     [
            for( int i = 0; i < 4; i++ ) {
                coords.push_back( make_pair( 1, i+1 ) );
                coords.push_back( make_pair( 5, i+1 ) ); }
            break; }
        case 6: {
            // ]   ===[
            // ] ==X==[
            // ]X===X=[
            // ]=X=== [
            // ]==X== [
            // ]===X  [
            for( int i = 0; i < 4; i++ ) coords.push_back( make_pair( i, 3-i ) );
            for( int i = 0; i < 2; i++ ) coords.push_back( make_pair( i+3, 4-i ) );
            break; }
        case 7: {
            // ]    ==[
            // ]===X= [
            // ]X===X=[
            // ]=X===X[
            // ] =X===[
            // ]==    [
            for( int i = 0; i < 3; i++ ) {
                coords.push_back( make_pair( i, 3-i ) );
                coords.push_back( make_pair( i+3, 4-i ) ); }
            break; }
        }
        break;
    }

    generateHolyPanels( coords );
    generateCrackedPanels( coords );
    generateIcePanels( coords );
    generatePoisonPanels( coords );
}

void Board::generateHolyPanels( vector<pair<int, int>>& coords ) {
    while( numHoly > 0 && !coords.empty() ) {
        int randPlace = rand() % coords.size();
        int xPos = coords[randPlace].first;
        int yPos = coords[randPlace].second;
        if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 ) map[xPos][yPos].state = 3;
        coords.erase( coords.begin() + randPlace );
        numHoly--;
    }
}
void Board::generateCrackedPanels( vector<pair<int, int>>& coords ) {
    while( numCracked > 0 && !coords.empty() ) {
        int randPlace = rand() % coords.size();
        int xPos = coords[randPlace].first;
        int yPos = coords[randPlace].second;
        if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 ) map[xPos][yPos].state = -1;
        coords.erase( coords.begin() + randPlace );
        numCracked--;
    }
}
void Board::generateIcePanels( vector<pair<int, int>>& coords ) {
    while( numIce > 0 && !coords.empty() ) {
        int randPlace = rand() % coords.size();
        int xPos = coords[randPlace].first;
        int yPos = coords[randPlace].second;
        if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 ) map[xPos][yPos].state = 1;
        coords.erase( coords.begin() + randPlace );
        numIce--;
    }
}
void Board::generatePoisonPanels( vector<pair<int, int>>& coords ) {
    while( numPoison > 0 && !coords.empty() ) {
        int randPlace = rand() % coords.size();
        int xPos = coords[randPlace].first;
        int yPos = coords[randPlace].second;
        if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 ) map[xPos][yPos].state = 2;
        coords.erase( coords.begin() + randPlace );
        numPoison--;
    }
}

void Board::upgradeAnimationAll() {
    for( int x = 0; x < 6; x++ ) {
        for( int y = 0; y < 6; y++ ) {
            map[x][y].upgradeInd = itemUpgradeTime;
        }
    }
}
