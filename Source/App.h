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
using namespace std;

class Player {
public:
	Player();
	int x, y;			// Coordinate position
    int x2, y2;         // Previous Coordinate position
    int xOffset, yOffset;   // Drawing Character parameters
    int hp;             // HP for NPCs        // Players do not have HP
    int timesHit;       // Records how many times Players hit an NPC
	int facing;			// Facing Direction		// 1 = Left		// 3 = Right
	bool moving, turning;
	int moveDir;		// Moving Direction		// 0 = Up		// 1 = Left		// 2 = Down		// 3 = Right
	int energy;			// Player Resource - Used for attacking with swords
    int type;           // 0 = Megaman      // 1 = Protoman     // 2 = Tomahawkman      // 3 = Colonel      // 4 = Slashman
    bool npc;           // Whether or not the character is the player or an NPC
    
    float animationDisplayAmt;	// Used for Animation timing
    float deathDisplayAmt;      // Used for animating defeated Characters
    float currentSwordAtkTime;	// Used for Sword attack display
    float hurtDisplayAmt;       // Used for showing that Characters got hit
    float npcActionTimer;       // NPCs wait a moment after Players act

	int animationType;			// -1 = Trapped    // 0 = Movement    // 1 = Attack    // 2 = Special Invalid movement    // 3,4 = Level Transition
	int selSwordAnimation;		// Select which sword attack is being Animated
    
    int energyDisplayed, energyDisplayed2;        // Shown amount of energy
	
	void reset();
};

class Tile {
public:
	Tile();
	int state;		// 0 = Solid Ground		// 1 = Cracked Floor			// 2 = Cracked Hole			// 3 = Hole
	int item;		// 0 = No Energy (Resource)			// 1 = 100 Energy
	int boxHP;		// 0 = No Rock
	int boxType;	// 0, 1 = Rock		    // 2 = Ice Rock w/ Item
    bool bigBoxDeath;   // Used for special Rock death animation
    bool isPurple;        // Used to determine which color sprite sheet to use
	float boxAtkInd;	// Used for Indicating that a Rock got Attacked and damaged
    float upgradeInd;   // Used for Indicating that an Item got Upgraded
	int prevDmg;
};

class DelayedHpLoss {		// Class used for displaying a Delayed HP Loss of a Rock or Character
public:
	DelayedHpLoss();
    DelayedHpLoss( int dmgAmt, int x, int y, float delayTime, bool isNpc = false );
	int dmg;
	int xPos, yPos;
	float delay;
    int npc;        // Whether or not the damage was from an NPC
};

class DelayedSound {
public:
	DelayedSound();
    DelayedSound( string name, float time, bool isNpc = false );
    bool npc;
	string soundName;
	float delay;
};

class DelayedETomaDisplay {
public:
    DelayedETomaDisplay();
    DelayedETomaDisplay( int x, int y, float time );
    int xPos, yPos;
    float delay;
    float animationTimer;
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
	Player player1;             // You, the player
    vector<Player> npcList;     // Potential NPC's
	bool done;					// If the current game is Over or not
    bool menuDisplay, quitMenuOn, resetMenuOn, diffSelMenuOn, trainMenuOn, menuSel;
    int charSel;
	int level, levelType;		// Level Types
	int lvlDiff, gameDiffSel, currentGameDiff;      // Difficulty settings
    int lvlsWithoutBoss;
	int currentEnergyGain;		// Current Player Energy gain for this Level
	float chargeDisplayPlusAmt;	// Used for showing that the Player picked up some Energy resource
	float chargeDisplayMinusAmt;// Used for showing that the Player spent Energy
	float itemAnimationAmt, bgAnimationAmt;		// Used for idle item animations		// Used for background animations
    bool npcAbleToAct;          // Whether or not the NPCs can do an Action

	int musicSel;					// Selects different music tracks
    bool musicMuted;
	float musicSwitchDisplayAmt;

    // Player functions
    void aiAction( Player &player );

	void face( Player &player, int dir);		// 0 = up		// 1 = left		// 2 = down		// 3 = right
    bool move( Player &player, int dir);		// 0 = up		// 1 = left		// 2 = down		// 3 = right
	void move2(Player &player, int dir);		// Move two Squares in the Facing Direction

    // swordAtk:		Damages Rocks based on selected Sword and Direction
    // swordDisplay:	Animates a Sword Attack Sprite
	bool swordAtk(Player &player);		void swordDisplay(const Player &player);
    bool longAtk(Player &player);		void longDisplay( const Player &player);
    bool wideAtk(Player &player);		void wideDisplay( const Player &player);
    bool crossAtk(Player &player);		void crossDisplay( const Player &player);
    bool spinAtk(Player &player);		void spinDisplay( const Player &player);
    bool stepAtk(Player &player);		void stepDisplay( const Player &player);
    bool lifeAtk(Player &player);		void lifeDisplay( const Player &player);

    bool heroAtk(Player &player);        void heroDisplay( const Player &player);
    bool protoAtk(Player &player);       void protoDisplay( const Player &player);

    bool vDivideAtk(Player &player);     void vDivideDisplay( const Player &player);
    bool upDivideAtk(Player &player);    void upDivideDisplay( const Player &player);
    bool downDivideAtk(Player &player);  void downDivideDisplay( const Player &player);
    bool xDivideAtk(Player &player);     void xDivideDisplay( const Player &player);
    bool zDivideAtk(Player &player);     void zDivideDisplay( const Player &player);

