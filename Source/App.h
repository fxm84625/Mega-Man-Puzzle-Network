/*--------------------------------------------------------------------------------
	Paper Swords

		Inspired by Capcom's Mega Man Battle Network
----------------------------------------------------------------------------------
	Controls
		W A S D         Movement
		Arrow keys      Moving also changes facing direction
						You can only face Left and Right

		1-7             Attack
		Keypad 1-7

		Shift           Turn around without moving

		R               Reset to the First Level

		Tab or F		Display info Menu

		X				Training Room

        C               Change Music Track

        V               Toggle Music

        Esc             Quit
----------------------------------------------------------------------------------
	Goal:
		Get as far as you can. Your score is the level count.
		Collect resources along the way.
		Using Weapons costs resources.
----------------------------------------------------------------------------------*/

#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <random>
#include <time.h>
#include <utility>
#include "Player.h"
#include "Panel.h"
#include "Board.h"
#include "DelayedEffects.h"
using namespace std;

class App {
public:
	App();
	~App();
    bool UpdateAndRender();
private:
	void Init();
	void Update(float &elapsed);
	void FixedUpdate();
	void Render();
	void checkKeys();			// Reads keyboard inputs

    void updateGame( float elapsed );
    void updatePlayer( float elapsed );

    Board board;    			// The current map
	Player *player1;            // You, the player
    vector<Player*> npcList;        // Potential NPC's
    vector<Player*> nextNpcList;    // NPC's that entered the next level - to be loaded in the next level
	bool done;					    // If the current game is Over or not

    int level;		                // Level Number
    int lvlDiff, currentGameDiff;   // Difficulty settings for the current Level, and for the current Run
    int lvlsWithoutBoss;
    bool reloadedTutorial;
	int currentEnergyGain;		    // Current Player Energy gain for this Level
	float chargeDisplayPlusAmt;	    // Used for showing that the Player picked up some Energy resource
	float chargeDisplayMinusAmt;    // Used for showing that the Player spent Energy
	float itemAnimationAmt, bgAnimationAmt, poisonAnimationAmt, holyAnimationAmt;		// Used for item, panel, and background animations
    bool npcAbleToAct;              // Whether or not the NPC's can act

    // Menu
    int menuNum, menuSel;               // Menu Number      // Menu Selector
    int charSel, gameDiffSel;           // Game Character and Difficulty Selector
    int musicSel, gameVol, musicVol;	// Sound Options - Music Track Selector, and Volume Options
    bool showInfobox;
	float musicSwitchDisplayAmt;

    // Player functions
    void aiAction( Player* const player );                  // Generate a move for the AI
    bool attack( Player* const player, int atkNum );
	void face( Player* const player, int dir);		            // -1 = left	// 1 = right
    bool move( Player* const player, int dir, int dist = 1 );	// DIR: direction   // 0 = up		// 1 = left		// 2 = down		// 3 = right
    
    // Display functions: Animates Sword attack trail Sprites
    void swordDisplay( Player* const player );
    void longDisplay( Player* const player );
    void wideDisplay( Player* const player );
    void crossDisplay( Player* const player );
    void spinDisplay( Player* const player );
    void stepDisplay( Player* const player );
    void lifeDisplay( Player* const player );

    void heroDisplay( Player* const player );
    void protoDisplay( Player* const player );

    void vDivideDisplay( Player* const player );
    void upDivideDisplay( Player* const player );
    void downDivideDisplay( Player* const player );
    void xDivideDisplay( Player* const player );
    void zDivideDisplay( Player* const player );

    void tomaDisplayA1( Player* const player );
    void tomaDisplayA2( Player* const player );
    void tomaDisplayB1( Player* const player );
    void tomaDisplayB2( Player* const player );
    void eTomaDisplay( int xPos, int yPos, float animationTime );

    void longSlashDisplay( Player* const player );
    void wideSlashDisplay( Player* const player );
    void stepCrossDisplay( Player* const player );

	void hitPanel(int xPos, int yPos, int dmg = 1);					// Deal Damage to a Panel at a specific position

