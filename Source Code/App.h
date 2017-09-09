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
//#include <SDL_mixer.h>
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
	int energy2;		// Amt of Energy resource at the beginning a floor
	
	void reset();
};

class Tile {
public:
	Tile();
	int state;		// 0 = Solid Ground		// 1 = Cracked Floor			// 2 = Cracked Hole			// 3 = Hole
	int item;		// 0 = No Energy (Resource)			// 1 = 100 Energy
	int boxHP;		// 0 = No box
	int boxType;	// 0 = Wooden			// 1 = Glass Box w/ Item		// 2 = Stone Box
	float boxAtkInd;	// Used for Indicating that a Box got Attacked and damaged
};

class DelayedHpLoss {		// Class used for displaying a Delayed HP Loss of a Box
public:
	DelayedHpLoss();
	int xPos, yPos;
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
	Tile map2[6][6];			// A Saved copy of the map at the beginning of the current level
	Player player;
	bool done;					// If the current game is Finished or not
	int mode;					// -1 = menu		// 0 = game
	int menuSel;
	int level, levelType;		// Negative levels are tutorials		// Level Types 0 to 19
	int refreshCount;
	float chargeDisplayPlusAmt;	// Used for showing that the Player picked up some Energy resource
	float chargeDisplayMinusAmt; // Used for showing that the Player spent Energy
	float animationDisplayAmt;	// Used for Animation timing
	int animationType;			// Types of Animations: Sword Attacks, Movement
	int selSwordAnimation;		// Select which sword attack is being Animated (0-6)

	void face(int dir);			// 0 = up		// 1 = left		// 2 = down		// 3 = right
	void move(int dir);			// 0 = up		// 1 = left		// 2 = down		// 3 = right
	void move2(int dir);		// Move two Squares in the Facing Direction
	void swordAtk(int dir);			void swordDisplay(int dir);
	void longAtk(int dir);			void longDisplay(int dir);
	void wideAtk(int dir);			void wideDisplay(int dir);
	void crossAtk(int dir);			void crossDisplay(int dir);
	void spinAtk(int dir);			void spinDisplay(int dir);
	void stepAtk(int dir);			void stepDisplay(int dir);
	void lifeAtk(int dir);			void lifeDisplay(int dir);
		// swordAtk:		Damages Boxes based on selected Sword and Direction
		// swordDisplay:	Animates a Sword Attack Sprite
	void hitBox(int xPos, int yPos);		// Damage a Box at a specific position
	void hitBoxDelay(int xPos, int yPos, float delay);		// Damage a Box with a delayed start-up

	void clearFloor();						// Resets all Tiles: No cracked panels, holes, boxes, or items
	bool isTileValid(int xPos, int yPos);	// Checks if a specific tile allows the Player to walk on it

	void loadLevel(int num);
	void generateLevel(int type, int num = -1);		// Generates a Level using the 3 functions below		// "num != -1" used for Debugging
	void generateItems(int amt, int type);			// Randomly places swords on the map
	void generateBoxes(int amt, int type);			// Randomly places boxes on the map
	void generateFloor(int amt, int type);			// Randomly damages the floor (Cracked floors or Broken floors)
	void refresh();		// Restarts current level
	void reset();		// Restarts from level 0
	void next();		// Goes to next level if completed
	void updateGame(float elapsed);

	// Display Functions
	void drawMenu();
	void drawPlayer();
	void drawSwordUI();
	void drawFloor();
	void drawBoxes();
	void drawBoxesHP();
	void drawItems();
	void drawTextUI();
	vector<DelayedHpLoss> delayedHpList;

	// Test/Debug functions
	void test();
	//void test2();
	
	SDL_Event event;
	float timeLeftOver;
	float lastFrameTicks;
	float fixedElapsed;
	GLuint textSheet1, textSheet2;
	GLuint playerPic;
	GLuint directionSheet;						// Indicates the Player's Facing direction
	GLuint swordIconSheet;						// Used for displaying Sword effects
	GLuint squarePic, trianglePic;				// Used for describing Sword effects
	GLuint boxSheet0, boxSheet1, boxSheet2;		// Stone, Dark Stone, Glass
	GLuint floorSheet;
	GLuint batteryPic, chargePic1, chargePic2, chargePic3;
		// battery: Energy Resource icon		// charge1: Energy pick-up		// charge2: Showing Energy gain		// charge3: Showing Energy loss
	GLuint goalPic;

	GLuint swordAtkSheet0, swordAtkSheet1, swordAtkSheet2, swordAtkSheet3,		// Sprites for Sword Attack Animations
		   longAtkSheet0, longAtkSheet1, longAtkSheet2, longAtkSheet3,
		   wideAtkSheet0, wideAtkSheet1, wideAtkSheet2, wideAtkSheet3,
		   crossAtkSheet0, crossAtkSheet1, crossAtkSheet2, crossAtkSheet3,
		   spinAtkSheet0, spinAtkSheet1, spinAtkSheet2, spinAtkSheet3,
		   stepAtkSheet0, stepAtkSheet1, stepAtkSheet2, stepAtkSheet3,
		   lifeAtkSheet0, lifeAtkSheet1, lifeAtkSheet2, lifeAtkSheet3;

	SDL_Window* displayWindow;
};
