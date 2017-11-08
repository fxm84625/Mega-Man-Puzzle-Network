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
using namespace std;

class Player {
public:
	Player();
	int x, y;			// Coordinate position
	int facing;			// Facing Direction		// 0 = Up		// 1 = Left		// 2 = Down		// 3 = Right
	bool moving, turning;
	int moveDir;		// Moving Direction		// 0 = Up		// 1 = Left		// 2 = Down		// 3 = Right
	int energy;			// Player Resource - Used for attacking with swords
    int type;           // 0 = Megaman      // 1 = Protoman
	
	void reset();
};

class Tile {
public:
	Tile();
	int state;		// 0 = Solid Ground		// 1 = Cracked Floor			// 2 = Cracked Hole			// 3 = Hole
	int item;		// 0 = No Energy (Resource)			// 1 = 100 Energy
	int boxHP;		// 0 = No Rock
	int boxType;	// 0, 1 = Rock		    // 2 = Ice Rock w/ Item
	float boxAtkInd;	// Used for Indicating that a Rock got Attacked and damaged
	int prevDmg;
};

class DelayedHpLoss {		// Class used for displaying a Delayed HP Loss of a Rock
public:
	DelayedHpLoss();
	int dmg;
	int xPos, yPos;
	float delay;
};

class DelayedSound {
public:
	DelayedSound();
	string soundName;
	float delay;
};

class App {
public:
	App();
	~App();
	void Init();
	void Update(float &elapsed);
	void FixedUpdate();
	void Render();
	bool UpdateAndRender();
	void checkKeys();			// Reads keyboard inputs

	Tile map[6][6];				// The current map
	Player player;
	bool done;					// If the current game is Over or not
    bool menuDisplay;
    bool quitMenuOn, resetMenuOn, diffSelMenuOn, charSelMenuOn, trainMenuOn, menuSel;
    int charSel;
    int energyDisplayed, energyDisplayed2;        // Shown amount of energy
	int level, levelType;		// Level Types
	int lvlDiff, gameDiffSel, currentGameDiff;      // Difficulty settings
	int currentEnergyGain;		// Current Player Energy gain for this Level
	float chargeDisplayPlusAmt;	// Used for showing that the Player picked up some Energy resource
	float chargeDisplayMinusAmt;// Used for showing that the Player spent Energy
	float animationDisplayAmt;	// Used for Animation timing
	float itemAnimationAmt, bgAnimationAmt;		// Used for idle item animations		// Used for background animations
	float currentSwordAtkTime;	// Used for Sword attack display
	int animationType;			// 0 = Movement		// 1 = Attack		2 = Special Invalid movement	3,4 = Level Transition
	int selSwordAnimation;		// Select which sword attack is being Animated (0-6)    // -1: None     // -2: Item

	int musicSel;					// Selects different music tracks
    bool musicMuted;
	float musicSwitchDisplayAmt;

	void face(int dir);			// 0 = up		// 1 = left		// 2 = down		// 3 = right
	void move(int dir);			// 0 = up		// 1 = left		// 2 = down		// 3 = right
	void move2(int dir);		// Move two Squares in the Facing Direction

    // swordAtk:		Damages Rocks based on selected Sword and Direction
    // swordDisplay:	Animates a Sword Attack Sprite
	void swordAtk(int dir);			void swordDisplay(int dir);
	void longAtk(int dir);			void longDisplay(int dir);
	void wideAtk(int dir);			void wideDisplay(int dir);
	void crossAtk(int dir);			void crossDisplay(int dir);
	void spinAtk(int dir);			void spinDisplay(int dir);
	void stepAtk(int dir);			void stepDisplay(int dir);
	void lifeAtk(int dir);			void lifeDisplay(int dir);
    void heroAtk(int dir);          void heroDisplay(int dir);
    void protoAtk(int dir);         void protoDisplay(int dir);
    void vDivideAtk(int dir);       void vDivideDisplay(int dir);
    void upDivideAtk(int dir);      void upDivideDisplay(int dir);
    void downDivideAtk(int dir);    void downDivideDisplay(int dir);
    void xDivideAtk(int dir);       void xDivideDisplay(int dir);
    void zDivideAtk(int dir);       void zDivideDisplay(int dir);