	bool isPanelValid(int xPos, int yPos, bool npc = false);	    // Checks if a specific panel allows the Player to walk on it

    void loadBossLevel(int type);
    void loadLevel(int num = -1);		            // Generates a Level based on Level Templates		// "num != -1" for defined difficulty
    void loadTutorialLevel( bool reload = false );  // Loads a training level   // Special cases where reload == true: So that Players cannot fail a Tutorial
    void loadPrevNpc(int diff, int gain);           // Loads NPC's that have entered the next level
    void spawnRandomItem(int xPos, int yPos);       // Spawns an item at a random location around (xPos, yPos)
    void upgradeItems();                            // Randomly Upgrades an Amount of Energy: Green into Blue, and Red into Green
	void reset();		            // Restarts from level 0
    void tutorial(int type = 1);    // Training Room
	void next();		            // Goes to next level if completed

    // Music Functions
    void playMusic( int track );

	// Display Functions
	void drawBg();
    void drawDimScreen();

    void drawMenu();
    void drawMainMenu();
    void drawNewRunMenu();
    void drawCharMoveset();
    void drawInfobox();
    void drawTutorialMenu();
    void drawControlsMenu();
    void drawOptionsMenu();

    void drawOverhead();        // Classic Overhead "Chip Select Ready" display
    void drawTextUI();
    void displayMusic();        // Displays music track Number and Name
    void drawPrevEnergy();      // Displays Previous Energy changes at their locations

	void drawPlayer(Player* const player);
    void drawSwordAtks(Player* const player);
    void drawStepShadow(Player* const player, int amt);
    void drawHp(Player* const player);
	void drawFloor();
	void drawRocks(int row);
	void drawItems(int row);
    void drawItemUpgrading(int row);

	vector<DelayedHpLoss> delayedHpList;
	vector<DelayedSound> delayedSoundList;
    vector<DelayedETomaDisplay> delayedETomaDisplayList;
    vector<DelayedEnergyDisplay> delayedPrevEnergyList;
	
	SDL_Event event;
	float timeLeftOver;
	float lastFrameTicks;
	float fixedElapsed;

    // Character Sprite Sheets
	GLuint
        megamanMoveSheet,  megamanAtkSheet,                  megamanHurtSheet,   megamanStepSheet,
        protoMoveSheet,    protoAtkSheet,                    protoHurtSheet,     protoStepSheet,
        colonelMoveSheet,  colonelAtkSheet,                  colonelHurtSheet,   colonelStepSheet,
        tmanMoveSheet,     tmanAtkSheet1,    tmanAtkSheet2,
        slashmanMoveSheet, slashmanAtkSheet,                                     slashmanStepSheet,
        
        darkMegamanMoveSheet,  darkMegamanAtkSheet,                     darkMegamanHurtSheet,    darkMegamanStepSheet,
        darkProtoMoveSheet,    darkProtoAtkSheet,                       darkProtoHurtSheet,      darkProtoStepSheet,
        darkColonelMoveSheet,  darkColonelAtkSheet,                     darkColonelHurtSheet,    darkColonelStepSheet,
        darkTmanMoveSheet,     darkTmanAtkSheet1,    darkTmanAtkSheet2,
        darkSlashmanMoveSheet, darkSlashmanAtkSheet,                                             darkSlashmanStepSheet;
    
    // Menu, UI, and Game Elements Sprites
    GLuint
        rockSheet, rockSheetItem, rockSheetItem2, rockSheetTrappedItem, rockDeathSheet, darkDeathSheet,
        energySheet, energySheet2, trappedEnergySheet, recoverSheet, energyGetSheet,
        floorSheet, floorMoveSheet, floorPoisonSheet, floorHolySheet,
        floorBottomPic1, floorBottomPic2, floorAtkIndPic,

        textSheetWhite, textSheetGreen, textSheetRed,
        textSheetWhite2, textSheetGreen2, textSheetRed2,
        textSheetRedTrap, textSheetPoison, textSheetPurpleNPC,
        lvBarPic, healthBoxPic, musicDisplayBox,

        bgA, bgB, bgC, bgD, bgMain,

