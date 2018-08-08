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

#include "App.h"
#include <sys/stat.h>
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
using namespace std;

const float fixed_timestep = 0.0166666f;
const int max_steps = 6;
const float pi = 3.14159265359f;

const int startingEnergy = 300;
const int energyGainAmt  = 100;			    // How much Energy the player gains when moving to a Pick-Up
const int energyGainAmt2 = 150;
const int energyLossAmt  =  50;
const int poisonLossAmt  = 25;
const int itemWorth      = energyGainAmt / 50;	// Item "Worth" is used to generate Levels
const int npcDmgAmt      = 100;             // How much Damage the NPC's do

const float iconDisplayTime = 0.45;		// Energy Gain Icon Display time
const float rockDmgTime = 0.4;			// Rock HP damage indicator time
const float itemUpgradeTime = 0.32;
const float charDeathTime = 0.2;
const float npcWaitTime = 0.14;

const float moveAnimationTime = 0.175;	// Player movement time
const float iceSlipAnimationTime = 0.1; // Time for the Player to slip on one Ice Panel
const float hurtAnimationTime = 0.5;    // Time to display a Player's hurt indicator
const float poisonHurtTime = 0.25;
const float pauseAnimationTime = 0.25;  // Short delay of inaction after being hit
const float menuExitTime = 0.2;         // Slight Delay after coming out of a Menu
const float menuChooseTime = 0.1;       // Slight Delay when navigating the Menu

const float preAtkTime          = 0.10;
const float preAtkTimeToma      = 0.20;
const float preAtkTimeEagleToma = 0.27;
const float preAtkTimeGutsPunch = 0.12;

const float swordAtkTime = 0.42;
const float stepAtkTime  = 0.38;
const float lifeAtkTime  = 0.52;
const float eTomaAtkTime = 0.55;

const float orthoX1 = -1.0;
const float orthoX2 = 7.0;
const float orthoY1 = 0.0;
const float orthoY2 = 5.4;

const float overallScale = 1.25;
const float scaleX = 0.8 * overallScale;
const float scaleY = 0.48 * overallScale;
const float playerScale = 1.2;
const float itemScale = 1.1;
const float uiScaleX = 0.25;
const float uiScaleY = 1 / 2.7;

const float mainVolScale = 10.0;    // Main sounds Volume Scaling - Attacking, Being Defeated, NPC Appearing
const float miscVolScale = 6.0;     // Misc. sounds Volume Scaling
const float musicVolScale = 4.0;    // Music Volume Scaling

enum playerTypes { MEGAMAN, PROTOMAN, TOMAHAWKMAN, COLONEL, SLASHMAN, GUTSMAN, NUM_CHAR_TYPES };
enum menuTypes { MAIN_MENU, NEW_RUN_MENU, TUTORIAL_MENU, CONTROLS_MENU, OPTIONS_MENU, EXIT };

bool checkFileExists( const string image_path ) {
    struct stat buffer;
    return ( stat( image_path.c_str(), &buffer ) == 0 );
}
GLuint loadTexture(const char *image_path) {
    if( !checkFileExists( image_path ) ) return GLuint();
	SDL_Surface *surface = IMG_Load(image_path);
    GLuint textureID;
    glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	SDL_FreeSurface(surface);
	return textureID;
}
void drawTexture(GLfloat* drawingPlace, GLfloat* texturePlace, GLuint& texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexPointer(2, GL_FLOAT, 0, drawingPlace);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texturePlace);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, 4);
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
void drawSpriteSheetSprite(GLfloat* place, GLuint& spriteTexture, int index, int spriteCountX, int spriteCountY) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat quadUVs[] = { u, v, u, v + spriteHeight, u + spriteWidth, v + spriteHeight, u + spriteWidth, v };
	drawTexture(place, quadUVs, spriteTexture);
}
void drawText(GLuint fontTexture, string text, float sizeX, float sizeY, float spacing, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	vector<float> colorData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		colorData.insert(colorData.end(), { r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a });
		vertexData.insert(vertexData.end(), { ((sizeX + spacing) * i) + (-0.5f * sizeX), 0.5f * sizeY, ((sizeX + spacing) * i) + (-0.5f * sizeX), -0.5f * sizeY,
		                                      ((sizeX + spacing) * i) + (0.5f * sizeX), -0.5f * sizeY, ((sizeX + spacing) * i) + (0.5f * sizeX),   0.5f * sizeY });
		texCoordData.insert(texCoordData.end(), { texture_x,                texture_y,                texture_x,                texture_y + texture_size,
		                                          texture_x + texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
	}
	glColorPointer(4, GL_FLOAT, 0, colorData.data());
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertexData.data());
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoordData.data());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDrawArrays(GL_QUADS, 0, text.size() * 4);
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
float lerp(float& v0, float& v1, float& t) {
    return ( 1.0 - t ) * v0 + t * v1;
}
float easeIn(float& from, float& to, float& time) {
	float tVal = time*time*time*time*time;
	return (1.0f - tVal)*from + tVal*to;
}
float easeOut(float& from, float& to, float& time) {
	float oneMinusT = 1.0f - time;
	float tVal = 1.0f - (oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT);
	return (1.0f - tVal)*from + tVal*to;
}
float easeInOut(float& from, float& to, float& time) {
	float tVal;
	if (time > 0.5) {
		float oneMinusT = 1.0f - ((0.5f - time)*-2.0f);
		tVal = 1.0f - ((oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT) * 0.5f);
	}
	else {
		time *= 2.0;
		tVal = (time*time*time*time*time) / 2.0;
	}
	return (1.0f - tVal)*from + tVal*to;
}
float easeOutElastic(float& from, float& to, float& time) {
	float p = 0.3f;
	float s = p / 4.0f;
	float diff = (to - from);
	return from + diff + (diff*pow(2.0f, -10.0f*time) * sin((time - s)*(2 * pi) / p));
}

long getRand(long num) {
    // Returns pseudo-random number between 0 and (num-1)
	srand(time(0));				// Gets a new set of random numbers based on time
	return rand() % num;		// Returns a Random number from the generated string
}

App::App() : done(false), timeLeftOver(0.0), fixedElapsed(0.0), lastFrameTicks(0.0),
			 menuNum(MAIN_MENU), menuSel(0), charSel(0), gameDiffSel(2), musicSel(0), gameVol(5), musicVol(0), showInfobox(true),
             level(0), currentEnergyGain(0), currentGameDiff(2), npcAbleToAct(false), lvlsWithoutBoss(0), reloadedTutorial(false),
			 chargeDisplayPlusAmt(0.0), chargeDisplayMinusAmt(0.0),
			 itemAnimationAmt(0.0), bgAnimationAmt(0.0), poisonAnimationAmt(0.0), holyAnimationAmt(0.0), musicSwitchDisplayAmt(0.0)
{
	Init();
}
void App::Init() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("Mega Man Puzzle Network", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
									 (orthoX2 - orthoX1) * 100, (orthoY2 - orthoY1) * 100, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glViewport(0, 0, (orthoX2 - orthoX1) * 100, (orthoY2 - orthoY1) * 100);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    player1 = new MegaMan();
    player1->npc = false;
    player1->hp = 1;

    // Texts and UI
    {
	textSheetWhite      = loadTexture("Pics\\Texts\\Texts - White.png");
	textSheetGreen      = loadTexture("Pics\\Texts\\Texts - Green.png");
	textSheetRed        = loadTexture("Pics\\Texts\\Texts - Red.png");
	textSheetWhite2     = loadTexture("Pics\\Texts\\Texts - White 2.png");
	textSheetGreen2     = loadTexture("Pics\\Texts\\Texts - Green 2.png");
	textSheetRed2       = loadTexture("Pics\\Texts\\Texts - Red 2.png");
    //textSheetRedTrap    = loadTexture("Pics\\Texts\\Texts - Red Trap.png");
    //textSheetPoison     = loadTexture("Pics\\Texts\\Texts - Purple Poison.png");
    //textSheetPurpleNPC  = loadTexture("Pics\\Texts\\Texts - Purple NPC.png");

    healthBoxPic        = loadTexture( "Pics\\Texts\\Health Box.png" );
    lvBarPic            = loadTexture( "Pics\\Texts\\Level Bar.png" );
    musicDisplayBox     = loadTexture( "Pics\\Texts\\MusicDisplay.png" );
    overheadSheet       = loadTexture( "Pics\\Texts\\Overhead.png" );
    }
    // Character Textures
    {
	megamanMoveSheet    = loadTexture("Pics\\Characters\\0 Mega Man Move Sheet.png");
	megamanAtkSheet     = loadTexture("Pics\\Characters\\0 Mega Man Atk Sheet.png");
    megamanHurtSheet    = loadTexture("Pics\\Characters\\0 Mega Man Hurt Sheet.png");
    megamanStepSheet    = loadTexture("Pics\\Characters\\0 Mega Man Step Sheet.png");
    protoMoveSheet      = loadTexture("Pics\\Characters\\1 Proto Man Move Sheet.png");
    protoAtkSheet       = loadTexture("Pics\\Characters\\1 Proto Man Atk Sheet.png");
    protoHurtSheet      = loadTexture("Pics\\Characters\\1 Proto Man Hurt Sheet.png");
    protoStepSheet      = loadTexture("Pics\\Characters\\1 Proto Man Step Sheet.png");
    tmanMoveSheet       = loadTexture("Pics\\Characters\\2 Toma Move Sheet.png");
    tmanAtkSheet1       = loadTexture("Pics\\Characters\\2 Toma Atk Sheet 1.png");
    tmanAtkSheet2       = loadTexture("Pics\\Characters\\2 Toma Atk Sheet 2.png");
    colonelMoveSheet    = loadTexture("Pics\\Characters\\3 Colonel Move Sheet.png");
    colonelAtkSheet     = loadTexture("Pics\\Characters\\3 Colonel Atk Sheet.png");
    colonelHurtSheet    = loadTexture("Pics\\Characters\\3 Colonel Hurt Sheet.png");
    colonelStepSheet    = loadTexture("Pics\\Characters\\3 Colonel Step Sheet.png");
    slashmanMoveSheet   = loadTexture("Pics\\Characters\\4 Slash Move Sheet.png");
    slashmanAtkSheet    = loadTexture("Pics\\Characters\\4 Slash Atk Sheet.png");
    slashmanStepSheet   = loadTexture("Pics\\Characters\\4 Slash Step Sheet.png");
    gutsMoveSheet       = loadTexture("Pics\\Characters\\5 Guts Move Sheet.png");
    gutsAtkSheet1       = loadTexture("Pics\\Characters\\5 Guts Atk Sheet 1.png");
    gutsAtkSheet2       = loadTexture("Pics\\Characters\\5 Guts Atk Sheet 2.png");
    gutsHurtSheet       = loadTexture("Pics\\Characters\\5 Guts Hurt Sheet.png");
    gutsStepSheet       = loadTexture("Pics\\Characters\\5 Guts Step Sheet.png");

    darkMegamanMoveSheet    = loadTexture("Pics\\NPC\\0 Dark Mega Man Move Sheet.png");
    darkMegamanAtkSheet     = loadTexture("Pics\\NPC\\0 Dark Mega Man Atk Sheet.png");
    darkMegamanHurtSheet    = loadTexture("Pics\\NPC\\0 Dark Mega Man Hurt Sheet.png");
    darkMegamanStepSheet    = loadTexture("Pics\\NPC\\0 Dark Mega Man Step Sheet.png");
    darkProtoMoveSheet      = loadTexture("Pics\\NPC\\1 Dark Proto Man Move Sheet.png");
    darkProtoAtkSheet       = loadTexture("Pics\\NPC\\1 Dark Proto Man Atk Sheet.png");
    darkProtoHurtSheet      = loadTexture("Pics\\NPC\\1 Dark Proto Man Hurt Sheet.png");
    darkProtoStepSheet      = loadTexture("Pics\\NPC\\1 Dark Proto Man Step Sheet.png");
    darkTmanMoveSheet       = loadTexture("Pics\\NPC\\2 Dark Toma Move Sheet.png");
    darkTmanAtkSheet1       = loadTexture("Pics\\NPC\\2 Dark Toma Atk Sheet 1.png");
    darkTmanAtkSheet2       = loadTexture("Pics\\NPC\\2 Dark Toma Atk Sheet 2.png");
    darkColonelMoveSheet    = loadTexture("Pics\\NPC\\3 Dark Colonel Move Sheet.png");
    darkColonelAtkSheet     = loadTexture("Pics\\NPC\\3 Dark Colonel Atk Sheet.png");
    darkColonelHurtSheet    = loadTexture("Pics\\NPC\\3 Dark Colonel Hurt Sheet.png");
    darkColonelStepSheet    = loadTexture("Pics\\NPC\\3 Dark Colonel Step Sheet.png");
    darkSlashmanMoveSheet   = loadTexture("Pics\\NPC\\4 Dark Slash Move Sheet.png");
    darkSlashmanAtkSheet    = loadTexture("Pics\\NPC\\4 Dark Slash Atk Sheet.png");
    darkSlashmanStepSheet   = loadTexture("Pics\\NPC\\4 Dark Slash Step Sheet.png");
    darkGutsMoveSheet       = loadTexture("Pics\\NPC\\5 Dark Guts Move Sheet.png");
    darkGutsAtkSheet1       = loadTexture("Pics\\NPC\\5 Dark Guts Atk Sheet 1.png");
    darkGutsAtkSheet2       = loadTexture("Pics\\NPC\\5 Dark Guts Atk Sheet 2.png");
    darkGutsHurtSheet       = loadTexture("Pics\\NPC\\5 Dark Guts Hurt Sheet.png");
    darkGutsStepSheet       = loadTexture("Pics\\NPC\\5 Dark Guts Step Sheet.png");
    }
    // Game Textures
    {
	rockSheet               = loadTexture("Pics\\Rocks\\Rock Sheet 1.png");
	rockSheetItem           = loadTexture("Pics\\Rocks\\Rock Sheet 2-1.png");
    rockSheetItem2          = loadTexture("Pics\\Rocks\\Rock Sheet 2-2.png");
    rockSheetTrappedItem    = loadTexture("Pics\\Rocks\\Rock Sheet 2-3.png");
	rockDeathSheet          = loadTexture("Pics\\Rocks\\Death Sheet.png");
    darkDeathSheet          = loadTexture("Pics\\Rocks\\Dark Death Sheet.png");

	floorSheet              = loadTexture("Pics\\Panels\\Panel Sheet.png");
    floorPoisonSheet        = loadTexture("Pics\\Panels\\Panel Sheet - Poison.png");
    floorHolySheet          = loadTexture("Pics\\Panels\\Panel Sheet - Holy.png");
	floorMoveSheet          = loadTexture("Pics\\Panels\\Panel Sheet - Move 2.png");
	floorBottomPic1         = loadTexture("Pics\\Panels\\Panel Bottom 1.png");
	floorBottomPic2         = loadTexture("Pics\\Panels\\Panel Bottom 2.png");
    floorAtkIndPic          = loadTexture("Pics\\Panels\\Panel Atk Indicator.png");

	energySheet             = loadTexture("Pics\\Energy\\Energy Sheet 1.png");
    energySheet2            = loadTexture("Pics\\Energy\\Energy Sheet 2.png");
    trappedEnergySheet      = loadTexture("Pics\\Energy\\Energy Sheet 3.png");
    recoverSheet            = loadTexture("Pics\\Energy\\Recover Sheet.png");
    energyGetSheet          = loadTexture("Pics\\Energy\\Energy Get Sheet.png");

	bgA                     = loadTexture("Pics\\Backgrounds\\Background A.png");
	bgB                     = loadTexture("Pics\\Backgrounds\\Background B.png");
	bgC                     = loadTexture("Pics\\Backgrounds\\Background C.png");
    bgD                     = loadTexture("Pics\\Backgrounds\\Background D.png");
    bgMain                  = loadTexture("Pics\\Backgrounds\\Background Main.png");
    }
    // Menu Textures
    dimScreenPic            = loadTexture( "Pics\\Menus\\DimScreen.png" );
    generalInfoboxPic       = loadTexture( "Pics\\Menus\\Options Infobox.png" );
    // Main Menu
    {
    mainMenuPic             = loadTexture( "Pics\\Menus\\0 Main Menu\\Main Menu Text.png" );
    mainSelPic0             = loadTexture( "Pics\\Menus\\0 Main Menu\\Menu Select 0 - New Run.png" );
    mainSelPic1             = loadTexture( "Pics\\Menus\\0 Main Menu\\Menu Select 1 - Tutorial.png" );
    mainSelPic2             = loadTexture( "Pics\\Menus\\0 Main Menu\\Menu Select 2 - Controls.png" );
    mainSelPic3             = loadTexture( "Pics\\Menus\\0 Main Menu\\Menu Select 3 - Options.png" );
    mainSelPic4             = loadTexture( "Pics\\Menus\\0 Main Menu\\Menu Select 4 - Exit.png" );
    }
    // New Run Menu
    {
    newRunMenuPic           = loadTexture( "Pics\\Menus\\1 New Run\\New Run - Empty Menu.png" );
    newRunSelPic0           = loadTexture( "Pics\\Menus\\1 New Run\\Menu Select 0 - Character.png" );
    newRunSelPic1           = loadTexture( "Pics\\Menus\\1 New Run\\Menu Select 1 - Difficulty.png" );
    newRunSelPic2           = loadTexture( "Pics\\Menus\\1 New Run\\Menu Select 2 - Start.png" );
    charSelPic0             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 0 - MegaMan.png" );
    charSelPic1             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 1 - ProtoMan.png" );
    charSelPic2             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 2 - TomahawkMan.png" );
    charSelPic3             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 3 - Colonel.png" );
    charSelPic4             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 4 - SlashMan.png" );
    charSelPic5             = loadTexture( "Pics\\Menus\\1 New Run\\Character Select 5 - GutsMan.png" );
    // chipPic0                = loadTexture( "Pics\\Menus\\1 New Run\\Chips 0 - MegaMan.png" );
    // chipPic1                = loadTexture( "Pics\\Menus\\1 New Run\\Chips 1 - ProtoMan.png" );
    // chipPic2                = loadTexture( "Pics\\Menus\\1 New Run\\Chips 2 - Tomahawk Man.png" );
    // chipPic3                = loadTexture( "Pics\\Menus\\1 New Run\\Chips 3 - Colonel.png" );
    // chipPic4                = loadTexture( "Pics\\Menus\\1 New Run\\Chips 4 - SlashMan.png" );
    diffSelPic0             = loadTexture( "Pics\\Menus\\1 New Run\\Difficulty Select 0.png" );
    diffSelPic1             = loadTexture( "Pics\\Menus\\1 New Run\\Difficulty Select 1.png" );
    diffSelPic2             = loadTexture( "Pics\\Menus\\1 New Run\\Difficulty Select 2.png" );
    diffSelPic3             = loadTexture( "Pics\\Menus\\1 New Run\\Difficulty Select 3.png" );
    diffSelPic4             = loadTexture( "Pics\\Menus\\1 New Run\\Difficulty Select 4.png" );
    movesetPic0             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 0 - MegaMan.png" );
    movesetPic1             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 1 - ProtoMan.png" );
    movesetPic2             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 2 - TomahawkMan.png" );
    movesetPic3             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 3 - Colonel.png" );
    movesetPic4             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 4 - SlashMan.png" );
    movesetPic5             = loadTexture( "Pics\\Menus\\1 New Run\\Moveset 5 - GutsMan.png" );
    }
    // Tutorial Menu
    {
    tutorialMenuPic         = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Menu.png" );
    tSelPic0                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 1.png" );
    tSelPic1                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 2.png" );
    tSelPic2                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 3.png" );
    tSelPic3                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 4.png" );
    tSelPic4                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 5.png" );
    tSelPic5                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 6.png" );
    tSelPic6                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 7.png" );
    tSelPic7                = loadTexture( "Pics\\Menus\\2 Tutorial\\Tutorial Select 8.png" );
    tInfoboxPic             = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\Tutorial Container.png" );
    tInfoPic0               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\01 - Basics - Energy.png" );
    tInfoPic1               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\02 - Basics - Attacking Vertically.png" );
    tInfoPic2               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\03 - Basics - Step-Swords.png" );
    tInfoPic3               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\04 - Panels - Cracked.png" );
    tInfoPic4               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\05 - Panels - Poison and Ice.png" );
    tInfoPic5               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\06 - Panels - Holy.png" );
    tInfoPic6               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\07 - Training Room.png" );
    tInfoPic7               = loadTexture( "Pics\\Menus\\2-2 Tutorial Level InfoBoxes\\08 - AI Training Room.png" );
    }
    controlMenuPic =        loadTexture( "Pics\\Menus\\3 Controls\\Controls Menu.png" );
    // Options Menu
    {
    optionMenuPic           = loadTexture( "Pics\\Menus\\4 Options\\0 Heading.png" );
    oSelPic0                = loadTexture( "Pics\\Menus\\4 Options\\Menu Select 0 - Infobox.png" );
    oSelPic1                = loadTexture( "Pics\\Menus\\4 Options\\Menu Select 1 - Game Volume.png" );
    oSelPic2                = loadTexture( "Pics\\Menus\\4 Options\\Menu Select 2 - Music Volume.png" );
    oSelPic3                = loadTexture( "Pics\\Menus\\4 Options\\Menu Select 3 - Music Track.png" );
    infoSelPic0             = loadTexture( "Pics\\Menus\\4 Options\\0 Infobox - 0 On.png" );
    infoSelPic1             = loadTexture( "Pics\\Menus\\4 Options\\0 Infobox - 1 Off.png" );
    {
    gameVolSelPic0          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 0.png" );
    gameVolSelPic1          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 1.png" );
    gameVolSelPic2          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 2.png" );
    gameVolSelPic3          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 3.png" );
    gameVolSelPic4          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 4.png" );
    gameVolSelPic5          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 5.png" );
    gameVolSelPic6          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 6.png" );
    gameVolSelPic7          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 7.png" );
    gameVolSelPic8          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 8.png" );
    gameVolSelPic9          = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 9.png" );
    gameVolSelPic10         = loadTexture( "Pics\\Menus\\4 Options\\1 Game Volume - 10.png" );
    }
    {
    musicVolSelPic0         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 0.png" );
    musicVolSelPic1         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 1.png" );
    musicVolSelPic2         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 2.png" );
    musicVolSelPic3         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 3.png" );
    musicVolSelPic4         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 4.png" );
    musicVolSelPic5         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 5.png" );
    musicVolSelPic6         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 6.png" );
    musicVolSelPic7         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 7.png" );
    musicVolSelPic8         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 8.png" );
    musicVolSelPic9         = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 9.png" );
    musicVolSelPic10        = loadTexture( "Pics\\Menus\\4 Options\\2 Music Volume - 10.png" );
    }
    {
    musicSelPic1            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 1 Organization.png" );
    musicSelPic2            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 2 An Incident.png" );
    musicSelPic3            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 3 Blast Speed.png" );
    musicSelPic4            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 4 Shark Panic.png" );
    musicSelPic5            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 5 Battle Field.png" );
    musicSelPic6            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 6 Doubt.png" );
    musicSelPic7            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 7 Distortion.png" );
    musicSelPic8            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 8 Surge of Power.png" );
    musicSelPic9            = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 9 Digital Strider.png" );
    musicSelPic10           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 10 Break the Storm.png" );
    musicSelPic11           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 11 Evil Spirit.png" );
    musicSelPic12           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 12 Hero.png" );
    musicSelPic13           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 13 Danger Zone.png" );
    musicSelPic14           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 14 Navi Customizer.png" );
    musicSelPic15           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 15 Graveyard.png" );
    musicSelPic16           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 16 The Count.png" );
    musicSelPic17           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 17 Secret Base.png" );
    musicSelPic18           = loadTexture( "Pics\\Menus\\4 Options\\3 Music Track - 18 Cybeasts.png" );
    }
    }

    // Sword Attack Trail Textures
    {
	swordAtkSheet1      = loadTexture("Pics\\Sword Attacks\\Sword Sheet 1.png");
	swordAtkSheet3      = loadTexture("Pics\\Sword Attacks\\Sword Sheet 3.png");
	longAtkSheet1       = loadTexture("Pics\\Sword Attacks\\Long Sheet 1.png");
	longAtkSheet3       = loadTexture("Pics\\Sword Attacks\\Long Sheet 3.png");
	wideAtkSheet1       = loadTexture("Pics\\Sword Attacks\\Wide Sheet 1.png");
	wideAtkSheet3       = loadTexture("Pics\\Sword Attacks\\Wide Sheet 3.png");
	crossAtkSheet1      = loadTexture("Pics\\Sword Attacks\\Cross Sheet 1.png");
	crossAtkSheet3      = loadTexture("Pics\\Sword Attacks\\Cross Sheet 3.png");
	spinAtkSheet1       = loadTexture("Pics\\Sword Attacks\\Spin Sheet 1.png");
	spinAtkSheet3       = loadTexture("Pics\\Sword Attacks\\Spin Sheet 3.png");
	lifeAtkSheet1       = loadTexture("Pics\\Sword Attacks\\Life Sheet 1.png");
	lifeAtkSheet3       = loadTexture("Pics\\Sword Attacks\\Life Sheet 3.png");

    heroAtkSheet1       = loadTexture("Pics\\Sword Attacks\\Hero Sheet 1.png");
    heroAtkSheet3       = loadTexture("Pics\\Sword Attacks\\Hero Sheet 3.png");
    protoAtkSheet1      = loadTexture("Pics\\Sword Attacks\\Proto Sheet 1.png");
    protoAtkSheet3      = loadTexture("Pics\\Sword Attacks\\Proto Sheet 3.png");

    screenDivVSheet1    = loadTexture("Pics\\Sword Attacks\\ScreenDivV Sheet 1.png");
    screenDivVSheet3    = loadTexture("Pics\\Sword Attacks\\ScreenDivV Sheet 3.png");
    screenDivUpSheet1   = loadTexture("Pics\\Sword Attacks\\ScreenDivUp Sheet 1.png");
    screenDivUpSheet3   = loadTexture("Pics\\Sword Attacks\\ScreenDivUp Sheet 3.png");
    screenDivDownSheet1 = loadTexture("Pics\\Sword Attacks\\ScreenDivDown Sheet 1.png");
    screenDivDownSheet3 = loadTexture("Pics\\Sword Attacks\\ScreenDivDown Sheet 3.png");
    screenDivXSheet1    = loadTexture("Pics\\Sword Attacks\\ScreenDivX Sheet 1.png");
    screenDivXSheet3    = loadTexture("Pics\\Sword Attacks\\ScreenDivX Sheet 3.png");
    screenDivZSheet     = loadTexture("Pics\\Sword Attacks\\ScreenDivZ Sheet.png");

    tomahawkAtkSheetA1  = loadTexture("Pics\\Sword Attacks\\Toma Sheet A1.png");
    tomahawkAtkSheetA3  = loadTexture("Pics\\Sword Attacks\\Toma Sheet A3.png");
    tomahawkAtkSheetB1  = loadTexture("Pics\\Sword Attacks\\Toma Sheet B1.png");
    tomahawkAtkSheetB3  = loadTexture("Pics\\Sword Attacks\\Toma Sheet B3.png");
    eagleTomaSheet      = loadTexture("Pics\\Sword Attacks\\Eagle Toma Sheet.png");

    longSlashSheet1     = loadTexture("Pics\\Sword Attacks\\Long Slash Sheet 1.png");
    longSlashSheet3     = loadTexture("Pics\\Sword Attacks\\Long Slash Sheet 3.png");
    wideSlashSheet1     = loadTexture("Pics\\Sword Attacks\\Wide Slash Sheet 1.png");
    wideSlashSheet3     = loadTexture("Pics\\Sword Attacks\\Wide Slash Sheet 3.png");

    shockwaveSheet1     = loadTexture("Pics\\Sword Attacks\\Shockwave Sheet 1.png");
    shockwaveSheet3     = loadTexture("Pics\\Sword Attacks\\Shockwave Sheet 3.png");
    }

    // Sound Channels and Channel Volumes
    {
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 4, 4096 );
    Mix_AllocateChannels( 10 );
    Mix_Volume( 0, gameVol * mainVolScale );    // Player Sword Attack Sounds, Player Deleted Sound
    Mix_Volume( 1, gameVol * miscVolScale );    // Item1 Sounds, Trapped Item Sounds, Item Upgrade Sounds
    Mix_Volume( 2, gameVol * miscVolScale );    // Panel Break sounds
    Mix_Volume( 3, musicVol * musicVolScale );  // Music
    Mix_Volume( 4, gameVol * miscVolScale );    // Rock Break sounds
    Mix_Volume( 5, gameVol * miscVolScale );    // Item2 Sounds
    Mix_Volume( 6, gameVol * mainVolScale );    // NPC Appearing Sound, NPC Sword Attack Sounds, NPC Deleted Sound
    Mix_Volume( 7, gameVol * miscVolScale );    // Menu Sounds
    Mix_Volume( 8, gameVol * mainVolScale );    // Player GutsMan Shockwave
    Mix_Volume( 9, gameVol * mainVolScale );    // NPC GutsMan Shockwave
    }

    // Sounds
    {
	swordSound      = Mix_LoadWAV( "Sounds\\SwordSwing.ogg" );
	lifeSwordSound  = Mix_LoadWAV( "Sounds\\LifeSword.ogg" );
    screenDivSound  = Mix_LoadWAV( "Sounds\\ScreenDivide.ogg" );
    tomahawkSound   = Mix_LoadWAV( "Sounds\\Tomahawk.ogg" );
    eTomaSound      = Mix_LoadWAV( "Sounds\\EToma.ogg" );
    spinSlashSound  = Mix_LoadWAV( "Sounds\\SpinSlash.ogg");
    gutsPunchSound  = Mix_LoadWAV( "Sounds\\Guts Punch.ogg");
    gutsHammerSound = Mix_LoadWAV( "Sounds\\Guts Hammer.ogg");
    shockwaveSound  = Mix_LoadWAV( "Sounds\\Shockwave.ogg" );

	itemSound       = Mix_LoadWAV( "Sounds\\GotItem.ogg" );
    itemSound2      = Mix_LoadWAV( "Sounds\\GotItem2.ogg" );
    hurtSound       = Mix_LoadWAV( "Sounds\\Hurt.ogg" );
    deletedSound    = Mix_LoadWAV( "Sounds\\Deleted.ogg" );
    recoverSound    = Mix_LoadWAV( "Sounds\\Recover.ogg" );
    bossAppearSound = Mix_LoadWAV( "Sounds\\Darkness Appear.ogg" );
    bossDeathSound  = Mix_LoadWAV( "Sounds\\Darkness Disappear.ogg" );
	rockBreakSound  = Mix_LoadWAV( "Sounds\\AreaGrabHit.ogg" );
	panelBreakSound = Mix_LoadWAV( "Sounds\\PanelCrack.ogg" );

    infoboxOpenSound  = Mix_LoadWAV( "Sounds\\infoOpen.ogg" );
    infoboxCloseSound = Mix_LoadWAV( "Sounds\\infoClose.ogg" );

    menuCancelSound = Mix_LoadWAV( "Sounds\\MenuCancel.ogg" );
    menuChooseSound = Mix_LoadWAV( "Sounds\\MenuChoose.ogg" );
    menuOpenSound   = Mix_LoadWAV( "Sounds\\MenuOpen.ogg" );
    }
    // Music Tracks
    {
    track01 = Mix_LoadWAV( "Music\\01 Organization.ogg" );
    track02 = Mix_LoadWAV( "Music\\02 An Incident.ogg" );
    track03 = Mix_LoadWAV( "Music\\03 Blast Speed.ogg" );
    track04 = Mix_LoadWAV( "Music\\04 Shark Panic.ogg" );
    track05 = Mix_LoadWAV( "Music\\05 Battle Field.ogg" );
    track06 = Mix_LoadWAV( "Music\\06 Doubt.ogg" );
    track07 = Mix_LoadWAV( "Music\\07 Distortion.ogg" );
    track08 = Mix_LoadWAV( "Music\\08 Surge of Power.ogg" );
    track09 = Mix_LoadWAV( "Music\\09 Digital Strider.ogg" );
    track10 = Mix_LoadWAV( "Music\\10 Break the Storm.ogg" );
    track11 = Mix_LoadWAV( "Music\\11 Evil Spirit.ogg" );
    track12 = Mix_LoadWAV( "Music\\12 Hero.ogg" );
    track13 = Mix_LoadWAV( "Music\\13 Danger Zone.ogg" );
    track14 = Mix_LoadWAV( "Music\\14 Navi Customizer.ogg" );
    track15 = Mix_LoadWAV( "Music\\15 Graveyard.ogg" );
    track16 = Mix_LoadWAV( "Music\\16 The Count.ogg" );
    track17 = Mix_LoadWAV( "Music\\17 Secret Base.ogg" );
    track18 = Mix_LoadWAV( "Music\\18 Cybeasts.ogg" );
    }
}
App::~App() {
    delete player1;
    for( int i = 0; i < npcList.size(); i++ ) delete npcList[i];
    for( int i = 0; i < nextNpcList.size(); i++ ) delete nextNpcList[i];

    Mix_Pause( -1 );    // Pause all Channels before free-ing up the Sound Chunks

	Mix_FreeChunk( swordSound );
	Mix_FreeChunk( lifeSwordSound );
    Mix_FreeChunk( screenDivSound );
    Mix_FreeChunk( tomahawkSound );
    Mix_FreeChunk( eTomaSound );
    Mix_FreeChunk( spinSlashSound );
    Mix_FreeChunk( gutsPunchSound );
    Mix_FreeChunk( gutsHammerSound );
    Mix_FreeChunk( shockwaveSound );

	Mix_FreeChunk( itemSound );
    Mix_FreeChunk( itemSound2 );
    Mix_FreeChunk( hurtSound );
    Mix_FreeChunk( deletedSound );
    Mix_FreeChunk( recoverSound );
    Mix_FreeChunk( bossAppearSound );
    Mix_FreeChunk( bossDeathSound );
	Mix_FreeChunk( rockBreakSound );
	Mix_FreeChunk( panelBreakSound );

	Mix_FreeChunk( infoboxOpenSound );
	Mix_FreeChunk( infoboxCloseSound );

    Mix_FreeChunk( menuCancelSound );
    Mix_FreeChunk( menuChooseSound );
    Mix_FreeChunk( menuOpenSound );

	Mix_FreeChunk( track01 );
    Mix_FreeChunk( track02 );
    Mix_FreeChunk( track03 );
    Mix_FreeChunk( track04 );
    Mix_FreeChunk( track05 );
    Mix_FreeChunk( track06 );
    Mix_FreeChunk( track07 );
    Mix_FreeChunk( track08 );
    Mix_FreeChunk( track09 );
	Mix_FreeChunk( track10 );
    Mix_FreeChunk( track11 );
    Mix_FreeChunk( track12 );
    Mix_FreeChunk( track13 );
    Mix_FreeChunk( track14 );
    Mix_FreeChunk( track15 );
    Mix_FreeChunk( track16 );
    Mix_FreeChunk( track17 );
    Mix_FreeChunk( track18 );

	SDL_Quit();
}