	void hitBox(int xPos, int yPos, int dmg = 1);						// Damage a Rock at a specific position
	void hitBoxDelay(int xPos, int yPos, float delay, int dmg = 1);		// Damage a Rock with a delayed start-up

	void clearFloor();						// Resets all Tiles
	bool isTileValid(int xPos, int yPos);	// Checks if a specific tile allows the Player to walk on it

	void loadLevel(int num);
	void generateLevel(int type, int num = -1);		// Generates a Level using the 3 functions below		// "num != -1" used for Debugging
	void generateItems(int amt, int type);			// Randomly places Energy on the map
	void generateBoxes(int amt, int type);			// Randomly places Rocks on the map
	void generateFloor(int amt, int type);			// Randomly damages the floor (Cracked floors or Broken floors)
	void reset();		// Restarts from level 0
	void next();		// Goes to next level if completed
	void updateGame(float elapsed);

    // Music Functions
    void changeMusic( int track = -1 );
    void toggleMusic();

	// Display Functions
	void drawBg();
    void drawDimScreen();
	void drawMenu();
    void drawQuitMenu();
    void drawResetMenu();
    void drawDiffSelMenu();
    void drawCharSelMenu();
    void drawTrainMenu();
    void drawTabMenuCtrl();
	void drawPlayer();
    void drawSwordAtks();
	void drawFloor();
	void drawBoxes(int row);
	void drawItems(int row);
	void drawTextUI();
    void displayMusic();        // Displays music track Number and Name

	vector<DelayedHpLoss> delayedHpList;
	vector<DelayedSound> delayedSoundList;

	// Test/Debug functions
	void test();
	void test2();
	
	SDL_Event event;
	float timeLeftOver;
	float lastFrameTicks;
	float fixedElapsed;

	GLuint textSheet1A, textSheet1B, textSheet1C, textSheet2A, textSheet2B, textSheet2C,
	       megamanMoveSheet, megamanAtkSheet,
           protoMoveSheet,   protoAtkSheet,
           colonelMoveSheet, colonelAtkSheet,
           lvBarPic, healthBoxPic,
           rockSheet, rockSheetItem, rockDeathSheet,
           floorSheet, floorMoveSheet, floorBottomPic1, floorBottomPic2,
           energySheet,
           bgA, bgB, bgC,
           dimScreenPic,
           infoBoxPic1, infoBoxPic2, infoBoxPic3,
           menuPic1, menuPic2, menuPic3,
           musicDisplayPic, tabMenuCtrlSheet,
           resetPicY, resetPicN,
           diffPic1, diffPic2, diffPic3, diffPic4, diffPic5,
           charSelPic0, charSelPic1, charSelPic2,
           quitPicY, quitPicN,
           trainPicY, trainPicN;

	GLuint swordAtkSheet1, swordAtkSheet3,		// Sprites for Sword Attack Animations
		   longAtkSheet1,  longAtkSheet3,
		   wideAtkSheet1,  wideAtkSheet3,
		   crossAtkSheet1, crossAtkSheet3,
		   spinAtkSheet1,  spinAtkSheet3,
		   stepAtkSheet1,  stepAtkSheet3,
		   lifeAtkSheet1,  lifeAtkSheet3,
           heroAtkSheet1,  heroAtkSheet3,
           protoAtkSheet1, protoAtkSheet3,
           screenDivVSheet1,    screenDivVSheet3,
           screenDivUpSheet1,   screenDivUpSheet3,
           screenDivDownSheet1, screenDivDownSheet3,
           screenDivXSheet1,    screenDivXSheet3,
           screenDivZSheet;

	Mix_Chunk *swordSound,      *lifeSwordSound,  *screenDivSound,
	          *itemSound,       *rockBreakSound,  *panelBreakSound,
	          *menuOpenSound,   *menuCloseSound,
              *quitCancelSound, *quitChooseSound, *quitOpenSound,
              *track01, *track02, *track03, *track04, *track05, *track06, *track07, *track08, *track09,
              *track10, *track11, *track12, *track13, *track14, *track15, *track16, *track17, *track18;

	SDL_Window *displayWindow;
};