    bool tomaAtkA1(Player &player);      void tomaDisplayA1( const Player &player);
    bool tomaAtkA2(Player &player);      void tomaDisplayA2( const Player &player);
    bool tomaAtkB1(Player &player);      void tomaDisplayB1( const Player &player);
    bool tomaAtkB2(Player &player);      void tomaDisplayB2( const Player &player);
    bool eTomaAtk(Player &player);       void eTomaDisplay(int xPos, int yPos, float animationTime);

                                         void longSlashDisplay(const Player &player);
                                         void wideSlashDisplay(const Player &player);
    bool stepCrossAtk(Player &player);   void stepCrossDisplay(const Player &player);
    bool spinSlashAtk(Player &player);

	void hitTile(int xPos, int yPos, int dmg = 1);						// Damage a Rock at a specific position

	void clearFloor();						// Resets all Tiles
	bool isTileValid(int xPos, int yPos, bool npc = false);	    // Checks if a specific tile allows the Player to walk on it

	void loadLevel(int num);
    void generateBossLevel(int type);
	void generateLevel(int type, int num = -1);		// Generates a Level using the Generate Functions below		// "num != -1" for defined difficulty
	void generateItems(int amt, int type, int trapAmt = 0);			// Randomly places Energy and Trapped Energy on the map
	void generateBoxes(int amt, int type);			// Randomly places Rocks on the map
	void generateFloor(int amt, int type);			// Randomly damages the floor (Cracked floors or Broken floors)
    void spawnRandomItem(int xPos, int yPos);       // Spawns an item at a random location around (xPos, yPos)
    void upgradeItems();                     // Randomly Upgrades an Amount of Energy: Green into Blue, and Red into Green
	void reset();		// Restarts from level 0
    void test();        // Training Room
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
    void drawTrainMenu();
    void drawTabMenuCtrl();
    void drawTextUI();
    void displayMusic();        // Displays music track Number and Name

	void drawPlayer(Player &player);
    void drawSwordAtks(const Player &player);
    void drawNpcHp();
	void drawFloor();
	void drawBoxes(int row);
	void drawItems(int row);
    void drawItemUpgrading(int row);

	vector<DelayedHpLoss> delayedHpList;
	vector<DelayedSound> delayedSoundList;
    vector<DelayedETomaDisplay> delayedETomaDisplayList;
	
	SDL_Event event;
	float timeLeftOver;
	float lastFrameTicks;
	float fixedElapsed;

	GLuint megamanMoveSheet,  megamanAtkSheet,                  megamanHurtSheet,
           protoMoveSheet,    protoAtkSheet,                    protoHurtSheet,
           colonelMoveSheet,  colonelAtkSheet,                  colonelHurtSheet,
           tmanMoveSheet,     tmanAtkSheet1,    tmanAtkSheet2,
           slashmanMoveSheet, slashmanAtkSheet,
           
           darkMegamanMoveSheet,  darkMegamanAtkSheet,                     darkMegamanHurtSheet,
           darkProtoMoveSheet,    darkProtoAtkSheet,                       darkProtoHurtSheet,
           darkColonelMoveSheet,  darkColonelAtkSheet,                     darkColonelHurtSheet,
           darkTmanMoveSheet,     darkTmanAtkSheet1,    darkTmanAtkSheet2,
           darkSlashmanMoveSheet, darkSlashmanAtkSheet,
           
           lvBarPic, healthBoxPic,
           rockSheet, rockSheetItem, rockSheetItem2, rockSheetTrappedItem, rockDeathSheet, darkDeathSheet,
           floorSheet, floorMoveSheet, floorBottomPic1, floorBottomPic2,
           energySheet, energySheet2, trappedEnergySheet, recoverSheet,
           
           textSheet1A, textSheet1B, textSheet1C, textSheet2A, textSheet2B, textSheet2C,
           bgA, bgB, bgC, bgD,
           dimScreenPic,
           menuPic0, menuPic1, menuPic2, menuPic3, menuPic4,
           musicDisplayPic, tabMenuCtrlSheet,
           diffPic1, diffPic2, diffPic3, diffPic4, diffPic5,
           resetPic0, resetPic1, resetPic2, resetPic3, resetPic4,
           quitPicY, quitPicN,
           trainPic0, trainPic1, trainPic2, trainPic3, trainPic4;

    // Sprites for Sword Attack Animations
	GLuint swordAtkSheet1, swordAtkSheet3,
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

	Mix_Chunk *swordSound,      *lifeSwordSound,  *screenDivSound,  *tomahawkSound, *eTomaSound, *spinSlashSound,
              
	          *itemSound,       *itemSound2,      *hurtSound,    *deletedSound,     *recoverSound,
              *bossAppearSound, *bossDeathSound,
              *rockBreakSound,  *panelBreakSound,
              
	          *menuOpenSound,   *menuCloseSound,
              *quitCancelSound, *quitChooseSound, *quitOpenSound,
              
              *track01, *track02, *track03, *track04, *track05, *track06, *track07, *track08, *track09,
              *track10, *track11, *track12, *track13, *track14, *track15, *track16, *track17, *track18;

	SDL_Window *displayWindow;
};