bool App::UpdateAndRender() {
	float ticks = (float)SDL_GetTicks() / 2000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	fixedElapsed = elapsed + timeLeftOver;
	if (fixedElapsed > fixed_timestep * max_steps) {
		fixedElapsed = fixed_timestep * max_steps; }
	while (fixedElapsed >= fixed_timestep) {
		fixedElapsed -= fixed_timestep;
		FixedUpdate();					   }
	timeLeftOver = fixedElapsed;

	Update(elapsed);
	Render();

    return done;
}
void App::Update(float &elapsed) {
    checkKeys();
	updateGame(elapsed);
}
void App::FixedUpdate() {
    checkKeys();
	updateGame(fixed_timestep);
}

void App::updateGame(float elapsed) {
    // Handle Displayed Energy amt
    float energyIncrement = (float) abs( player1->totalEnergyChange ) / ( iconDisplayTime / elapsed );
    if( energyIncrement < 1 ) energyIncrement = 1;

    if( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) {
        player1->energyDisplayed = player1->energy;
        player1->totalEnergyChange = 0;
        energyIncrement = 0;
    }
    else if( player1->energyDisplayed > player1->energy ) player1->energyDisplayed -= energyIncrement;
    else if( player1->energyDisplayed < player1->energy ) player1->energyDisplayed += energyIncrement;
    if( abs( player1->energyDisplayed - player1->energy ) <= energyIncrement ) player1->energyDisplayed = player1->energy;

    if( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) player1->energyDisplayed2 = currentEnergyGain;
    else if( player1->energyDisplayed2 > currentEnergyGain ) player1->energyDisplayed2 -= energyIncrement;
    else if( player1->energyDisplayed2 < currentEnergyGain ) player1->energyDisplayed2 += energyIncrement;
    if( abs( player1->energyDisplayed2 - currentEnergyGain ) <= energyIncrement ) player1->energyDisplayed2 = currentEnergyGain;
    
    //player1->totalEnergyChange -= energyIncrement;

	// Display parameter for Energy gain
	chargeDisplayPlusAmt -= elapsed;
    if( chargeDisplayPlusAmt < 0 ) { chargeDisplayPlusAmt = 0; }

	// Display parameter for Energy usage
	chargeDisplayMinusAmt -= elapsed;
    if( chargeDisplayMinusAmt < 0 ) { chargeDisplayMinusAmt = 0; }
    
    updatePlayer( elapsed );

	// Item Display parameter
	itemAnimationAmt += elapsed * 10;
    if( itemAnimationAmt >= 8.0 ) { itemAnimationAmt = 0; }
    // Background Display parameter
    bgAnimationAmt += elapsed * 3.5;
    if( bgAnimationAmt >= 8.0 ) { bgAnimationAmt = 0; }
    // Poison Panel Display parameter
	poisonAnimationAmt += elapsed * 3;
	if ( poisonAnimationAmt >= 8.0) { poisonAnimationAmt = 0; }
    // Holy Panel Display parameter
	holyAnimationAmt += elapsed * 3;
	if ( holyAnimationAmt >= 7.0) { holyAnimationAmt = 0; }

	// Rock HP Indicators, Item Upgrade Indicators
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			board.map[i][j].rockAtkInd -= elapsed;
			if (board.map[i][j].rockAtkInd < 0) { board.map[i][j].rockAtkInd = 0; }
			if (board.map[i][j].rockHP < 0) { board.map[i][j].rockHP = 0; }

            board.map[i][j].upgradeInd -= elapsed;
            if ( board.map[i][j].upgradeInd < 0 ) { board.map[i][j].upgradeInd = 0; }
	} }

	// Handles delayed Damage to Rocks
    for ( int i = 0; i < delayedDmgList.size(); i++ ) {
        if ( delayedDmgList[i].npc && npcList.empty() ) {
            delayedDmgList.erase( delayedDmgList.begin() + i );
            i--;
        }
        else {
            delayedDmgList[i].delay -= elapsed;
            if ( delayedDmgList[i].delay <= 0 ) {
                hitPanel( delayedDmgList[i].xPos, delayedDmgList[i].yPos, delayedDmgList[i].dmg, delayedDmgList[i].npc );
                delayedDmgList.erase( delayedDmgList.begin() + i );
                i--;
            }
        }
    }

	// Handles delayed Sounds
	for ( int i = 0; i < delayedSoundList.size(); i++ ) {
		delayedSoundList[i].delay -= elapsed;
		if( delayedSoundList[i].delay <= 0 ) {
			if     ( delayedSoundList[i].soundName == "sword" )  {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, swordSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, swordSound, 0 );
            }
			else if( delayedSoundList[i].soundName == "life" )   {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, lifeSwordSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, lifeSwordSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "divide" ) {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, screenDivSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, screenDivSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "toma" )   {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, tomahawkSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, tomahawkSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "eToma" )  {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, eTomaSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, eTomaSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "punch" ) {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, gutsPunchSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, gutsPunchSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "hammer" ) {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, gutsHammerSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 6, gutsHammerSound, 0 );
            }
            else if( delayedSoundList[i].soundName == "shockwave" ) {
                if( !delayedSoundList[i].npc ) Mix_PlayChannel( 8, shockwaveSound, 0 );
                else if( !npcList.empty() )    Mix_PlayChannel( 9, shockwaveSound, 0 );
            }
            delayedSoundList.erase( delayedSoundList.begin() + i );
            i--;
	    }
    }

    // Handles delayed Attack Displays
    for ( int i = 0; i < delayedAtkDisplayList.size(); i++ ) {
        if( delayedAtkDisplayList[i].delay <= 0 ) { delayedAtkDisplayList[i].animationTimer -= elapsed; }
        else { delayedAtkDisplayList[i].delay -= elapsed; }
        if( delayedAtkDisplayList[i].animationTimer <= -0.2 ) {
            delayedAtkDisplayList.erase( delayedAtkDisplayList.begin() + i );
            i--;
        }
    }

    // Handles delayed Previous Energy Displays
    for( int i = 0; i < delayedPrevEnergyList.size(); i++ ) {
        delayedPrevEnergyList[i].animationTimer -= elapsed;
        if( delayedPrevEnergyList[i].animationTimer < 0 ) {
            delayedPrevEnergyList.erase( delayedPrevEnergyList.begin() + i );
            i--;
        }
    }

    // Handles display time of Music track Number and Name
    musicSwitchDisplayAmt -= elapsed;
    if ( musicSwitchDisplayAmt < 0 ) { musicSwitchDisplayAmt = 0; }

    // Handle reloading Tutorial Level, so that Players cannot fail a Tutorial Level
    if( -7 <= level && level <= -1 && player1->x != -1 && player1->x != 6 && player1->energy < player1->getAtkCost( 0 ) && !reloadedTutorial ) { loadTutorialLevel( true ); }
}
void App::updatePlayer(float elapsed) {
    // Handle Collecting Energy Item Resources
    int xPos = round( player1->xDisplay ), yPos = round( player1->yDisplay );
    if( 0 <= xPos && xPos <= 5 && 0 <= yPos && yPos <= 5 && board.map[xPos][yPos].item != 0 && board.map[xPos][yPos].rockHP == 0 ) {
        chargeDisplayPlusAmt = iconDisplayTime;
        chargeDisplayMinusAmt = 0;
        // Blue Energy
        if( board.map[xPos][yPos].item == 2 ) {
            Mix_PlayChannel( 5, itemSound2, 0 );
            if( player1->animationType != 1 ) player1->actionNumber = -3;
            player1->energy += energyGainAmt2;
            player1->totalEnergyChange += energyGainAmt2;
            currentEnergyGain += energyGainAmt2;
            delayedPrevEnergyList.push_back( DelayedEnergyDisplay( xPos, yPos, energyGainAmt2, 1, iconDisplayTime ) );
            reloadedTutorial = false;
        }
        // Trapped Red Energy
        else if( board.map[xPos][yPos].item == -1 ) {
            Mix_PlayChannel( 1, hurtSound, 0 );
            chargeDisplayPlusAmt = 0;
            chargeDisplayMinusAmt = iconDisplayTime;
            if( player1->animationType != 1 ) {
                player1->animationType = -1;
                player1->actionNumber = -4;
                player1->animationDisplayAmt = pauseAnimationTime; }
            player1->hurtDisplayAmt = hurtAnimationTime;
            player1->energy -= energyLossAmt;
            if( player1->energy < 0 ) player1->energy = 0;
            player1->totalEnergyChange -= energyLossAmt;
            if( player1->energy < 0 ) player1->energy = 0;
            currentEnergyGain -= energyLossAmt;
            delayedPrevEnergyList.push_back( DelayedEnergyDisplay( xPos, yPos, -energyLossAmt, 1, iconDisplayTime ) );
        }
        // Green Energy
        else {
            Mix_PlayChannel( 1, itemSound, 0 );
            if( player1->animationType != 1 ) player1->actionNumber = -2;
            player1->energy += energyGainAmt;
            player1->totalEnergyChange += energyGainAmt;
            currentEnergyGain += energyGainAmt;
            delayedPrevEnergyList.push_back( DelayedEnergyDisplay( xPos, yPos, energyGainAmt, 1, iconDisplayTime ) );
            reloadedTutorial = false;
        }
        board.map[xPos][yPos].item = 0;
    }

    player1->hurtDisplayAmt -= elapsed;
    player1->animationDisplayAmt -= elapsed;
    // Defeated Animation
    if( player1->hp <= 0 && player1->hp > -10 ) {
        player1->deathDisplayAmt -= elapsed;
        if( player1->deathDisplayAmt < 0 ) {
            player1->hp = -10;
            board.map[player1->x][player1->y].rockHP = 1;
            board.map[player1->x][player1->y].bigRockDeath = true;
            hitPanel( player1->x, player1->y );
            Mix_PlayChannel( 0, deletedSound, 0 );
        }
    }
    // Invalid movement when trying to move back onto the Starting Panel
    else if( player1->x == -1 && player1->y == 0 && player1->animationDisplayAmt <= 0 ) {
        player1->animationType = 2;
        player1->facing = -1;
        player1->x = 0;
        player1->animationDisplayAmt = moveAnimationTime;
    }
    // Level Transition parameter
    else if( player1->x == 6 && player1->y == 5 && player1->animationDisplayAmt <= 0 ) {
        player1->animationType = 3;
        player1->animationDisplayAmt = moveAnimationTime * 3;
        npcAbleToAct = false;
        for( int i = 0; i < npcList.size(); i++ ) { npcList[i]->npcActionTimer = 0; }
    }
    // Handles Level Transition
    else if( player1->animationType == 3 && player1->animationDisplayAmt > 0 && player1->animationDisplayAmt <= moveAnimationTime * 2 ) {
        next();
        player1->animationType = 4;
        npcAbleToAct = false;
        for( int i = 0; i < npcList.size(); i++ ) { npcList[i]->npcActionTimer = 0; }
    }
    // Handle Slipping on Ice
    else if( player1->animationDisplayAmt <= 0 && board.map[xPos][yPos].state == 1 && player1->moveType == 2 && ( player1->animationType == 0 || player1->animationType == 5 ) ) {
        bool slipped = false;
        if( player1->moveDir == 0 && isPanelValid( player1->x, player1->y + 1, player1->npc ) ) {
            player1->y++;
            slipped = true;
        }
        // Left
        else if( player1->moveDir == 1 && isPanelValid( player1->x - 1, player1->y, player1->npc ) ) {
            player1->x--;
            slipped = true;
        }
        // Down
        else if( player1->moveDir == 2 && isPanelValid( player1->x, player1->y - 1, player1->npc ) ) {
            player1->y--;
            slipped = true;
        }
        // Right
        else if( player1->moveDir == 3 && isPanelValid( player1->x + 1, player1->y, player1->npc ) ) {
            player1->x++;
            slipped = true;
        }
        else {
            player1->animationType = -2;
        }
        if( !slipped ) return;

        player1->animationType = 5;
        player1->animationDisplayAmt = iceSlipAnimationTime;
        switch( board.map[player1->x][player1->y].state ) {
        default:
            player1->onHolyPanel = false;
            break;
        case 2:     // Poison Panels damage you when you step on them
            player1->energy -= poisonLossAmt;
            if( player1->energy < 0 ) player1->energy = 0;
            player1->totalEnergyChange -= poisonLossAmt;
            currentEnergyGain -= poisonLossAmt;
            chargeDisplayPlusAmt = 0;
            chargeDisplayMinusAmt = iconDisplayTime;
            player1->hurtDisplayAmt = poisonHurtTime;
            delayedPrevEnergyList.push_back( DelayedEnergyDisplay( player1->x, player1->y, -poisonLossAmt, 2, iconDisplayTime ) );
            break;
        case 3:     // Holy Panels can save you some Energy for Attack Costs
            player1->onHolyPanel = true;
            break;
        }
    }
    // Reload default parameters when nothing is happening or being animated
    else if( player1->animationDisplayAmt <= 0 ) {
        player1->animationDisplayAmt = 0;
        player1->moveType = 0;
        player1->moveDir = -1;
        player1->animationType = -2;
    }

    // Handle NPC's
    for( int i = 0; i < npcList.size(); i++ ) {
        npcList[i]->hurtDisplayAmt -= elapsed;
        npcList[i]->animationDisplayAmt -= elapsed;
        if( npcAbleToAct ) { npcList[i]->npcActionTimer = npcWaitTime; }

        if( ( npcList[i]->x == player1->x ) && ( npcList[i]->y == player1->y ) ) {
            npcList[i]->hp = -1;
        }
        // Handle NPC Defeated
        if( npcList[i]->hp <= 0 ) {
            npcList[i]->deathDisplayAmt -= elapsed;
            npcList[i]->hurtDisplayAmt = 0;
            if( npcList[i]->deathDisplayAmt < 0 ) {
                board.map[npcList[i]->x][npcList[i]->y].rockHP = 1;
                board.map[npcList[i]->x][npcList[i]->y].bigRockDeath = true;
                board.map[npcList[i]->x][npcList[i]->y].isPurple = true;
                hitPanel( npcList[i]->x, npcList[i]->y );
                // Reward for Defeating an NPC: Energy drops
                upgradeItems();
                while( npcList[i]->timesHit > 0 ) {
                    spawnRandomItem( npcList[i]->x, npcList[i]->y );
                    npcList[i]->timesHit--;
                }
                Mix_PlayChannel( 6, bossDeathSound, 0 );
                delete npcList[i];
                npcList.erase( npcList.begin() + i );
                i--;
                continue;
            }
        }
        // Normal NPC AI Action
        else if( npcList[i]->x != 8 && npcList[i]->animationDisplayAmt <= 0 && npcList[i]->npcActionTimer > 0 ) {
            npcList[i]->npcActionTimer -= elapsed;
            if( npcList[i]->npcActionTimer <= 0 ) { aiAction( npcList[i] ); }
        }
        // Level Transition Animation
        else if( npcList[i]->x == 6 && npcList[i]->y == 5 && npcList[i]->animationDisplayAmt < 0 ) {
            npcList[i]->animationType = 3;
            npcList[i]->animationDisplayAmt = moveAnimationTime * 3;
        }
        // Handle NPC's entering the next level
        else if( npcList[i]->animationType == 3 && npcList[i]->animationDisplayAmt <= moveAnimationTime * 2 ) {
            npcList[i]->x = 8;
            npcList[i]->animationDisplayAmt = 0;
            npcList[i]->moveType = 0;
            npcList[i]->moveDir = -1;
            npcList[i]->animationType = -2;
            nextNpcList.push_back( npcList[i] );
            npcList.erase( npcList.begin() + i );
            i--;
        }
        // NPC Idle
        else if( npcList[i]->animationDisplayAmt <= 0 ) {
            npcList[i]->animationDisplayAmt = 0;
            npcList[i]->moveType = 0;
            npcList[i]->moveDir = -1;
            npcList[i]->animationType = -2;
        }
    }
    npcAbleToAct = false;
}
void App::checkKeys() {
    // Check keys for Player Actions

    // Player can't act until NPCs have finished acting
    for ( int i = 0; i < npcList.size(); i++ ) {
        if ( npcList[i]->animationDisplayAmt > 0 ) return;
    }

	// Menu Controls
    while( SDL_PollEvent( &event ) ) {
        if( event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE ) { done = true; }
        else if( event.type == SDL_KEYDOWN && player1->animationDisplayAmt <= 0 ) {
            // Read Single Button presses
            SDL_Scancode key = event.key.keysym.scancode;
            switch( event.key.keysym.scancode ) {
            case SDL_SCANCODE_ESCAPE:
            case SDL_SCANCODE_BACKSPACE: {
                player1->animationDisplayAmt = menuExitTime;
                switch( menuNum ) {
                case -1:        // If In-Game -> Open Main Menu
                    menuNum = MAIN_MENU;
                    menuSel = 0;
                    Mix_PlayChannel( 7, menuOpenSound, 0 );
                    break;
                case MAIN_MENU: // Main Menu is already Open -> Close if In-Game, or select "Exit" if not
                    level != 0 ? menuNum = -1 : menuSel = EXIT-1;
                    Mix_PlayChannel( 7, menuCancelSound, 0 );
                    break;
                default:        // Other Menu is Open -> Return to Main Menu
                    menuSel = menuNum - 1;
                    menuNum = 0;
                    Mix_PlayChannel( 7, menuCancelSound, 0 );
                    break;
                }
                break;
            }
            case SDL_SCANCODE_TAB:
            case SDL_SCANCODE_F: {
                if( level == 0 ) break;
                player1->animationDisplayAmt = menuExitTime;
                if( showInfobox ) {
                    showInfobox = false;
                    Mix_PlayChannel( 7, infoboxCloseSound, 0 );
                }
                else {
                    showInfobox = true;
                    Mix_PlayChannel( 7, infoboxOpenSound, 0 );
                }
                break;
            }
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP: {
                switch( menuNum ) {
                case -1: break;
                case TUTORIAL_MENU:
                    if( menuSel >= 3 ) {
                        menuSel -= 3;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case CONTROLS_MENU: break;
                default:
                    if( menuSel > 0 ) {
                        menuSel--;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                }
                break;
            }
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN: {
                switch( menuNum ) {
                default: break;
                case MAIN_MENU:
                    if( menuSel < 4 ) {
                        menuSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case NEW_RUN_MENU:
                    if( menuSel < 2 ) {
                        menuSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case TUTORIAL_MENU:
                    if( menuSel <= 4 ) {
                        menuSel += 3;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case OPTIONS_MENU:
                    if( menuSel < 3 ) {
                        menuSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                }
                break;
            }
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT: {
                switch( menuNum ) {
                default: break;
                case NEW_RUN_MENU:
                    if( menuSel == 0 && charSel > 0 ) {
                        charSel--;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 1 && gameDiffSel > 0 ) {
                        gameDiffSel--;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case TUTORIAL_MENU:
                    if( menuSel > 0 ) {
                        menuSel--;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case OPTIONS_MENU:
                    if( menuSel == 0 ) {
                        showInfobox = !showInfobox;
                        showInfobox ? Mix_PlayChannel( 7, infoboxOpenSound, 0 ) : Mix_PlayChannel( 7, infoboxCloseSound, 0 );
                    }
                    else if( menuSel == 1 && gameVol > 0 ) {
                        gameVol--;
                        Mix_Volume( 0, gameVol * mainVolScale );
                        Mix_Volume( 1, gameVol * miscVolScale );
                        Mix_Volume( 2, gameVol * miscVolScale );
                        Mix_Volume( 4, gameVol * miscVolScale );
                        Mix_Volume( 5, gameVol * miscVolScale );
                        Mix_Volume( 6, gameVol * mainVolScale );
                        Mix_Volume( 7, gameVol * miscVolScale );
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 2 && musicVol > 0 ) {
                        musicVol--;
                        Mix_Volume( 3, musicVol * musicVolScale );
                        if( musicVol == 0 ) Mix_Pause(3);
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 3 && musicSel > 0 ) {
                        musicSel--;
                        if( musicVol != 0 ) playMusic( musicSel );
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                }
                break;
            }
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT: {
                switch( menuNum ) {
                default: break;
                case NEW_RUN_MENU:
                    if( menuSel == 0 && charSel < NUM_CHAR_TYPES-1 ) {
                        charSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 1 && gameDiffSel < 4 ) {
                        gameDiffSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case TUTORIAL_MENU:
                    if( menuSel < 7 ) {
                        menuSel++;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                case OPTIONS_MENU:
                    if( menuSel == 0 ) {
                        showInfobox = !showInfobox;
                        showInfobox ? Mix_PlayChannel( 7, infoboxOpenSound, 0 ) : Mix_PlayChannel( 7, infoboxCloseSound, 0 );
                    }
                    else if( menuSel == 1 && gameVol < 10 ) {
                        gameVol++;
                        Mix_Volume( 0, gameVol * mainVolScale );
                        Mix_Volume( 1, gameVol * miscVolScale );
                        Mix_Volume( 2, gameVol * miscVolScale );
                        Mix_Volume( 4, gameVol * miscVolScale );
                        Mix_Volume( 5, gameVol * miscVolScale );
                        Mix_Volume( 6, gameVol * mainVolScale );
                        Mix_Volume( 7, gameVol * miscVolScale );
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 2 && musicVol < 10 ) {
                        musicVol++;
                        Mix_Volume( 3, musicVol * musicVolScale );
                        if( musicVol == 1 ) playMusic( musicSel );
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    else if( menuSel == 3 && musicSel < 17 ) {
                        musicSel++;
                        if( musicVol != 0 ) playMusic( musicSel );
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                    }
                    break;
                }
                break;
            }
            case SDL_SCANCODE_SPACE: {
                // If not in a Menu, make the Player turn
                // If in a Menu, Space is a Menu Confirm
                if( menuNum == -1 && player1->animationDisplayAmt <= 0 && player1->hp > 0 ) {
                    face( player1, player1->facing * -1 );
                    break;
                }
            }
            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_KP_ENTER: {
                if( menuNum == -1 ) return;
                player1->animationDisplayAmt = menuExitTime;
                switch( menuNum ) {
                case MAIN_MENU:
                    if( menuSel == EXIT-1 ) {
                        Mix_PlayChannel( 7, menuCancelSound, 0 );
                        if( level != 0 ) {
                            level = 0;
                            menuSel = 0;
                        }
                        else done = true;
                    }
                    else {
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                        menuNum = menuSel+1;
                        menuSel = 0;
                        charSel = player1->type;
                        gameDiffSel = currentGameDiff;
                    }
                    break;
                case NEW_RUN_MENU:
                    if( menuSel != 2 ) {
                        menuSel = 2;
                        Mix_PlayChannel( 7, menuChooseSound, 0 );
                        break;
                    }
                    menuNum = -1;
                    reset();
                    switch( charSel ) {
                    default:
                    case MEGAMAN:       player1 = new MegaMan(); break;
                    case PROTOMAN:      player1 = new ProtoMan(); break;
                    case TOMAHAWKMAN:   player1 = new TomahawkMan(); break;
                    case COLONEL:       player1 = new Colonel(); break;
                    case SLASHMAN:      player1 = new SlashMan(); break;
                    case GUTSMAN:       player1 = new GutsMan(); break;
                    }
                    player1->npc = false;
                    currentGameDiff = gameDiffSel;
                    level = 1;
                    loadLevel();
                    Mix_PlayChannel( 7, menuChooseSound, 0 );
                    break;
                case TUTORIAL_MENU:
                    menuNum = -1;
                    tutorial( menuSel+1 );
                    Mix_PlayChannel( 7, menuChooseSound, 0 );
                    break;
                default:
                    menuSel = menuNum-1;
                    menuNum = MAIN_MENU;
                    Mix_PlayChannel( 7, menuCancelSound, 0 );
                    break;
                }
                break;
            }
            case SDL_SCANCODE_LSHIFT: {
                if( menuNum == -1 && player1->animationDisplayAmt <= 0 && player1->hp > 0 )
                    face( player1, player1->facing * -1 );
                break;
            }
            // case SDL_SCANCODE_X: debugTest(); break;
            }
        }
    }

    // Player Movement and Attacks
    if( menuNum == -1 && player1->animationDisplayAmt <= 0 && player1->hp > 0 ) {
        // The player can't Move or Attack until after the previous Action is completed, or if a Menu is open

        const Uint8* keystates = SDL_GetKeyboardState( NULL );		// Read Multiple Button presses simultaneously

        // Move Up
        if     ( keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP] )    move( player1, 0 );
        // Move Left
        else if( keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT] )  move( player1, 1 );
        // Move Down
        else if( keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN] )  move( player1, 2 );
        // Move Right
        else if( keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT] ) move( player1, 3 );

        // Sword Attacks
        else if( keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1] ) attack( player1, 1 );
        else if( keystates[SDL_SCANCODE_2] || keystates[SDL_SCANCODE_KP_2] ) attack( player1, 2 );
        else if( keystates[SDL_SCANCODE_3] || keystates[SDL_SCANCODE_KP_3] ) attack( player1, 3 );
        else if( keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4] ) attack( player1, 4 );
        else if( keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5] ) attack( player1, 5 );
        else if( keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6] ) attack( player1, 6 );
        else if( keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7] ) attack( player1, 7 );
    }
}

void App::playMusic( int track ) {
    musicSwitchDisplayAmt = 1.5;

    switch ( musicSel ) {
    case 0: Mix_PlayChannel( 3, track01, -1 ); break;
    case 1: Mix_PlayChannel( 3, track02, -1 ); break;
    case 2: Mix_PlayChannel( 3, track03, -1 ); break;
    case 3: Mix_PlayChannel( 3, track04, -1 ); break;
    case 4: Mix_PlayChannel( 3, track05, -1 ); break;
    case 5: Mix_PlayChannel( 3, track06, -1 ); break;
    case 6: Mix_PlayChannel( 3, track07, -1 ); break;
    case 7: Mix_PlayChannel( 3, track08, -1 ); break;
    case 8: Mix_PlayChannel( 3, track09, -1 ); break;
    case 9: Mix_PlayChannel( 3, track10, -1 ); break;
    case 10: Mix_PlayChannel( 3, track11, -1 ); break;
    case 11: Mix_PlayChannel( 3, track12, -1 ); break;
    case 12: Mix_PlayChannel( 3, track13, -1 ); break;
    case 13: Mix_PlayChannel( 3, track14, -1 ); break;
    case 14: Mix_PlayChannel( 3, track15, -1 ); break;
    case 15: Mix_PlayChannel( 3, track16, -1 ); break;
    case 16: Mix_PlayChannel( 3, track17, -1 ); break;
    case 17: Mix_PlayChannel( 3, track18, -1 ); break;
    }
}

// Display Functions
void App::Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

    if( level == 0 ) {
        drawBg();
        drawMenu();
        SDL_GL_SwapWindow( displayWindow );
        return;
    }

	drawBg();
	drawFloor();

    drawOverhead();
    if( showInfobox ) {
        drawCharMoveset();
        drawInfobox();
    }
    if( musicSwitchDisplayAmt || menuNum != -1 ) { displayMusic(); }
    drawTextUI();

	// Draw game elements, back row first, so the front row is displayed in front
    for ( int i = 5; i >= 0; i-- ) {
        for( int j = 0; j < npcList.size(); j++ ) {
            if( i == npcList[j]->y ) drawPlayer( npcList[j] );
        }
        drawItems( i );
        drawRocks( i );
        drawItemUpgrading( i );
        if ( i == player1->y ) { drawPlayer( player1 ); }
    }

    for ( int i = 0; i < npcList.size(); i++ ) {
        drawSwordAtks( npcList[i] );
        drawHp( npcList[i] );
   }
    drawSwordAtks( player1 );
    drawExtraAtks();
    drawHp( player1 );
    drawPrevEnergy();
    
    if( menuNum != -1 ) {
        drawDimScreen();
        drawMenu();
    }

	SDL_GL_SwapWindow(displayWindow);
}

void App::drawBg() {
    if( level == 0 ) {
        float menuSizeX = 1;
        float menuSizeY = 1;
        GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                             menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
        drawSpriteSheetSprite( place, bgMain, 0, 1, 1 );
        return;
    }

	GLuint texture;
	float bgSizeX = 0.768 * 8 * uiScaleX;
	float bgSizeY = 0.768 * 8 * uiScaleY;
	if      ( lvlDiff == 0 ) { texture = bgA; }
	else if ( lvlDiff == 1 ) { texture = bgB; }
	else if ( lvlDiff == 2 ) { texture = bgC; }
    else if ( lvlDiff == 3 ) { texture = bgD; }
	glTranslatef(0.064 * 2 * uiScaleX * bgAnimationAmt, -0.064 * 2 * uiScaleY * bgAnimationAmt, 0);
	GLfloat place[] = { -bgSizeX, bgSizeY, -bgSizeX, -bgSizeY,     bgSizeX, -bgSizeY, bgSizeX, bgSizeY };
	drawSpriteSheetSprite(place, texture, 0, 1, 1);
}
void App::drawDimScreen() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    drawSpriteSheetSprite( place, dimScreenPic, 0, 1, 1 );
}

void App::drawMenu() {
    switch( menuNum ) {
    case MAIN_MENU: drawMainMenu(); break;
    case NEW_RUN_MENU: drawNewRunMenu(); break;
    case TUTORIAL_MENU: drawTutorialMenu(); break;
    case CONTROLS_MENU: drawControlsMenu(); break;
    case OPTIONS_MENU: drawOptionsMenu(); break;
    }
}
void App::drawMainMenu() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, mainMenuPic, 0, 1, 1 );
    switch( menuSel ) {
    case 0: drawSpriteSheetSprite( place, mainSelPic0, 0, 1, 1 ); break;
    case 1: drawSpriteSheetSprite( place, mainSelPic1, 0, 1, 1 ); break;
    case 2: drawSpriteSheetSprite( place, mainSelPic2, 0, 1, 1 ); break;
    case 3: drawSpriteSheetSprite( place, mainSelPic3, 0, 1, 1 ); break;
    case 4: drawSpriteSheetSprite( place, mainSelPic4, 0, 1, 1 ); break;
    }
}
void App::drawNewRunMenu() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, newRunMenuPic, 0, 1, 1 );
    switch( menuSel ) {
    case 0: drawSpriteSheetSprite( place, newRunSelPic0, 0, 1, 1 ); break;
    case 1: drawSpriteSheetSprite( place, newRunSelPic1, 0, 1, 1 ); break;
    case 2: drawSpriteSheetSprite( place, newRunSelPic2, 0, 1, 1 ); break;
    }
    switch( charSel ) {
        case MEGAMAN: {
            drawSpriteSheetSprite( place, charSelPic0, 0, 1, 1 );
            // drawSpriteSheetSprite( place, chipPic0, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic0, 0, 1, 1 );
            break;
        }
        case PROTOMAN: {
            drawSpriteSheetSprite( place, charSelPic1, 0, 1, 1 );
            // drawSpriteSheetSprite( place, chipPic1, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic1, 0, 1, 1 );
            break;
        }
        case TOMAHAWKMAN: {
            drawSpriteSheetSprite( place, charSelPic2, 0, 1, 1 );
            // drawSpriteSheetSprite( place, chipPic2, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic2, 0, 1, 1 );
            break;
        }
        case COLONEL: {
            drawSpriteSheetSprite( place, charSelPic3, 0, 1, 1 );
            // drawSpriteSheetSprite( place, chipPic3, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic3, 0, 1, 1 );
            break;
        }
        case SLASHMAN: {
            drawSpriteSheetSprite( place, charSelPic4, 0, 1, 1 );
            // drawSpriteSheetSprite( place, chipPic4, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic4, 0, 1, 1 );
            break;
        }
        case GUTSMAN: {
            drawSpriteSheetSprite( place, charSelPic5, 0, 1, 1 );
            drawSpriteSheetSprite( place, movesetPic5, 0, 1, 1 );
        }
    }
    switch( gameDiffSel ) {
    case 0: drawSpriteSheetSprite( place, diffSelPic0, 0, 1, 1 ); break;
    case 1: drawSpriteSheetSprite( place, diffSelPic1, 0, 1, 1 ); break;
    case 2: drawSpriteSheetSprite( place, diffSelPic2, 0, 1, 1 ); break;
    case 3: drawSpriteSheetSprite( place, diffSelPic3, 0, 1, 1 ); break;
    case 4: drawSpriteSheetSprite( place, diffSelPic4, 0, 1, 1 ); break;
    }
}
void App::drawCharMoveset() {
	glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    switch( player1->type ) {
    case MEGAMAN:       drawSpriteSheetSprite( place, movesetPic0, 0, 1, 1 ); break;
    case PROTOMAN:      drawSpriteSheetSprite( place, movesetPic1, 0, 1, 1 ); break;
    case TOMAHAWKMAN:   drawSpriteSheetSprite( place, movesetPic2, 0, 1, 1 ); break;
    case COLONEL:       drawSpriteSheetSprite( place, movesetPic3, 0, 1, 1 ); break;
    case SLASHMAN:      drawSpriteSheetSprite( place, movesetPic4, 0, 1, 1 ); break;
    case GUTSMAN:       drawSpriteSheetSprite( place, movesetPic5, 0, 1, 1 ); break;
    }
}
void App::drawInfobox() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, generalInfoboxPic, 0, 1, 1 );
    if( level < 0 ) drawSpriteSheetSprite( place, tInfoboxPic, 0, 1, 1 );
    else return;

    GLuint texture;
    switch( level ) {
    case -1: texture = tInfoPic0; break;
    case -2: texture = tInfoPic1; break;
    case -3: texture = tInfoPic2; break;
    case -4: texture = tInfoPic3; break;
    case -5: texture = tInfoPic4; break;
    case -6: texture = tInfoPic5; break;
    case -7: texture = tInfoPic6; break;
    default:
    case -8: texture = tInfoPic7; break;
    }
    drawSpriteSheetSprite( place, texture, 0, 1, 1 );
}
void App::drawTutorialMenu() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, tutorialMenuPic, 0, 1, 1 );
    switch( menuSel ) {
    case 0: drawSpriteSheetSprite( place, tSelPic0, 0, 1, 1 ); break;
    case 1: drawSpriteSheetSprite( place, tSelPic1, 0, 1, 1 ); break;
    case 2: drawSpriteSheetSprite( place, tSelPic2, 0, 1, 1 ); break;
    case 3: drawSpriteSheetSprite( place, tSelPic3, 0, 1, 1 ); break;
    case 4: drawSpriteSheetSprite( place, tSelPic4, 0, 1, 1 ); break;
    case 5: drawSpriteSheetSprite( place, tSelPic5, 0, 1, 1 ); break;
    case 6: drawSpriteSheetSprite( place, tSelPic6, 0, 1, 1 ); break;
    case 7: drawSpriteSheetSprite( place, tSelPic7, 0, 1, 1 ); break;
    }
}
void App::drawControlsMenu() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, controlMenuPic, 0, 1, 1 );
}
void App::drawOptionsMenu() {
    glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, optionMenuPic, 0, 1, 1 );
    switch( menuSel ) {
    case 0: drawSpriteSheetSprite( place, oSelPic0, 0, 1, 1 ); break;
    case 1: drawSpriteSheetSprite( place, oSelPic1, 0, 1, 1 ); break;
    case 2: drawSpriteSheetSprite( place, oSelPic2, 0, 1, 1 ); break;
    case 3: drawSpriteSheetSprite( place, oSelPic3, 0, 1, 1 ); break;
    }
    drawSpriteSheetSprite( place, showInfobox ? infoSelPic0 : infoSelPic1, 0, 1, 1 );
    switch( gameVol ) {
    case 0:  drawSpriteSheetSprite( place, gameVolSelPic0, 0, 1, 1 ); break;
    case 1:  drawSpriteSheetSprite( place, gameVolSelPic1, 0, 1, 1 ); break;
    case 2:  drawSpriteSheetSprite( place, gameVolSelPic2, 0, 1, 1 ); break;
    case 3:  drawSpriteSheetSprite( place, gameVolSelPic3, 0, 1, 1 ); break;
    case 4:  drawSpriteSheetSprite( place, gameVolSelPic4, 0, 1, 1 ); break;
    case 5:  drawSpriteSheetSprite( place, gameVolSelPic5, 0, 1, 1 ); break;
    case 6:  drawSpriteSheetSprite( place, gameVolSelPic6, 0, 1, 1 ); break;
    case 7:  drawSpriteSheetSprite( place, gameVolSelPic7, 0, 1, 1 ); break;
    case 8:  drawSpriteSheetSprite( place, gameVolSelPic8, 0, 1, 1 ); break;
    case 9:  drawSpriteSheetSprite( place, gameVolSelPic9, 0, 1, 1 ); break;
    case 10: drawSpriteSheetSprite( place, gameVolSelPic10, 0, 1, 1 ); break;
    }
    switch( musicVol ) {
    case 0:  drawSpriteSheetSprite( place, musicVolSelPic0, 0, 1, 1 ); break;
    case 1:  drawSpriteSheetSprite( place, musicVolSelPic1, 0, 1, 1 ); break;
    case 2:  drawSpriteSheetSprite( place, musicVolSelPic2, 0, 1, 1 ); break;
    case 3:  drawSpriteSheetSprite( place, musicVolSelPic3, 0, 1, 1 ); break;
    case 4:  drawSpriteSheetSprite( place, musicVolSelPic4, 0, 1, 1 ); break;
    case 5:  drawSpriteSheetSprite( place, musicVolSelPic5, 0, 1, 1 ); break;
    case 6:  drawSpriteSheetSprite( place, musicVolSelPic6, 0, 1, 1 ); break;
    case 7:  drawSpriteSheetSprite( place, musicVolSelPic7, 0, 1, 1 ); break;
    case 8:  drawSpriteSheetSprite( place, musicVolSelPic8, 0, 1, 1 ); break;
    case 9:  drawSpriteSheetSprite( place, musicVolSelPic9, 0, 1, 1 ); break;
    case 10: drawSpriteSheetSprite( place, musicVolSelPic10, 0, 1, 1 ); break;
    }
    switch( musicSel ) {
    case 0:  drawSpriteSheetSprite( place, musicSelPic1, 0, 1, 1 ); break;
    case 1:  drawSpriteSheetSprite( place, musicSelPic2, 0, 1, 1 ); break;
    case 2:  drawSpriteSheetSprite( place, musicSelPic3, 0, 1, 1 ); break;
    case 3:  drawSpriteSheetSprite( place, musicSelPic4, 0, 1, 1 ); break;
    case 4:  drawSpriteSheetSprite( place, musicSelPic5, 0, 1, 1 ); break;
    case 5:  drawSpriteSheetSprite( place, musicSelPic6, 0, 1, 1 ); break;
    case 6:  drawSpriteSheetSprite( place, musicSelPic7, 0, 1, 1 ); break;
    case 7:  drawSpriteSheetSprite( place, musicSelPic8, 0, 1, 1 ); break;
    case 8:  drawSpriteSheetSprite( place, musicSelPic9, 0, 1, 1 ); break;
    case 9:  drawSpriteSheetSprite( place, musicSelPic10, 0, 1, 1 ); break;
    case 10: drawSpriteSheetSprite( place, musicSelPic11, 0, 1, 1 ); break;
    case 11: drawSpriteSheetSprite( place, musicSelPic12, 0, 1, 1 ); break;
    case 12: drawSpriteSheetSprite( place, musicSelPic13, 0, 1, 1 ); break;
    case 13: drawSpriteSheetSprite( place, musicSelPic14, 0, 1, 1 ); break;
    case 14: drawSpriteSheetSprite( place, musicSelPic15, 0, 1, 1 ); break;
    case 15: drawSpriteSheetSprite( place, musicSelPic16, 0, 1, 1 ); break;
    case 16: drawSpriteSheetSprite( place, musicSelPic17, 0, 1, 1 ); break;
    case 17: drawSpriteSheetSprite( place, musicSelPic18, 0, 1, 1 ); break;
    }
}