        dimScreenPic, generalInfoboxPic, overheadSheet,
        // Main Menu
        mainMenuPic, mainSelPic0, mainSelPic1, mainSelPic2, mainSelPic3, mainSelPic4,
        // New Run Menu
        newRunMenuPic, newRunSelPic0, newRunSelPic1, newRunSelPic2,
        charSelPic0, charSelPic1, charSelPic2, charSelPic3, charSelPic4,
        chipPic0, chipPic1, chipPic2, chipPic3, chipPic4,
        diffSelPic0, diffSelPic1, diffSelPic2, diffSelPic3, diffSelPic4,
        movesetPic0, movesetPic1, movesetPic2, movesetPic3, movesetPic4,
        // Tutorial Menu
        tutorialMenuPic, tSelPic0, tSelPic1, tSelPic2, tSelPic3, tSelPic4, tSelPic5, tSelPic6, tSelPic7,
        tInfoboxPic, tInfoPic0, tInfoPic1, tInfoPic2, tInfoPic3, tInfoPic4, tInfoPic5, tInfoPic6, tInfoPic7,
        // Controls Menu
        controlMenuPic,
        // Options Menu
        optionMenuPic, oSelPic0, oSelPic1, oSelPic2, oSelPic3,
        infoSelPic0, infoSelPic1,
        gameVolSelPic0, gameVolSelPic1, gameVolSelPic2, gameVolSelPic3, gameVolSelPic4, gameVolSelPic5,
                        gameVolSelPic6, gameVolSelPic7, gameVolSelPic8, gameVolSelPic9, gameVolSelPic10,
        musicVolSelPic0, musicVolSelPic1, musicVolSelPic2, musicVolSelPic3, musicVolSelPic4, musicVolSelPic5,
                         musicVolSelPic6, musicVolSelPic7, musicVolSelPic8, musicVolSelPic9, musicVolSelPic10,
        musicSelPic1,  musicSelPic2,  musicSelPic3,  musicSelPic4,  musicSelPic5,  musicSelPic6,
        musicSelPic7,  musicSelPic8,  musicSelPic9,  musicSelPic10, musicSelPic11, musicSelPic12,
        musicSelPic13, musicSelPic14, musicSelPic15, musicSelPic16, musicSelPic17, musicSelPic18;

    // Sword Attack Animation Sprite Sheets
	GLuint
        swordAtkSheet1, swordAtkSheet3,
		longAtkSheet1,  longAtkSheet3,
		wideAtkSheet1,  wideAtkSheet3,
		crossAtkSheet1, crossAtkSheet3,
		spinAtkSheet1,  spinAtkSheet3,
		lifeAtkSheet1,  lifeAtkSheet3,
        
        heroAtkSheet1,  heroAtkSheet3,
        protoAtkSheet1, protoAtkSheet3,
        
        screenDivVSheet1,    screenDivVSheet3,
        screenDivUpSheet1,   screenDivUpSheet3,
        screenDivDownSheet1, screenDivDownSheet3,
        screenDivXSheet1,    screenDivXSheet3,
        screenDivZSheet,
        
        tomahawkAtkSheetA1, tomahawkAtkSheetA3,
        tomahawkAtkSheetB1, tomahawkAtkSheetB3,
        eagleTomaSheet,
        
        longSlashSheet1,    longSlashSheet3,
        wideSlashSheet1,    wideSlashSheet3;

	Mix_Chunk
        *swordSound,      *lifeSwordSound,  *screenDivSound,  *tomahawkSound, *eTomaSound, *spinSlashSound,
        
	    *itemSound,       *itemSound2,      *hurtSound,    *deletedSound,     *recoverSound,
        *bossAppearSound, *bossDeathSound,
        *rockBreakSound,  *panelBreakSound,
        
	    *infoboxOpenSound,   *infoboxCloseSound,
        *menuCancelSound, *menuChooseSound, *menuOpenSound,
        
        *track01, *track02, *track03, *track04, *track05, *track06, *track07, *track08, *track09,
        *track10, *track11, *track12, *track13, *track14, *track15, *track16, *track17, *track18;

	SDL_Window *displayWindow;
};