void App::drawOverhead() {
    glLoadIdentity();
    glTranslatef( 0, 0.945, 0 );
    float sizeX = 1.44 * uiScaleX;
    float sizeY = 0.15 * uiScaleY;
    GLfloat place[] = { -sizeX, +sizeY, -sizeX, -sizeY,
                        +sizeX, -sizeY, +sizeX, +sizeY };
    if      ( itemAnimationAmt <= 2.0 ) { drawSpriteSheetSprite( place, overheadSheet, 0, 1, 4 ); }
    else if ( itemAnimationAmt <= 4.0 ) { drawSpriteSheetSprite( place, overheadSheet, 1, 1, 4 ); }
    else if ( itemAnimationAmt <= 6.0 ) { drawSpriteSheetSprite( place, overheadSheet, 2, 1, 4 ); }
    else if ( itemAnimationAmt <= 8.0 ) { drawSpriteSheetSprite( place, overheadSheet, 3, 1, 4 ); }
}
void App::drawTextUI() {
	// Display current Level number
	glLoadIdentity();
	glTranslatef(0.895, 0.95, 0.0);
	if (level >= 10)   { glTranslatef(-0.04, 0.0, 0.0); }
	if (level >= 100)  { glTranslatef(-0.04, 0.0, 0.0); }
	if (level >= 1000) { glTranslatef(-0.04, 0.0, 0.0); }
	float barX = 0.23;
	float barY = -0.0125;
	float barSizeX = 1.20 * uiScaleX;
	float barSizeY = 0.12 * uiScaleY;
	GLfloat barPlace[] = { barX - barSizeX, barY + barSizeY, barX - barSizeX, barY - barSizeY,
	                       barX + barSizeX, barY - barSizeY, barX + barSizeX, barY + barSizeY };
	drawSpriteSheetSprite(barPlace, lvBarPic, 0, 1, 1);
	glTranslatef(0, -0.005, 0);
	drawText(textSheetWhite, "Lv" + to_string(level), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, -0.00);

	// Draw Energy Amount
	glLoadIdentity();
	glTranslatef(-0.889, 0.942, 0);
	float rockX = 0.0;
	float rockY = 0.0;
	float rockSizeX = 0.44 * uiScaleX;
	float rockSizeY = 0.16 * uiScaleY;
	GLfloat rockPlace[] = { rockX - rockSizeX, rockY + rockSizeY, rockX - rockSizeX, rockY - rockSizeY,
	                       rockX + rockSizeX, rockY - rockSizeY, rockX + rockSizeX, rockY + rockSizeY };
	drawSpriteSheetSprite(rockPlace, healthBoxPic, 0, 1, 1);
	glTranslatef(0.061, 0.0075, 0);
	
    int energyToDisplay = player1->energyDisplayed;
    if ( player1->hp <= 0 )            energyToDisplay = 0;
	else if ( energyToDisplay < 1 )    energyToDisplay = 1;
	else if ( energyToDisplay > 9999 ) energyToDisplay = 9999;

    if ( energyToDisplay >= 10 )   glTranslatef(-0.04, 0, 0);
	if ( energyToDisplay >= 100 )  glTranslatef(-0.04, 0, 0);
	if ( energyToDisplay >= 1000 ) glTranslatef(-0.04, 0, 0);
	if      (chargeDisplayPlusAmt  > 0) 
        drawText(textSheetGreen2, to_string( energyToDisplay ), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0);
	else if (chargeDisplayMinusAmt > 0 || energyToDisplay <= 1)
        drawText(textSheetRed2,   to_string( energyToDisplay ), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0);
    else
        drawText(textSheetWhite2, to_string( energyToDisplay ), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0);

	// Draw Current Energy gain for this level
	/*
    glLoadIdentity();
	glTranslatef(-0.828, 0.8495, 0);
	if (abs( player1->energyDisplayed2 ) >= 10)   { glTranslatef(-0.04, 0, 0); }
	if (abs( player1->energyDisplayed2 ) >= 100)  { glTranslatef(-0.04, 0, 0); }
	if (abs( player1->energyDisplayed2 ) >= 1000) { glTranslatef(-0.04, 0, 0); }
	if (level > 0) {
        if      ( player1->energyDisplayed2 > 0 ) {
            drawText( textSheet1B, to_string( abs( player1->energyDisplayed2 ) ), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0 ); }
        else if ( player1->energyDisplayed2 < 0 ) {
            drawText( textSheet1C, to_string( abs( player1->energyDisplayed2 ) ), 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0 ); }
	}
    */

	// Display Sword Name and Cost
	glLoadIdentity();
	glTranslatef(-0.97, -0.93, 0);
	string text = "";
	GLuint texture;

    // Previous Energy Gain and Sword Attack Text Display
    switch( player1->actionNumber ) {
    case -1: return;
    case -2:
        texture = textSheetGreen;
        text = "Energy" + to_string( energyGainAmt );
        break;
    case -3:
        texture = textSheetGreen;
        text = "Energy" + to_string( energyGainAmt2 );
        break;
    case -4:
        texture = textSheetRed;
        text = "Energy" + to_string( energyLossAmt );
        break;
    default:
        texture = textSheetRed;
        text = player1->getAtkName() + to_string( player1->lastAtkCost );
        break;
    }
    drawText( texture, text, 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0 );
}
void App::displayMusic() {
    string text;
    if( musicVol == 0 ) { text = "Music Off"; }
    else {
        switch ( musicSel ) {
        case 0:  text = "01 Organization";      break;
        case 1:  text = "02 An Incident";       break;
        case 2:  text = "03 Blast Speed";       break;
        case 3:  text = "04 Shark Panic";       break;
        case 4:  text = "05 Battle Field";      break;
        case 5:  text = "06 Doubt";             break;
        case 6:  text = "07 Distortion";        break;
        case 7:  text = "08 Surge of Power";    break;
        case 8:  text = "09 Digital Strider";   break;
        case 9:  text = "10 Break the Storm";   break;
        case 10: text = "11 Evil Spirit";       break;
        case 11: text = "12 Hero";              break;
        case 12: text = "13 Danger Zone";       break;
        case 13: text = "14 Navi Customizer";   break;
        case 14: text = "15 Graveyard";         break;
        case 15: text = "16 The Count";         break;
        case 16: text = "17 Secret Base";       break;
        case 17: text = "18 Cybeasts";          break;
        }
    }
    glLoadIdentity();
    float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX,  menuSizeY, -menuSizeX, -menuSizeY,
                         menuSizeX, -menuSizeY,  menuSizeX,  menuSizeY };
    drawSpriteSheetSprite( place, musicDisplayBox, 0, 1, 1 );

    glLoadIdentity();
    glTranslatef( 0.30, -0.95, 0 );
    drawText( textSheetWhite, text, 0.08 * 2 * uiScaleX, 0.16 * 2 * uiScaleY, 0 );
}
void App::drawPrevEnergy() {
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 0.8, 0.0 );

    GLuint texture = energyGetSheet;
    float sizeX = 0.43 * 0.75;
    float sizeY = 0.18 * 0.75;
    for( int i = 0; i < delayedPrevEnergyList.size(); i++ ) {
        int xPos = delayedPrevEnergyList[i].x;
        int yPos = delayedPrevEnergyList[i].y;

        //glTranslatef( 0.5 + xPos * scaleX, 0.7 + yPos * scaleY + 0.16 * 1.5, 0.0 );

        int amtToShow = delayedPrevEnergyList[i].amt;
        /*glTranslatef( -0.08 * 1.5 / 2, 0, 0 );
        if( amtToShow >= 10 )   { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
        if( amtToShow >= 100 )  { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
        if( amtToShow >= 1000 ) { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }*/

        int picIndex = 0;
        switch( delayedPrevEnergyList[i].type ) {
        default:
        case 0: continue;
        case 1:
            if( amtToShow < 0 ) picIndex = 3;
            else if( amtToShow == energyGainAmt ) picIndex = 0;
            else if( amtToShow == energyGainAmt2 ) picIndex = 1;
            break;
        case 2: picIndex = 2; break;
        case 3:
            if( amtToShow == -npcDmgAmt ) picIndex = 4;
            else if( amtToShow == -2 * npcDmgAmt ) picIndex = 5;
            else if( amtToShow == -5 * npcDmgAmt ) picIndex = 6;
            break;
        }

        float timer = delayedPrevEnergyList[i].animationTimer;
        float displace = ( iconDisplayTime - timer );
        float alpha = ( timer >= iconDisplayTime ? 1 : 2 * timer / iconDisplayTime );
        glTranslatef( 0, displace, 0 );
        GLfloat place[] = { xPos * scaleX - sizeX, yPos * scaleY + sizeY, xPos * scaleX - sizeX, yPos * scaleY - sizeY,
	                        xPos * scaleX + sizeX, yPos * scaleY - sizeY, xPos * scaleX + sizeX, yPos * scaleY + sizeY };
        drawSpriteSheetSprite( place, texture, picIndex, 7, 1 );
        glTranslatef( 0, -displace, 0 );
    }
}

void App::drawPlayer(Player * const player) {
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);
	
    player->xDisplay = player->x;
    player->yDisplay = player->y;

    float playerSizeX, playerSizeY;
	float displace = 0.0;                       // Movement Offset - texture displacement when doing movement actions
	float displaceX = 0.0, displaceY = 0.0;     // Texture Offset  - displacement to make sure the character texture is correctly positioned

    GLuint texture;
    int textureSheetWidth = 4;
    int picIndex = 0;

    switch( player->type ) {
    default:
    case MEGAMAN: {
        playerSizeX = 0.35 * playerScale;
		playerSizeY = 0.54 * playerScale;
        displaceX = 0;
        displaceY = 0;
        texture = ( player->npc ? darkMegamanMoveSheet : megamanMoveSheet );
        break;
    }
    case PROTOMAN: {
        playerSizeX = 0.53 * playerScale;
        playerSizeY = 0.59 * playerScale;
        displaceX = 0.08 * playerScale;
        displaceY = 0.05 * playerScale;
        texture = ( player->npc ? darkProtoMoveSheet : protoMoveSheet );
        break;
    }
    case TOMAHAWKMAN: {
        playerSizeX = 0.52 * playerScale;
        playerSizeY = 0.60 * playerScale;
        displaceX = 0.2 * playerScale;
        displaceY = 0.05 * playerScale;
        texture = ( player->npc ? darkTmanMoveSheet : tmanMoveSheet );
        break;
    }
    case COLONEL: {
        playerSizeX = 0.61 * playerScale;
        playerSizeY = 0.63 * playerScale;
        displaceX = -0.1 * playerScale;
        displaceY = 0.04 * playerScale;
        texture = ( player->npc ? darkColonelMoveSheet : colonelMoveSheet );
        break;
    }
    case SLASHMAN: {
        playerSizeX = 0.66 * playerScale;
        playerSizeY = 0.56 * playerScale;
        displaceX = 0.2 * playerScale;
        displaceY = 0.0 * playerScale;
        texture = ( player->npc ? darkSlashmanMoveSheet : slashmanMoveSheet );
        break;
    }
    case GUTSMAN: {
        playerSizeX = 0.59 * playerScale;
        playerSizeY = 0.64 * playerScale;
        displaceX = 0.15 * playerScale;
        displaceY = 0.0 * playerScale;
        texture = ( player->npc ? darkGutsMoveSheet : gutsMoveSheet );
        break;
    }
    }

    bool draw = true;
    int randDraw = ceil( player->hurtDisplayAmt * 100.0 );
    if( ( player->hurtDisplayAmt > 0 ) && ( randDraw % 2 ) ) draw = false;

    // Death Animation
    if( player->type != TOMAHAWKMAN && player->type != SLASHMAN && player->hp <= 0 && player->hp > -10 && player->deathDisplayAmt > 0 ) {
        int textureSheetWidth = 2;
        switch( player->type ) {
        case MEGAMAN: {
            playerSizeX = 0.40 * playerScale;
            playerSizeY = 0.48 * playerScale;
            displaceX = 0.014;
            displaceY = -0.06 * playerScale;
            texture = ( player->npc ? darkMegamanHurtSheet : megamanHurtSheet );
            break;
        }
        case PROTOMAN: {
            playerSizeX = 0.70 * playerScale;
		    playerSizeY = 0.49 * playerScale;
            displaceX = -0.01 * playerScale;
            displaceY = -0.05 * playerScale;
            texture = ( player->npc ? darkProtoHurtSheet : protoHurtSheet );
            break;
        }
        case COLONEL: {
            playerSizeX = 0.65 * playerScale;
            playerSizeY = 0.67 * playerScale;
            displaceX = -0.184 * playerScale;
            displaceY = 0.038 * playerScale;
            texture = ( player->npc ? darkColonelHurtSheet : colonelHurtSheet );
            break;
        }
        case GUTSMAN: {
            playerSizeX = 0.48 * playerScale;
            playerSizeY = 0.64 * playerScale;
            displaceX = 0.02 * playerScale;
            displaceY = 0.0545 * playerScale;
            textureSheetWidth = 1;
            texture = ( player->npc ? darkGutsHurtSheet : gutsHurtSheet );
            break;
        }
        }

        glTranslatef( player->facing * displaceX, displaceY, 0 );
        if( player->facing == -1 ) picIndex += textureSheetWidth;

        GLfloat playerPlace[] = { player->x  * scaleX - playerSizeX, player->y * scaleY + playerSizeY,
                                  player->x  * scaleX - playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x  * scaleX + playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x  * scaleX + playerSizeX, player->y * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
	// Step Sword Move Animation
	else if( player->animationType == 1 && player->moveType == 3 && player->animationDisplayAmt > 0 ) {
        if( !draw ) return;

        int textureSheetWidth = 8;
        switch( player->type ) {
        case MEGAMAN: {
            playerSizeX = 0.66 * playerScale;
            playerSizeY = 0.56 * playerScale;
            displaceX = 0.3;
            displaceY = 0.02;
            texture = ( player->npc ? darkMegamanAtkSheet : megamanAtkSheet );
            break;
        }
        case PROTOMAN: {
            playerSizeX = 0.78 * playerScale;
            playerSizeY = 0.64 * playerScale;
            displaceX = 0.22;
            displaceY = 0.08 * playerScale + 0.02;
            texture = ( player->npc ? darkProtoAtkSheet : protoAtkSheet );
            break;
        }
        case COLONEL: {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            displaceX = 0.3 * playerScale;
            displaceY = 0.15 * playerScale + 0.02;
            texture = ( player->npc ? darkColonelAtkSheet : colonelAtkSheet );
            break;
        }
        case SLASHMAN: {
            playerSizeX = 0.94 * playerScale;
            playerSizeY = 0.88 * playerScale;
            displaceX = 0.20;
            displaceY = 0.065 * playerScale + 0.02;
            textureSheetWidth = 9;
            texture = ( player->npc ? darkSlashmanAtkSheet : slashmanAtkSheet );
            break;
        }
        case GUTSMAN: {
            playerSizeX = 0.97 * playerScale;
            playerSizeY = 0.53  * playerScale;
            displaceX = 0.27 * playerScale;
            displaceY = -0.03 * playerScale;
            textureSheetWidth = 5;
            texture = ( player->npc ? darkGutsAtkSheet1 : gutsAtkSheet1 );
            break;
        }
        }
        glTranslatef( player->facing * displaceX, displaceY, 0 );

        // Displacement of Animation - Slow start, then speed to up to attack when Step-Sword Attacking
            // displace: 2.0 -> 1.9    40% of total movement time
        if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.60 ) {
            displace = 2.0 - 0.1 * ( stepAtkTime + moveAnimationTime - player->animationDisplayAmt ) / 0.4 / moveAnimationTime; }
            // displace: 1.9 -> 0.0    50% of total movement time
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.10 ) {
            displace = 1.9 - 1.9 * ( stepAtkTime + 0.6 * moveAnimationTime - player->animationDisplayAmt ) / 0.5 / moveAnimationTime; }

        if( player->type == SLASHMAN ) {
            if( ( player->energy / 100 ) % 2 ) {
                if     ( player->animationDisplayAmt > stepAtkTime - 0.06 ) picIndex = 0;
                else if( player->animationDisplayAmt > stepAtkTime - 0.18 ) picIndex = 1;
                else                                                        picIndex = 2;
            }
            else {
                if     ( player->animationDisplayAmt > stepAtkTime - 0.06 ) picIndex = 2;
                else if( player->animationDisplayAmt > stepAtkTime - 0.18 ) picIndex = 3;
                else                                                        picIndex = 4;
            }
        }
        else if( player->type == GUTSMAN ) {
            if     ( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.80 )     picIndex = 0;
            else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.60 )     picIndex = 1;
            else if( player->animationDisplayAmt > stepAtkTime )                                picIndex = 2;
            else if( player->animationDisplayAmt > stepAtkTime - 0.40 )                         picIndex = 3;
            else                                                                                picIndex = 4;
        }
        else {
            if     ( player->animationDisplayAmt > stepAtkTime - 0.06 ) picIndex = 1;
            else if( player->animationDisplayAmt > stepAtkTime - 0.10 ) picIndex = 2;
            else if( player->animationDisplayAmt > stepAtkTime - 0.14 ) picIndex = 3;
            else if( player->animationDisplayAmt > stepAtkTime - 0.20 ) picIndex = 4;
            else if( player->animationDisplayAmt > stepAtkTime - 0.26 ) picIndex = 5;
            else if( player->animationDisplayAmt > stepAtkTime - 0.32 ) picIndex = 6;
            else                                                        picIndex = 7;
        }

        player->xDisplay = (float) player->x + (float) player->facing * -displace;
        player->yDisplay = player->y;

        // Draw Step Sword Attack Shadows
        int numShadows = 0;
        if     ( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.60 ) numShadows = 0;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.50 ) numShadows = 1;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.40 ) numShadows = 2;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.30 ) numShadows = 3;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.25 ) numShadows = 4;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.20 ) numShadows = 5;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.15 ) numShadows = 6;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.10 ) numShadows = 7;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.05 ) numShadows = 7;
        else if( player->animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.00 ) numShadows = 7;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.05 ) numShadows = 6;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.10 ) numShadows = 5;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.15 ) numShadows = 4;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.20 ) numShadows = 3;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.25 ) numShadows = 2;
        else if( player->animationDisplayAmt > stepAtkTime - moveAnimationTime * 0.30 ) numShadows = 1;
        drawStepShadow( player, numShadows );

        if( player->facing == -1 ) picIndex += textureSheetWidth;
        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->y * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->y * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->y * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->y * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
	// Attack Animation
	else if(player->animationType == 1 && player->animationDisplayAmt > 0 ) {
        if( !draw ) return;

        int textureSheetWidth = 8;
        switch( player->type ) {
        case MEGAMAN: {
            playerSizeX = 0.66 * playerScale;
            playerSizeY = 0.56 * playerScale;
            displaceX = 0.3;
            displaceY = 0.02;
            texture = ( player->npc ? darkMegamanAtkSheet : megamanAtkSheet );
            break;
        }
        case PROTOMAN: {
            playerSizeX = 0.78 * playerScale;
            playerSizeY = 0.64 * playerScale;
            displaceX = 0.22;
            displaceY = 0.08 * playerScale + 0.02;
            texture = ( player->npc ? darkProtoAtkSheet : protoAtkSheet );
            break;
        }
        case COLONEL: {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            displaceX = 0.3 * playerScale;
            displaceY = 0.15 * playerScale + 0.02;
            texture = ( player->npc ? darkColonelAtkSheet : colonelAtkSheet );
            break;
        }
        case SLASHMAN: {
            playerSizeX = 0.94 * playerScale;
            playerSizeY = 0.88 * playerScale;
            displaceX = 0.20;
            displaceY = 0.065 * playerScale + 0.02;
            textureSheetWidth = 9;
            texture = ( player->npc ? darkSlashmanAtkSheet : slashmanAtkSheet );
            break;
        }
        }

        // Animation for TomahawkMan's Eagle Tomahawk attack
        if( player->type == TOMAHAWKMAN && player->actionNumber == 5 ) {
            playerSizeX = 1.10 * playerScale;
            playerSizeY = 0.99  * playerScale;
            displaceX = 0.17;
            displaceY = 0.43 * playerScale + 0.02;
            texture = ( player->npc ? darkTmanAtkSheet2 : tmanAtkSheet2 );
            textureSheetWidth = 5;
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 0;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.18 ) picIndex = 1;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 2;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.24 ) picIndex = 3;
            else                                                                        picIndex = 4;
        }
        // TomahawkMan's regular attacks
        else if( player->type == TOMAHAWKMAN ) {
            playerSizeX = 0.80 * playerScale;
            playerSizeY = 0.65 * playerScale;
            displaceX = 0.22;
            displaceY = 0.1 * playerScale + 0.02;
            texture = ( player->npc ? darkTmanAtkSheet1 : tmanAtkSheet1 );
            textureSheetWidth = 4;
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.20 ) picIndex = 0;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.24 ) picIndex = 1;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.28 ) picIndex = 2;
			else                                                                        picIndex = 3;
        }
        // Slashman's LongSlash
        else if( player->type == SLASHMAN && player->actionNumber == 1 ) {
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 2;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 3;
			else                                                                        picIndex = 4;
        }
        // Slashman's WideSlash
        else if( player->type == SLASHMAN && player->actionNumber == 2 ) {
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 0;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 1;
			else                                                                        picIndex = 2;
        }
        // Slashman's CrossSlash
        else if( player->type == SLASHMAN && player->actionNumber == 3 ) {
            if ( ( player->energy / 100 ) % 2 ) {
                if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 0;
			    else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 1;
			    else                                                                        picIndex = 2;
            }
            else {
                if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 2;
			    else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 3;
			    else                                                                        picIndex = 4;
            }
        }
        // Slashman's SpinSlash
        else if( player->type == SLASHMAN && player->actionNumber == 5 ) {
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 5;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.16 ) picIndex = 6;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 7;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.28 ) picIndex = 6;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.34 ) picIndex = 8;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.40 ) picIndex = 6;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.46 ) picIndex = 7;
			else                                                                        picIndex = 6;
        }
		// GutsMan's Punches
        else if( player->type == GUTSMAN && player->actionNumber == 1 ) {
            playerSizeX = 0.97 * playerScale;
            playerSizeY = 0.53  * playerScale;
            displaceX = 0.27 * playerScale;
            displaceY = -0.03 * playerScale;
            texture = ( player->npc ? darkGutsAtkSheet1 : gutsAtkSheet1 );
            textureSheetWidth = 5;
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.03 ) picIndex = 0;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.06 ) picIndex = 1;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.12 ) picIndex = 2;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.15 ) picIndex = 3;
            else                                                                        picIndex = 4;
        }
        // GutsMan's Hammer Attacks
        else if( player->type == GUTSMAN ) {
            playerSizeX = 0.99 * playerScale;
            playerSizeY = 0.81 * playerScale;
            displaceX = 0.11 * playerScale;
            displaceY = 0.11 * playerScale;
            texture = ( player->npc ? darkGutsAtkSheet2 : gutsAtkSheet2 );
            textureSheetWidth = 5;
            if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.04 ) picIndex = 0;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.16 ) picIndex = 1;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.20 ) picIndex = 2;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.24 ) picIndex = 3;
            else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.28 ) picIndex = 4;
            else                                                                        picIndex = 3;
        }
        // Animation for the rest of the Attacks
		else {
			if     ( player->animationDisplayAmt > player->currentSwordAtkTime - 0.10 ) picIndex = 1;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.14 ) picIndex = 2;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.18 ) picIndex = 3;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.22 ) picIndex = 4;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.26 ) picIndex = 5;
			else if( player->animationDisplayAmt > player->currentSwordAtkTime - 0.30 ) picIndex = 6;
			else                                                                        picIndex = 7;
        }
        
        glTranslatef( player->facing * displaceX, displaceY, 0 );
        if( player->facing == -1 ) picIndex += textureSheetWidth;
        GLfloat playerPlace[] = { player->x  * scaleX - playerSizeX, player->y * scaleY + playerSizeY,
								  player->x  * scaleX - playerSizeX, player->y * scaleY - playerSizeY,
		                          player->x  * scaleX + playerSizeX, player->y * scaleY - playerSizeY,
								  player->x  * scaleX + playerSizeX, player->y * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
	// Movement Animation
    else if( ( player->animationType == 0 && player->moveType == 2 ) ) {
        if( !draw ) return;

        const float beforeHurtTime = ( player->animationType == -1 ? pauseAnimationTime - moveAnimationTime : 0 );
        if     ( player->animationDisplayAmt > ( moveAnimationTime * 0.70 + beforeHurtTime ) ) { displace = 1; picIndex = 0; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.60 + beforeHurtTime ) ) { displace = 1; picIndex = 1; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.50 + beforeHurtTime ) ) { displace = 1; picIndex = 2; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.45 + beforeHurtTime ) ) { displace = 0; picIndex = 3; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.35 + beforeHurtTime ) ) { displace = 0; picIndex = 2; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.25 + beforeHurtTime ) ) { displace = 0; picIndex = 1; }
        else if( player->animationDisplayAmt > ( moveAnimationTime * 0.00 + beforeHurtTime ) ) { displace = 0; picIndex = 0; }

        glTranslatef( 0, displaceY, 0 );
        // Up   // Left     // Down     // Right
        if (player->moveDir == 0) {
            player->yDisplay = (float) player->y - displace;
            glTranslatef( player->facing * displaceX, 0, 0 );
            if( player->facing == -1 ) picIndex += textureSheetWidth;
		}
		else if (player->moveDir == 1) {
            player->xDisplay = (float) player->x + displace;
			glTranslatef( -displaceX, 0, 0);
            picIndex += textureSheetWidth;
		}
		else if (player->moveDir == 2) {
            player->yDisplay = (float) player->y + displace;
            glTranslatef( player->facing * displaceX, 0, 0 );
            if( player->facing == -1 ) picIndex += textureSheetWidth;
		}
		else if (player->moveDir == 3) {
            player->xDisplay = (float) player->x - displace;
			glTranslatef( displaceX, 0, 0);
		}

        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
    // Trapped Energy Hurt Animation
    else if( player->animationType == -1 && player->animationDisplayAmt > 0 ) {
        int textureSheetWidth = 2;

        switch( player->type ) {
        case MEGAMAN: {
            playerSizeX = 0.40 * playerScale;
            playerSizeY = 0.48 * playerScale;
            displaceX = 0.014;
            displaceY = -0.06 * playerScale;
            texture = ( player->npc ? darkMegamanHurtSheet : megamanHurtSheet );
            break;
        }
        case PROTOMAN: {
            playerSizeX = 0.70 * playerScale;
            playerSizeY = 0.49 * playerScale;
            displaceX = -0.01 * playerScale;
            displaceY = -0.05 * playerScale;
            texture = ( player->npc ? darkProtoHurtSheet : protoHurtSheet );
            break;
        }
        case COLONEL: {
            playerSizeX = 0.65 * playerScale;
            playerSizeY = 0.67 * playerScale;
            displaceX = -0.184 * playerScale;
            displaceY = 0.088 * playerScale;
            texture = ( player->npc ? darkColonelHurtSheet : colonelHurtSheet );
            break;
        }
        case GUTSMAN: {
            playerSizeX = 0.48 * playerScale;
            playerSizeY = 0.64 * playerScale;
            displaceX = 0.02 * playerScale;
            displaceY = 0.0545 * playerScale;
            textureSheetWidth = 1;
            texture = ( player->npc ? darkGutsHurtSheet : gutsHurtSheet );
            break;
        }
        case TOMAHAWKMAN:
        case SLASHMAN: {
            textureSheetWidth = 4;
            break;
        }
        }

        if( !draw ) return;
        GLfloat playerPlace[] = { player->x  * scaleX - playerSizeX, player->y * scaleY + playerSizeY,
                                  player->x  * scaleX - playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x  * scaleX + playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x  * scaleX + playerSizeX, player->y * scaleY + playerSizeY };
        glTranslatef( player->facing * displaceX, displaceY, 0 );
        // TomahawkMan and SlashMan do not have special hurt animations
        if( player->type != TOMAHAWKMAN && player->type != SLASHMAN && player->type != GUTSMAN && player->animationDisplayAmt <= 0.03 ) picIndex = 1;
        if( player->facing == -1 ) picIndex += textureSheetWidth;
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
	// Turning Animation
    else if( player->animationType == 0 && player->moveType == 1 ) {
        bool reverse = true;

        if     ( player->animationDisplayAmt > moveAnimationTime * 0.70 ) { picIndex = 0; reverse = true; }
        else if( player->animationDisplayAmt > moveAnimationTime * 0.60 ) { picIndex = 1; reverse = true; }
        else if( player->animationDisplayAmt > moveAnimationTime * 0.50 ) { picIndex = 2; reverse = true; }
        else if( player->animationDisplayAmt > moveAnimationTime * 0.45 ) { picIndex = 3; reverse = false; }
        else if( player->animationDisplayAmt > moveAnimationTime * 0.35 ) { picIndex = 2; reverse = false; }
        else if( player->animationDisplayAmt > moveAnimationTime * 0.25 ) { picIndex = 1; reverse = false; }
        else                                                              { picIndex = 0; reverse = false; }

        glTranslatef( 0, displaceY, 0 );
        if( player->facing == -1 ) {
            picIndex += ( reverse ? 0 : textureSheetWidth );
            reverse ? glTranslatef( displaceX, 0, 0 ) : glTranslatef( -displaceX, 0, 0 );
        }
        else if( player->facing == 1 ) {
            picIndex += ( reverse ? textureSheetWidth : 0 );
            reverse ? glTranslatef( -displaceX, 0, 0 ) : glTranslatef( displaceX, 0, 0 );
        }

        GLfloat playerPlace[] = { player->x * scaleX - playerSizeX, player->y * scaleY + playerSizeY,
                                  player->x * scaleX - playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x * scaleX + playerSizeX, player->y * scaleY - playerSizeY,
                                  player->x * scaleX + playerSizeX, player->y * scaleY + playerSizeY };
        if( draw ) drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
    // Slipping on Ice Movement Animation
    else if( player->animationType == 5 && player->moveType == 2 ) {
        if( !draw ) return;

        displace = player->animationDisplayAmt / iceSlipAnimationTime;
        glTranslatef( 0, displaceY, 0 );
        // Up   // Left     // Down     // Right
        if( player->moveDir == 0 ) {
            player->yDisplay = (float) player->y - displace;
            glTranslatef( player->facing * displaceX, 0, 0 );
            if( player->facing == -1 ) picIndex += textureSheetWidth;
        }
        else if( player->moveDir == 1 ) {
            player->xDisplay = (float) player->x + displace;
            glTranslatef( -displaceX, 0, 0 );
            picIndex += textureSheetWidth;
        }
        else if( player->moveDir == 2 ) {
            player->yDisplay = (float) player->y + displace;
            glTranslatef( player->facing * displaceX, 0, 0 );
            if( player->facing == -1 ) picIndex += textureSheetWidth;
        }
        else if( player->moveDir == 3 ) {
            player->xDisplay = (float) player->x - displace;
            glTranslatef( displaceX, 0, 0 );
        }

        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
    // Special Invalid Movement animation for Start Panel
	else if( player->animationType == 2 && player->animationDisplayAmt > 0) {
        displace = player->animationDisplayAmt / moveAnimationTime;
        player->xDisplay = (float) player->x - displace;        
        glTranslatef( -displaceX, displaceY, 0 );

        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, textureSheetWidth, textureSheetWidth, 2 );
	}
	// Level Transition pt. 1
	else if( player->animationType == 3 && player->animationDisplayAmt > moveAnimationTime * 2) {
        displace = 1 - (player->animationDisplayAmt - moveAnimationTime * 2) / moveAnimationTime;
        player->xDisplay = (float) player->x + displace;
        glTranslatef( displaceX, displaceY, 0 );
        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
		drawSpriteSheetSprite(playerPlace, texture, 0, textureSheetWidth, 2);
	}
	// Level Transition pt. 2
	else if( player->animationType == 4 && player->animationDisplayAmt > 0 && player->animationDisplayAmt <= moveAnimationTime * 2) {
        displace = player->animationDisplayAmt / moveAnimationTime;
        player->xDisplay = (float) player->x - displace;
        glTranslatef( displaceX, displaceY, 0 );
        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
		drawSpriteSheetSprite(playerPlace, texture, 0, textureSheetWidth, 2);
	}
    // Player Standing Still
    else if( player->hp > 0 ) {
        if( !draw ) return;

        if( player->facing == -1 ) picIndex += textureSheetWidth;
        glTranslatef( player->facing * displaceX, displaceY, 0 );

        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY + playerSizeY,
								  player->xDisplay * scaleX - playerSizeX, player->yDisplay * scaleY - playerSizeY,
		                          player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY - playerSizeY,
								  player->xDisplay * scaleX + playerSizeX, player->yDisplay * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
}
void App::drawSwordAtks(Player* const player) {
    if( player->animationDisplayAmt <= 0 || player->animationType != 1 ) return;

    glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);

    switch( player->type ) {
    default:
    case MEGAMAN:
        switch( player->actionNumber ) {
        default: return;
        case 1: swordDisplay( player ); return;
        case 2: longDisplay( player ); return;
        case 3: wideDisplay( player ); return;
        case 4: crossDisplay( player ); return;
        case 5: spinDisplay( player ); return;
        case 6: stepDisplay( player ); return;
        case 7: lifeDisplay( player ); return;
        }
    case PROTOMAN:
        switch( player->actionNumber ) {
        default: return;
        case 1: swordDisplay( player ); return;
        case 2: longDisplay( player ); return;
        case 3: wideDisplay( player ); return;
        case 4: stepDisplay( player ); return;
        case 5: heroDisplay( player ); return;
        case 6: protoDisplay( player ); return;
        }
    case TOMAHAWKMAN:
        switch( player->actionNumber ) {
        default: return;
        case 1: tomaDisplayA1( player ); return;
        case 2: tomaDisplayB1( player ); return;
        case 3: tomaDisplayA2( player ); return;
        case 4: tomaDisplayB2( player ); return;
        // case 5: Eagle Tomahawk is displayed in drawExtraAtks()
        }
    case COLONEL:
        switch( player->actionNumber ) {
        default: return;
        case 1: vDivideDisplay( player ); return;
        case 2: upDivideDisplay( player ); return;
        case 3: downDivideDisplay( player ); return;
        case 4: xDivideDisplay( player ); return;
        case 5: zDivideDisplay( player ); return;
        }
    case SLASHMAN:
        switch( player->actionNumber ) {
        default: return;
        case 1: longSlashDisplay( player ); return;
        case 2: wideSlashDisplay( player ); return;
        case 3: crossDisplay( player ); return;
        case 4: stepCrossDisplay( player ); return;
        case 5: return;
        }
    case GUTSMAN: return;
        // GutsMan's Shockwave attacks are displayed in drawExtraAtks()
    }
}
void App::drawExtraAtks() {
    for( int i = 0; i < delayedAtkDisplayList.size(); i++ ) {
        if( delayedAtkDisplayList[i].type == "eToma" && delayedAtkDisplayList[i].delay <= 0 ) {
            eTomaDisplay( delayedAtkDisplayList[i].xPos, delayedAtkDisplayList[i].yPos, delayedAtkDisplayList[i].animationTimer );
        }
    }
    for( int i = 0; i < delayedAtkDisplayList.size(); i++ ) {
        if( delayedAtkDisplayList[i].type == "shockwave" && delayedAtkDisplayList[i].delay <= 0 ) {
            shockwaveDisplay( delayedAtkDisplayList[i].xPos, delayedAtkDisplayList[i].yPos, delayedAtkDisplayList[i].dir, delayedAtkDisplayList[i].animationTimer );
        }
    }
}
void App::drawStepShadow( Player* const player, int amt ) {
    GLuint texture;
    int picIndex = 0;
    int textureSheetWidth = 1;
    float playerSizeX, playerSizeY;

    switch( player->type ) {
    default: return;
    case MEGAMAN:
        playerSizeX = 0.66 * playerScale;
        playerSizeY = 0.56 * playerScale;
        texture = player->npc ? darkMegamanStepSheet : megamanStepSheet;
        break;
    case PROTOMAN:
        playerSizeX = 0.78 * playerScale;
        playerSizeY = 0.64 * playerScale;
        texture = player->npc ? darkProtoStepSheet : protoStepSheet;
        break;
    case COLONEL:
        playerSizeX = 1.00 * playerScale;
        playerSizeY = 0.76 * playerScale;
        texture = player->npc ? darkColonelStepSheet : colonelStepSheet;
        break;
    case SLASHMAN:
        playerSizeX = 0.94 * playerScale;
        playerSizeY = 0.88 * playerScale;
        textureSheetWidth = 2;
        texture = player->npc ? darkSlashmanStepSheet : slashmanStepSheet;
        if( ( player->energy / 100 ) % 2 == 0 ) picIndex = 1;
        break;
    case GUTSMAN:
        playerSizeX = 0.97 * playerScale;
        playerSizeY = 0.53 * playerScale;
        texture = player->npc ? darkGutsStepSheet : gutsStepSheet;
    }

    if( player->facing == -1 ) picIndex += textureSheetWidth;
    float displace = 0.0;
    while( amt > 0 ) {
        displace += 0.25;
        amt--;
        GLfloat playerPlace[] = { player->xDisplay * scaleX - playerSizeX - displace * player->facing, player->y * scaleY + playerSizeY,
                                  player->xDisplay * scaleX - playerSizeX - displace * player->facing, player->y * scaleY - playerSizeY,
                                  player->xDisplay * scaleX + playerSizeX - displace * player->facing, player->y * scaleY - playerSizeY,
                                  player->xDisplay * scaleX + playerSizeX - displace * player->facing, player->y * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
    }
}
void App::drawHp(Player* const player) {
    // Display Player Energy or NPC HP
    if( !player->npc && player->hp < 0 ) return;

    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5 + player->xDisplay * scaleX, 0.7 + player->yDisplay * scaleY, 0.0 );

    GLuint texture = textSheetWhite;
    if     ( !player->npc && chargeDisplayPlusAmt > 0 ) { texture = textSheetGreen; }
    else if( player->hurtDisplayAmt > 0 )               { texture = textSheetRed; }

    int energyToDisplay = player->npc ? player->hp : player->energyDisplayed;
    if     ( energyToDisplay <= 0 )   { energyToDisplay = 0; }
    else if( energyToDisplay < 1 )    { energyToDisplay = 1; }
    else if( energyToDisplay > 9999 ) { energyToDisplay = 9999; }

    if( energyToDisplay >= 10 )   { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
    if( energyToDisplay >= 100 )  { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
    if( energyToDisplay >= 1000 ) { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
    if( energyToDisplay >= 0 )    { drawText( texture, to_string( energyToDisplay ), 0.08 * 1.5, 0.16 * 1.5, 0.00 ); }
}

void App::drawFloor() {
    // Draw Panels
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 0.7, 0.0 );

    int index = 0;
    float width = 0.40 * overallScale;
    float height = 0.24 * overallScale;
    float height2 = 0.06 * overallScale;

    // Draw the Board - 6x6 Panels
    for( int i = 0; i < 6; i++ ) {
        for( int j = 5; j >= 0; j-- ) {
            GLfloat place[] = { i * scaleX + width, j * scaleY + height, i * scaleX + width, j * scaleY - height,
                                i * scaleX - width, j * scaleY - height, i * scaleX - width, j * scaleY + height };
            // If the panel is being attacked, display the attack indicator
            if( board.map[i][j].rockAtkInd > rockDmgTime * 0.6 ) drawSpriteSheetSprite( place, floorAtkIndPic, 0, 1, 1 );
            // Else, draw normally
            else {
                int textureSheetWidth;
                GLuint texture;

                switch( board.map[i][j].state ) {
                default:
                    texture = floorSheet;
                    textureSheetWidth = 5;
                    index = 0;
                    break;
                case 0: case -1: case -2: case -3:
                    // Normal Panels, Cracked Panels, Cracked Hole Panels, Hole Panels
                    texture = floorSheet;
                    textureSheetWidth = 5;
                    index = abs( board.map[i][j].state );
                    break;
                case 1:
                    // Ice Panels
                    texture = floorSheet;
                    textureSheetWidth = 5;
                    index = 4;
                    break;
                case 2:
                    // Poison Panels
                    texture = floorPoisonSheet;
                    textureSheetWidth = 5;
                    index = poisonAnimationAmt < 5.0 ? poisonAnimationAmt : 3 - ( poisonAnimationAmt - 5 );
                    break;
                case 3:
                    // Holy Panels
                    texture = floorHolySheet;
                    textureSheetWidth = 7;
                    index = holyAnimationAmt;
                    break;
                }
                if( j == 0 || j == 1 ) index += textureSheetWidth * 2;
                else if( j == 2 || j == 3 ) index += textureSheetWidth;
                drawSpriteSheetSprite( place, texture, index, textureSheetWidth, 3 );
            }
        }
    }

    // Draw Start Panel
    float startX = -1;
    float startY = 0;
    GLfloat startPlace[] = { startX * scaleX + width, startY * scaleY + height, startX * scaleX + width, startY * scaleY - height,
                             startX * scaleX - width, startY * scaleY - height, startX * scaleX - width, startY * scaleY + height };
    if( itemAnimationAmt >= 0 && itemAnimationAmt <= 1 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 8, 4, 3 ); }
    else if( itemAnimationAmt >= 1 && itemAnimationAmt <= 2 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 9, 4, 3 ); }
    else if( itemAnimationAmt >= 2 && itemAnimationAmt <= 3 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 10, 4, 3 ); }
    else if( itemAnimationAmt >= 3 && itemAnimationAmt <= 4 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 11, 4, 3 ); }
    else if( itemAnimationAmt >= 4 && itemAnimationAmt <= 5 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 8, 4, 3 ); }
    else if( itemAnimationAmt >= 5 && itemAnimationAmt <= 6 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 9, 4, 3 ); }
    else if( itemAnimationAmt >= 6 && itemAnimationAmt <= 7 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 10, 4, 3 ); }
    else if( itemAnimationAmt >= 7 && itemAnimationAmt <= 8 ) { drawSpriteSheetSprite( startPlace, floorMoveSheet, 11, 4, 3 ); }

    // Draw Goal Panel
    float goalX = 6;
    float goalY = 5;
    GLfloat goalPlace[] = { goalX * scaleX + width, goalY * scaleY + height, goalX * scaleX + width, goalY * scaleY - height,
                            goalX * scaleX - width, goalY * scaleY - height, goalX * scaleX - width, goalY * scaleY + height };
    if     ( itemAnimationAmt >= 0 && itemAnimationAmt <= 1 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 0, 4, 3 );
    else if( itemAnimationAmt >= 1 && itemAnimationAmt <= 2 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 1, 4, 3 );
    else if( itemAnimationAmt >= 2 && itemAnimationAmt <= 3 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 2, 4, 3 );
    else if( itemAnimationAmt >= 3 && itemAnimationAmt <= 4 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 3, 4, 3 );
    else if( itemAnimationAmt >= 4 && itemAnimationAmt <= 5 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 0, 4, 3 );
    else if( itemAnimationAmt >= 5 && itemAnimationAmt <= 6 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 1, 4, 3 );
    else if( itemAnimationAmt >= 6 && itemAnimationAmt <= 7 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 2, 4, 3 );
    else if( itemAnimationAmt >= 7 && itemAnimationAmt <= 8 ) drawSpriteSheetSprite( goalPlace, floorMoveSheet, 3, 4, 3 );

    // Draw Panel Bottom pieces
    for( int x = -1; x <= 6; x++ ) {
        float y = -0.63;
        if( x == 6 ) { y += 5; }
        GLfloat bottomPlace[] = { x * scaleX + width, y * scaleY + height2, x * scaleX + width, y * scaleY - height2,
                                  x * scaleX - width, y * scaleY - height2, x * scaleX - width, y * scaleY + height2 };
        drawSpriteSheetSprite( bottomPlace, x == 6 ? floorBottomPic2 : floorBottomPic1, 0, 1, 1 );
    }
}
void App::drawRocks( int row ) {			// Draw the Rock Obstacles on the Floor
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 1.01, 0.0 );

    float sizeX = 0.32 * itemScale;
    float sizeY = 0.40 * itemScale;
    float sizeX2 = 0.55 * itemScale;
    float sizeY2 = 0.55 * itemScale;

    GLuint textureSheet;
    int sheetHeight = 3;
    int index = 0;

    for( int i = 0; i < 6; i++ ) {
        if( board.map[i][row].rockHP > 0 ) {
            GLfloat place[] = { i * scaleX + sizeX, row * scaleY + sizeY, i * scaleX + sizeX, row * scaleY - sizeY,
                                i * scaleX - sizeX, row * scaleY - sizeY, i * scaleX - sizeX, row * scaleY + sizeY };

            if( board.map[i][row].rockType == 0 ) {
                sheetHeight = 5;
                textureSheet = rockSheet;
            }
            else if( board.map[i][row].rockType == 1 ) {
                sheetHeight = 3;
                if( board.map[i][row].item == 2 )       textureSheet = rockSheetItem2;
                else if( board.map[i][row].item == -1 ) textureSheet = rockSheetTrappedItem;
                else                                    textureSheet = rockSheetItem;
            }

            index = ( board.map[i][row].rockHP - 1 ) * 3;
            if     ( board.map[i][row].prevDmg == 1 && board.map[i][row].rockAtkInd > 0 ) index += 4; 
            else if( board.map[i][row].prevDmg == 2 && board.map[i][row].rockAtkInd > 0 ) index += 8;

            drawSpriteSheetSprite( place, textureSheet, index, 3, sheetHeight );
        }
        else if( board.map[i][row].rockAtkInd > 0 && board.map[i][row].prevDmg > 0 ) {
            textureSheet = ( board.map[i][row].isPurple ? darkDeathSheet : rockDeathSheet );
            if( board.map[i][row].bigRockDeath ) {
                sizeX2 = 0.75 * itemScale;
                sizeY2 = 0.75 * itemScale;
            }

            GLfloat place[] = { i * scaleX + sizeX2, row * scaleY + sizeY2, i * scaleX + sizeX2, row * scaleY - sizeY2,
                                i * scaleX - sizeX2, row * scaleY - sizeY2, i * scaleX - sizeX2, row * scaleY + sizeY2 };
            if     ( board.map[i][row].rockAtkInd > rockDmgTime * 0.60 ) index = 0;
            else if( board.map[i][row].rockAtkInd > rockDmgTime * 0.40 ) index = 1;
            else if( board.map[i][row].rockAtkInd > rockDmgTime * 0.20 ) index = 2;
            else                                                         index = 3;
            drawSpriteSheetSprite( place, textureSheet, index, 4, 1 );
        }
    }
}
void App::drawItems(int row) {		// Draw the Collectable Resources on the Map
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.11, 0.0);

	float itemSizeX = 0.23 * itemScale / 1.5;
	float itemSizeY = 0.61 * itemScale / 1.5;

	for (int i = 0; i < 6; i++) {
		GLfloat place[] = { i + itemSizeX * scaleX, row * scaleY + itemSizeY, i + itemSizeX * scaleX, row * scaleY - itemSizeY,
							i - itemSizeX * scaleX, row * scaleY - itemSizeY, i - itemSizeX * scaleX, row * scaleY + itemSizeY };
        if( board.map[i][row].item != 0 && board.map[i][row].rockHP <= 0 ) {
            GLuint texture = energySheet;
            int picIndex = 0;
            if     ( board.map[i][row].item ==  2 ) texture = energySheet2;
            else if( board.map[i][row].item == -1 ) texture = trappedEnergySheet;

            if     ( itemAnimationAmt >= 0 && itemAnimationAmt < 1 ) picIndex = 0;
            else if( itemAnimationAmt >= 1 && itemAnimationAmt < 2 ) picIndex = 1;
            else if( itemAnimationAmt >= 2 && itemAnimationAmt < 3 ) picIndex = 2;
            else if( itemAnimationAmt >= 3 && itemAnimationAmt < 4 ) picIndex = 3;
            else if( itemAnimationAmt >= 4 && itemAnimationAmt < 5 ) picIndex = 4;
            else if( itemAnimationAmt >= 5 && itemAnimationAmt < 6 ) picIndex = 5;
            else if( itemAnimationAmt >= 6 && itemAnimationAmt < 7 ) picIndex = 6;
            else                                                     picIndex = 7;
            drawSpriteSheetSprite( place, texture, picIndex, 8, 1 );
        }
    }
}
void App::drawItemUpgrading( int row ) {
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 1.11, 0.0 );

    float itemSizeX = 0.31 * itemScale / 1.0;
    float itemSizeY = 0.59 * itemScale / 1.0;

    for( int i = 0; i < 6; i++ ) {
        GLfloat place[] = { i + itemSizeX * scaleX, row * scaleY + itemSizeY, i + itemSizeX * scaleX, row * scaleY - itemSizeY,
                            i - itemSizeX * scaleX, row * scaleY - itemSizeY, i - itemSizeX * scaleX, row * scaleY + itemSizeY };
        if( board.map[i][row].upgradeInd > 0 ) {
            GLuint texture = recoverSheet;
            int picIndex = 0;

            if     ( board.map[i][row].upgradeInd > itemUpgradeTime * 0.875 ) picIndex = 0;
            else if( board.map[i][row].upgradeInd > itemUpgradeTime * 0.750 ) picIndex = 1;
            else if( board.map[i][row].upgradeInd > itemUpgradeTime * 0.625 ) picIndex = 2;
            else if( board.map[i][row].upgradeInd > itemUpgradeTime * 0.500 ) picIndex = 3;
            else if( board.map[i][row].upgradeInd > itemUpgradeTime * 0.250 ) picIndex = 4;
            else                                                              picIndex = 5;
            drawSpriteSheetSprite( place, texture, picIndex, 6, 1 );
        }
    }
}

// Sword Attack Animations
void App::swordDisplay( Player* const player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.22 * playerScale;
	int xPos = player->x;
	int yPos = player->y;
    GLuint texture;

	if ( player->facing == -1 ) {
        xPos--;
        texture = swordAtkSheet1; }
    else if ( player->facing == 1 ) {
        xPos++;
        texture = swordAtkSheet3; }

	GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };

	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::longDisplay( Player* const player) {
	float sizeX = 0.70 * playerScale;
	float sizeY = 0.37 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos -= 1.5;
        texture = longAtkSheet1; }
	else if (player->facing == 1) {
		xPos += 1.5;
        texture = longAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideDisplay( Player* const player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (player->facing == 1) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::crossDisplay( Player* const player) {
	float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (player->facing == 1) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::spinDisplay( Player* const player) {
	float sizeX = 1.10 * 1.25 * playerScale;
	float sizeY = 0.72 * 1.25 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
        texture = spinAtkSheet1; }
	else if (player->facing == 1) {
        texture = spinAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player->animationDisplayAmt > 0)                   { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::stepDisplay( Player* const player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (player->facing == 1) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player->animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::lifeDisplay( Player* const player) {
	float sizeX = 0.95 * playerScale;
	float sizeY = 0.80 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos -= 1.5;
        texture = lifeAtkSheet1; }
	else if (player->facing == 1) {
		xPos += 1.5;
        texture = lifeAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::heroDisplay( Player* const player ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.45 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 2;
        texture = heroAtkSheet1; }
    else if ( player->facing == 1 ) {
        xPos += 2;
        texture = heroAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::protoDisplay( Player* const player ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.71 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 2;
        texture = protoAtkSheet1; }
    else if ( player->facing == 1 ) {
        xPos += 2;
        texture = protoAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::vDivideDisplay( Player* const player ) {
    float sizeX = 0.68 * playerScale;
    float sizeY = 0.53 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 0.75;
        texture = screenDivVSheet3; }
    else if ( player->facing == 1 ) {
        xPos += 0.75;
        texture = screenDivVSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::upDivideDisplay( Player* const player ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1;
        texture = screenDivUpSheet3; }
    else if ( player->facing == 1 ) {
        xPos += 1;
        texture = screenDivUpSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::downDivideDisplay( Player* const player ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1;
        texture = screenDivDownSheet3; }
    else if ( player->facing == 1 ) {
        xPos += 1;
        texture = screenDivDownSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::xDivideDisplay( Player* const player ) {
    float sizeX = 0.94 * playerScale;
    float sizeY = 1.03 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1;
        texture = screenDivXSheet3; }
    else if ( player->facing == 1 ) {
        xPos += 1;
        texture = screenDivXSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player->animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::zDivideDisplay( Player* const player) {
	float sizeX = 1.32 * playerScale;
	float sizeY = 0.60 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos -= 1; }
	else if (player->facing == 1) {
		xPos += 1; }
    texture = screenDivZSheet;

    GLfloat atkPlace[] = { xPos * scaleX - sizeX, yPos * scaleY + sizeY, xPos * scaleX - sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX + sizeX, yPos * scaleY - sizeY, xPos * scaleX + sizeX, yPos * scaleY + sizeY };
    if		(player->animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > lifeAtkTime - 0.16) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.28) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player->animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA1( Player* const player) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetA1; }
    else if ( player->facing == 1 ) {
        xPos += 1;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player->animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB1( Player* const player) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetB1; }
    else if ( player->facing == 1 ) {
        xPos += 1;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player->animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA2( Player* const player) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetA1; }
    else if ( player->facing == 1 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player->animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB2( Player* const player) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player->x;
    float yPos = player->y;
    GLuint texture;

    if ( player->facing == -1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetB1; }
    else if ( player->facing == 1 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player->animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player->animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::eTomaDisplay(int xPos, int yPos, float animationTime) {
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 1.2, 0.0 );
    if( animationTime <= 0 ) return;

    float sizeX = 0.52 * playerScale;
    float sizeY = 0.72 * playerScale;
    glTranslatef( 0, 0.2, 0 );
    GLuint texture = eagleTomaSheet;
    int picIndex = 0;

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if     ( animationTime >  0.12 ) picIndex = 0;
    else if( animationTime >  0.08 ) picIndex = 1;
    else if( animationTime >  0.04 ) picIndex = 2;
    else                             picIndex = 3;
    drawSpriteSheetSprite( atkPlace, texture, picIndex, 4, 1 );
}
void App::longSlashDisplay( Player* const player) {
    float sizeX = 0.84 * playerScale;
	float sizeY = 0.54 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos -= 1.5;
        texture = longSlashSheet1; }
	else if (player->facing == 1) {
		xPos += 1.5;
        texture = longSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideSlashDisplay( Player* const player) {
    float sizeX = 0.39 * playerScale;
	float sizeY = 0.95 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos--;
        texture = wideSlashSheet1; }
	else if (player->facing == 1) {
		xPos++;
        texture = wideSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player->animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player->animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player->animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::stepCrossDisplay( Player* const player) {
    float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player->x;
	float yPos = player->y;
    GLuint texture;

	if (player->facing == -1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (player->facing == 1) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player->animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player->animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player->animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::shockwaveDisplay(int xPos, int yPos, int dir, float animationTime) {
    glLoadIdentity();
    glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
    glTranslatef( 0.5, 1.2, 0.0 );
    if( animationTime <= 0 ) return;

    float sizeX = 0.47 * playerScale;
    float sizeY = 0.43 * playerScale;
    glTranslatef( dir * -0.1, -0.2, 0 );
    GLuint texture = ( dir == -1 ? shockwaveSheet1 : shockwaveSheet3 );
    int picIndex = 0;

    GLfloat atkPlace[] = { xPos * scaleX - sizeX, yPos * scaleY + sizeY, xPos * scaleX - sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX + sizeX, yPos * scaleY - sizeY, xPos * scaleX + sizeX, yPos * scaleY + sizeY };
    if     ( animationTime > 0.22 ) picIndex = 0;
    else if( animationTime > 0.16 ) picIndex = 1;
    else if( animationTime > 0.12 ) picIndex = 2;
    else if( animationTime > 0.04 ) picIndex = 3;
    else                            picIndex = 4;
    drawSpriteSheetSprite( atkPlace, texture, picIndex, 5, 1 );
}

// Player Control Functions: Attacking & Movement
bool App::attack( Player* const player, int atkNum ) {
    int cost = player->getAtkCost( atkNum );
    if( player->energy < cost ) return false;

    vector< DelayedDamage > atkCoords = player->attack( atkNum );
    if( atkCoords.empty() ) return false;

    Mix_Chunk* soundToPlay = nullptr;

    switch( player->type ) {
    default:
    case MEGAMAN: {
        switch( atkNum ) {
        default:
            player->animationDisplayAmt = swordAtkTime;
            soundToPlay = swordSound;
            break;
        case 6:
            if( !move( player, player->facing == -1 ? 1 : 3, 2 ) ) return false;
            player->moveType = 3;
            player->animationDisplayAmt = stepAtkTime + moveAnimationTime;
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, player->npc ) );
            break;
        case 7:
            player->animationDisplayAmt = lifeAtkTime;
            soundToPlay = lifeSwordSound;
            break;
        }
        break;
    }
    case PROTOMAN: {
        switch( atkNum ) {
        default:
            player->animationDisplayAmt = swordAtkTime;
            soundToPlay = swordSound;
            break;
        case 4:
            if( !move( player, player->facing == -1 ? 1 : 3, 2 ) ) return false;
            player->moveType = 3;
            player->animationDisplayAmt = stepAtkTime + moveAnimationTime;
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, player->npc ) );
            break;
        case 5: case 6:
            player->animationDisplayAmt = lifeAtkTime;
            soundToPlay = swordSound;
            break;
        }
        break;
    }
    case TOMAHAWKMAN: {
        switch( atkNum ) {
        default:
            player->animationDisplayAmt = lifeAtkTime;
            delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma, player->npc ) );
            break;
        case 5: {
            player->animationDisplayAmt = eTomaAtkTime;
            delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma, player->npc ) );
            for( int i = 1; i <= 5; i++ ) {
                int xPos = player->x + player->facing * i;
                float delay = preAtkTimeEagleToma - 0.08 + 0.04 * i;
                delayedAtkDisplayList.push_back( DelayedAttackDisplay( "eToma", xPos, player->y, delay ) );
            }
            break;
        }
        }
        break;
    }
    case COLONEL: {
        switch( atkNum ) {
        default:
            player->animationDisplayAmt = swordAtkTime;
            soundToPlay = screenDivSound;
            break;
        case 4:
            if( !move( player, player->facing == -1 ? 1 : 3, 2 ) ) return false;
            player->moveType = 3;
            player->animationDisplayAmt = stepAtkTime + moveAnimationTime;
            delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime, player->npc ) );
            break;
        case 5:
            player->animationDisplayAmt = lifeAtkTime;
            soundToPlay = screenDivSound;
            break;
        }
        break;
    }
    case SLASHMAN: {
        switch( atkNum ) {
        default:
            player->animationDisplayAmt = swordAtkTime;
            soundToPlay = swordSound;
            break;
        case 4:
            if( !move( player, player->facing == -1 ? 1 : 3, 2 ) ) return false;
            player->moveType = 3;
            player->animationDisplayAmt = stepAtkTime + moveAnimationTime;
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, player->npc ) );
            break;
        case 5:
            player->animationDisplayAmt = lifeAtkTime;
            soundToPlay = spinSlashSound;
            break;
        }
        break;
    }
    case GUTSMAN: {
        switch( atkNum ) {
        case 1:
            player->animationDisplayAmt = swordAtkTime;
            soundToPlay = gutsPunchSound;
            break;
        case 2:
            player->animationDisplayAmt = lifeAtkTime;
            delayedSoundList.push_back( DelayedSound( "hammer", preAtkTimeToma, player->npc ) );
            break;
        case 3:
            if( !move( player, player->facing == -1 ? 1 : 3, 2, true ) ) return false;
            player->moveType = 3;
            player->animationDisplayAmt = stepAtkTime + moveAnimationTime;
            delayedSoundList.push_back( DelayedSound( "punch", preAtkTimeGutsPunch, player->npc ) );
            break;
        case 4: {
            player->animationDisplayAmt = lifeAtkTime;
            delayedSoundList.push_back( DelayedSound( "hammer", preAtkTimeToma, player->npc ) );
            for( int i = 1; i <= 5; i++ ) {
                float xPos = player->x + player->facing * i;
                float delay = preAtkTimeToma + 0.06 * (i-1);
                delayedSoundList.push_back( DelayedSound( "shockwave", delay, player->npc ) );
                delayedAtkDisplayList.push_back( DelayedAttackDisplay( "shockwave", xPos, player->y, delay, player->facing ) );
            }
            break;
        }
        case 5: {
            player->animationDisplayAmt = lifeAtkTime;
            delayedSoundList.push_back( DelayedSound( "hammer", preAtkTimeToma, player->npc ) );
            for( int i = 1; i <= 3; i++ ) {
                int xPos = player->x + player->facing * i;
                float delay = preAtkTimeToma + 0.06 * (i-1);
                delayedSoundList.push_back( DelayedSound( "shockwave", delay, player->npc ) );
                for( int j = -1; j <= 1; j++ ) {
                    int yPos = player->y + j;
                    delayedAtkDisplayList.push_back( DelayedAttackDisplay( "shockwave", xPos, yPos, delay, player->facing ) );
                }
            }
            break;
        }
        }
        break;
    }
    }

    for( int i = 0; i < atkCoords.size(); i++ ) delayedDmgList.push_back( atkCoords[i] );
    
    player->energy -= cost;
    if( !player->npc ) {
        player->totalEnergyChange -= cost;
        player->lastAtkCost = cost;
        delayedPrevEnergyList.push_back( DelayedEnergyDisplay( player->x, player->y, -cost, 0, iconDisplayTime ) );
    }
    player->animationType = 1;
    player->actionNumber = atkNum;
    player->currentSwordAtkTime = player->animationDisplayAmt;
    player->x2 = player->x;
    player->y2 = player->y;
    if( !player->npc ) {
        if( player->x != 6 ) npcAbleToAct = true;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= cost;
    }
    if( soundToPlay != nullptr ) Mix_PlayChannel( player->npc ? 6 : 0, soundToPlay, 0 );
    return true;
}
void App::face( Player* const player, int dir ) {
    // Facing directions:   // -1 = Left    // 1 = Right
    if( ( dir == -1 || dir == 1 ) && player->facing != dir ) {
        player->facing = dir;
        player->x2 = player->x;
        player->y2 = player->y;
        player->moveType = 1;
        player->animationType = 0;
        player->animationDisplayAmt = moveAnimationTime;
        if( !player->npc ) npcAbleToAct = true;
    }
}
bool App::move(Player* const player, int dir, int dist, bool forced) {
    // Move (dist) panels in a (direction) - Returns true / false for if the Player acted
    // If (forced), force the player to move - Used only with GutsMan's Dash Punch
    bool acted = false;
    bool turned = false;

    player->x2 = player->x;
    player->y2 = player->y;

    // Move Up
    if( dir == 0 && isPanelValid( player->x, player->y + dist, player->npc ) ) {
        player->y += dist;
        player->moveDir = 0;
        acted = true;
    }
    // Move Left
    else if ( dir == 1 ) {
        if( player->facing != -1 ) {
            face( player, -1 );
            acted = true;
            turned = true;
        }
        if ( isPanelValid( player->x - dist, player->y, player->npc, forced ) ) {
            player->x -= dist;
            player->moveDir = 1;
            acted = true;
        }
        // If the Player only turned around, and didn't move
        else if( turned ) return true;
    }
    // Move Down
    else if( dir == 2 && isPanelValid( player->x, player->y - dist, player->npc ) ) {
        player->y -= dist;
        player->moveDir = 2;
        acted = true;
    }
    // Move Right
    else if ( dir == 3 ) {
        if( player->facing != 1 ) {
            face( player, 1 );
            acted = true;
            turned = true;
        }
        if ( isPanelValid( player->x + dist, player->y, player->npc, forced ) ) {
            player->x += dist;
            player->moveDir = 3;
            acted = true;
        }
        // If the Player only turned around, and didn't move
        else if( turned ) return true;
    }

    if( !acted ) return false;
    player->moveType = 2;
    player->animationType = 0;
    player->animationDisplayAmt = moveAnimationTime;

    if( player->npc ) return true;
    npcAbleToAct = ( player->x != -1 && player->x != 6 );

    // Walk off Cracked Panel -> Cracked panel becomes a Hole
    if( ( player->x2 != player->x || player->y2 != player->y ) && board.map[player->x2][player->y2].state == -1 ) {
        board.map[player->x2][player->y2].state = -2;
        Mix_PlayChannel( 2, panelBreakSound, 0 );
    }

    // Handle other Special Panels for the Player
    switch( board.map[player->x][player->y].state ) {
    default:
        player->onHolyPanel = false;
        break;
    case 2:     // Poison Panels damage you when you step on them
        player->energy -= poisonLossAmt;
        if( player->energy < 0 ) player->energy = 0;
        player->totalEnergyChange -= poisonLossAmt;
        currentEnergyGain -= poisonLossAmt;
        delayedPrevEnergyList.push_back( DelayedEnergyDisplay( player->x, player->y, -poisonLossAmt, 2, iconDisplayTime ) );
        chargeDisplayPlusAmt = 0;
        chargeDisplayMinusAmt = iconDisplayTime;
        player->hurtDisplayAmt = poisonHurtTime;
        break;
    case 3:     // Holy Panels can save you some Energy for Attack Costs
        player->onHolyPanel = true;
        break;
    }
    
    return true;
}

void App::hitPanel( int xPos, int yPos, int dmg, bool fromNpc ) {
    if( xPos < 0 || xPos > 5 || yPos < 0 || yPos > 5 ) return;
    
    board.map[xPos][yPos].rockAtkInd = rockDmgTime;
    board.map[xPos][yPos].prevDmg = 0;

    // Attacking a Rock
    if( board.map[xPos][yPos].rockHP > 0 ) {
        if ( board.map[xPos][yPos].rockHP == 1 ) { board.map[xPos][yPos].prevDmg = 1; }
        else { board.map[xPos][yPos].prevDmg = dmg; }
        board.map[xPos][yPos].rockHP -= dmg;
        if ( board.map[xPos][yPos].rockHP <= 0 && !board.map[xPos][yPos].bigRockDeath ) { Mix_PlayChannel( 4, rockBreakSound, 0 ); }
    }
    // NPC attacks against the Player
    else if( fromNpc && xPos == player1->x && yPos == player1->y ) {
        player1->energy -= dmg * npcDmgAmt;
        player1->totalEnergyChange -= dmg * npcDmgAmt;
        currentEnergyGain -= dmg * npcDmgAmt;
        chargeDisplayPlusAmt = 0;
        chargeDisplayMinusAmt = iconDisplayTime;
        player1->hurtDisplayAmt = hurtAnimationTime;
        delayedPrevEnergyList.push_back( DelayedEnergyDisplay( xPos, yPos, -dmg * npcDmgAmt, 3, iconDisplayTime ) );
        if ( player1->energy <= 0 ) {
            player1->hp = 0;
        }
        else if ( player1->animationType == 0 ) {
            player1->animationType = -1;
            player1->animationDisplayAmt += pauseAnimationTime / 2;
        }
        else if ( player1->animationType == -2 ) {
            player1->animationType = -1;
            player1->animationDisplayAmt = pauseAnimationTime;
        }
        Mix_PlayChannel( 1, hurtSound, 0 );
    }
    // Player attacks on NPCs
    else if( !fromNpc ) {
        for( int i = 0; i < npcList.size(); i++ ) {
            if( xPos == npcList[i]->x && yPos == npcList[i]->y ) {
                npcList[i]->hp -= dmg;
                npcList[i]->timesHit++;
                npcList[i]->hurtDisplayAmt = hurtAnimationTime;

                if( npcList[i]->animationType == 0 ) {
                    npcList[i]->animationType = -1;
                    npcList[i]->animationDisplayAmt += 0.1125;
                }
                else if( npcList[i]->animationType == -2 ) {
                    npcList[i]->animationType = -1;
                    npcList[i]->animationDisplayAmt = pauseAnimationTime;
                }
                Mix_PlayChannel( 1, hurtSound, 0 );
            }
        }
    }
}

// Map Functions
bool App::isPanelValid(int xPos, int yPos, bool npc, bool forced) {
    // Checks if a specific panel allows the Player to walk on it
    if( xPos == -1 && yPos == 0 && !npc ) return true;			// Start Panel
    if( xPos ==  6 && yPos == 5 )         return true;			// Goal Panel
    if( xPos < 0 || xPos > 5 || yPos < 0 || yPos > 5 ) return false;		// Map boundaries are 0 to 5 on both X and Y axes
    if( board.map[xPos][yPos].state <= -2 ) return false;		// Players cannot walk onto a Hole
    if( board.map[xPos][yPos].rockHP > 2 ) return false;        // Players cannot walk into a Rock,
    if( !forced && board.map[xPos][yPos].rockHP > 0 ) return false;		// Players cannot walk into a Rock, Unless "forced"
                                                                        // Forced is only when the Player is GutsMan Dash Punching, and the rock at most 2 HP
    if( xPos == player1->x && yPos == player1->y ) return false;                 // NPCs can't walk onto the Player's position
    for( int i = 0; i < npcList.size(); i++ ) {
        if( xPos == npcList[i]->x && yPos == npcList[i]->y ) return false; }     // Players can't walk onto NPC positions
	return true;
}
void App::reset() {
    srand( time( 0 ) );
	board.clear();
    delete player1;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	
	delayedDmgList.clear();
    delayedSoundList.clear();
    delayedAtkDisplayList.clear();
    delayedPrevEnergyList.clear();
    for( int i = 0; i < npcList.size(); i++ ) delete npcList[i];
    npcList.clear();
    for( int i = 0; i < nextNpcList.size(); i++ ) delete nextNpcList[i];
    nextNpcList.clear();
    lvlsWithoutBoss = 0;
	currentEnergyGain = 0;
    
    menuNum = -1;
    menuSel = 0;
    reloadedTutorial = true;
}
void App::tutorial( int type ) {
	reset();
    player1 = new MegaMan();
    player1->npc = false;
	level = -1 * type;
	
    /*for ( int i = 1; i < 5; i++ ) {
        board.map[0][i].rockHP = 5;
        board.map[1][i].rockHP = 5;
        board.map[2][i].rockHP = 5;    board.map[2][i].state = -1;
        board.map[4][i].rockHP = 3;    board.map[4][i].item = 1;     board.map[4][i].rockType = 1;
        board.map[5][i].rockHP = 3;    board.map[5][i].item = 2;     board.map[5][i].rockType = 1;

        board.map[0][i].state = 2;
        board.map[1][i].state = 1;
        board.map[3][i].state = 3;
    }
    board.map[0][5].item = -1;    board.map[1][5].item = -1;    board.map[2][5].item = -1;
    board.map[4][0].item = -1;    board.map[5][0].item = -1;*/

    loadTutorialLevel();
}
void App::next() {
    player1->x = 0;		player1->y = 0;
    player1->x2 = 0;    player1->y2 = 0;

    for( int i = 0; i < npcList.size(); i++ ) delete npcList[i];
    npcList.clear();
    for( int i = 0; i < nextNpcList.size(); i++ ) nextNpcList[i]->animationDisplayAmt = 0;

    if( level >= 1 ) {
	    level++;
        loadLevel();
    }
    else if( level <= -1 ) {
        player1->energy = player1->energyDisplayed = 0;
        level--;
        loadTutorialLevel();
        reloadedTutorial = true;
    }

	player1->facing = 1;
	currentEnergyGain = 0;
    player1->actionNumber = -1;
    menuNum = -1;
    menuSel = true;
}

void App::loadBossLevel( int type ) {
    Mix_PlayChannel( 6, bossAppearSound, 0 );

    player1->energy = 10000;

    Player *boss;
    int bossType = rand() % NUM_CHAR_TYPES;
    switch( bossType ) {
    default:
    case MEGAMAN:       boss = new MegaMan(); break;
    case PROTOMAN:      boss = new ProtoMan(); break;
    case TOMAHAWKMAN:   boss = new TomahawkMan(); break;
    case COLONEL:       boss = new Colonel(); break;
    case SLASHMAN:      boss = new SlashMan(); break;
    case GUTSMAN:       boss = new GutsMan(); break;
    }
    boss->x = 5;     boss->y = 4;     boss->x2 = 5;    boss->y2 = 4;
    boss->facing = -1;
    boss->npc = true;
    boss->hp = 5;
    boss->type = bossType;
    boss->energy = 2000;
    npcList.push_back( boss );

    board.generateBossLevel( rand() % 10 );
    loadPrevNpc( 1, 50 );
}
void App::loadLevel( int num ) {
    if( level == 1 ) player1->energy = player1->energyDisplayed = startingEnergy;
    int cappedLvl = level;		if( cappedLvl > 60 ) { cappedLvl = 60; }

    // Bounds used in determining difficulty - which is based on current level number
    // bound1 = Percent chance of Easy level
    int bound1 = cappedLvl <= 20
        ? 90 - cappedLvl
        : 70 - ( cappedLvl - 20 ) * 2;
    // bound2 = 100% - Percent chance of Hard level
    int bound2 = cappedLvl <= 20
        ? 100
        : 100 - ( cappedLvl - 20 ) * 2;
    // Level		Easy   Medium	Hard
    //   0          90%     10%      0%
    //   5          85%     15%      0%
    //  10          80%     20%      0%
    //  15          75%     25%      0%
    //  20          70%     30%      0%
    //  25          60%     30%     10%
    //  30          50%     30%     20%
    //  35          40%     30%     30%
    //  40          30%     30%     40%
    //  55          20%     30%     50%
    //  60+         10%		30%     60%

    if( num <= -1 ) num = rand() % 100;                 // Random number between 0 and 99 - used to determine difficulty
    int diff = 0;				                        // difficulty 0, 1, 2 = easy, medium, hard
    if( level == 1 ) {}
    else if( num >= bound2 ) diff = 2;
    else if( num >= bound1 ) diff = 1;
    lvlDiff = diff;

    int gain = 0;       // Gain determines avg "profit" per level
    if     ( currentGameDiff == 0 ) { gain =  2 - diff * 9; }
    else if( currentGameDiff == 1 ) { gain =  0 - diff * 9; }
    else if( currentGameDiff == 2 ) { gain = -2 - diff * 9; }
    else if( currentGameDiff == 3 ) { gain = -4 - diff * 9; }
    else if( currentGameDiff == 4 ) { gain = -6 - diff * 9; }

    board.generateLevel( cappedLvl, diff, gain );

    // Previous Bosses that entered the next level
    loadPrevNpc(diff, gain);

    // Chance for Boss to appear
    if( currentGameDiff >= 2 && diff != 0  && level >= 20 ) {
        if ( lvlsWithoutBoss >= 10
        || ( diff >= 1 && rand() % 10 == 0 ) ) {

            int xPos = -1, yPos = -1;
            for( int y = 5; y >= 1; y-- ) {
                if( xPos != -1 && yPos != -1 ) break;
                for( int x = 5; x >= 4; x-- ) {
                    if( x == 5 && y == 5 ) continue;
                    if( isPanelValid( x, y ) ) {
                        xPos = x;
                        yPos = y;
                        break;
                    }
                }
            }
            if ( xPos != -1 && yPos != -1 ) {
                Player *boss;
                int bossType = rand() % NUM_CHAR_TYPES;
                switch( bossType ) {
                default:
                case MEGAMAN:       boss = new MegaMan(); break;
                case PROTOMAN:      boss = new ProtoMan(); break;
                case TOMAHAWKMAN:   boss = new TomahawkMan(); break;
                case COLONEL:       boss = new Colonel(); break;
                case SLASHMAN:      boss = new SlashMan(); break;
                case GUTSMAN:       boss = new GutsMan(); break;
                }
                boss->x = xPos;     boss->y = yPos;     boss->x2 = xPos;    boss->y2 = yPos;
                boss->facing = -1;
                boss->npc = true;
                boss->hp = currentGameDiff + diff + 1;
                boss->type = bossType;
                int minEnergy = 200;
                if( diff == 1 ) minEnergy = 300;
                else if( diff == 2 ) minEnergy = 400;
                boss->energy = max( abs( gain ) * 25, minEnergy );
                npcList.push_back( boss );
                Mix_PlayChannel( 6, bossAppearSound, 0 );
            }
        }
    }

    npcList.empty() ? lvlsWithoutBoss++ : lvlsWithoutBoss = 0;
}
void App::loadTutorialLevel( bool reload ) {
    // Boss Levels are at Levels -8 to -17
    // After the last Boss Level, it repeats back to the first Boss Level
    if( level < -17 ) level = -8;
    // Levels -1 to -6 are Tutorial Levels
    // Level -7 is a "Training" Room
    if( level >= -7 ) board.generateTrainingLevel( -level, reload );
    // Boss Level
    else loadBossLevel( -8 - level );

    // Handle Reloading Tutorials, so that Players cannot fail a Tutorial Level
    if( !reload ) return;
    reloadedTutorial = true;
    board.upgradeAnimationAll();
    
    if( level == -7 ) {
        int pX = player1->x, pY = player1->y;
        if( pX == -1 || pX == 6 ) return;
        board.map[pX][pY].reset();
        if( pX != 0 ) {
            for( int i = 0; i < 6; i++ ) board.map[pX][i].reset();
        }

        switch( rand() % 5 ) {
        default: break;
        case 2:
            for( int x = 0; x < 6; x++ ) {
                for( int y = 0; y < 6; y++ ) {
                    board.map[x][y].state = 1;
                }
            }
            break;
        case 3:
            for( int x = 0; x < 6; x++ ) {
                for( int y = 0; y < 6; y++ ) {
                    board.map[x][y].state = 2;
                }
            }
            break;
        case 4:
            for( int x = 0; x < 6; x++ ) {
                for( int y = 0; y < 6; y++ ) {
                    board.map[x][y].state = 3;
                }
            }
            break;
        }

        board.map[0][0].state = 0;
        board.map[5][5].state = 0;
    }
}
void App::loadPrevNpc(int diff, int gain) {
    while( !nextNpcList.empty() ) {
        Player* npc = nextNpcList[nextNpcList.size() - 1];
        nextNpcList.pop_back();

        int xPos = -1, yPos = -1;
        for( int y = 5; y >= 1; y-- ) {
            if( xPos != -1 && yPos != -1 ) break;
            for( int x = 5; x >= 4; x-- ) {
                if( x == 5 && y == 5 ) continue;
                if( isPanelValid( x, y ) ) {
                    xPos = x;
                    yPos = y;
                    break;
                }
            }
        }
        if( xPos != -1 && yPos != -1 ) {
            npc->facing = -1;
            int minEnergy = 200;
            if( diff == 1 ) minEnergy = 300;
            else if( diff == 2 ) minEnergy = 400;
            npc->energy += abs( gain ) * 25;
            npc->energy = max( npc->energy, minEnergy );
            npc->x = xPos;
            npc->y = yPos;
            npcList.push_back( npc );
            Mix_PlayChannel( 6, bossAppearSound, 0 );
        }
        else delete npc;
    }
}

void App::spawnRandomItem( int xPos, int yPos ) {
    int itemToPut = 1;

    // Try to spawn an item in a random location within 1 panel of the given coordinates
    for( int i = 0; i < 30; i++ ) {
        int randX = rand() % 3 - 1 + xPos;
        if( randX < 0 ) randX = 0;
        if( randX > 5 ) randX = 5;
        int randY = rand() % 3 - 1 + yPos;
        if( randY < 0 ) randY = 0;
        if( randY > 5 ) randY = 5;
        if( isPanelValid( randX, randY ) && board.map[randX][randY].item == 0 ) {
            board.map[randX][randY].item = itemToPut;
            board.map[randX][randY].upgradeInd = itemUpgradeTime;
            return;
        }
    }

    // Try to spawn an item in a random location within 2 panels of the given coordinates
    for( int i = 0; i < 30; i++ ) {
        int randX = rand() % 5 - 2 + xPos;
        if( randX < 0 ) randX = 0;
        if( randX > 5 ) randX = 5;
        int randY = rand() % 5 - 2 + yPos;
        if( randY < 0 ) randY = 0;
        if( randY > 5 ) randY = 5;
        if( isPanelValid( randX, randY ) && board.map[randX][randY].item == 0 ) {
            board.map[randX][randY].item = itemToPut;
            board.map[randX][randY].upgradeInd = itemUpgradeTime;
            return;
        }
    }

    // Try to spawn an item in a random location within 3 panels of the given coordinates
    for( int i = 0; i < 30; i++ ) {
        int randX = rand() % 7 - 3 + xPos;
        if( randX < 0 ) randX = 0;
        if( randX > 5 ) randX = 5;
        int randY = rand() % 7 - 3 + yPos;
        if( randY < 0 ) randY = 0;
        if( randY > 5 ) randY = 5;
        if( isPanelValid( randX, randY ) && board.map[randX][randY].item == 0 ) {
            board.map[randX][randY].item = itemToPut;
            board.map[randX][randY].upgradeInd = itemUpgradeTime;
            return;
        }
    }

    // Try to spawn an item in a random location within 4 panels of the given coordinates
    for( int i = 0; i < 30; i++ ) {
        int randX = rand() % 9 - 4 + xPos;
        if( randX < 0 ) randX = 0;
        if( randX > 5 ) randX = 5;
        int randY = rand() % 9 - 4 + yPos;
        if( randY < 0 ) randY = 0;
        if( randY > 5 ) randY = 5;
        if( isPanelValid( randX, randY ) && board.map[randX][randY].item == 0 ) {
            board.map[randX][randY].item = itemToPut;
            board.map[randX][randY].upgradeInd = itemUpgradeTime;
            return;
        }
    }
}
void App::upgradeItems() {
    // Upgrades all items, then spawns Bonus Green Energies
    // Red(-50) -> Dissapears
    // Green(100) -> Blue(150)
    for ( int i = 0; i < 6; i++ ) {
        for ( int j = 0; j < 6; j++ ) {
            if ( board.map[i][j].item == -1 ) {
                board.map[i][j].item = 0;
                if( board.map[i][j].rockHP > 0 ) board.map[i][j].rockType = 0;
                board.map[i][j].upgradeInd = itemUpgradeTime;
            }
            else if ( board.map[i][j].item == 1 ) {
                board.map[i][j].item = 2;
                board.map[i][j].upgradeInd = itemUpgradeTime;
            }
        }
    }

    // Bonus Green Energies
    for( int i = 0; i < 1; i++ ) spawnRandomItem( rand() % 6, rand() % 6 );

    Mix_PlayChannel( 1, recoverSound, 0 );
}

void App::aiAction( Player* const npc ) {
    // Generates a random move for the NPC's    // Checks for Attack options, then Movement options
    int randNum2 = rand() % 2;
    int randNum5 = rand() % 5;

    int x = npc->x, y = npc->y, dir = npc->facing;
    int pX = player1->x, pY = player1->y;
    if( randNum5 > 1 ) { pX = player1->x2; pY = player1->y2; }

    // NPC's will attempt to run when out of Energy to attack
    int minEnergy = 0;

    // Check attack options
    switch( npc->type ) {
    case MEGAMAN: {
        minEnergy = 150;
        // Check Life Sword
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX ) && abs( y - pY ) <= 1 ) {
            if( attack( npc, 7 ) ) return;
        }
        // Check Step Sword
        if( rand() % 3 && x + 3 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 6 ) ) return;
        }
        // Check Spin Sword
        if( rand() % 3 && abs( x - pX ) <= 1 && abs( y - pY ) <= 1 ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Cross Sword
        if( rand() % 3 && ( x + 1 * dir == pX && y == pY )
                          || ( ( x + 2 * dir == pX || x == pX ) && abs( y - pY ) == 1 ) ) {
            if( attack( npc, 4 ) ) return;
        }
        // Check Wide Sword
        if( rand() % 3 && x + 1 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 3 ) ) return;
        }
        // Check Long Sword
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX ) && y == pY ) {
            if( attack( npc, 2 ) ) return;
        }
        // Check Sword
        if( rand() % 3 &&  x + 1 * dir == pX && y == pY ) {
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    case PROTOMAN: {
        minEnergy = 150;
        // Check Proto Cross
        if( rand() % 3 && ( ( ( x + 1 * dir == pX || x + 2 * dir == pX || x + 3 * dir == pX ) && y == pY )
                          || ( x + 2 * dir == pX && abs( y - pY ) <= 1 ) ) ) {
            if( attack( npc, 6 ) ) return;
        }
        // Check Hero Sword
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX || x + 3 * dir == pX ) && y == pY ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Step Sword
        if( rand() % 3 && x + 3 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 4 ) ) return;
        }
        // Check Wide Sword
        if( rand() % 3 && x + 1 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 3 ) ) return;
        }
        // Check Long Sword
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX ) && y == pY ) {
            if( attack( npc, 2 ) ) return;
        }
        // Check Sword
        if( rand() % 3 && x + 1 * dir == pX && y == pY ) {
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    case TOMAHAWKMAN: {
        minEnergy = 125;
        // Check Eagle Tomahawk Earthquake
        if( rand() % 3 && y == pY && ( ( dir == -1 && pX < x ) || ( dir == 1 && pX > x ) ) ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Tomahawk Swing
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX ) && abs( y - pY ) <= 1 ) {
            if( attack( npc, 4 ) ) return;
            if( attack( npc, 3 ) ) return;
        }
        // Check Wide Swing
        if( rand() % 3 && x + 1 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 2 ) ) return;
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    case COLONEL: {
        minEnergy = 125;
        // Check Z-Saber Neo Screen Divide
        if( rand() % 3 && ( x + 1 * dir == pX && y == pY )
                          || ( ( x + 2 * dir == pX || x + 1 * dir == pX || x == pX ) && abs( y - pY ) == 1 ) ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Step Cross Divide
        if( rand() % 3 && ( ( x + 3 * dir == pX && y == pY )
                          || ( ( x + 4 * dir == pX || x + 2 * dir == pX ) && abs( y - pY ) == 1 ) ) ) {
            if( attack( npc, 4 ) ) return;
        }
        // Check Screen Divide Down
        if( rand() % 3 && ( ( x + 1 * dir == pX && y == pY )
                          || ( x == pX && y + 1 == pY )
                          || ( x + 2 * dir == pX && y - 1 == pY ) ) ) {
            if( attack( npc, 3 ) ) return;
        }
        // Check Screen Divide Up
        if( rand() % 3 && ( ( x + 1 * dir == pX && y == pY )
                          || ( x == pX && y - 1 == pY )
                          || ( x + 2 * dir == pX && y + 1 == pY ) ) ) {
            if( attack( npc, 2 ) ) return;
        }
        // Check Arc Divide
        if( rand() % 3 && ( ( x + 1 * dir == pX && y == pY )
                          || ( x == pX && abs( y - pY ) == 1 ) ) ) {
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    case SLASHMAN: {
        minEnergy = 100;
        // Check Tornado Spin Slash
        if( rand() % 3 && abs( x - pX ) <= 1 && abs( y - pY ) <= 1 ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Step Cross
        if( rand() % 3 && ( ( x + 3 * dir == pX && y == pY )
                          || ( ( x + 4 * dir == pX || x + 2 * dir == pX ) && abs( y - pY ) == 1 ) ) ) {
            if( attack( npc, 4 ) ) return;
        }
        // Check Cross Slash
        if( rand() % 3 && ( ( x + 1 * dir == pX && y == pY )
                          || ( ( x + 2 * dir == pX || x == pX ) && abs( y - pY ) == 1 ) ) ) {
            if( attack( npc, 3 ) ) return;
        }
        // Check Wide Slash
        if( rand() % 3 && x + 1 * dir == pX && abs( y - pY ) <= 1 ) {
            if( attack( npc, 2 ) ) return;
        }
        // Check Long Slash
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX ) && y == pY ) {
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    case GUTSMAN: {
        minEnergy = 125;
        // Check Guts Hammer Shockwave Slam
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX || x + 3 * dir == pX ) && abs( y - pY ) <= 1 ) {
            if( attack( npc, 5 ) ) return;
        }
        // Check Guts Hammer Shockwave
        if( rand() % 3 && y == pY && ( ( dir == -1 && pX < x ) || ( dir == 1 && pX > x ) ) ) {
            if( attack( npc, 4 ) ) return;
        }
        // Check Guts Dash Punch
        if( rand() % 3 && ( x + 1 * dir == pX || x + 2 * dir == pX || x + 3 * dir == pX ) && y == pY ) {
            if( attack( npc, 3 ) ) return;
        }
        // Check Guts Hammer and Guts Punch
        if( rand() % 3 && x + 1 * dir == pX && y == pY ) {
            if( attack( npc, 2 ) ) return;
            if( attack( npc, 1 ) ) return;
        }
        break;
    }
    }

    // -- Movement -- //
    // Attempt to run to the Next Level if out of Energy
    if( npc->energy < minEnergy ) {
        if( x > y || ( x == y && randNum2 ) ) {
            if( move( npc, 0 ) ) return;
            if( isPanelValid( x+1, y, true ) ) {
                if( move( npc, 3 ) ) return;
            }
            if( move( npc, 2 ) ) return;
            if( isPanelValid( x-1, y, true ) ) {
                if( move( npc, 1 ) ) return;
            }
        }
        else {
            if( isPanelValid( x+1, y, true ) ) {
                if( move( npc, 3 ) ) return;
            }
            if( move( npc, 0 ) ) return;
            if( isPanelValid( x-1, y, true ) ) {
                if( move( npc, 1 ) ) return;
            }
            if( move( npc, 2 ) ) return;
        }
    }
    // If the Player is within one panel
    else if( abs( x - pX ) <= 1 && abs( y - pY ) <= 1 ) {
        // Positions behind the NPC
        if( x - dir == pX ) {
            if( npc->type == PROTOMAN || npc->type == TOMAHAWKMAN || npc->type == GUTSMAN || randNum2 ) face( npc, -dir );
            else move( npc, dir );
            return;
        }
        // Positions in front of the NPC
        else if( x + dir == pX ) {
            if( y == pY ) move( npc, dir );
            else move( npc, y - pY + 1 );
                         // y - pY + 1:  // If the Player is above, move the NPC up
                                         // If the Player if below, move the NPC down
            return;
        }
        // The Position directly above the NPC
        else if( y+1 == pY ) {
            if( move( npc, 0 ) ) return;
            if( !isPanelValid( x + dir, y, true ) || !isPanelValid( x + dir, y+1, true ) ) {
                if( move( npc, dir+2 ) ) return;
            }
            if( move( npc, 2-dir ) ) return;
        }
        // The Position directly below the NPC
        else {
            if( move( npc, 2 ) ) return;
            if( !isPanelValid( x + dir, y, true ) || !isPanelValid( x + dir, y-1, true ) ) {
                if( move( npc, dir+2 ) ) return;
            }
            if( move( npc, 2-dir ) ) return;
            if( move( npc, 0 ) ) return;
        }
    }
    // If the Player is more than one panel away
    else {
        // Up
        if( pX == x && pY > y ) {
            if( move( npc, 0 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
            }
            else {
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
            }
        }
        // Up Left
        else if( pX < x && pY > y ) {
            if( move( npc, 0 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
            }
            else {
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
            }
            if( move( npc, 1 ) ) return;
        }
        // Left
        else if( pX < x && pY == y ) {
            if( move( npc, 1 ) ) return;
            if( randNum2 ) {
                if( move( npc, 0 ) ) return;
                if( move( npc, 2 ) ) return;
            }
            else {
                if( move( npc, 2 ) ) return;
                if( move( npc, 0 ) ) return;
            }
        }
        // Down Left
        else if( pX < x && pY < y ) {
            if( move( npc, 2 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
                if( move( npc, 0 ) ) return;
            }
            else {
                if( move( npc, 0 ) ) return;
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
            }
            if( move( npc, 1 ) ) return;
        }
        // Down
        else if( pX == x && pY < y ) {
            if( move( npc, 2 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
            }
            else {
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
            }
            if( move( npc, 0 ) ) return;
        }
        // Down Right
        else if( pX > x && pY < y ) {
            if( move( npc, 2 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
                if( move( npc, 0 ) ) return;
            }
            else {
                if( move( npc, 0 ) ) return;
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
            }
            if( move( npc, 3 ) ) return;
        }
        // Right
        else if( pX > x && pY == y ) {
            if( move( npc, 3 ) ) return;
            if( randNum2 ) {
                if( move( npc, 0 ) ) return;
                if( move( npc, 2 ) ) return;
            }
            else {
                if( move( npc, 2 ) ) return;
                if( move( npc, 0 ) ) return;
            }
        }
        // Up Right
        else {
            if( move( npc, 0 ) ) return;
            if( randNum2 ) {
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
            }
            else {
                if( isPanelValid( x-1, y, true ) ) {
                    if( move( npc, 1 ) ) return;
                }
                if( isPanelValid( x+1, y, true ) ) {
                    if( move( npc, 3 ) ) return;
                }
            }
            if( move( npc, 3 ) ) return;
        }
    }
}

void App::debugTest() {
    reset();
    player1 = new GutsMan();
    player1->npc = false;
    player1->animationDisplayAmt = menuExitTime;
    player1->energy = player1->energyDisplayed = 10000;

    level = 1;
    for( int x = 0; x < 6; x++ ) {
        for( int y = 0; y < 6; y++ ) {
            board.map[x][y].state = 1;
        }
    }
}
