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
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
using namespace std;

const float fixed_timestep = 0.0166666f;
const int max_steps = 6;
const float pi = 3.14159265359f;

const int swordCost =  50;
const int longCost  =  75;
const int wideCost  = 100;
const int wideCost2 =  75;
const int crossCost = 125;
const int spinCost  = 150;
const int stepCost  = 175;
const int stepCost2 = 150;
const int lifeCost  = 250;

const int heroCost  = 150;
const int protoCost = 225;

const int vDivideCost    =  75;
const int upDivideCost   = 100;
const int downDivideCost = 125;
const int xDivideCost    = 175;
const int zDivideCost    = 250;

const int tomaCostA1     =  75;
const int tomaCostB1     = 125;
const int tomaCostA2     = 125;
const int tomaCostB2     = 175;
const int eTomaCost      = 250;

const int crossCost2     = 100;
const int stepCrossCost  = 175;
const int spinSlashCost  = 225;

const int startingEnergy = 300;
const int energyGainAmt  = 100;			    // How much Energy the player gains when moving to a Pick-Up
const int energyGainAmt2 = 150;
const int energyLossAmt  =  50;
const int itemWorth = energyGainAmt / 50;	// Item "Worth" is used to generate Levels

const float iconDisplayTime = 0.45;		// Energy Gain Icon Display time
const float boxDmgTime = 0.4;			// Box HP damage indicator time
const float itemUpgradeTime = 0.32;
const float charDeathTime = boxDmgTime * 0.5;
const float npcWaitTime = 0.125;

const float moveAnimationTime = 0.175;	// Player movement time
const float hurtAnimationTime = 0.5;
const float pauseAnimationTime = moveAnimationTime / 2.0 + 0.2;   // .2875
const float menuExitTime = 0.2;         // Delay after coming out of a Menu

const float preAtkTime   = 0.10;
const float preAtkTimeToma      = 0.20;
const float preAtkTimeEagleToma = 0.27;

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

Player::Player() : hp(1), x(0), y(0), x2(0), y2(0), facing(3), moving(false), turning(false), moveDir(-1), energy(0), type(0), npc(true), timesHit(0),
                   animationType(-2), selSwordAnimation(-1), animationDisplayAmt(0.0), currentSwordAtkTime(0.0),
                   hurtDisplayAmt(0.0), npcActionTimer(0.0), deathDisplayAmt( charDeathTime ) {}
Tile::Tile() : state(0), item(-1), boxHP(0), boxType(0), boxAtkInd(0.0), upgradeInd(0.0), bigBoxDeath(false), isPurple(false), prevDmg(0) {}
DelayedHpLoss::DelayedHpLoss() : dmg(0), xPos(0), yPos(0), delay(0.0), npc(false) {}
DelayedHpLoss::DelayedHpLoss( int dmgAmt, int x, int y, float delayTime, bool isNpc ) {
    dmg = dmgAmt;
    xPos = x;
    yPos = y;
    delay = delayTime;
    npc = isNpc;
}
DelayedSound::DelayedSound() : soundName(""), delay(0.0), npc(false) {}
DelayedSound::DelayedSound( string name, float delayTime, bool isNpc ) {
    soundName = name;
    delay = delayTime;
    npc = isNpc;
}
DelayedETomaDisplay::DelayedETomaDisplay() : xPos(0), yPos(0), delay(0.0), animationTimer(0.1) {}
DelayedETomaDisplay::DelayedETomaDisplay( int x, int y, float delayTime ) : animationTimer( 0.1 ) {
    xPos = x;
    yPos = y;
    delay = delayTime;
}

GLuint loadTexture(const char *image_path) {
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
	return (1.0 - t)*v0 + t*v1;
}
float easeIn(float& from, float& to, float& time) {
	float tVal = time*time*time*time*time;
	return (1.0f - tVal)*from + tVal*to;
}
float easeOut(float& from, float& to, float& time) {
	float oneMinusT = 1.0f - time;
	float tVal = 1.0f - (oneMinusT * oneMinusT * oneMinusT *
		oneMinusT * oneMinusT);
	return (1.0f - tVal)*from + tVal*to;
}
float easeInOut(float& from, float& to, float& time) {
	float tVal;
	if (time > 0.5) {
		float oneMinusT = 1.0f - ((0.5f - time)*-2.0f);
		tVal = 1.0f - ((oneMinusT * oneMinusT * oneMinusT * oneMinusT *
			oneMinusT) * 0.5f);
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
			 menuDisplay(true), quitMenuOn(false), resetMenuOn(false), diffSelMenuOn(false), trainMenuOn(false), menuSel(true), charSel(0),
             player1(Player()), level(0), currentEnergyGain(0), gameDiffSel(2), currentGameDiff(2), npcAbleToAct(false), lvlsWithoutBoss(0),
			 musicSel(1), musicMuted(true),
			 chargeDisplayPlusAmt(0.0), chargeDisplayMinusAmt(0.0),
			 itemAnimationAmt(0.0), bgAnimationAmt(0.0), musicSwitchDisplayAmt(0.0) {
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

	textSheet1A = loadTexture("Pics\\Texts 1 A.png");
	textSheet1B = loadTexture("Pics\\Texts 1 B.png");
	textSheet1C = loadTexture("Pics\\Texts 1 C.png");
	textSheet2A = loadTexture("Pics\\Texts 2 A.png");
	textSheet2B = loadTexture("Pics\\Texts 2 B.png");
	textSheet2C = loadTexture("Pics\\Texts 2 C.png");

    // Character Textures
    {
	megamanMoveSheet  = loadTexture("Pics\\Characters\\Mega Man Move Sheet.png");
	megamanAtkSheet   = loadTexture("Pics\\Characters\\Mega Man Atk Sheet.png");
    megamanHurtSheet  = loadTexture("Pics\\Characters\\Mega Man Hurt Sheet.png");
    protoMoveSheet    = loadTexture("Pics\\Characters\\Proto Man Move Sheet.png");
    protoAtkSheet     = loadTexture("Pics\\Characters\\Proto Man Atk Sheet.png");
    protoHurtSheet    = loadTexture("Pics\\Characters\\Proto Man Hurt Sheet.png");
    colonelMoveSheet  = loadTexture("Pics\\Characters\\Colonel Move Sheet.png");
    colonelAtkSheet   = loadTexture("Pics\\Characters\\Colonel Atk Sheet.png");
    colonelHurtSheet  = loadTexture("Pics\\Characters\\Colonel Hurt Sheet.png");
    tmanMoveSheet     = loadTexture("Pics\\Characters\\Toma Move Sheet.png");
    tmanAtkSheet1     = loadTexture("Pics\\Characters\\Toma Atk Sheet 1.png");
    tmanAtkSheet2     = loadTexture("Pics\\Characters\\Toma Atk Sheet 2.png");
    slashmanMoveSheet = loadTexture("Pics\\Characters\\Slash Move Sheet.png");
    slashmanAtkSheet  = loadTexture("Pics\\Characters\\Slash Atk Sheet.png");

    darkMegamanMoveSheet  = loadTexture("Pics\\NPC\\Dark Mega Man Move Sheet.png");
    darkMegamanAtkSheet   = loadTexture("Pics\\NPC\\Dark Mega Man Atk Sheet.png");
    darkMegamanHurtSheet  = loadTexture("Pics\\NPC\\Dark Mega Man Hurt Sheet.png");
    darkProtoMoveSheet    = loadTexture("Pics\\NPC\\Dark Proto Man Move Sheet.png");
    darkProtoAtkSheet     = loadTexture("Pics\\NPC\\Dark Proto Man Atk Sheet.png");
    darkProtoHurtSheet    = loadTexture("Pics\\NPC\\Dark Proto Man Hurt Sheet.png");
    darkColonelMoveSheet  = loadTexture("Pics\\NPC\\Dark Colonel Move Sheet.png");
    darkColonelAtkSheet   = loadTexture("Pics\\NPC\\Dark Colonel Atk Sheet.png");
    darkColonelHurtSheet  = loadTexture("Pics\\NPC\\Dark Colonel Hurt Sheet.png");
    darkTmanMoveSheet     = loadTexture("Pics\\NPC\\Dark Toma Move Sheet.png");
    darkTmanAtkSheet1     = loadTexture("Pics\\NPC\\Dark Toma Atk Sheet 1.png");
    darkTmanAtkSheet2     = loadTexture("Pics\\NPC\\Dark Toma Atk Sheet 2.png");
    darkSlashmanMoveSheet = loadTexture("Pics\\NPC\\Dark Slash Move Sheet.png");
    darkSlashmanAtkSheet  = loadTexture("Pics\\NPC\\Dark Slash Atk Sheet.png");
    }
    // Game Textures
    {
	rockSheet        = loadTexture("Pics\\Rock Sheet 1.png");
	rockSheetItem    = loadTexture("Pics\\Rock Sheet 2.png");
    rockSheetItem2   = loadTexture("Pics\\Rock Sheet 2-2.png");
    rockSheetTrappedItem = loadTexture("Pics\\Rock Sheet 2-3.png");
	rockDeathSheet   = loadTexture("Pics\\Death Sheet.png");
    darkDeathSheet   = loadTexture("Pics\\Dark Death Sheet.png");
	floorSheet       = loadTexture("Pics\\Tile Sheet 2.png");
	floorMoveSheet   = loadTexture("Pics\\Move Tile Sheet 2.png");
	floorBottomPic1  = loadTexture("Pics\\Tile Bottom 1.png");
	floorBottomPic2  = loadTexture("Pics\\Tile Bottom 2.png");
	energySheet      = loadTexture("Pics\\Energy Sheet.png");
    energySheet2     = loadTexture("Pics\\Energy Sheet 2.png");
    trappedEnergySheet = loadTexture("Pics\\Energy Sheet 3.png");
    recoverSheet     = loadTexture("Pics\\Recover Sheet.png");
	bgA              = loadTexture("Pics\\Background A.png");
	bgB              = loadTexture("Pics\\Background B.png");
	bgC              = loadTexture("Pics\\Background C.png");
    bgD              = loadTexture("Pics\\Background D.png");
    dimScreenPic     = loadTexture("Pics\\DimScreen.png");
    menuPic0         = loadTexture("Pics\\Menu0-B.png");
	menuPic1         = loadTexture("Pics\\Menu1-B.png");
    menuPic2         = loadTexture("Pics\\Menu2-B.png");
    menuPic3         = loadTexture("Pics\\Menu3-B.png");
    menuPic4         = loadTexture("Pics\\Menu4-B.png");
	lvBarPic         = loadTexture("Pics\\Level Bar.png");
	healthBoxPic     = loadTexture("Pics\\Health Box.png");
    musicDisplayPic  = loadTexture("Pics\\MusicDisplay.png");
    tabMenuCtrlSheet = loadTexture("Pics\\TabMenuControlSheet.png");
    quitPicY         = loadTexture("Pics\\QuitY.png");
    quitPicN         = loadTexture("Pics\\QuitN.png");
    resetPic0        = loadTexture("Pics\\Reset 0.png");
    resetPic1        = loadTexture("Pics\\Reset 1.png");
    resetPic2        = loadTexture("Pics\\Reset 2.png");
    resetPic3        = loadTexture("Pics\\Reset 3.png");
    resetPic4        = loadTexture("Pics\\Reset 4.png");
    diffPic1         = loadTexture("Pics\\Difficulty1.png");
    diffPic2         = loadTexture("Pics\\Difficulty2.png");
    diffPic3         = loadTexture("Pics\\Difficulty3.png");
    diffPic4         = loadTexture("Pics\\Difficulty4.png");
    diffPic5         = loadTexture("Pics\\Difficulty5.png");
    trainPic0        = loadTexture("Pics\\Training 0.png");
    trainPic1        = loadTexture("Pics\\Training 1.png");
    trainPic2        = loadTexture("Pics\\Training 2.png");
    trainPic3        = loadTexture("Pics\\Training 3.png");
    trainPic4        = loadTexture("Pics\\Training 4.png");
    }
    // Sword Attack Trail Textures
    { 
	swordAtkSheet1 = loadTexture("Pics\\Sword Attacks\\Sword Sheet 1.png");
	swordAtkSheet3 = loadTexture("Pics\\Sword Attacks\\Sword Sheet 3.png");
	longAtkSheet1  = loadTexture("Pics\\Sword Attacks\\Long Sheet 1.png");
	longAtkSheet3  = loadTexture("Pics\\Sword Attacks\\Long Sheet 3.png");
	wideAtkSheet1  = loadTexture("Pics\\Sword Attacks\\Wide Sheet 1.png");
	wideAtkSheet3  = loadTexture("Pics\\Sword Attacks\\Wide Sheet 3.png");
	crossAtkSheet1 = loadTexture("Pics\\Sword Attacks\\Cross Sheet 1.png");
	crossAtkSheet3 = loadTexture("Pics\\Sword Attacks\\Cross Sheet 3.png");
	spinAtkSheet1  = loadTexture("Pics\\Sword Attacks\\Spin Sheet 1.png");
	spinAtkSheet3  = loadTexture("Pics\\Sword Attacks\\Spin Sheet 3.png");
	lifeAtkSheet1  = loadTexture("Pics\\Sword Attacks\\Life Sheet 1.png");
	lifeAtkSheet3  = loadTexture("Pics\\Sword Attacks\\Life Sheet 3.png");

    heroAtkSheet1  = loadTexture("Pics\\Sword Attacks\\Hero Sheet 1.png");
    heroAtkSheet3  = loadTexture("Pics\\Sword Attacks\\Hero Sheet 3.png");
    protoAtkSheet1 = loadTexture("Pics\\Sword Attacks\\Proto Sheet 1.png");
    protoAtkSheet3 = loadTexture("Pics\\Sword Attacks\\Proto Sheet 3.png");

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
    }

    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 4, 4096 );
    Mix_AllocateChannels( 8 );
    Mix_Volume( 0, 48 );    // Player Sword Attack Sounds, Player Deleted Sound
    Mix_Volume( 1, 32 );    // Item1 Sounds, Trapped Item Sounds, Item Upgrade Sounds
    Mix_Volume( 2, 32 );    // Panel Break sounds
    Mix_Volume( 3, 07 );    // Music
    Mix_Volume( 4, 32 );    // Rock Break sounds
    Mix_Volume( 5, 32 );    // Item2 Sounds
    Mix_Volume( 6, 48 );    // NPC Appearing Sound, NPC Sword Attack Sounds, NPC Deleted Sound
    Mix_Volume( 7, 32 );    // Menu Sounds

    // Sounds
    {
	swordSound      = Mix_LoadWAV( "Sounds\\SwordSwing.ogg" );
	lifeSwordSound  = Mix_LoadWAV( "Sounds\\LifeSword.ogg" );
    screenDivSound  = Mix_LoadWAV( "Sounds\\ScreenDivide.ogg" );
    tomahawkSound   = Mix_LoadWAV( "Sounds\\Tomahawk.ogg" );
    eTomaSound      = Mix_LoadWAV( "Sounds\\EToma.ogg" );
    spinSlashSound  = Mix_LoadWAV( "Sounds\\SpinSlash.ogg");

	itemSound       = Mix_LoadWAV( "Sounds\\GotItem.ogg" );
    itemSound2      = Mix_LoadWAV( "Sounds\\GotItem2.ogg" );
    hurtSound       = Mix_LoadWAV( "Sounds\\Hurt.ogg" );
    deletedSound    = Mix_LoadWAV( "Sounds\\Deleted.ogg" );
    recoverSound    = Mix_LoadWAV( "Sounds\\Recover.ogg" );
    bossAppearSound = Mix_LoadWAV( "Sounds\\Darkness Appear.ogg" );
    bossDeathSound  = Mix_LoadWAV( "Sounds\\Darkness Disappear.ogg" );
	rockBreakSound  = Mix_LoadWAV( "Sounds\\AreaGrabHit.ogg" );
	panelBreakSound = Mix_LoadWAV( "Sounds\\PanelCrack.ogg" );

	menuOpenSound   = Mix_LoadWAV( "Sounds\\ChipDesc.ogg" );
	menuCloseSound  = Mix_LoadWAV( "Sounds\\ChipDescClose.ogg" );

    quitCancelSound = Mix_LoadWAV( "Sounds\\QuitCancel.ogg" );
    quitChooseSound = Mix_LoadWAV( "Sounds\\QuitChoose.ogg" );
    quitOpenSound   = Mix_LoadWAV( "Sounds\\QuitOpen.ogg" );
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

	reset();
    player1.npc = false;
    resetMenuOn = true;
    menuDisplay = true;
}
App::~App() {
	Mix_FreeChunk( swordSound );
	Mix_FreeChunk( lifeSwordSound );
    Mix_FreeChunk( screenDivSound );
    Mix_FreeChunk( tomahawkSound );
    Mix_FreeChunk( eTomaSound );
    Mix_FreeChunk( spinSlashSound );

	Mix_FreeChunk( itemSound );
    Mix_FreeChunk( itemSound2 );
    Mix_FreeChunk( hurtSound );
    Mix_FreeChunk( deletedSound );
    Mix_FreeChunk( recoverSound );
    Mix_FreeChunk( bossAppearSound );
    Mix_FreeChunk( bossDeathSound );
	Mix_FreeChunk( rockBreakSound );
	Mix_FreeChunk( panelBreakSound );

	Mix_FreeChunk( menuOpenSound );
	Mix_FreeChunk( menuCloseSound );

    Mix_FreeChunk( quitCancelSound );
    Mix_FreeChunk( quitChooseSound );
    Mix_FreeChunk( quitOpenSound );

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

    // Handles Displayed Energy amt
    if ( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) { player1.energyDisplayed = player1.energy; }
    else if ( player1.energyDisplayed > player1.energy ) { player1.energyDisplayed -= 500 * elapsed; }
    else if ( player1.energyDisplayed < player1.energy ) { player1.energyDisplayed += 500 * elapsed; }
    if ( abs( player1.energyDisplayed - player1.energy ) <= 500 * elapsed ) { player1.energyDisplayed = player1.energy; }

    if ( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) { player1.energyDisplayed2 = currentEnergyGain; }
    else if ( player1.energyDisplayed2 > currentEnergyGain ) { player1.energyDisplayed2 -= 500 * elapsed; }
    else if ( player1.energyDisplayed2 < currentEnergyGain ) { player1.energyDisplayed2 += 500 * elapsed; }
    if ( abs( player1.energyDisplayed2 - currentEnergyGain ) <= 500 * elapsed ) { player1.energyDisplayed2 = currentEnergyGain; }

	// Display parameter for Energy gain
	chargeDisplayPlusAmt -= elapsed;
	if (chargeDisplayPlusAmt < 0) { chargeDisplayPlusAmt = 0; }

	// Display parameter for Energy usage
	chargeDisplayMinusAmt -= elapsed;
	if (chargeDisplayMinusAmt < 0) { chargeDisplayMinusAmt = 0; }
    
    player1.hurtDisplayAmt -= elapsed;
    player1.animationDisplayAmt -= elapsed;
    // Death Animation
    if ( player1.hp <= 0 && player1.hp > -10 ) {
        player1.deathDisplayAmt -= elapsed;
        if ( player1.deathDisplayAmt < 0 ) {
            player1.hp = -10;
            map[player1.x][player1.y].boxHP = 1;
            map[player1.x][player1.y].bigBoxDeath = true;
            hitTile( player1.x, player1.y );
            Mix_PlayChannel( 0, deletedSound, 0 );
        }
    }
	// Invalid movement when trying to move back onto the Starting Tile
	else if (player1.x == -1 && player1.y == 0 && player1.animationDisplayAmt < 0) {
        player1.animationType = 2;
		player1.facing = 1;
		player1.x = 0;
        player1.animationDisplayAmt = moveAnimationTime;
        npcAbleToAct = false;
    }
	// Level Transition parameter
	else if (player1.x == 6 && player1.y == 5 && player1.animationDisplayAmt < 0) {
        player1.animationType = 3;
        player1.animationDisplayAmt = moveAnimationTime * 3;
        npcAbleToAct = false;
        for ( int i = 0; i < npcList.size(); i++ ) { npcList[i].npcActionTimer = 0; }
    }
	// Reload default parameters when nothing is happening or being animated
	else if ( player1.animationDisplayAmt <= 0) {
        player1.animationDisplayAmt = 0;
		player1.moving = false;
		player1.turning = false;
		player1.moveDir = -1;
        player1.animationType = -2;
    }
	// Handles Level Transition
	if ( player1.animationType == 3 && player1.animationDisplayAmt > 0 && player1.animationDisplayAmt <= moveAnimationTime * 2) {
		next();
        player1.animationType = 4;
        npcAbleToAct = false;
        for ( int i = 0; i < npcList.size(); i++ ) { npcList[i].npcActionTimer = 0; }
	}
	
    // Handles NPC actions
    for ( int i = 0; i < npcList.size(); i++ ) {
        npcList[i].hurtDisplayAmt -= elapsed;
        npcList[i].animationDisplayAmt -= elapsed;
        if ( npcAbleToAct ) { npcList[i].npcActionTimer = npcWaitTime; }
        if ( ( npcList[i].x == player1.x ) && ( npcList[i].y == player1.y ) ) {
            npcList[i].hp = -1; }
        if ( npcList[i].hp <= 0 ) {
            npcList[i].deathDisplayAmt -= elapsed; 
            npcList[i].hurtDisplayAmt = 0;
            if ( npcList[i].deathDisplayAmt < 0 ) {
                map[npcList[i].x][npcList[i].y].boxHP = 1;
                map[npcList[i].x][npcList[i].y].bigBoxDeath = true;
                map[npcList[i].x][npcList[i].y].isPurple = true;
                hitTile( npcList[i].x, npcList[i].y );
                upgradeItems();
                Mix_PlayChannel( 6, bossDeathSound, 0 );
                npcList.erase( npcList.begin() + i );
                i--;
            }
        }
        // else if ( npcList[i].x != 8 && npcList[i].animationDisplayAmt <= 0 && npcAbleToAct && npcList[i].npcActionTimer <= 0 )
        else if ( npcList[i].x != 8 && npcList[i].animationDisplayAmt <= 0 && npcList[i].npcActionTimer > 0 ) {
            npcList[i].npcActionTimer -= elapsed;
            if ( npcList[i].npcActionTimer <= 0 ) { aiAction( npcList[i] ); }
        }
        else if ( npcList[i].x == 6 && npcList[i].y == 5 && npcList[i].animationDisplayAmt < 0 ) {
            npcList[i].animationType = 3;
            npcList[i].animationDisplayAmt = moveAnimationTime * 3; }
        else if ( npcList[i].animationDisplayAmt <= 0 ) {
            npcList[i].animationDisplayAmt = 0;
            npcList[i].moving = false;
            npcList[i].turning = false;
            npcList[i].moveDir = -1;
            npcList[i].animationType = -2; }
        if ( npcList[i].animationType == 3 && npcList[i].animationDisplayAmt <= moveAnimationTime * 2 ) {
            npcList[i].x = 8;
        }
    }
    npcAbleToAct = false;

	// Item Display parameter
	itemAnimationAmt += elapsed * 10;
	if (itemAnimationAmt > 8.0) { itemAnimationAmt = 0; }
    // Background Display parameter
	bgAnimationAmt += elapsed * 3.5;
	if (bgAnimationAmt > 8.0) { bgAnimationAmt = 0; }

	// Rock HP Indicators, Item Upgrade Indicators
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map[i][j].boxAtkInd -= elapsed;
			if (map[i][j].boxAtkInd < 0) { map[i][j].boxAtkInd = 0; }
			if (map[i][j].boxHP < 0) { map[i][j].boxHP = 0; }

            map[i][j].upgradeInd -= elapsed;
            if ( map[i][j].upgradeInd < 0 ) { map[i][j].upgradeInd = 0; }
	} }

	// Handles delayed Damage to Rocks
    for ( int i = 0; i < delayedHpList.size(); i++ ) {
        if ( delayedHpList[i].npc && npcList.empty() ) {
            delayedHpList.erase( delayedHpList.begin() + i );
            i--;
        }
        else {
            delayedHpList[i].delay -= elapsed;
            if ( delayedHpList[i].delay <= 0 ) {
                hitTile( delayedHpList[i].xPos, delayedHpList[i].yPos, delayedHpList[i].dmg );
                delayedHpList.erase( delayedHpList.begin() + i );
                i--;
            }
        }
    }

	// Handles delayed Sounds
	for ( int i = 0; i < delayedSoundList.size(); i++ ) {
		delayedSoundList[i].delay -= elapsed;
		if ( delayedSoundList[i].delay <= 0 ) {
			if      ( delayedSoundList[i].soundName == "sword" )  {
                if ( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, swordSound, 0 );
                else if ( !npcList.empty() )    Mix_PlayChannel( 6, swordSound, 0 );
            }
			else if ( delayedSoundList[i].soundName == "life" )   {
                if ( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, lifeSwordSound, 0 );
                else if ( !npcList.empty() )    Mix_PlayChannel( 6, lifeSwordSound, 0 );
            }
            else if ( delayedSoundList[i].soundName == "divide" ) {
                if ( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, screenDivSound, 0 );
                else if ( !npcList.empty() )    Mix_PlayChannel( 6, screenDivSound, 0 );
            }
            else if ( delayedSoundList[i].soundName == "toma" )   {
                if ( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, tomahawkSound, 0 );
                else if ( !npcList.empty() )    Mix_PlayChannel( 6, tomahawkSound, 0 );
            }
            else if ( delayedSoundList[i].soundName == "eToma" )  {
                if ( !delayedSoundList[i].npc ) Mix_PlayChannel( 0, eTomaSound, 0 );
                else if ( !npcList.empty() )    Mix_PlayChannel( 6, eTomaSound, 0 );
            }
			delayedSoundList.erase( delayedSoundList.begin() + i);
	}	}

    // Handles delayed Eagle Tomahawk attack Display
    for ( int i = 0; i < delayedETomaDisplayList.size(); i++ ) {
        if ( delayedETomaDisplayList[i].delay <= 0 ) { delayedETomaDisplayList[i].animationTimer -= elapsed; }
        else { delayedETomaDisplayList[i].delay -= elapsed; }
        if ( delayedETomaDisplayList[i].animationTimer <= -0.2 ) { delayedETomaDisplayList.erase( delayedETomaDisplayList.begin() + i ); }
        // else { eTomaDisplay( delayedETomaDisplayList[i].xPos, delayedETomaDisplayList[i].yPos, delayedETomaDisplayList[i].animationTimer ); }
        // Don't Render objects outside of the Render function
    }

    // Handles display time of Music track Number and Name
    musicSwitchDisplayAmt -= elapsed;
    if ( musicSwitchDisplayAmt < 0 ) { musicSwitchDisplayAmt = 0; }
}
void App::checkKeys() {
    // Check keys for Player Actions

    // Player can't act until NPCs have finished acting
    for ( int i = 0; i < npcList.size(); i++ ) {
        if ( npcList[i].animationDisplayAmt > 0 ) { return; } }

	// Menu Controls
	while (SDL_PollEvent(&event)) {
		if      ( event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE ) { done = true; }
		else if ( event.type == SDL_KEYDOWN && player1.animationDisplayAmt <= 0 ) {		// Read Single Button presses
            if      ( event.key.keysym.scancode == SDL_SCANCODE_C ) { changeMusic(); }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_V ) { toggleMusic(); }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) {
                if      ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else                      { quitMenuOn    = true;  Mix_PlayChannel( 7, quitOpenSound, 0 ); menuSel = true; }
                player1.animationDisplayAmt = menuExitTime;
            }
            else if ( player1.animationDisplayAmt <= 0 && (event.key.keysym.scancode == SDL_SCANCODE_TAB || event.key.keysym.scancode == SDL_SCANCODE_F) ) {
                if ( menuDisplay ) { menuDisplay = false; Mix_PlayChannel( 7, menuCloseSound, 0 ); }
                else               { menuDisplay = true;  Mix_PlayChannel( 7, menuOpenSound, 0 ); }
                player1.animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_R ) {
                if      ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else                      { resetMenuOn   = true;  Mix_PlayChannel( 7, quitOpenSound, 0 ); menuSel = true; }
                player1.animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_X ) {
                if      ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                else                      { trainMenuOn   = true;  Mix_PlayChannel( 7, quitOpenSound, 0 ); menuSel = true; }
                player1.animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_W || event.key.keysym.scancode == SDL_SCANCODE_UP ) {
                if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel > 1 ) {       charSel -= 2;      Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_S || event.key.keysym.scancode == SDL_SCANCODE_DOWN ) {
                if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel < 3 ) {       charSel += 2;      Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_A || event.key.keysym.scancode == SDL_SCANCODE_LEFT ) {
                if ( quitMenuOn ) {
                    if ( !menuSel ) {          menuSel = true;    Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
                else if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel > 0 && charSel % 2 == 1) {
                                               charSel--;         Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
                else if ( diffSelMenuOn ) {
                    if ( gameDiffSel > 0 ) {   gameDiffSel--;     Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_RIGHT ) {
                if ( quitMenuOn ) {
                    if ( menuSel ) {           menuSel = false;   Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
                else if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel < 3 && charSel % 2 == 0 ) {
                                               charSel++;         Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
                else if ( diffSelMenuOn ) {
                    if ( gameDiffSel < 4 ) {   gameDiffSel++;     Mix_PlayChannel( 7, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_1      || event.key.keysym.scancode == SDL_SCANCODE_KP_1 ||
                      event.key.keysym.scancode == SDL_SCANCODE_2      || event.key.keysym.scancode == SDL_SCANCODE_KP_2 ||
                      event.key.keysym.scancode == SDL_SCANCODE_3      || event.key.keysym.scancode == SDL_SCANCODE_KP_3 ||
                      event.key.keysym.scancode == SDL_SCANCODE_4      || event.key.keysym.scancode == SDL_SCANCODE_KP_4 ||
                      event.key.keysym.scancode == SDL_SCANCODE_5      || event.key.keysym.scancode == SDL_SCANCODE_KP_5 ||
                      event.key.keysym.scancode == SDL_SCANCODE_6      || event.key.keysym.scancode == SDL_SCANCODE_KP_6 ||
                      event.key.keysym.scancode == SDL_SCANCODE_7      || event.key.keysym.scancode == SDL_SCANCODE_KP_7 ||
                      event.key.keysym.scancode == SDL_SCANCODE_RETURN || event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER ) {
                if ( quitMenuOn ) {
                    if ( menuSel ) { done = true; }
                    else {
                        quitMenuOn = false;
                        Mix_PlayChannel( 7, quitCancelSound, 0 );
                        player1.animationDisplayAmt = menuExitTime; }
                }
                else if ( resetMenuOn ) { 
                    if ( menuSel ) {
                        resetMenuOn = false;
                        diffSelMenuOn = true;
                        gameDiffSel = currentGameDiff;
                        Mix_PlayChannel( 7, quitChooseSound, 0 ); }
                    else {
                        resetMenuOn = false;
                        Mix_PlayChannel( 7, quitCancelSound, 0 ); }
                    player1.animationDisplayAmt = menuExitTime;
                }
                else if ( diffSelMenuOn ) {
                    diffSelMenuOn = false;
                    player1.type = charSel;
                    currentGameDiff = gameDiffSel;
                    reset();
                    Mix_PlayChannel( 7, quitChooseSound, 0 );
                    player1.animationDisplayAmt = menuExitTime;
                }
                else if ( trainMenuOn ) {
                    trainMenuOn = false;
                    player1.type = charSel;
                    test();
                    Mix_PlayChannel( 7, quitChooseSound, 0 );
                    player1.animationDisplayAmt = menuExitTime;
                }
            }
            else if ( ( event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_SPACE ) && 
                        !quitMenuOn && !resetMenuOn && !diffSelMenuOn && !trainMenuOn && player1.animationDisplayAmt <= 0 && player1.hp > 0 ) {
                if      ( player1.facing == 1 ) { face( player1, 3 ); }
                else if ( player1.facing == 3 ) { face( player1, 1 ); }
            }
	    }
    }

    // Player Movement and Attacks
    if ( player1.animationDisplayAmt <= 0 && player1.hp > 0 && !quitMenuOn && !resetMenuOn && !diffSelMenuOn && !trainMenuOn ) {
        // The player can't Move or Attack until after the previous Action is completed, or if a Menu is open

	    const Uint8* keystates = SDL_GetKeyboardState(NULL);		// Read Multiple Button presses simultaneously

		// Move Up
		if		(keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]) { move(player1, 0); }
		// Move Left
		else if (keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT]) { move(player1, 1); }
		// Move Down
		else if (keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN]) { move(player1, 2); }
		// Move Right
		else if (keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT]) { move(player1, 3); }

		// Sword Attacks
		else if (keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1]) {
            if      ( player1.type == 0 || player1.type == 1 ) { swordAtk( player1 ); }
            else if ( player1.type == 2 )                      { tomaAtkA1( player1 ); }
            else if ( player1.type == 3 )                      { vDivideAtk( player1 ); }
            else if ( player1.type == 4 )                      { longAtk( player1 ); }
        }
		else if (keystates[SDL_SCANCODE_2] || keystates[SDL_SCANCODE_KP_2]) {
            if      ( player1.type == 0 || player1.type == 1 ) { longAtk( player1 ); }
            else if ( player1.type == 2 )                      { tomaAtkB1( player1 ); }
            else if ( player1.type == 3 )                      { upDivideAtk( player1 ); }
            else if ( player1.type == 4 )                      { wideAtk( player1 ); }
        }
		else if (keystates[SDL_SCANCODE_3] || keystates[SDL_SCANCODE_KP_3]) {
            if      ( player1.type == 0 || player1.type == 1 ) { wideAtk( player1 ); }
            else if ( player1.type == 2 )                      { tomaAtkA2( player1 ); }
            else if ( player1.type == 3 )                      { downDivideAtk( player1 ); }
            else if ( player1.type == 4 )                      { crossAtk( player1 ); }
        }
		else if (keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4]) { 
            if      ( player1.type == 0 ) { crossAtk( player1 );   }
            else if ( player1.type == 1 ) { stepAtk( player1 );    }
            else if ( player1.type == 2 ) { tomaAtkB2( player1 ); }
            else if ( player1.type == 3 ) { xDivideAtk( player1 ); }
            else if ( player1.type == 4 ) { stepCrossAtk( player1 ); }
        }
		else if (keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5]) {
            if      ( player1.type == 0 ) { spinAtk( player1 );    }
            else if ( player1.type == 1 ) { heroAtk( player1 );    }
            else if ( player1.type == 2 ) { eTomaAtk( player1 ); }
            else if ( player1.type == 3 ) { zDivideAtk( player1 ); }
            else if ( player1.type == 4 ) { spinSlashAtk( player1 ); }
        }
		else if (keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6]) {
            if      ( player1.type == 0 ) { stepAtk(player1); }
            else if ( player1.type == 1 ) { protoAtk( player1 ); } }
		else if (keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7]) {
            if (player1.type == 0) { lifeAtk(player1); } }
    }
}
void App::changeMusic( int track ) {
    musicSwitchDisplayAmt = 1.5;

    if (track == -1 ) {
        if ( !musicMuted ) { musicSel++; }
	    if ( musicSel > 18 ) { musicSel = 1; } }
    else { musicSel = track; }

    musicMuted = false;

    switch ( musicSel ) {
    case 0:
        Mix_Pause( 3 );
        break;
    case 1:
        Mix_PlayChannel( 3, track01, -1 );
        break;
    case 2:
        Mix_PlayChannel( 3, track02, -1 );
        break;
    case 3:
        Mix_PlayChannel( 3, track03, -1 );
        break;
    case 4:
        Mix_PlayChannel( 3, track04, -1 );
        break;
    case 5:
        Mix_PlayChannel( 3, track05, -1 );
        break;
    case 6:
        Mix_PlayChannel( 3, track06, -1 );
        break;
    case 7:
        Mix_PlayChannel( 3, track07, -1 );
        break;
    case 8:
        Mix_PlayChannel( 3, track08, -1 );
        break;
    case 9:
        Mix_PlayChannel( 3, track09, -1 );
        break;
    case 10:
        Mix_PlayChannel( 3, track10, -1 );
        break;
    case 11:
        Mix_PlayChannel( 3, track11, -1 );
        break;
    case 12:
        Mix_PlayChannel( 3, track12, -1 );
        break;
    case 13:
        Mix_PlayChannel( 3, track13, -1 );
        break;
    case 14:
        Mix_PlayChannel( 3, track14, -1 );
        break;
    case 15:
        Mix_PlayChannel( 3, track15, -1 );
        break;
    case 16:
        Mix_PlayChannel( 3, track16, -1 );
        break;
    case 17:
        Mix_PlayChannel( 3, track17, -1 );
        break;
    case 18:
        Mix_PlayChannel( 3, track18, -1 );
        break;
    }
}
void App::toggleMusic() {
    musicSwitchDisplayAmt = 1.5;

    if ( musicMuted ) {
        changeMusic();
    }
    else {
        musicMuted = true;
        Mix_Pause( 3 );
    }
}

// Display Functions
void App::Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	drawBg();
	drawFloor();

    drawTabMenuCtrl();
    if ( menuDisplay ) { drawMenu(); }
    if ( musicSwitchDisplayAmt || menuDisplay ) { displayMusic(); }
    drawTextUI();

	// Draw game elements, back row first, so the front row is displayed in front
    for ( int i = 5; i >= 0; i-- ) {
        for ( int j = 0; j < npcList.size(); j++ ) {
            if ( i == npcList[j].y ) { drawPlayer( npcList[j] ); } }
        drawItems( i );
        drawBoxes( i );
        drawItemUpgrading( i );
        if ( i == player1.y ) { drawPlayer( player1 ); }
    }

    for ( int i = 0; i < npcList.size(); i++ ) { drawSwordAtks( npcList[i] ); }
    drawSwordAtks( player1 );
    drawNpcHp();

    if ( quitMenuOn ) {
        drawDimScreen();
        drawQuitMenu(); }
    else if ( resetMenuOn ) {
        drawDimScreen();
        drawResetMenu(); }
    else if ( diffSelMenuOn ) {
        drawDimScreen();
        drawDiffSelMenu(); }
    else if ( trainMenuOn ) {
        drawDimScreen();
        drawTrainMenu(); }

	SDL_GL_SwapWindow(displayWindow);
}
void App::drawBg() {
	GLuint texture;
	float bgSizeX = 0.768 * 8 / 4;
	float bgSizeY = 0.768 * 8 / 2.7;
	if      ( lvlDiff == 0 ) { texture = bgA; }
	else if ( lvlDiff == 1 ) { texture = bgB; }
	else if ( lvlDiff == 2 ) { texture = bgC; }
    else if ( lvlDiff == 3 ) { texture = bgD; }
	glTranslatef(0.064 * 2 / 4 * bgAnimationAmt, -0.064 * 2 / 2.7 * bgAnimationAmt, 0);
	GLfloat place[] = { -bgSizeX, bgSizeY, -bgSizeX, -bgSizeY,     bgSizeX, -bgSizeY, bgSizeX, bgSizeY };
	drawSpriteSheetSprite(place, texture, 0, 1, 1);
}
void App::drawDimScreen() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    drawSpriteSheetSprite( place, dimScreenPic, 0, 1, 1 );
}
void App::drawMenu() {
	glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX, menuSizeY, -menuSizeX, -menuSizeY,     menuSizeX, -menuSizeY, menuSizeX, menuSizeY };
    if      ( player1.type == 0 ) { drawSpriteSheetSprite( place, menuPic0, 0, 1, 1 ); }
    else if ( player1.type == 1 ) { drawSpriteSheetSprite( place, menuPic1, 0, 1, 1 ); }
    else if ( player1.type == 2 ) { drawSpriteSheetSprite( place, menuPic2, 0, 1, 1 ); }
    else if ( player1.type == 3 ) { drawSpriteSheetSprite( place, menuPic3, 0, 1, 1 ); }
    else if ( player1.type == 4 ) { drawSpriteSheetSprite( place, menuPic4, 0, 1, 1 ); }
}
void App::drawQuitMenu() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    if ( menuSel ) { drawSpriteSheetSprite( place, quitPicY, 0, 1, 1 ); }
    else           { drawSpriteSheetSprite( place, quitPicN, 0, 1, 1 ); }
}
void App::drawResetMenu() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    if      ( charSel == 0 ) { drawSpriteSheetSprite( place, resetPic0, 0, 1, 1 ); }
    else if ( charSel == 1 ) { drawSpriteSheetSprite( place, resetPic1, 0, 1, 1 ); }
    else if ( charSel == 2 ) { drawSpriteSheetSprite( place, resetPic2, 0, 1, 1 ); }
    else if ( charSel == 3 ) { drawSpriteSheetSprite( place, resetPic3, 0, 1, 1 ); }
    else if ( charSel == 4 ) { drawSpriteSheetSprite( place, resetPic4, 0, 1, 1 ); }
}
void App::drawDiffSelMenu() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    if      ( gameDiffSel == 0 ) { drawSpriteSheetSprite( place, diffPic1, 0, 1, 1 ); }
    else if ( gameDiffSel == 1 ) { drawSpriteSheetSprite( place, diffPic2, 0, 1, 1 ); }
    else if ( gameDiffSel == 2 ) { drawSpriteSheetSprite( place, diffPic3, 0, 1, 1 ); }
    else if ( gameDiffSel == 3 ) { drawSpriteSheetSprite( place, diffPic4, 0, 1, 1 ); }
    else if ( gameDiffSel == 4 ) { drawSpriteSheetSprite( place, diffPic5, 0, 1, 1 ); }
}
void App::drawTrainMenu() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    if      ( charSel == 0 ) { drawSpriteSheetSprite( place, trainPic0, 0, 1, 1 ); }
    else if ( charSel == 1 ) { drawSpriteSheetSprite( place, trainPic1, 0, 1, 1 ); }
    else if ( charSel == 2 ) { drawSpriteSheetSprite( place, trainPic2, 0, 1, 1 ); }
    else if ( charSel == 3 ) { drawSpriteSheetSprite( place, trainPic3, 0, 1, 1 ); }
    else if ( charSel == 4 ) { drawSpriteSheetSprite( place, trainPic4, 0, 1, 1 ); }
}
void App::drawTabMenuCtrl() {
    glLoadIdentity();
    glTranslatef( 0, 0.945, 0 );
    float sizeX = 1.44 / 4;
    float sizeY = 0.15 / 2.7;
    GLfloat place[] = { -sizeX, +sizeY, -sizeX, -sizeY,
                        +sizeX, -sizeY, +sizeX, +sizeY };
    if      ( itemAnimationAmt <= 2.0 ) { drawSpriteSheetSprite( place, tabMenuCtrlSheet, 0, 1, 4 ); }
    else if ( itemAnimationAmt <= 4.0 ) { drawSpriteSheetSprite( place, tabMenuCtrlSheet, 1, 1, 4 ); }
    else if ( itemAnimationAmt <= 6.0 ) { drawSpriteSheetSprite( place, tabMenuCtrlSheet, 2, 1, 4 ); }
    else if ( itemAnimationAmt <= 8.0 ) { drawSpriteSheetSprite( place, tabMenuCtrlSheet, 3, 1, 4 ); }
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
	float barSizeX = 1.20 / 4;
	float barSizeY = 0.12 / 2.7;
	GLfloat barPlace[] = { barX - barSizeX, barY + barSizeY, barX - barSizeX, barY - barSizeY,
	                       barX + barSizeX, barY - barSizeY, barX + barSizeX, barY + barSizeY };
	drawSpriteSheetSprite(barPlace, lvBarPic, 0, 1, 1);
	glTranslatef(0, -0.005, 0);
	drawText(textSheet1A, "Lv" + to_string(level), 0.08 * 2 / 4, 0.16 * 2 / 2.7, -0.00);

	// Draw Energy Amount
	glLoadIdentity();
	glTranslatef(-0.889, 0.942, 0);
	float boxX = 0.0;
	float boxY = 0.0;
	float boxSizeX = 0.44 / 4;
	float boxSizeY = 0.16 / 2.7;
	GLfloat boxPlace[] = { boxX - boxSizeX, boxY + boxSizeY, boxX - boxSizeX, boxY - boxSizeY,
	                       boxX + boxSizeX, boxY - boxSizeY, boxX + boxSizeX, boxY + boxSizeY };
	drawSpriteSheetSprite(boxPlace, healthBoxPic, 0, 1, 1);
	glTranslatef(0.061, 0.0075, 0);
	
    int energyToDisplay = player1.energyDisplayed;
    if ( player1.hp <= 0 )             { energyToDisplay = 0; }
	else if ( energyToDisplay < 1 )    { energyToDisplay = 1; }
	else if ( energyToDisplay > 9999 ) { energyToDisplay = 9999; }
    if ( energyToDisplay >= 10 )   { glTranslatef(-0.04, 0, 0); }
	if ( energyToDisplay >= 100 )  { glTranslatef(-0.04, 0, 0); }
	if ( energyToDisplay >= 1000 ) { glTranslatef(-0.04, 0, 0); }
	if      (chargeDisplayPlusAmt  > 0) { drawText(textSheet2B, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else if (chargeDisplayMinusAmt > 0 || energyToDisplay <= 1) {
	                                      drawText(textSheet2C, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else                                { drawText(textSheet2A, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }

	// Draw Current Energy gain for this level
	glLoadIdentity();
	glTranslatef(-0.828, 0.8495, 0);
	if (abs( player1.energyDisplayed2 ) >= 10)   { glTranslatef(-0.04, 0, 0); }
	if (abs( player1.energyDisplayed2 ) >= 100)  { glTranslatef(-0.04, 0, 0); }
	if (abs( player1.energyDisplayed2 ) >= 1000) { glTranslatef(-0.04, 0, 0); }
	if (level > 0) {
        if      ( player1.energyDisplayed2 > 0 ) {
            drawText( textSheet1B, to_string( abs( player1.energyDisplayed2 ) ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 ); }
        else if ( player1.energyDisplayed2 < 0 ) {
            drawText( textSheet1C, to_string( abs( player1.energyDisplayed2 ) ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 ); }
	}

	// Display Sword Name and Cost
	glLoadIdentity();
	glTranslatef(-0.97, -0.93, 0);
	string text = "";
	GLuint texture;

    // Previous Sword Attack Text Display
    if ( player1.selSwordAnimation != -1 ) {
        if ( player1.selSwordAnimation == -2 ) {
            texture = textSheet1B;
            text = "Energy" + to_string( energyGainAmt ); }
        else if ( player1.selSwordAnimation == -3 ) {
            texture = textSheet1B;
            text = "Energy" + to_string( energyGainAmt2 ); }
        else if ( player1.selSwordAnimation == -4 ) {
            texture = textSheet1C;
            text = "Energy" + to_string( energyLossAmt ); }
        else {
            texture = textSheet1C;
            if      ( player1.selSwordAnimation == 0 )  { text = "Sword"    + to_string( swordCost ); }
            else if ( player1.selSwordAnimation == 1 )  { text = "LongSwrd" + to_string( longCost ); }
            else if ( player1.selSwordAnimation == 2 ) {
                if      ( player1.type == 0 )           { text = "WideSwrd" + to_string( wideCost ); }
                else if ( player1.type == 1 )           { text = "WideSwrd" + to_string( wideCost2 ); } }
            else if ( player1.selSwordAnimation == 3 ) {
                if      ( player1.type == 0 )           { text = "CrossSrd" + to_string( crossCost ); }
                else if ( player1.type == 4 )           { text = "CrossSls" + to_string( crossCost2 ); } }
            else if ( player1.selSwordAnimation == 4 )  { text = "SpinSwrd" + to_string( spinCost ); }
            else if ( player1.selSwordAnimation == 5 ) {
                if      ( player1.type == 0 )           { text = "StepSwrd" + to_string( stepCost ); }
                else if ( player1.type == 1 )           { text = "StepSwrd" + to_string( stepCost2 ); } }
            else if ( player1.selSwordAnimation == 6 )  { text = "LifeSwrd" + to_string( lifeCost ); } 
            else if ( player1.selSwordAnimation == 7 )  { text = "HeroSwrd" + to_string( heroCost ); }
            else if ( player1.selSwordAnimation == 8 )  { text = "ProtoCrs" + to_string( protoCost ); }
            else if ( player1.selSwordAnimation == 9 )  { text = "ScrnDivV" + to_string( vDivideCost ); }
            else if ( player1.selSwordAnimation == 10 ) { text = "ScreenDv" + to_string( upDivideCost ); }
            else if ( player1.selSwordAnimation == 11 ) { text = "ScreenDv" + to_string( downDivideCost ); }
            else if ( player1.selSwordAnimation == 12 ) { text = "CrossDiv" + to_string( xDivideCost ); }
            else if ( player1.selSwordAnimation == 13 ) { text = "NeoSnDiv" + to_string( zDivideCost ); }
            else if ( player1.selSwordAnimation == 14 ) { text = "WideSwng" + to_string( tomaCostA1 ); }
            else if ( player1.selSwordAnimation == 15 ) { text = "WdSwngEX" + to_string( tomaCostB1 ); }
            else if ( player1.selSwordAnimation == 16 ) { text = "Tomahawk" + to_string( tomaCostA2 ); }
            else if ( player1.selSwordAnimation == 17 ) { text = "TomahkEX" + to_string( tomaCostB2 ); }
            else if ( player1.selSwordAnimation == 18 ) { text = "ETomahwk" + to_string( eTomaCost ); }
            else if ( player1.selSwordAnimation == 19 ) { text = "LongSlsh" + to_string( longCost ); }
            else if ( player1.selSwordAnimation == 20 ) { text = "WideSlsh" + to_string( wideCost ); }
            else if ( player1.selSwordAnimation == 21 ) { text = "StepCrss" + to_string( stepCrossCost ); }
            else if ( player1.selSwordAnimation == 22 ) { text = "SpinSlsh" + to_string( spinSlashCost ); }
        }
        drawText( texture, text, 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 );
    }
}
void App::displayMusic() {
    string text;
    if ( musicMuted ) { text = "Music Off"; }
    else {
        switch ( musicSel ) {
        case 1:  text = "01 Organization";      break;
        case 2:  text = "02 An Incident";       break;
        case 3:  text = "03 Blast Speed";       break;
        case 4:  text = "04 Shark Panic";       break;
        case 5:  text = "05 Battle Field";      break;
        case 6:  text = "06 Doubt";             break;
        case 7:  text = "07 Distortion";        break;
        case 8:  text = "08 Surge of Power";    break;
        case 9:  text = "09 Digital Strider";   break;
        case 10: text = "10 Break the Storm";   break;
        case 11: text = "11 Evil Spirit";       break;
        case 12: text = "12 Hero";              break;
        case 13: text = "13 Danger Zone";       break;
        case 14: text = "14 Navi Customizer";   break;
        case 15: text = "15 Graveyard";         break;
        case 16: text = "16 The Count";         break;
        case 17: text = "17 Secret Base";       break;
        case 18: text = "18 Cybeasts";          break;
        }
    }
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    drawSpriteSheetSprite( place, musicDisplayPic, 0, 1, 1 );

    glLoadIdentity();
    glTranslatef( 0.30, -0.95, 0 );
    drawText( textSheet1A, text, 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 );
}

void App::drawPlayer(Player &player) {
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);
    if ( player.type == 3 ) { glTranslatef( 0, -0.05 * playerScale, 0 ); }
	
    float playerSizeX, playerSizeY;
	int picIndex = 0;
	float displace = 0.0;
	float displace2 = 0.3;
    GLuint texture;

    bool draw = true;
    int num = ceil( player.hurtDisplayAmt * 100.0 );
    if ( (player.hurtDisplayAmt > 0) && (num % 2) ) draw = false;

    // Death Animation
    if ( player.type != 2 && player.type != 4 && player.hp <= 0 && player.hp > -10 && player.deathDisplayAmt > 0 ) {
        int spriteSheetWidth = 2;
        if ( player.type == 0 ) {
		    playerSizeX = 0.40 * playerScale;
		    playerSizeY = 0.48 * playerScale;
            glTranslatef( 0, -0.06 * playerScale, 0 );
            displace2 = 0.014;
            if ( player.npc ) { texture = darkMegamanHurtSheet; }
            else              { texture = megamanHurtSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.70 * playerScale;
		    playerSizeY = 0.49 * playerScale;
            glTranslatef( 0, -0.05 * playerScale, 0 );
            displace2 = -0.01 * playerScale;
            if ( player.npc ) { texture = darkProtoHurtSheet; }
            else              { texture = protoHurtSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
            spriteSheetWidth = 4;
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.65 * playerScale;
		    playerSizeY = 0.67 * playerScale;
            glTranslatef( 0, 0.088 * playerScale, 0 );
            displace2 = -0.184 * playerScale;
            if ( player.npc ) { texture = darkColonelHurtSheet; }
            else              { texture = colonelHurtSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
            playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
            spriteSheetWidth = 4;
        }

         GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
                                   player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
                                   player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
                                   player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

        if (player.facing == 1) {
			glTranslatef(-displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex + spriteSheetWidth, spriteSheetWidth, 2);
		}
		else if (player.facing == 3) {
			glTranslatef(displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex, spriteSheetWidth, 2);
		}
    }
	// Step Sword Move Animation
	else if ( player.animationType == 1 && ( player.selSwordAnimation == 5 || player.selSwordAnimation == 12 || player.selSwordAnimation == 21)) {

        int textureSheetWidth = 8;
        if      ( player.type == 0 ) {
		    playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            displace2 = 0.3;
            if ( player.npc ) { texture = darkMegamanAtkSheet; }
            else              { texture = megamanAtkSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.78 * playerScale;
		    playerSizeY = 0.64 * playerScale; 
            glTranslatef( 0, 0.08 * playerScale, 0 );
            displace2 -= 0.08;
            if ( player.npc ) { texture = darkProtoAtkSheet; }
            else              { texture = protoAtkSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            glTranslatef( 0, 0.2 * playerScale, 0 );
            if ( player.npc ) { texture = darkColonelAtkSheet; }
            else              { texture = colonelAtkSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.94 * playerScale;
		    playerSizeY = 0.88 * playerScale; 
            glTranslatef( 0, 0.065 * playerScale, 0 );
            displace2 = 0.20;
            textureSheetWidth = 9;
            if ( player.npc ) { texture = darkSlashmanAtkSheet; }
            else              { texture = slashmanAtkSheet; }
        }

		if		( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 1.0 ) { displace = 2.000 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.9 ) { displace = 1.975 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.8 ) { displace = 1.950 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.7 ) { displace = 1.925 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.6 ) { displace = 1.900 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.5 ) { displace = 1.520 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.4 ) { displace = 1.140 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.3 ) { displace = 0.760 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.2 ) { displace = 0.380 * scaleX; }
		else if ( player.animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.1 ) { displace = 0.000 * scaleX; }

        if ( player.type == 4 ) {
            if ( ( player.energy / 100 ) % 2 ) {
                if      ( player.animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 0; }
		        else if ( player.animationDisplayAmt > stepAtkTime - 0.18) { picIndex = 1; }
		        else if ( player.animationDisplayAmt > 0)                  { picIndex = 2; } }
            else {
                if      ( player.animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 2; }
		        else if ( player.animationDisplayAmt > stepAtkTime - 0.18) { picIndex = 3; }
		        else if ( player.animationDisplayAmt > 0)                  { picIndex = 4; } }
        }
        else {
		    if      ( player.animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 1; }
		    else if ( player.animationDisplayAmt > stepAtkTime - 0.10) { picIndex = 2; }
		    else if ( player.animationDisplayAmt > stepAtkTime - 0.14) { picIndex = 3; }
		    else if ( player.animationDisplayAmt > stepAtkTime - 0.20) { picIndex = 4; }
		    else if ( player.animationDisplayAmt > stepAtkTime - 0.26) { picIndex = 5; }
		    else if ( player.animationDisplayAmt > stepAtkTime - 0.32) { picIndex = 6; }
		    else if ( player.animationDisplayAmt > 0)                  { picIndex = 7; } }

		if (player.facing == 1) {
            glTranslatef(  displace - displace2, 0.02, 0 );
            picIndex += textureSheetWidth;
            player.xOffset = displace * scaleX;
        }
		else if (player.facing == 3) {
            glTranslatef( -displace + displace2, 0.02, 0 );
            player.xOffset = -displace * scaleX;
        }

        GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
        
        if ( draw ) drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
	// Attack Animation
	else if (player.animationType == 1 && player.animationDisplayAmt > 0) {

        int textureSheetWidth = 8;
		if ( player.type == 0 ) {
		    playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            if ( player.npc ) { texture = darkMegamanAtkSheet; }
            else              { texture = megamanAtkSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.78 * playerScale;
		    playerSizeY = 0.64 * playerScale;
            glTranslatef( 0, 0.08 * playerScale, 0 );
            displace2 -= 0.08;
            if ( player.npc ) { texture = darkProtoAtkSheet; }
            else              { texture = protoAtkSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            glTranslatef( 0, 0.2 * playerScale, 0 );
            if ( player.npc ) { texture = darkColonelAtkSheet; }
            else              { texture = colonelAtkSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.94 * playerScale;
		    playerSizeY = 0.88 * playerScale; 
            glTranslatef( 0, 0.065 * playerScale, 0 );
            displace2 = 0.20;
            textureSheetWidth = 9;
            if ( player.npc ) { texture = darkSlashmanAtkSheet; }
            else              { texture = slashmanAtkSheet; }
        }

        // Altered animation for Tomahawkman's regular attacks
        if ( player.selSwordAnimation >= 14 && player.selSwordAnimation <= 17 ) {
            playerSizeX = 0.80 * playerScale;
            playerSizeY = 0.65 * playerScale;
            displace2 = 0.22;
            glTranslatef( 0, 0.1 * playerScale, 0 );
            if ( player.npc ) { texture = darkTmanAtkSheet1; }
            else              { texture = tmanAtkSheet1; }
            textureSheetWidth = 4;
            if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.20) { picIndex = 0; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.24) { picIndex = 1; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.28) { picIndex = 2; }
			else if ( player.animationDisplayAmt > 0)                                 { picIndex = 3; } }
        // Tomahawkman's Eagle Tomahawk attack
        else if ( player.selSwordAnimation == 18 ) {
            playerSizeX = 1.10 * playerScale;
            playerSizeY = 0.99  * playerScale;
            glTranslatef( 0, 0.43 * playerScale, 0 );
            displace2 = 0.17;
            if ( player.npc ) { texture = darkTmanAtkSheet2; }
            else              { texture = tmanAtkSheet2; }
            textureSheetWidth = 5;
            if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 0; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.18) { picIndex = 1; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 2; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.24) { picIndex = 3; }
            else if ( player.animationDisplayAmt > 0)                                 { picIndex = 4; } }
        // Slashman's LongSlash and CrossSlash
        else if ( player.selSwordAnimation == 19 ) {
            if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 2; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 3; }
			else if ( player.animationDisplayAmt > 0)                                 { picIndex = 4; }
        }
        // Slashman's WideSlash
        else if ( player.selSwordAnimation == 20 ) {
            if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 0; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 1; }
			else if ( player.animationDisplayAmt > 0)                                 { picIndex = 2; }
        }
        // Slashman's CrossSlash
        else if ( player.selSwordAnimation == 3 && player.type == 4 ) {
            if ( ( player.energy / 100 ) % 2 ) {
                if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 0; }
			    else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 1; }
			    else if ( player.animationDisplayAmt > 0)                                 { picIndex = 2; } }
            else {
                if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 2; }
			    else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 3; }
			    else if ( player.animationDisplayAmt > 0)                                 { picIndex = 4; } }
        }
        // Slashman's SpinSlash
        else if ( player.selSwordAnimation == 22 ) {
            if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 5; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.16) { picIndex = 6; }
            else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 7; }
            else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.28) { picIndex = 6; }
            else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.34) { picIndex = 8; }
            else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.40) { picIndex = 6; }
            else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.46) { picIndex = 7; }
			else if ( player.animationDisplayAmt > 0)                                 { picIndex = 6; }
        }
		// Animation for the rest of the Attacks
		else {
			if      ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.10) { picIndex = 1; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.14) { picIndex = 2; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.18) { picIndex = 3; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.22) { picIndex = 4; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.26) { picIndex = 5; }
			else if ( player.animationDisplayAmt > player.currentSwordAtkTime - 0.30) { picIndex = 6; }
			else if ( player.animationDisplayAmt > 0)                                 { picIndex = 7; } }

		if (player.facing == 1) {
			glTranslatef(-displace2, 0.02, 0);
			picIndex += textureSheetWidth; }
		else if (player.facing == 3) {
			glTranslatef( displace2, 0.02, 0); }

        GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
        if ( draw ) drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
	// Movement Animation
    else if ( player.animationType == 0 && player.moving
              || ( player.animationType == -1 && player.animationDisplayAmt > 0.2 ) ) {

        if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            if ( player.npc ) { texture = darkMegamanMoveSheet; }
            else              { texture = megamanMoveSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            if ( player.npc ) { texture = darkProtoMoveSheet; }
            else              { texture = protoMoveSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            if ( player.npc ) { texture = darkColonelMoveSheet; }
            else              { texture = colonelMoveSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
        }

		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

        const float beforeHurtTime = pauseAnimationTime - moveAnimationTime;
		if		(player.animationDisplayAmt > (moveAnimationTime * 0.70 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 1; picIndex = 0; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.60 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 1; picIndex = 1; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.50 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 1; picIndex = 2; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.45 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 0; picIndex = 3; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.35 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 0; picIndex = 2; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.25 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 0; picIndex = 1; }
		else if (player.animationDisplayAmt > (moveAnimationTime * 0.00 + (player.animationType == -1 ? beforeHurtTime : 0))) { displace = 0; picIndex = 0; }
		if ( draw ) {
            if (player.moveDir == 0) {
                player.yOffset = -displace * scaleY;
			    glTranslatef(0, -displace * scaleY, 0);
			    if      (player.facing == 1) {
                    glTranslatef( -displace2, 0, 0 );
                    drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2); }
			    else if (player.facing == 3) {
                    glTranslatef(  displace2, 0, 0 );
                    drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);     }
		    }
		    else if (player.moveDir == 1) {
                player.xOffset = displace * scaleX;
			    glTranslatef(displace * scaleX - displace2, 0, 0);
			    drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2);
		    }
		    else if (player.moveDir == 2) {
                player.yOffset = displace * scaleY;
			    glTranslatef(0.0, displace * scaleY, 0.0);
			    if      (player.facing == 1) {
                    glTranslatef( -displace2, 0, 0 );
                    drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2);  }
			    else if (player.facing == 3) {
                    glTranslatef( displace2, 0, 0 );
                    drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);      }
		    }
		    else if (player.moveDir == 3) {
                player.xOffset = -displace * scaleX;
			    glTranslatef(-displace * scaleX + displace2, 0, 0);
			    drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);
		    }
        }
	}
    // Trapped Energy Hurt Animation
    else if ( player.animationType == -1 && player.animationDisplayAmt > 0 && player.animationDisplayAmt < 0.2 ) {
        int spriteSheetWidth = 2;

        if ( player.type == 0 ) {
		    playerSizeX = 0.40 * playerScale;
		    playerSizeY = 0.48 * playerScale;
            glTranslatef( 0, -0.06 * playerScale, 0 );
            displace2 = 0.014;
            if ( player.npc ) { texture = darkMegamanHurtSheet; }
            else              { texture = megamanHurtSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.70 * playerScale;
		    playerSizeY = 0.49 * playerScale;
            glTranslatef( 0, -0.05 * playerScale, 0 );
            displace2 = -0.01 * playerScale;
            if ( player.npc ) { texture = darkProtoHurtSheet; }
            else              { texture = protoHurtSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
            spriteSheetWidth = 4;
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.65 * playerScale;
		    playerSizeY = 0.67 * playerScale;
            glTranslatef( 0, 0.088 * playerScale, 0 );
            displace2 = -0.184 * playerScale;
            if ( player.npc ) { texture = darkColonelHurtSheet; }
            else              { texture = colonelHurtSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
            playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
            spriteSheetWidth = 4;
        }

		if		( player.animationDisplayAmt > 0.02 )   { picIndex = 0; }
        else if ( player.animationDisplayAmt > 0.00 ) {
            if ( player.type == 2 || player.type == 4 ) { picIndex = 0; }
                                                   else { picIndex = 1; }
        }

        GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
                                  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
                                  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
                                  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

        if ( draw ) {
		    if (player.facing == 1) {
			    glTranslatef(-displace2, 0, 0);
			    drawSpriteSheetSprite(playerPlace, texture, picIndex + spriteSheetWidth, spriteSheetWidth, 2);
		    }
		    else if (player.facing == 3) {
			    glTranslatef(displace2, 0, 0);
			    drawSpriteSheetSprite(playerPlace, texture, picIndex, spriteSheetWidth, 2);
		    }
        }
    }
	// Turning Animation
	else if ( player.animationType == 0 && player.turning) {

		bool reverse = true;
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            if ( player.npc ) { texture = darkMegamanMoveSheet; }
            else              { texture = megamanMoveSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.08 * playerScale;
            if ( player.npc ) { texture = darkProtoMoveSheet; }
            else              { texture = protoMoveSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace = -0.1 * playerScale;
            if ( player.npc ) { texture = darkColonelMoveSheet; }
            else              { texture = colonelMoveSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
        }

		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

		if		(player.animationDisplayAmt > moveAnimationTime * 0.70) { picIndex = 0; reverse = true; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.60) { picIndex = 1; reverse = true; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.50) { picIndex = 2; reverse = true; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.45) { picIndex = 3; reverse = false; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.35) { picIndex = 2; reverse = false; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.25) { picIndex = 1; reverse = false; }
		else if (player.animationDisplayAmt > moveAnimationTime * 0.00) { picIndex = 0; reverse = false; }

		if (player.facing == 1) {
            picIndex += (reverse ? 0 : 4);
            reverse ? glTranslatef(  displace, 0, 0 ) : glTranslatef( -displace, 0, 0 ); }
		else if (player.facing == 3) {
            picIndex += (reverse ? 4 : 0);
            reverse ? glTranslatef( -displace, 0, 0 ) : glTranslatef(  displace, 0, 0 ); }
        if ( draw ) drawSpriteSheetSprite( playerPlace, texture, picIndex, 4, 2 );
	}
	// Special Invalid Movement animation for Start Tile
	else if ( player.animationType == 2 && player.animationDisplayAmt > 0) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet;
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet;
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
             texture = tmanMoveSheet;
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet;
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet;
        }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = player.animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace - displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 0, 4, 2);
	}
	// Level Transition pt. 1
	else if ( player.animationType == 3 && player.animationDisplayAmt > moveAnimationTime * 2) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            if ( player.npc ) { texture = darkMegamanMoveSheet; }
            else              { texture = megamanMoveSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.08 * playerScale;
            if ( player.npc ) { texture = darkProtoMoveSheet; }
            else              { texture = protoMoveSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace = -0.1 * playerScale;
            if ( player.npc ) { texture = darkColonelMoveSheet; }
            else              { texture = colonelMoveSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
        }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = 1 - (player.animationDisplayAmt - moveAnimationTime * 2) / moveAnimationTime;
		glTranslatef(displace + displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 4, 4, 2);
	}
	// Level Transition pt. 2
	else if ( player.animationType == 4 && player.animationDisplayAmt > 0 && player.animationDisplayAmt <= moveAnimationTime * 2) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet;
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet;
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = tmanMoveSheet;
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet;
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet;
        }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = player.animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace + displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 4, 4, 2);
	}
    // Player Standing Still
    else if ( player.hp > 0 ) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            if ( player.npc ) { texture = darkMegamanMoveSheet; }
            else              { texture = megamanMoveSheet; }
        }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.08 * playerScale;
            if ( player.npc ) { texture = darkProtoMoveSheet; }
            else              { texture = protoMoveSheet; }
        }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkTmanMoveSheet; }
            else              { texture = tmanMoveSheet; }
        }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace = -0.1 * playerScale;
            if ( player.npc ) { texture = darkColonelMoveSheet; }
            else              { texture = colonelMoveSheet; }
        }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace = 0.2 * playerScale;
            if ( player.npc ) { texture = darkSlashmanMoveSheet; }
            else              { texture = slashmanMoveSheet; }
        }

        player.xOffset = 0;     player.yOffset = 0;
		if      (player.facing == 1) { picIndex = 0; glTranslatef( -displace, 0, 0 ); }
		else if (player.facing == 3) { picIndex = 4; glTranslatef(  displace, 0, 0 ); }

        GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
            player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
            player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
            player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };
        if ( draw ) drawSpriteSheetSprite( playerPlace, texture, picIndex, 4, 2 );
	}
}
void App::drawSwordAtks(const Player &player) {
    glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);
	if (player.animationDisplayAmt > 0 && player.animationType == 1) {
		if      (player.selSwordAnimation == 0) { swordDisplay(player); }
		else if (player.selSwordAnimation == 1) { longDisplay(player); }
		else if (player.selSwordAnimation == 2) { wideDisplay(player); }
		else if (player.selSwordAnimation == 3) { crossDisplay(player); }
		else if (player.selSwordAnimation == 4) { spinDisplay(player); }
		else if (player.selSwordAnimation == 5) { stepDisplay(player); }
		else if (player.selSwordAnimation == 6) { lifeDisplay(player); }
        else if (player.selSwordAnimation == 7) { heroDisplay(player); }
        else if (player.selSwordAnimation == 8) { protoDisplay(player); }
        else if (player.selSwordAnimation == 9)  { vDivideDisplay(player); }
        else if (player.selSwordAnimation == 10) { upDivideDisplay(player); }
        else if (player.selSwordAnimation == 11) { downDivideDisplay(player); }
        else if (player.selSwordAnimation == 12) { xDivideDisplay(player); }
        else if (player.selSwordAnimation == 13) { zDivideDisplay(player); }
        else if (player.selSwordAnimation == 14) { tomaDisplayA1(player); }
        else if (player.selSwordAnimation == 15) { tomaDisplayB1(player); }
        else if (player.selSwordAnimation == 16) { tomaDisplayA2(player); }
        else if (player.selSwordAnimation == 17) { tomaDisplayB2(player); }
        else if (player.selSwordAnimation == 18) {
            for ( int i = 0; i < delayedETomaDisplayList.size(); i++ ) {
                if ( delayedETomaDisplayList[i].delay <= 0 ) {
                    eTomaDisplay( delayedETomaDisplayList[i].xPos, delayedETomaDisplayList[i].yPos, delayedETomaDisplayList[i].animationTimer );
                }
            }
        }
        else if (player.selSwordAnimation == 19) { longSlashDisplay(player); }
        else if (player.selSwordAnimation == 20) { wideSlashDisplay(player); }
        else if (player.selSwordAnimation == 21) { stepCrossDisplay(player); }
    }
}
void App::drawNpcHp() {
    for ( int i = 0; i < npcList.size(); i++ ) {
        glLoadIdentity();
        glOrtho( orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0 );
        glTranslatef( 0.5 + npcList[i].xOffset, 0.7 + npcList[i].yOffset, 0.0 );

        GLuint texture = textSheet1A;
        if ( npcList[i].hurtDisplayAmt > 0 || npcList[i].hp == 0 ) { texture = textSheet1C; }

        glTranslatef( npcList[i].x * scaleX, npcList[i].y * scaleY, 0 );
        if ( npcList[i].hp >= 10 )   { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
        if ( npcList[i].hp >= 100 )  { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
        if ( npcList[i].hp >= 1000 ) { glTranslatef( -0.08 * 1.5 / 2, 0, 0 ); }
        if ( npcList[i].hp >= 0 ) { drawText( texture, to_string( npcList[i].hp ), 0.08 * 1.5, 0.16 * 1.5, 0.00 ); }
    }
}

void App::drawFloor() {
	// Draw Tiles
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 0.7, 0.0);

	float index = 0;
	float width  = 0.40 * overallScale;
	float height = 0.24 * overallScale;
	float height2 = 0.06 * overallScale;

	for (int i = 0; i < 6; i++) {
		for (int j = 5; j >= 0; j--) {
			GLfloat place[] = { i * scaleX + width, j * scaleY + height, i * scaleX + width, j * scaleY - height,
								i * scaleX - width, j * scaleY - height, i * scaleX - width, j * scaleY + height };
			index = map[i][j].state;
			if (j == 0 || j == 1) { index += 8; }
			else if (j == 2 || j == 3) { index += 4; }
			drawSpriteSheetSprite(place, floorSheet, index, 4, 3);
	}	}

	// Draw Start Tile
	float startX = -1;
	float startY = 0;
	GLfloat startPlace[] = { startX * scaleX + width, startY * scaleY + height, startX * scaleX + width, startY * scaleY - height,
							 startX * scaleX - width, startY * scaleY - height, startX * scaleX - width, startY * scaleY + height };
	if      (itemAnimationAmt >= 0 && itemAnimationAmt <= 1) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 8, 4, 3); }
	else if (itemAnimationAmt >= 1 && itemAnimationAmt <= 2) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 9, 4, 3); }
	else if (itemAnimationAmt >= 2 && itemAnimationAmt <= 3) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 10, 4, 3); }
	else if (itemAnimationAmt >= 3 && itemAnimationAmt <= 4) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 11, 4, 3); }
	else if (itemAnimationAmt >= 4 && itemAnimationAmt <= 5) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 8, 4, 3); }
	else if (itemAnimationAmt >= 5 && itemAnimationAmt <= 6) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 9, 4, 3); }
	else if (itemAnimationAmt >= 6 && itemAnimationAmt <= 7) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 10, 4, 3); }
	else if (itemAnimationAmt >= 7 && itemAnimationAmt <= 8) { drawSpriteSheetSprite(startPlace, floorMoveSheet, 11, 4, 3); }

	// Draw Goal Tile
	float goalX = 6;
	float goalY = 5;
	GLfloat goalPlace[] = { goalX * scaleX + width, goalY * scaleY + height, goalX * scaleX + width, goalY * scaleY - height,
							goalX * scaleX - width, goalY * scaleY - height, goalX * scaleX - width, goalY * scaleY + height };
	if      (itemAnimationAmt >= 0 && itemAnimationAmt <= 1) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 0, 4, 3); }
	else if (itemAnimationAmt >= 1 && itemAnimationAmt <= 2) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 1, 4, 3); }
	else if (itemAnimationAmt >= 2 && itemAnimationAmt <= 3) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 2, 4, 3); }
	else if (itemAnimationAmt >= 3 && itemAnimationAmt <= 4) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 3, 4, 3); }
	else if (itemAnimationAmt >= 4 && itemAnimationAmt <= 5) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 0, 4, 3); }
	else if (itemAnimationAmt >= 5 && itemAnimationAmt <= 6) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 1, 4, 3); }
	else if (itemAnimationAmt >= 6 && itemAnimationAmt <= 7) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 2, 4, 3); }
	else if (itemAnimationAmt >= 7 && itemAnimationAmt <= 8) { drawSpriteSheetSprite(goalPlace, floorMoveSheet, 3, 4, 3); }

	// Draw Tile Bottom pieces
	for (int x = -1; x <= 6; x++) {
		float y = -0.63;
		if (x == 6) { y += 5; }
		GLfloat bottomPlace[] = { x * scaleX + width, y * scaleY + height2, x * scaleX + width, y * scaleY - height2,
								  x * scaleX - width, y * scaleY - height2, x * scaleX - width, y * scaleY + height2 };
		if (x == 6) { drawSpriteSheetSprite(bottomPlace, floorBottomPic2, 0, 1, 1); }
		else        { drawSpriteSheetSprite(bottomPlace, floorBottomPic1, 0, 1, 1); }
	}
}
void App::drawBoxes(int row) {			// Draw the Rock Obstacles on the Floor
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.01, 0.0);

	float sizeX  = 0.32 * itemScale;
	float sizeY  = 0.40 * itemScale;
	float sizeX2 = 0.55 * itemScale;
	float sizeY2 = 0.55 * itemScale;
    
	GLuint textureSheet;
	int sheetHeight = 3;
	int index = 0;

	for (int i = 0; i < 6; i++) {
		if (map[i][row].boxHP > 0 || map[i][row].boxAtkInd > 0) {
			if ( map[i][row].boxHP > 0 ) {
				GLfloat place[] = { i * scaleX + sizeX, row * scaleY + sizeY, i * scaleX + sizeX, row * scaleY - sizeY,
									i * scaleX - sizeX, row * scaleY - sizeY, i * scaleX - sizeX, row * scaleY + sizeY };

				if      (map[i][row].boxType <= 1) { sheetHeight = 5; textureSheet = rockSheet; }
                else if (map[i][row].boxType == 2) {
                    sheetHeight = 3;
                    if      ( map[i][row].item == 2 )  { textureSheet = rockSheetItem2; }
                    else if ( map[i][row].item == -1 ) { textureSheet = rockSheetTrappedItem; }
                    else                               { textureSheet = rockSheetItem; }
                }

				index = (map[i][row].boxHP - 1) * 3;
				if      (map[i][row].prevDmg == 1 && map[i][row].boxAtkInd > 0) { index += 4; }
				else if (map[i][row].prevDmg == 2 && map[i][row].boxAtkInd > 0) { index += 8; }

                drawSpriteSheetSprite(place, textureSheet, index, 3, sheetHeight);
            }
			else {
                textureSheet = rockDeathSheet;
                if ( map[i][row].bigBoxDeath ) {
                    sizeX2 = 0.75 * itemScale;
                    sizeY2 = 0.75 * itemScale; }
                if ( map[i][row].isPurple ) {
                    textureSheet = darkDeathSheet; }

				GLfloat place[] = { i * scaleX + sizeX2, row * scaleY + sizeY2, i * scaleX + sizeX2, row * scaleY - sizeY2,
									i * scaleX - sizeX2, row * scaleY - sizeY2, i * scaleX - sizeX2, row * scaleY + sizeY2 };
				if      (map[i][row].boxAtkInd > boxDmgTime * 0.60) { drawSpriteSheetSprite(place, textureSheet, 0, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.40) { drawSpriteSheetSprite(place, textureSheet, 1, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.20) { drawSpriteSheetSprite(place, textureSheet, 2, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.00) { drawSpriteSheetSprite(place, textureSheet, 3, 4, 1); }
}	}	}	}
void App::drawItems(int row) {		// Draw the Collectable Resources on the Map
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.11, 0.0);

	float itemSizeX = 0.23 * itemScale / 1.5;
	float itemSizeY = 0.61 * itemScale / 1.5;

	for (int i = 0; i < 6; i++) {
		GLfloat place[] = { i + itemSizeX * scaleX, row * scaleY + itemSizeY, i + itemSizeX * scaleX, row * scaleY - itemSizeY,
							i - itemSizeX * scaleX, row * scaleY - itemSizeY, i - itemSizeX * scaleX, row * scaleY + itemSizeY };
		if (map[i][row].item != 0 && map[i][row].boxHP <= 0) {
            GLuint texture = energySheet;
            if      ( map[i][row].item == 2 )  { texture = energySheet2; }
            else if ( map[i][row].item == -1 ) { texture = trappedEnergySheet; }

			if      (itemAnimationAmt >= 0 && itemAnimationAmt < 1) { drawSpriteSheetSprite(place, texture, 0, 8, 1); }
			else if (itemAnimationAmt >= 1 && itemAnimationAmt < 2) { drawSpriteSheetSprite(place, texture, 1, 8, 1); }
			else if (itemAnimationAmt >= 2 && itemAnimationAmt < 3) { drawSpriteSheetSprite(place, texture, 2, 8, 1); }
			else if (itemAnimationAmt >= 3 && itemAnimationAmt < 4) { drawSpriteSheetSprite(place, texture, 3, 8, 1); }
			else if (itemAnimationAmt >= 4 && itemAnimationAmt < 5) { drawSpriteSheetSprite(place, texture, 4, 8, 1); }
			else if (itemAnimationAmt >= 5 && itemAnimationAmt < 6) { drawSpriteSheetSprite(place, texture, 5, 8, 1); }
			else if (itemAnimationAmt >= 6 && itemAnimationAmt < 7) { drawSpriteSheetSprite(place, texture, 6, 8, 1); }
			else if (itemAnimationAmt >= 7 && itemAnimationAmt < 8) { drawSpriteSheetSprite(place, texture, 7, 8, 1); } }
    }
}
void App::drawItemUpgrading(int row) {
    glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.11, 0.0);

    float itemSizeX = 0.31 * itemScale / 1.0;
	float itemSizeY = 0.59 * itemScale / 1.0;

    for (int i = 0; i < 6; i++) {
		GLfloat place[] = { i + itemSizeX * scaleX, row * scaleY + itemSizeY, i + itemSizeX * scaleX, row * scaleY - itemSizeY,
							i - itemSizeX * scaleX, row * scaleY - itemSizeY, i - itemSizeX * scaleX, row * scaleY + itemSizeY };
		if (map[i][row].upgradeInd > 0) {
            GLuint texture = recoverSheet;

			if      ( map[i][row].upgradeInd > itemUpgradeTime * 0.875 ) { drawSpriteSheetSprite(place, texture, 0, 6, 1); }
			else if ( map[i][row].upgradeInd > itemUpgradeTime * 0.750 ) { drawSpriteSheetSprite(place, texture, 1, 6, 1); }
			else if ( map[i][row].upgradeInd > itemUpgradeTime * 0.625 ) { drawSpriteSheetSprite(place, texture, 2, 6, 1); }
			else if ( map[i][row].upgradeInd > itemUpgradeTime * 0.500 ) { drawSpriteSheetSprite(place, texture, 3, 6, 1); }
			else if ( map[i][row].upgradeInd > itemUpgradeTime * 0.250 ) { drawSpriteSheetSprite(place, texture, 4, 6, 1); }
			else if ( map[i][row].upgradeInd > itemUpgradeTime * 0.000 ) { drawSpriteSheetSprite(place, texture, 5, 6, 1); }
        }
    }
}

// Sword Attack Animations
void App::swordDisplay( const Player &player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.22 * playerScale;
	int xPos = player.x;
	int yPos = player.y;
    GLuint texture;

	if ( player.facing == 1 ) {
        xPos--;
        texture = swordAtkSheet1; }
    else if ( player.facing == 3 ) {
        xPos++;
        texture = swordAtkSheet3; }

	GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };

	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::longDisplay( const Player &player) {
	float sizeX = 0.70 * playerScale;
	float sizeY = 0.37 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos -= 1.5;
        texture = longAtkSheet1; }
	else if (player.facing == 3) {
		xPos += 1.5;
        texture = longAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideDisplay( const Player &player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (player.facing == 3) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::crossDisplay( const Player &player) {
	float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (player.facing == 3) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::spinDisplay( const Player &player) {
	float sizeX = 1.10 * 1.25 * playerScale;
	float sizeY = 0.72 * 1.25 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
        texture = spinAtkSheet1; }
	else if (player.facing == 3) {
        texture = spinAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player.animationDisplayAmt > 0)                   { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::stepDisplay( const Player &player) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (player.facing == 3) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player.animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::lifeDisplay( const Player &player) {
	float sizeX = 0.95 * playerScale;
	float sizeY = 0.80 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos -= 1.5;
        texture = lifeAtkSheet1; }
	else if (player.facing == 3) {
		xPos += 1.5;
        texture = lifeAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::heroDisplay( const Player &player ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.45 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 2;
        texture = heroAtkSheet1; }
    else if ( player.facing == 3 ) {
        xPos += 2;
        texture = heroAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::protoDisplay( const Player &player ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.71 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 2;
        texture = protoAtkSheet1; }
    else if ( player.facing == 3 ) {
        xPos += 2;
        texture = protoAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::vDivideDisplay( const Player &player ) {
    float sizeX = 0.68 * playerScale;
    float sizeY = 0.53 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 0.75;
        texture = screenDivVSheet3; }
    else if ( player.facing == 3 ) {
        xPos += 0.75;
        texture = screenDivVSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::upDivideDisplay( const Player &player ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1;
        texture = screenDivUpSheet3; }
    else if ( player.facing == 3 ) {
        xPos += 1;
        texture = screenDivUpSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::downDivideDisplay( const Player &player ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1;
        texture = screenDivDownSheet3; }
    else if ( player.facing == 3 ) {
        xPos += 1;
        texture = screenDivDownSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::xDivideDisplay( const Player &player ) {
    float sizeX = 0.94 * playerScale;
    float sizeY = 1.03 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1;
        texture = screenDivXSheet3; }
    else if ( player.facing == 3 ) {
        xPos += 1;
        texture = screenDivXSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player.animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::zDivideDisplay( const Player &player) {
	float sizeX = 1.32 * playerScale;
	float sizeY = 0.60 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos -= 1; }
	else if (player.facing == 3) {
		xPos += 1; }
    texture = screenDivZSheet;

    GLfloat atkPlace[] = { xPos * scaleX - sizeX, yPos * scaleY + sizeY, xPos * scaleX - sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX + sizeX, yPos * scaleY - sizeY, xPos * scaleX + sizeX, yPos * scaleY + sizeY };
    if		(player.animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > lifeAtkTime - 0.16) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.28) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (player.animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA1( const Player &player) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetA1; }
    else if ( player.facing == 3 ) {
        xPos += 1;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player.animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB1( const Player &player) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetB1; }
    else if ( player.facing == 3 ) {
        xPos += 1;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player.animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA2( const Player &player) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetA1; }
    else if ( player.facing == 3 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player.animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB2( const Player &player) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( player.facing == 1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetB1; }
    else if ( player.facing == 3 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( player.animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( player.animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::eTomaDisplay(int xPos, int yPos, float animationTime) {
    float sizeX = 0.52 * playerScale;
    float sizeY = 0.72 * playerScale;
    if ( animationTime > 0 ) { glTranslatef( 0, 0.2, 0 ); }
    GLuint texture = eagleTomaSheet;

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationTime >= 0.10 ) {}
    else if ( animationTime >  0.08 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 4, 1 ); }
    else if ( animationTime >  0.04 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 4, 1 ); }
    else if ( animationTime >  0.02 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 4, 1 ); }
    else if ( animationTime >  0.00 ) { drawSpriteSheetSprite( atkPlace, texture, 3, 4, 1 ); }
}
void App::longSlashDisplay( const Player &player) {
    float sizeX = 0.84 * playerScale;
	float sizeY = 0.54 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos -= 1.5;
        texture = longSlashSheet1; }
	else if (player.facing == 3) {
		xPos += 1.5;
        texture = longSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideSlashDisplay( const Player &player) {
    float sizeX = 0.39 * playerScale;
	float sizeY = 0.95 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos--;
        texture = wideSlashSheet1; }
	else if (player.facing == 3) {
		xPos++;
        texture = wideSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(player.animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (player.animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (player.animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::stepCrossDisplay( const Player &player) {
    float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (player.facing == 1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (player.facing == 3) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( player.animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( player.animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( player.animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}

// Player Control Functions: Attacking & Movement
void App::face( Player &player, int dir ) {
    if ( dir == 1 || dir == 3 ) {
        if ( player.facing != dir ) {
            player.x2 = player.x;
            player.y2 = player.y;
            player.turning = true;
            player.facing = dir;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( !player.npc ) npcAbleToAct = true;
        }
    }
}
bool App::move(Player &player, int dir) {
    bool acted = false;
    if ( dir == 0 ) {			// Move Up			// Can't move out of map, onto a box, or onto a hole in the floor
        if ( isTileValid( player.x, player.y + 1, player.npc ) ) {
            if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.y++;
            player.moving = true;
            player.moveDir = 0;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.animationType = -1;
                    player.animationDisplayAmt = pauseAnimationTime;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc ) npcAbleToAct = true;
            acted = true;
        }
    }
    else if ( dir == 1 ) {	// Move Left
        if ( player.facing != 1 ) {
            face( player, 1 );
            acted = true;
        }
        if ( isTileValid( player.x - 1, player.y, player.npc ) ) {
            // Walk off Cracked tile -> Cracked tile becomes a Hole
            if ( map[player.x][player.y].state == 1 && !( player.x == 0 && player.y == 0 ) && !player.npc ) {
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.x--;
            player.moving = true;
            player.moveDir = 1;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.animationType = -1;
                    player.animationDisplayAmt = pauseAnimationTime;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc && player.x == -1 && player.y == 0 ) npcAbleToAct = false;
            else if ( !player.npc ) npcAbleToAct = true;
            acted = true;
        }
    }
    else if ( dir == 2 ) {	// Move Down
        if ( isTileValid( player.x, player.y - 1, player.npc ) ) {
            if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.y--;
            player.moving = true;
            player.moveDir = 2;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.animationType = -1;
                    player.animationDisplayAmt = pauseAnimationTime;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc ) npcAbleToAct = true;
            acted = true;
        }
    }
    else if ( dir == 3 ) {	// Move Right
        if ( player.facing != 3 ) {
            face( player, 3 );
            acted = true;
        }
        if ( isTileValid( player.x + 1, player.y, player.npc ) ) {
            if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.x++;
            player.moving = true;
            player.moveDir = 3;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.animationType = -1;
                    player.animationDisplayAmt = pauseAnimationTime;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc && player.x == 6 && player.y == 5 ) npcAbleToAct = false;
            else if ( !player.npc ) npcAbleToAct = true;
            acted = true;
        }
    }
    return acted;
}
void App::move2(Player &player, int dir) {		// Move Two Tiles
	if ( dir == 0 ) {			// Move Up			// Can't move out of map, onto a box, or onto a hole in the floor
		if ( isTileValid( player.x, player.y + 2, player.npc ) ) {
			if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
            player.x2 = player.x;
            player.y2 = player.y;
			player.y += 2;
			player.moving = true;
			player.moveDir = 0;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
			if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.selSwordAnimation = -4;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
    else if ( dir == 1 ) {	// Move Left
        if ( isTileValid( player.x - 2, player.y, player.npc ) ) {
            if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.x -= 2;
            player.moving = true;
            player.moveDir = 1;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.selSwordAnimation = -4;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc && player.x == -1 && player.y == 0 ) npcAbleToAct = false;
        }
    }
	else if ( dir == 2 ) {	// Move Down
		if ( isTileValid(player.x, player.y - 2, player.npc ) ) {
			if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
            player.x2 = player.x;
            player.y2 = player.y;
			player.y -= 2;
			player.moving = true;
			player.moveDir = 2;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
			if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.selSwordAnimation = -4;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
    else if ( dir == 3 ) {	// Move Right
        if ( isTileValid( player.x + 2, player.y, player.npc ) ) {
            if ( map[player.x][player.y].state == 1 && !player.npc ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
                map[player.x][player.y].state = 2;
                Mix_PlayChannel( 2, panelBreakSound, 0 );
            }
            player.x2 = player.x;
            player.y2 = player.y;
            player.x += 2;
            player.moving = true;
            player.moveDir = 3;
            player.animationType = 0;
            player.animationDisplayAmt = moveAnimationTime;
            if ( player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0 && !player.npc ) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 5, itemSound2, 0 );
                    player.selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2;
                }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, hurtSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    player.selSwordAnimation = -4;
                    player.hurtDisplayAmt = hurtAnimationTime;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt;
                }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    player.selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt;
                }
                map[player.x][player.y].item = 0;
            }
            if ( !player.npc && player.x == 6 && player.y == 5 ) npcAbleToAct = false;
        }
    }
}

bool App::swordAtk(Player &player) {			// Does One Dmg to one square in front of the player
	if (player.energy >= swordCost) {
		player.energy -= swordCost;
		if      (player.facing == 1) { delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y, preAtkTime, player.npc ) ); }
		else if (player.facing == 3) { delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y, preAtkTime, player.npc ) ); }
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 0;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= swordCost;
            npcAbleToAct = true;
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::longAtk(Player &player) {		// Does One Dmg to a 2x1 area in front of the player
	if (player.energy >= longCost) {
		player.energy -= longCost;
		if (player.facing == 1) {											                                //
			delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y, preAtkTime, player.npc ) );	//	xxP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y, preAtkTime, player.npc ) );	//
		}
		else if (player.facing == 3) {									                                    //
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y, preAtkTime, player.npc ) );	//	  Pxx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y, preAtkTime, player.npc ) );	//
		}
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        if      ( player.type == 0 || player.type == 1 ) { player.selSwordAnimation = 1; }
        else if ( player.type == 4 )                     { player.selSwordAnimation = 19; }
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= longCost;
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::wideAtk(Player &player) {			// Does One Dmg to a 1x3 area in front of the player
	if ( player.type == 0 && player.energy >= wideCost ||
         player.type == 4 && player.energy >= wideCost ||
         player.type == 1 && player.energy >= wideCost2 ) {
        if      ( player.type == 0 || player.type == 4) { player.energy -= wideCost;  }
        else if ( player.type == 1 )                    { player.energy -= wideCost2; }
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y + 1, preAtkTime, player.npc ) );  // x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     preAtkTime, player.npc ) );  // xP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y - 1, preAtkTime, player.npc ) );  // x
		}
		else if (player.facing == 3) {
			delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y + 1, preAtkTime, player.npc ) );  //  x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     preAtkTime, player.npc ) );  // Px
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y - 1, preAtkTime, player.npc ) );  //  x
		}
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
		if      ( player.type == 0 || player.type == 1) { player.selSwordAnimation = 2; }
        else if ( player.type == 4  )                   { player.selSwordAnimation = 20; }
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            if ( player.type == 0 || player.type == 4 ) { currentEnergyGain -= wideCost; }
            else if ( player.type == 1 )                { currentEnergyGain -= wideCost2; }
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::crossAtk(Player &player) {			// Does One Dmg in an X pattern in front of the player		// The Middle of the X cross takes Two Dmg total
	if ( player.type == 0 && player.energy >= crossCost ||
         player.type == 4 && player.energy >= crossCost2 ) {
        if      ( player.type == 0 ) { player.energy -= crossCost; }
        else if ( player.type == 4 ) { player.energy -= crossCost2; }
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y + 1, preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y - 1, preAtkTime, player.npc ) );  //	 XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, preAtkTime, player.npc ) );
		}
		else if (player.facing == 3) {
			delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y + 1, preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y - 1, preAtkTime, player.npc ) );  //	PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, preAtkTime, player.npc ) );
		}
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 3;
		
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
		    chargeDisplayPlusAmt = 0;
            if      ( player.type == 0 ) { currentEnergyGain -= crossCost; }
            else if ( player.type == 4 ) { currentEnergyGain -= crossCost2; }
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::spinAtk(Player &player) {		// Does One Dmg to all squares adjacent to the player (including diagonal adjacents)
	if (player.energy >= spinCost) {
		player.energy -= spinCost;
        delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y + 1, preAtkTime, player.npc ) );  //	xxx
        delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     preAtkTime, player.npc ) );  //	xPx
        delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y - 1, preAtkTime, player.npc ) );  //	xxx
        delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y + 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y - 1, preAtkTime, player.npc ) );

		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 4;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= spinCost;
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::stepAtk(Player &player) {			// Moves the player two squares in the facing Direction, then uses WideSword
	if ( player.type == 0 && player.energy >= stepCost || player.type == 1 && player.energy >= stepCost2 ) {
		if (player.facing == 1 && isTileValid(player.x - 2, player.y)) {
			if      ( player.type == 0 ) { player.energy -= stepCost; }
            else if ( player.type == 1 ) { player.energy -= stepCost2; }

			move2(player, 1);
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  // x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  // xP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  // x

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
			player.selSwordAnimation = 5;
			
            if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                if      ( player.type == 0 ) { currentEnergyGain -= stepCost; }
                else if ( player.type == 1 ) { currentEnergyGain -= stepCost2; }
            }
            else { delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, true ) ); }
            return true;
		}
		else if (player.facing == 3 && isTileValid(player.x + 2, player.y)) {
			if      ( player.type == 0 ) { player.energy -= stepCost; }
            else if ( player.type == 1 ) { player.energy -= stepCost2; }

			move2(player, 3);
			delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  //  x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  // Px
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  //  x

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
            player.selSwordAnimation = 5;
			if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                if      ( player.type == 0 ) { currentEnergyGain -= stepCost; }
                else if ( player.type == 1 ) { currentEnergyGain -= stepCost2; }
            }
            else { delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, true ) ); }
            return true;
		}
	}
    return false;
}
bool App::lifeAtk(Player &player) {		// Does Two Dmg to a 2x3 area in front the player
	if (player.energy >= lifeCost) {
		player.energy -= lifeCost;
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, preAtkTime, player.npc ) );  // XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y,     preAtkTime, player.npc ) );  // XXP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, preAtkTime, player.npc ) );  // XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y - 1, preAtkTime, player.npc ) );
		}
		else if (player.facing == 3) {
			delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, preAtkTime, player.npc ) );  //  XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y,     preAtkTime, player.npc ) );  // PXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, preAtkTime, player.npc ) );  //  XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y - 1, preAtkTime, player.npc ) );
		}
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 6;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, lifeSwordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= lifeCost;
        }
        else { Mix_PlayChannel( 6, lifeSwordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::heroAtk( Player &player ) {      // Does Two Dmg to a 3x1 area in front of the player
    if ( player.energy >= heroCost ) {
        player.energy -= heroCost;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y, preAtkTime, player.npc ) );  // XXXP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 3, player.y, preAtkTime, player.npc ) );
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y, preAtkTime, player.npc ) );  // PXXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 3, player.y, preAtkTime, player.npc ) );
        }
        // Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 7;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= heroCost;
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::protoAtk( Player &player ) {     // Does Two Dmg in a Plus pattern in front of the player - HeroSword + WideSword
    if ( player.energy >= protoCost ) {
        player.energy -= protoCost;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y,     preAtkTime, player.npc ) );  // XXXP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 3, player.y,     preAtkTime, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, preAtkTime, player.npc ) );
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y,     preAtkTime, player.npc ) );  // XXXP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 3, player.y,     preAtkTime, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, preAtkTime, player.npc ) );
        }
        // Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 8;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, swordSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= protoCost;
        }
        else { Mix_PlayChannel( 6, swordSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::vDivideAtk( Player &player ) {   // Attacks in a V pattern   // Does 2 dmg at the tip of the V
    if (player.energy >= vDivideCost) {
		player.energy -= vDivideCost;
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, preAtkTime, player.npc ) );  //   x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     preAtkTime, player.npc ) );  //  xP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, preAtkTime, player.npc ) );  //   x  
        }
		else if (player.facing == 3) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, preAtkTime, player.npc ) );  //  x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     preAtkTime, player.npc ) );  //  Px
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, preAtkTime, player.npc ) );  //  x
        }
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 9;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, screenDivSound, 0 );
            npcAbleToAct = true;
		    chargeDisplayMinusAmt = iconDisplayTime;
		    chargeDisplayPlusAmt = 0;
		    currentEnergyGain -= vDivideCost;
        }
        else { Mix_PlayChannel( 6, screenDivSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::upDivideAtk( Player &player ) {  // Attacks Diagonally Upwards       Does 2 dmg
    if (player.energy >= upDivideCost) {
		player.energy -= upDivideCost;
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, preAtkTime, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );  //   XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, preAtkTime, player.npc ) );  //    X 
        }
		else if (player.facing == 3) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, preAtkTime, player.npc ) );  //     X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );  //   PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, preAtkTime, player.npc ) );  //   X
        }
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 10;

		if ( !player.npc ) {
            Mix_PlayChannel( 0, screenDivSound, 0 );
            npcAbleToAct = true;
		    chargeDisplayMinusAmt = iconDisplayTime;
		    chargeDisplayPlusAmt = 0;
		    currentEnergyGain -= upDivideCost;
        }
        else { Mix_PlayChannel( 6, screenDivSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::downDivideAtk( Player &player ) {    // Attacks Diagonally Downward      Does 2 dmg
    if (player.energy >= downDivideCost) {
		player.energy -= downDivideCost;
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, preAtkTime, player.npc ) );  //     X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );  //    XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, preAtkTime, player.npc ) );  //   X
        }
		else if (player.facing == 3) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, preAtkTime, player.npc ) );  //   X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );  //   PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, preAtkTime, player.npc ) );  //     X
        }
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = swordAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 11;

		if ( !player.npc ) {
            Mix_PlayChannel( 0, screenDivSound, 0 );
            npcAbleToAct = true;
		    chargeDisplayMinusAmt = iconDisplayTime;
		    chargeDisplayPlusAmt = 0;
		    currentEnergyGain -= downDivideCost;
        }
        else { Mix_PlayChannel( 6, screenDivSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::xDivideAtk( Player &player ) {			// Moves the player two squares in the facing Direction, then uses CrossSword
    if ( player.energy >= xDivideCost ) {
		if (player.facing == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= xDivideCost;

			move2(player, 1);
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  //	X X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  //	 XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  //	X X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
            player.selSwordAnimation = 12;
			
            if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                currentEnergyGain -= xDivideCost;
            }
            else { delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime, true ) ); }
            return true;
		}
		else if (player.facing == 3 && isTileValid(player.x + 2, player.y)) {
			player.energy -= xDivideCost;

			move2(player, 3);
			delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  //	X X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  //	PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  //	X X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
            player.selSwordAnimation = 12;

			if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                currentEnergyGain -= xDivideCost;
            }
            else { delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime, true ) ); }
            return true;
		}
	}
    return false;
}
bool App::zDivideAtk(Player &player) {		// Does Two Dmg in a Z pattern
	if (player.energy >= zDivideCost) {
		player.energy -= zDivideCost;
		if (player.facing == 1) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, preAtkTime, player.npc ) );  //	XXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, preAtkTime, player.npc ) );  //	 XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y + 1, preAtkTime, player.npc ) );  //	XXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y - 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, preAtkTime, player.npc ) );
		}
		else if (player.facing == 3) {
			delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, preAtkTime, player.npc ) );  //	XXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, preAtkTime, player.npc ) );  //	PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y + 1, preAtkTime, player.npc ) );  //	XXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y - 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, preAtkTime, player.npc ) );
		}
		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 13;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, screenDivSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= zDivideCost;
        }
        else { Mix_PlayChannel( 6, screenDivSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}
bool App::tomaAtkA1(Player &player) {
    if ( player.energy >= tomaCostA1 ) { 
        player.energy -= tomaCostA1;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y + 1, preAtkTimeToma, player.npc ) );  // x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     preAtkTimeToma, player.npc ) );  // xP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y - 1, preAtkTimeToma, player.npc ) );  // x
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y + 1, preAtkTimeToma, player.npc ) );  // x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     preAtkTimeToma, player.npc ) );  // xP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y - 1, preAtkTimeToma, player.npc ) );  // x
        }
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 14;
        
        if ( !player.npc ) {
            delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= tomaCostA1;
        }
        else { delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma, true ) ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::tomaAtkB1(Player &player) {
    if ( player.energy >= tomaCostA2 ) {
        player.energy -= tomaCostA2;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y + 1, preAtkTimeToma, player.npc ) );  // X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTimeToma, player.npc ) );  // XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y - 1, preAtkTimeToma, player.npc ) );  // X
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y + 1, preAtkTimeToma, player.npc ) );  //  X
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTimeToma, player.npc ) );  // PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y - 1, preAtkTimeToma, player.npc ) );  //  X
        }
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 15;

        if ( !player.npc ) {
            delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= tomaCostB1;
        }
        else { delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma, true ) ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::tomaAtkA2(Player &player) {
    if ( player.energy >= tomaCostB1 ) { 
        player.energy -= tomaCostB1;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y + 1, preAtkTimeToma, player.npc ) );  // xx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y,     preAtkTimeToma, player.npc ) );  // xxP
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y - 1, preAtkTimeToma, player.npc ) );  // xx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y + 1, preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y,     preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 1, player.y - 1, preAtkTimeToma, player.npc ) );
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y + 1, preAtkTimeToma, player.npc ) );  //  xx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y,     preAtkTimeToma, player.npc ) );  // Pxx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y - 1, preAtkTimeToma, player.npc ) );  //  xx
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y + 1, preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y,     preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 1, player.y - 1, preAtkTimeToma, player.npc ) );
        }
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 16;
        
        if ( !player.npc ) {
            delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= tomaCostA2;
        }
        else { delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma, true ) ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::tomaAtkB2(Player &player) {
    if ( player.energy >= tomaCostB2 ) { 
        player.energy -= tomaCostB2;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y + 1, preAtkTimeToma, player.npc ) );  // XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y,     preAtkTimeToma, player.npc ) );  // XXP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 2, player.y - 1, preAtkTimeToma, player.npc ) );  // XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y + 1, preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y - 1, preAtkTimeToma, player.npc ) );
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y + 1, preAtkTimeToma, player.npc ) );  //  XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y,     preAtkTimeToma, player.npc ) );  // PXX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 2, player.y - 1, preAtkTimeToma, player.npc ) );  //  XX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y + 1, preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTimeToma, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y - 1, preAtkTimeToma, player.npc ) );
        }
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 17;
        
        if ( !player.npc ) {
            delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= tomaCostB2;
        }
        else { delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma, true ) ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::eTomaAtk(Player &player) {
    if ( player.energy >= eTomaCost ) {
        player.energy -= eTomaCost;
        if ( player.facing == 1 ) {
            delayedHpList.push_back( DelayedHpLoss( 5, player.x - 1, player.y, preAtkTimeToma - 0.06, player.npc ) );   // XXXXXP
            delayedHpList.push_back( DelayedHpLoss( 5, player.x - 2, player.y, preAtkTimeToma,        player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x - 3, player.y, preAtkTimeToma + 0.04, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x - 4, player.y, preAtkTimeToma + 0.08, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x - 5, player.y, preAtkTimeToma + 0.12, player.npc ) );
            
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 2, player.y, preAtkTimeEagleToma ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 3, player.y, preAtkTimeEagleToma + 0.04 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 4, player.y, preAtkTimeEagleToma + 0.08 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 5, player.y, preAtkTimeEagleToma + 0.12 ) );
        }
        else if ( player.facing == 3 ) {
            delayedHpList.push_back( DelayedHpLoss( 5, player.x + 1, player.y, preAtkTimeToma - 0.06, player.npc ) );   // PXXXXX
            delayedHpList.push_back( DelayedHpLoss( 5, player.x + 2, player.y, preAtkTimeToma,        player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x + 3, player.y, preAtkTimeToma + 0.04, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x + 4, player.y, preAtkTimeToma + 0.08, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 5, player.x + 5, player.y, preAtkTimeToma + 0.12, player.npc ) );

            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 2, player.y, preAtkTimeEagleToma ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 3, player.y, preAtkTimeEagleToma + 0.04 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 4, player.y, preAtkTimeEagleToma + 0.08 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 5, player.y, preAtkTimeEagleToma + 0.12 ) );
        }
        player.animationType = 1;
        player.animationDisplayAmt = eTomaAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 18;
        
        if ( !player.npc ) {
            delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma ) );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= eTomaCost;
        }
        else { delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma, true ) ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
    }
    return false;
}
bool App::stepCrossAtk(Player &player) {
    if ( player.energy >= stepCrossCost ) {
		if (player.facing == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= stepCrossCost;

			move2(player, 1);
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x - 2, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  //	 XP
            delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
            player.selSwordAnimation = 21;
            
            if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                currentEnergyGain -= stepCrossCost;
            }
            else { delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, true ) ); }
            return true;
		}
		else if (player.facing == 3 && isTileValid(player.x + 2, player.y)) {
            player.energy -= stepCrossCost;

			move2(player, 3);
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x + 2, player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );  //	PX
            delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     moveAnimationTime + preAtkTime, player.npc ) );  //	x x
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y + 1, moveAnimationTime + preAtkTime, player.npc ) );
            delayedHpList.push_back( DelayedHpLoss( 1, player.x,     player.y - 1, moveAnimationTime + preAtkTime, player.npc ) );

            player.animationType = 1;
            player.animationDisplayAmt = stepAtkTime + moveAnimationTime;
            player.currentSwordAtkTime = player.animationDisplayAmt;
            player.selSwordAnimation = 21;

            if ( !player.npc ) {
                delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
                npcAbleToAct = true;
                chargeDisplayMinusAmt = iconDisplayTime;
                chargeDisplayPlusAmt = 0;
                currentEnergyGain -= stepCrossCost;
            }
            else { delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime, true ) ); }
            return true;
		}
	}
    return false;
}
bool App::spinSlashAtk(Player &player) {
    if (player.energy >= spinSlashCost) {
		player.energy -= spinSlashCost;
		delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y + 1, preAtkTime, player.npc ) );  //	XXX
        delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y,     preAtkTime, player.npc ) );  //	XPX
        delayedHpList.push_back( DelayedHpLoss( 2, player.x - 1, player.y - 1, preAtkTime, player.npc ) );  //	XXX
        delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y + 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 2, player.x,     player.y - 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y + 1, preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y,     preAtkTime, player.npc ) );
        delayedHpList.push_back( DelayedHpLoss( 2, player.x + 1, player.y - 1, preAtkTime, player.npc ) );

		// Start Sword Attack Animation
        player.animationType = 1;
        player.animationDisplayAmt = lifeAtkTime;
        player.currentSwordAtkTime = player.animationDisplayAmt;
        player.selSwordAnimation = 22;
		
        if ( !player.npc ) {
            Mix_PlayChannel( 0, spinSlashSound, 0 );
            npcAbleToAct = true;
            chargeDisplayMinusAmt = iconDisplayTime;
            chargeDisplayPlusAmt = 0;
            currentEnergyGain -= spinSlashCost;
        }
        else { Mix_PlayChannel( 6, spinSlashSound, 0 ); }
        player.x2 = player.x;
        player.y2 = player.y;
        return true;
	}
    return false;
}

void App::hitTile( int xPos, int yPos, int dmg ) {
    if ( xPos >= 0 && xPos <= 5 && yPos >= 0 && yPos <= 5 ) {

        // Attacking a Box
        if ( map[xPos][yPos].boxHP > 0 ) {
            if ( map[xPos][yPos].boxHP == 1 ) { map[xPos][yPos].prevDmg = 1; }
            else { map[xPos][yPos].prevDmg = dmg; }
            map[xPos][yPos].boxHP -= dmg;
            map[xPos][yPos].boxAtkInd = boxDmgTime;
            if ( map[xPos][yPos].boxHP <= 0 && !map[xPos][yPos].bigBoxDeath ) { Mix_PlayChannel( 4, rockBreakSound, 0 ); }
        }
        // NPC attacks on the Player
        else if ( xPos == player1.x && yPos == player1.y ) {
            player1.energy -= dmg * 100;
            currentEnergyGain -= dmg * 100;
            chargeDisplayPlusAmt = 0;
            chargeDisplayMinusAmt = iconDisplayTime;
            player1.hurtDisplayAmt = hurtAnimationTime;
            if ( player1.energy <= 0 ) {
                player1.hp = 0;
            }
            else if ( player1.animationType == 0 ) {
                player1.animationType = -1;
                player1.animationDisplayAmt += 0.1125;
            }
            else if ( player1.animationType == -2 ) {
                player1.animationType = -1;
                player1.animationDisplayAmt = pauseAnimationTime;
            }
            Mix_PlayChannel( 1, hurtSound, 0 );
        }
        // Player attacks on NPCs
        else {
            for ( int i = 0; i < npcList.size(); i++ ) {
                if ( xPos == npcList[i].x && yPos == npcList[i].y ) {
                    npcList[i].hp -= dmg;
                    npcList[i].timesHit++;
                    npcList[i].hurtDisplayAmt = hurtAnimationTime;
                    spawnRandomItem( npcList[i].x, npcList[i].y );
                    if ( npcList[i].animationType == 0 ) {
                        npcList[i].animationType = -1;
                        npcList[i].animationDisplayAmt += 0.1125;
                    }
                    else if ( npcList[i].animationType == -2 ) {
                        npcList[i].animationType = -1;
                        npcList[i].animationDisplayAmt = pauseAnimationTime;
                    }
                    Mix_PlayChannel( 1, hurtSound, 0 );
                }
            }
        }
    }
}

// Map Functions
bool App::isTileValid(int xPos, int yPos, bool npc) {	// Checks if a specific tile allows the Player to walk on it
    if ( xPos == -1 && yPos == 0 && !npc ) return true;			// Start Tile
    if ( xPos ==  6 && yPos == 5 )         return true;			// Goal Tile
    if ( xPos < 0 || xPos > 5 || yPos < 0 || yPos > 5 ) return false;		// Map boundaries are 0 to 5 on both X and Y axes
    if ( map[xPos][yPos].boxHP >  0 ) return false;		// Players cannot walk into a Rock
    if ( map[xPos][yPos].state >= 2 ) return false;		// Players cannot walk onto a Hole
    if ( xPos == player1.x && yPos == player1.y ) return false;                 // NPCs can't walk onto the Player's position
    for ( int i = 0; i < npcList.size(); i++ ) {
        if ( xPos == npcList[i].x && yPos == npcList[i].y ) return false; }     // Players can't walk onto NPC positions
	return true;
}
void App::clearFloor() {		// Clears all tiles
    for ( int i = 0; i < 6; i++ ) {
        for ( int j = 0; j < 6; j++ ) {
            map[i][j].bigBoxDeath = false;
            map[i][j].boxAtkInd = 0;
            map[i][j].boxHP = 0;
            map[i][j].boxType = 0;
            map[i][j].isPurple = false;
            map[i][j].item = 0;
            map[i][j].prevDmg = 0;
            map[i][j].upgradeInd = 0;
            map[i][j].state = 0;
        }
    }
}
void Player::reset() {
    x = 0;		y = 0;      x2 = 0;     y2 = 0;
    facing = 3;
    moveDir = -1;
	moving = false;
	turning = false;
	
    energy = 0;
    hp = 1;

    animationType = -2;
    animationDisplayAmt = 0;
    hurtDisplayAmt = 0;
    deathDisplayAmt = charDeathTime;
    selSwordAnimation = -1;
    currentSwordAtkTime = 0;
}
void App::reset() {
    srand( time( 0 ) );
	clearFloor();
	player1.reset();
	player1.energy = player1.energyDisplayed = startingEnergy;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	
	delayedHpList.clear();
    lvlsWithoutBoss = 0;
	level = 1;
	loadLevel(level);
	currentEnergyGain = 0;
    
    quitMenuOn = resetMenuOn = diffSelMenuOn = trainMenuOn = false;
    menuSel = true;
}
void App::test() {
	reset();
	clearFloor();
	level = -1;
	player1.energy = player1.energyDisplayed = 10000;
	
    for ( int i = 1; i < 5; i++ ) {
        map[0][i].boxHP = 5;
        map[1][i].boxHP = 5;
        map[2][i].boxHP = 5;    map[2][i].state = 1;
        map[4][i].boxHP = 3;    map[4][i].item = 1;     map[4][i].boxType = 2;
        map[5][i].boxHP = 3;    map[5][i].item = 2;     map[5][i].boxType = 2;
    }
    map[4][0].item = -1;    map[5][0].item = -1;
    map[2][4].state = 0;
}
void App::next() {
    clearFloor();
    player1.x = 0;		player1.y = 0;
    player1.x2 = 0;     player1.y2 = 0;

    for ( int i = 0; i < npcList.size(); i++ ) {
        npcList[i].animationDisplayAmt = 0; }

    if ( level >= 1 ) {
	    level++;
	    loadLevel(level);
    }
    else if ( level <= -1 ) {
        level--;
        if ( level <= -12 ) {
            level = -1;
            npcList.clear();
            test();
            return;
        }
        player1.energy = 10000;
        generateBossLevel( -2 - level );
    }

	player1.facing = 3;
	currentEnergyGain = 0;
    player1.selSwordAnimation = -1;
    quitMenuOn = resetMenuOn = trainMenuOn = false;
    menuSel = true;
}
void App::loadLevel(int num) {
    if ( level == 1 ) { generateLevel( 0, 0 ); }
	else { generateLevel( getRand( 46 ) ); }
}

void App::generateBossLevel( int type ) {
    Mix_PlayChannel( 6, bossAppearSound, 0 );
    Player boss;
    boss.x = 5;     boss.y = 5;     boss.x2 = 5;    boss.y2 = 5;
    boss.facing = 1;
    boss.npc = true;
    boss.hp = 5;
    boss.type = rand() % 5;
    boss.energy = 2000;
    npcList.push_back( boss );

    if ( type == 0 ) {
        for ( int i = 0; i < 3; i++ ) { map[i][5].state = 3;    map[5][i].state = 3; }
        for ( int i = 0; i < 2; i++ ) { map[i][4].state = 3;    map[4][i].state = 3; }
        map[0][3].state = 3;    map[3][0].state = 3;
    }
    else if ( type == 1 ) {
        for ( int i = 0; i < 5; i++ ) { map[i][5].state = 3;    map[i + 1][0].state = 3; }
    }
    else if ( type == 2 ) {
        map[1][1].state = 3;    map[1][4].state = 3;    map[4][1].state = 3;    map[4][4].state = 3;
    }
    else if ( type == 3 ) {
        for ( int i = 0; i < 2; i++ ) { map[1][i + 3].state = 3;    map[4][i + 1].state = 3; }
    }
    else if ( type == 4 ) {
        for ( int i = 0; i < 2; i++ ) { map[1][i + 1].state = 3;    map[4][i + 3].state = 3; }
    }
    else if ( type == 5 ) {
        for ( int i = 0; i < 2; i++ ) { map[i + 1][4].state = 3;    map[i + 3][1].state = 3; }
    }
    else if ( type == 6 ) {
        for ( int i = 0; i < 2; i++ ) { map[i + 1][1].state = 3;    map[i + 3][4].state = 3; }
    }
    else if ( type == 7 ) {
        for ( int i = 0; i < 2; i++ ) { map[2][i + 2].state = 3;    map[3][i + 2].state = 3; }
    }
    else if ( type == 8 ) {
        for ( int i = 0; i < 2; i++ ) { map[1][i + 2].state = 3;    map[4][i + 2].state = 3; }
    }
    else if ( type == 9 ) {
        for ( int i = 0; i < 2; i++ ) { map[i + 2][1].state = 3;    map[i + 2][4].state = 3; }
    }
    else { generateBossLevel( 0 ); return; }
}
void App::generateLevel(int type, int num) {        // (levelType, difficulty)          // difficulty -1 = randomly generated difficulty
    
    if ( num <= -1 ) { num = rand() % 100; }				// Random number between 0 and 99, inclusive - used to determine difficulty
    int x = level;		if ( x > 50 ) { x = 50; }
    int bound2 = 120 - x * 3 / 2;						// Bounds used in determining difficulty - based on current level number
    if ( level >= 75 ) { bound2 = 40; }
    if ( level >= 100 ) { bound2 = 35; }
    int bound1 = bound2 - 30;
    bool boss = false;
    int diff = 0;				// difficulty 0, 1, 2 = easy, medium, hard			// level		easy   medium	hard
    if ( num > bound1 && num <= bound2 ) { diff = 1; }								//   0			90%		10%		0%
    else if ( num > bound2 ) { diff = 2; }											//  10			75%		25%		0%
    lvlDiff = diff;																    //  20			60%		30%		10%
                                                                                    //  30			45%		30%		25%
                                                                                    //  40			30%		30%		40%
                                                                                    //  50			15%		30%		55%
                                                                                    //  75			10%		30%		60%
                                                                                    // 100			 5%		30%		65%
    int gain = 0;       // Gain determines avg "profit" per level
    if ( currentGameDiff == 0 ) { gain = 2 - diff * 9; }      //  2  -07  -16
    else if ( currentGameDiff == 1 ) { gain = 0 - diff * 9; }      //  0  -09  -18
    else if ( currentGameDiff == 2 ) { gain = -2 - diff * 9; }      // -2  -11  -20
    else if ( currentGameDiff == 3 ) { gain = -4 - diff * 9; }      // -4  -13  -22
    else if ( currentGameDiff == 4 ) { gain = -6 - diff * 9; }      // -6  -15  -24
    int extraBoxes = 0, extraItems = 0, floorDmgs = 0, trappedItems = 0;		// Number of things per floor based on level number and difficulty
    
	// Map types
    {
	// Rooms
	if (type == 0) {			// Rooms A
		map[2][2].boxHP = 1;	map[2][5].boxHP = 1;	map[4][2].boxHP = 1;							//	] =O=O=[
		map[2][3].boxHP = 1;	map[4][0].boxHP = 1;	map[4][3].boxHP = 1;							//	] =O=  [
		map[2][4].boxHP = 1;	map[4][1].boxHP = 1;	map[4][5].boxHP = 1;							//	] =O=O=[
		map[0][1].state = 3;	map[1][1].state = 3;	map[2][1].state = 3;							//	] =O=O=[
		map[4][4].state = 3;	map[5][4].state = 3;													//	]   =O=[
		map[0][2].state = 3;	map[0][3].state = 3;	map[0][4].state = 3;	map[0][5].state = 3;	//	]====O=[
		extraBoxes = 3 + x / 5 + diff * 2;			// Number of Rocks based on Level number
		extraItems = 9 + extraBoxes + gain;			// Number of Items based on "gain"
		floorDmgs = rand() % 2 + 1 + diff * 2;		// 1-2	// 3-4	// 5-6
													// Number of Damaged Tiles based on difficulty
	}
	else if (type == 1) {			// Rooms B
		for ( int i = 0; i < 3; i++ ) {										//	]OO==O=[
			map[0][i + 3].boxHP = 1;	map[1][i + 3].boxHP = 1;			//	]OO==  [
			map[4][i].boxHP = 1;		map[5][i].boxHP = 1; }				//	]OO==  [
		for ( int i = 0; i < 2; i++ ) {										//	]  ==OO[
			map[0][i + 1].state = 3;	map[1][i + 1].state = 3;			//	]  ==OO[
			map[4][i + 3].state = 3;	map[5][i + 3].state = 3; }			//	]====OO[
		map[4][5].boxHP = 1;
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 13 + extraBoxes + gain;
		floorDmgs = rand() % 2 + 1 + diff * 2;		// 1-2	// 3-4	// 5-6
	}
	else if (type == 2) {		// Rooms C
		map[0][2].state = 3;	map[2][0].state = 3;
		map[3][5].state = 3;	map[5][3].state = 3;							//	]=== ==[
		map[2][2].boxHP = 1;	map[3][3].boxHP = 1;							//	]=OO===[
		for (int i = 0; i < 2; i++) {											//	]=OOO= [
			map[1][i + 3].boxHP = 1;		map[2][i + 3].boxHP = 1;			//	] =OOO=[
			map[3][i + 1].boxHP = 1;		map[4][i + 1].boxHP = 1; }			//	]===OO=[
                                                                                //	]== ===[
		extraBoxes = 3 + x / 5 + diff * 2;
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 2 + 1 + diff * 2;		// 1-2	// 3-4	// 5-6
	}
	else if (type == 3) {		// Rooms D
		for (int i = 0; i < 2; i++) {																	//	]=    =[
			map[0][2 + i].boxHP = 1;		map[2][2 + i].boxHP = 1;									//	]=O==O=[
			map[3][2 + i].boxHP = 1;		map[5][2 + i].boxHP = 1; }									//	]O=OO=O[
		for (int i = 0; i < 4; i++) {																	//	]O=OO=O[
			map[i + 1][0].state = 3;		map[i + 1][5].state = 3; }									//	]=O==O=[
		map[1][1].boxHP = 1;	map[1][4].boxHP = 1;	map[4][1].boxHP = 1;	map[4][4].boxHP = 1;	//	]=    =[
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 3 + diff * 3;			// 0-2	// 3-5	// 6-8
	}
	else if (type == 4) {		// Rooms E
		map[0][3].boxHP = 1;	map[1][2].boxHP = 1;	map[1][3].boxHP = 1;	map[2][1].boxHP = 1;	//	]==OO==[
		map[2][2].boxHP = 1;	map[2][4].boxHP = 1;	map[2][5].boxHP = 1;	map[3][0].boxHP = 1;	//	]==O=O=[
		map[3][1].boxHP = 1;	map[3][5].boxHP = 1;	map[4][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]OO===O[
		map[5][2].boxHP = 1;	map[5][3].boxHP = 1;													//	]=OO=OO[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]==OO==[
		extraItems = 14 + extraBoxes + gain;															//	]===O==[
		floorDmgs = rand() % 4 + 2 + diff * 3;		// 2-5	// 5-8	// 8-11
	}
    else if (type == 5) {		// Rooms F
		map[0][3].boxHP = 1;	map[2][1].boxHP = 1;	map[3][5].boxHP = 1;	//	] ==O==[
		map[1][2].boxHP = 1;	map[2][4].boxHP = 1;	map[4][2].boxHP = 1;	//	]==OO==[
		map[1][3].boxHP = 1;	map[3][1].boxHP = 1;	map[4][3].boxHP = 1;	//	]OO==O=[
		map[2][0].boxHP = 1;	map[3][4].boxHP = 1;	map[5][2].boxHP = 1;	//	]=O==OO[
        map[0][5].state = 3;    map[5][0].state = 3;                            //	]==OO==[
                                                                                //	]==O== [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + 2 + diff * 4;		// 2-6	// 6-10	// 10-14
	}
	// Plus
	else if (type == 6) {		// Plus A
		for (int i = 0; i < 3; i++) {											// ] ==O==[
			map[0][i + 3].state = 3;		map[i + 3][0].state = 3;			// ] =OOO=[
			map[i + 2][2].boxHP = 1;		map[2][i + 2].boxHP = 1;			// ]=OO=OO[
			map[i + 2][4].boxHP = 1;		map[4][i + 2].boxHP = 1; }			// ]==OOO=[
		map[1][3].boxHP = 1;		map[3][5].boxHP = 1;						// ]===O==[
		map[3][1].boxHP = 1;		map[5][3].boxHP = 1;						// ]====  [
		map[0][3].state = 0;		map[3][0].state = 0;
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 3;			// 0-3	// 3-6	// 6-9
	}
	else if (type == 7) {		// Plus B
		for (int i = 0; i < 3; i++) {											// ] ===O=[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] =OOOO[
			map[i + 2][2].boxHP = 1;	map[2][i + 2].boxHP = 1;				// ] =O=O=[
			map[i + 2][4].boxHP = 1;	map[4][i + 2].boxHP = 1; }				// ]=OOOO=[
		map[1][2].boxHP = 1;	map[4][5].boxHP = 1;							// ]==O===[
		map[2][1].boxHP = 1;	map[5][4].boxHP = 1;							// ]===   [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 1 + diff * 3;		// 1-4	// 4-7	// 7-10
	}
	else if (type == 8) {		// Plus C
        map[4][4].boxHP = 1;
		for (int i = 0; i < 3; i++) {											// ] =O===[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] OOOO=[
			map[i + 3][2].boxHP = 1;	map[2][i + 3].boxHP = 1;				// ] =O=O=[
			map[i + 1][4].boxHP = 1;	map[4][i + 1].boxHP = 1; }				// ]===OOO[
		extraBoxes = 4 + x / 5 + diff * 2;										// ]====O=[
		extraItems = 11 + extraBoxes + gain;									// ]===   [
		floorDmgs = rand() % 3 + 1 + diff * 2;		// 1-3	// 3-5	// 5-7
	}
    else if (type == 9) {		// Plus D
		map[4][4].state = 3;
		for (int i = 0; i < 3; i++) {											// ] O=O==[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] O=O =[
			map[i + 3][1].boxHP = 1;	map[1][i + 3].boxHP = 1;				// ] O=OOO[
			map[i + 3][3].boxHP = 1;	map[3][i + 3].boxHP = 1; }				// ]======[
		extraBoxes = 4 + x / 5 + diff * 2;										// ]===OOO[
		extraItems = 11 + extraBoxes + gain;									// ]===   [
		floorDmgs = rand() % 3 + 1 + diff * 2;		// 1-3	// 3-5	// 5-7
	}
	else if (type == 10) {		// Plus E
		for (int i = 0; i < 3; i++) {											// ] OOO==[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] O=O==[
			map[i + 3][1].boxHP = 1;	map[1][i + 3].boxHP = 1;				// ] O=OOO[
			map[i + 3][3].boxHP = 1;	map[3][i + 3].boxHP = 1; }				// ]==O==O[
		map[2][2].boxHP = 1;	map[2][5].boxHP = 1;	map[5][2].boxHP = 1;	// ]===OOO[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]===   [
		extraItems = 14 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 2;			// 0-3	// 2-5	// 4-7
	}
	else if (type == 11) {		// Plus F
		for ( int i = 0; i < 2; i++ ) {											// ]O== ==[
			map[0][i + 4].boxHP = 1;	map[i + 1][3].boxHP = 1;				// ]O==O==[
			map[3][i + 1].boxHP = 1;	map[i + 4][0].boxHP = 1; }				// ]=OO=O [
		map[3][4].boxHP = 1;	map[3][5].state = 3;							// ]===O==[
		map[4][3].boxHP = 1;	map[5][3].state = 3;							// ]===O==[
		extraBoxes = 3 + x / 5 + diff * 2;										// ]====OO[
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 5 + 2 + diff * 2;		// 2-6	// 4-8	// 6-10
	}
    else if (type == 12) {		// Plus G
		for ( int i = 0; i < 2; i++ ) {
            map[i][5].state = 3;        map[5][i].state = 3;                    // ]  O=O=[
			map[2][i + 1].boxHP = 1;	map[i + 1][2].boxHP = 1;				// ]=OO=OO[
            map[2][i + 4].boxHP = 1;    map[i + 4][2].boxHP = 1;                // ]===O==[
            map[4][i + 1].boxHP = 1;    map[i + 1][4].boxHP = 1;                // ]=OO=OO[
			map[4][i + 4].boxHP = 1;	map[i + 4][4].boxHP = 1; }				// ]= O=O [
		map[3][3].boxHP = 1;	map[1][1].state = 3;							// ]===== [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 13 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;		    // 0-4	// 3-7	// 6-10
	}
	// Paths
	else if (type == 13) {		// Paths A
        for ( int i = 0; i < 2; i++ ) {                             //	]=OO===[
            map[i + 1][2].state = 3;    map[i + 3][3].state = 3; }  //	]=OO===[
        for ( int i = 0; i < 3; i++ ) {                             //	]=OO  X[
            map[1][i + 3].boxHP = 1;    map[3][i].boxHP = 1;        //	]=  OO=[
            map[2][i + 3].boxHP = 1;    map[4][i].boxHP = 1; }      //	]===OO=[
		map[5][3].state = 1;										//	]===OO=[
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 1 + diff;			// 1-3	// 2-4	// 3-5
	}
	else if (type == 14) {		// Paths B
		map[5][3].state = 1;
		map[2][2].state = 3;	map[3][2].state = 3;	map[3][3].state = 3;	map[4][3].state = 3;	//	]==OOO=[
		map[2][3].boxHP = 1;	map[2][5].boxHP = 1;	map[3][4].boxHP = 1;	map[4][4].boxHP = 1;	//	]==OOO=[
		map[2][4].boxHP = 1;							map[3][5].boxHP = 1;	map[4][5].boxHP = 1;	//	]==O  X[
		map[2][0].boxHP = 1;	map[3][0].boxHP = 1;	map[4][0].boxHP = 1;	map[4][2].boxHP = 1;	//	]==  O=[
		map[2][1].boxHP = 1;	map[3][1].boxHP = 1;	map[4][1].boxHP = 1;							//	]==OOO=[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]==OOO=[
		extraItems = 14 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 1 + diff;		// 1-3	// 2-4	// 3-5
	}
	else if (type == 15) {		// Paths C
        for ( int i = 0; i < 2; i++ ) {                                                     //	] =OO==[
            map[i][1].state = 3;    map[i + 2][4].boxHP = 1;    map[i + 4][1].state = 3;    //	]==OO==[
            map[i][2].boxHP = 1;    map[i + 2][5].boxHP = 1;    map[i + 4][2].boxHP = 1;    //	]OO==OO[
            map[i][3].boxHP = 1;    map[i + 4][0].state = 3;    map[i + 4][3].boxHP = 1; }  //	]OO==OO[
        map[0][5].state = 3;                                                                //	]  ==  [
		extraBoxes = 4 + x / 5 + diff * 2;                                                  //	]====  [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;		// 0-4	// 3-7	// 6-10
	}
	else if (type == 16) {		// Paths D
		for ( int i = 0; i < 2; i++ ) {														//	]    ==[
			map[i][3].boxHP = 1;	map[i + 2][1].boxHP = 1;	map[5][i].state = 3;		//	]OO==OO[
			map[i][4].boxHP = 1;	map[i + 2][2].boxHP = 1;	map[i + 4][3].boxHP = 1;	//	]OO==OO[
			map[i][5].state = 3;	map[i + 2][5].state = 3;	map[i + 4][4].boxHP = 1; }	//	]==OO==[
		map[0][1].state = 3;																//	] =OO= [
		extraBoxes = 4 + x / 5 + diff * 2;													//	]===== [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 17) {		// Paths E
		for ( int i = 0; i < 2; i++ ) {														//	]  =OO=[
			map[1][i + 2].boxHP = 1;	map[3][i].boxHP = 1;	map[3][i + 4].boxHP = 1;	//	]===OO=[
			map[2][i + 2].boxHP = 1;	map[4][i].boxHP = 1;	map[4][i + 4].boxHP = 1;	//	]=OO= =[
			map[1][i].state = 3;	    map[4][i + 2].state = 3;    map[i][5].state = 3; }  //	]=OO= =[
		                    																//	]= =OO=[
															                                //	]= =OO=[
        extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2 + 1;      // 1-5	// 3-7	// 5-9
	}
    else if (type == 18) {		// Paths F
		for ( int i = 0; i < 2; i++ ) {														//	]=OO= =[
			map[1][i].boxHP = 1;	map[1][i + 4].boxHP = 1;	map[3][i + 2].boxHP = 1;	//	]=OO= =[
			map[2][i].boxHP = 1;	map[2][i + 4].boxHP = 1;	map[4][i + 2].boxHP = 1;	//	]= =OO=[
			map[1][i + 2].state = 3;	map[4][i].state = 3;    map[4][i + 4].state = 3; }  //	]= =OO=[
		                    																//	]=OO= =[
															                                //	]=OO= =[
        extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 19) {		// Paths G
		for ( int i = 0; i < 2; i++ ) {									//	] =OO==[
			map[0][i + 2].boxHP = 1;	map[2][i + 0].boxHP = 1;	    //	]==OO==[
			map[1][i + 2].boxHP = 1;	map[3][i + 0].boxHP = 1;	    //	]OO==OO[
			map[4][i + 2].boxHP = 1;    map[2][i + 4].boxHP = 1;        //	]OO==OO[
            map[5][i + 2].boxHP = 1;    map[3][i + 4].boxHP = 1; }      //	]==OO==[
        map[0][5].state = 3;    map[5][0].state = 3;                    //	]==OO= [
        extraBoxes = 5 + x / 5 + diff * 2;
		extraItems = 16 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;	    // 0-8	// 4-12	// 8-16
	}
	// Cross
	else if (type == 20) {		// Cross A
        for ( int i = 0; i < 3; i++ ) {                                         //	]=== ==[
            map[i][i + 2].boxHP = 1;        map[i][4 - i].boxHP = 1;            //	]O=OO=O[
            map[i + 3][i + 2].boxHP = 1;    map[i + 3][4 - i].boxHP = 1; }      //	]=O==O=[
        map[3][0].state = 3;    map[3][1].state = 3;    map[3][5].state = 3;    //	]O=OO=O[
                                                                                //	]=== ==[
                                                                                //	]=== ==[
		extraBoxes = 3 + x / 5 + diff * 2;										
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8
	}
	else if (type == 21) {		// Cross B
        for ( int i = 0; i < 3; i++ ) {                                         //	]==O=O=[
            map[i + 2][i].boxHP = 1;        map[i + 2][i + 3].boxHP = 1;        //	]===O==[
            map[i + 2][2 - i].boxHP = 1;    map[i + 2][5 - i].boxHP = 1; }      //	]  O=O [
        map[0][3].state = 3;    map[1][3].state = 3;    map[5][3].state = 3;    //	]==O=O=[
                                                                                //	]===O==[
                                                                                //	]==O=O=[
		extraBoxes = 3 + x / 5 + diff * 2;										
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8
	}
	else if (type == 22) {		// Cross C
        for ( int i = 0; i < 2; i++ ) {
            map[1][i].boxHP = 1;    map[1][i + 4].boxHP = 1;    map[2][i + 2].boxHP = 1;        //	]=O=O==[
            map[3][i].boxHP = 1;    map[3][i + 4].boxHP = 1;                                    //	]=O=O==[
            map[0][i + 2].state = 3;    map[4][i + 2].state = 3;    map[5][i + 2].state = 3; }  //	] =O=  [
                                                                                                //	] =O=  [
                                                                                                //	]=O=O==[
                                                                                                //	]=O=O==[
		extraBoxes = 3 + x / 5 + diff * 2;
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;	// 0-4	// 2-6	// 4-8
	}
	else if (type == 23) {		// Cross D
        for ( int i = 0; i < 2; i++ ) {                                                         // ]== ===[
            map[i][1].boxHP = 1;    map[2][i + 2].boxHP = 1;    map[i + 3][1].boxHP = 1;        // ]OO=OO [
            map[i][4].boxHP = 1;                                map[i + 3][4].boxHP = 1; }      // ]==O== [
        for ( int i = 0; i < 5; i++ ) { map[5][i].state = 3; }                                  // ]==O== [
        map[2][5].state = 3;                                                                    // ]OO=OO [
		extraBoxes = 3 + x / 5 + diff * 2;                                                      // ]===== [
		extraItems = 10 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 24) {		// Cross E
        for ( int i = 0; i < 2; i++ ) {                                                         // ]  == =[
            map[1][i].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[1][i + 3].boxHP = 1;        // ]=O==O=[
            map[4][i].boxHP = 1;                                map[4][i + 3].boxHP = 1; }      // ]=O==O=[
        map[0][5].state = 3;    map[1][5].state = 3;    map[4][5].state = 3;                    // ]==OO= [
        map[5][2].state = 3;                                                                    // ]=O==O=[
		extraBoxes = 3 + x / 5 + diff * 2;                                                      // ]=O==O=[
		extraItems = 10 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 25) {		// Cross F
        for ( int i = 0; i < 3; i++ ) {                                         // ] ==O=O[
                                                                                // ] = =O=[
            map[i + 1][i + 1].boxHP = 1;    map[3 - i][i + 1].boxHP = 1;        // ]=O=O=O[
            map[i + 3][i + 3].boxHP = 1;    map[5 - i][i + 3].boxHP = 1; }      // ]==O= =[
        map[0][4].state = 3;    map[0][5].state = 3;    map[2][4].state = 3;    // ]=O=O==[
        map[4][0].state = 3;    map[4][2].state = 3;    map[5][0].state = 3;    // ]====  [
		extraBoxes = 3 + x / 5 + diff * 2;
		extraItems = 9 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 26) {		// Cross G
        for ( int i = 0; i < 3; i++ ) {                                         // ] O=O==[
            map[i + 1][i + 3].boxHP = 1;    map[5 - i][i + 1].boxHP = 1;        // ]==O= =[
            map[i + 3][i + 1].boxHP = 1;    map[3 - i][i + 3].boxHP = 1; }      // ]=O=O=O[
        map[1][1].state = 3;    map[2][1].state = 3;                            // ]=  =O=[
        map[1][2].state = 3;    map[2][2].state = 3;                            // ]=  O=O[
        map[0][5].state = 3;    map[4][4].state = 3;    map[5][0].state = 3;    // ]===== [
		extraBoxes = 3 + x / 5 + diff * 2;
		extraItems = 9 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 2;	    // 0-4	// 2-6	// 4-8
	}
	// Gallery
	else if (type == 27) {		// Gallery A
		for (int i = 1; i < 5; i++) {											//	]======[
			map[i][1].boxHP = 1;	map[i][2].boxHP = 1;						//	]=OOOO=[
			map[i][3].boxHP = 1;	map[i][4].boxHP = 1; }						//	]=OOOO=[
		extraBoxes = 5 + x / 5 + diff * 2;										//	]=OOOO=[
		extraItems = 16 + extraBoxes + gain;									//	]=OOOO=[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]======[
	}
	else if (type == 28) {		// Gallery B									// ]======[
		for (int i = 1; i < 5; i++) {											// ]OO==OO[
			map[0][i].boxHP = 1;	map[1][i].boxHP = 1;						// ]OO==OO[
			map[4][i].boxHP = 1;	map[5][i].boxHP = 1; }						// ]OO==OO[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]OO==OO[
		extraItems = 16 + extraBoxes + gain;									// ]======[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 29) {		// Gallery C
		for (int i = 1; i < 5; i++) {											// ]=OOOO=[
			map[i][0].boxHP = 1;	map[i][3].boxHP = 1;						// ]======[
			map[i][2].boxHP = 1;	map[i][5].boxHP = 1; }						// ]=OOOO=[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]=OOOO=[
		extraItems = 16 + extraBoxes + gain;									// ]======[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			// ]=OOOO=[
	}
	else if (type == 30) {		// Gallery D
        for (int i = 1; i < 5; i++) {                                           // ]======[
			map[0][i].boxHP = 1;	map[3][i].boxHP = 1;                        // ]O=OO=O[
			map[2][i].boxHP = 1;	map[5][i].boxHP = 1; }                      // ]O=OO=O[
		extraBoxes = 5 + x / 5 + diff * 2;                                      // ]O=OO=O[
		extraItems = 16 + extraBoxes + gain;                                    // ]O=OO=O[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8          // ]======[
	}
    else if (type == 31) {		// Gallery E
        for (int i = 0; i < 3; i++) {                                           // ]=OOO==[
			map[0][i + 2].boxHP = 1;	map[i + 1][5].boxHP = 1;                // ]O=====[
			map[i + 2][0].boxHP = 1;	map[5][i + 1].boxHP = 1; }              // ]O=OO=O[
        map[2][2].boxHP = 1;    map[3][2].boxHP = 1;                            // ]O=OO=O[
        map[2][3].boxHP = 1;    map[3][3].boxHP = 1;                            // ]=====O[
                                                                                // ]==OOO=[
		extraBoxes = 5 + x / 5 + diff * 2;
		extraItems = 16 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8          
	}
    else if (type == 32) {		// Gallery F
        for ( int i = 0; i < 2; i++ ) {                                     // ]OO====[
            map[0][i + 4].boxHP = 1;    map[1][i + 4].boxHP = 1;            // ]OO=OO=[
            map[1][i + 1].boxHP = 1;    map[2][i + 1].boxHP = 1;            // ]===OO=[
            map[3][i + 3].boxHP = 1;    map[4][i + 3].boxHP = 1;            // ]=OO===[
            map[4][i].boxHP = 1;        map[5][i].boxHP = 1; }              // ]=OO=OO[
                                                                            // ]====OO[
		extraBoxes = 5 + x / 5 + diff * 2;
		extraItems = 16 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8          
	}
	// Scattered
	else if (type == 33) {		// Scattered A
		for (int i = 0; i < 6; i += 2) {										//	]==O=O=[
			map[2][i + 1].boxHP = 1;											//	]===O=O[
			map[3][i].boxHP = 1;												//	]==O=O=[
			map[4][i + 1].boxHP = 1;											//	]===O=O[
			map[5][i].boxHP = 1; }												//	]==O=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]===O=O[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;		// 2-6	// 6-10	// 8-12
	}
	else if (type == 34) {		// Scattered B
		for (int i = 0; i < 4; i++) {
			map[    i % 2][2 + i].boxHP = 1;									//	]=O=O=O[
			map[2 + i % 2][2 + i].boxHP = 1;									//	]O=O=O=[
			map[4 + i % 2][2 + i].boxHP = 1; }									//	]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]O=O=O=[
		extraItems = 12 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12			//	]======[
	}
	else if (type == 35) {		// Scattered C
        for ( int i = 0; i < 3; i++ ) {                                         // ]   =O=[
            map[i][5].state = 3;    map[5][i].state = 3; }                      // ]=O=O=O[
        for ( int i = 0; i < 4; i++ ) {                                         // ]O=O=O=[
            map[i    ][3 - i].boxHP = 1;    map[i + 1][4 - i].boxHP = 1;        // ]=O=O= [
            map[i + 1][i + 2].boxHP = 1;    map[i + 2][i + 1].boxHP = 1; }      // ]==O=O [		
		extraBoxes = 4 + x / 5 + diff * 2;										// ]===O= [			
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
	else if (type == 36) {		// Scattered D
        for ( int i = 1; i <= 5; i += 2 ) {
            map[i][2].boxHP = 1;    map[i][4].boxHP = 1;                    // ] =O=O=[
            map[2][i].boxHP = 1;    map[4][i].boxHP = 1; }                  // ] O=O=O[
        map[0][4].state = 3;    map[0][5].state = 3;                        // ]==O=O=[
        map[4][0].state = 3;    map[5][0].state = 3;                        // ]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;                                  // ]==O=O=[
		extraItems = 12 + extraBoxes + gain;                                // ]====  [
        floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
    else if (type == 37) {		// Scattered E
        for ( int i = 0; i < 5; i++ ) { map[i + 1][5 - i].boxHP = 1; }  // ]=O=O==[
        for ( int i = 0; i < 3; i++ ) { map[i + 3][5 - i].boxHP = 1; }  // ]O=O=O=[
        for ( int i = 0; i < 2; i++ ) {                                 // ]=O=O=O[
            map[i][4 - i].boxHP = 1;    map[i + 3][1 - i].boxHP = 1; }  // ] ===O=[
        map[0][2].state = 3;    map[2][0].state = 3;                    // ]===O=O[
                                                                        // ]== =O=[
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
        floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12
	}
    else if (type == 38) {		// Scattered F
        for ( int i = 0; i < 6; i++ ) { map[i + 0][5 - i].boxHP = 1; }  // ]O=O ==[
        for ( int i = 0; i < 4; i++ ) { map[i + 2][5 - i].boxHP = 1; }  // ]=O=O==[
        map[0][3].boxHP = 1;    map[3][0].boxHP = 1;                    // ]O=O=O [
        map[0][2].state = 3;    map[2][0].state = 3;                    // ] ==O=O[
        map[3][5].state = 3;    map[5][3].state = 3;                    // ]====O=[
                                                                        // ]== O=O[
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
        floorDmgs = rand() % 5 + 2 + diff * 2;	// 2-6	// 4-8	// 6-10
	}
	// Layers
	else if (type == 39) {		// Layers A
        map[1][0].state = 3;    map[2][0].state = 3;    map[4][0].state = 3;
        map[1][5].state = 3;    map[3][0].state = 3;    map[4][1].state = 3;    //	]= OO==[
		for (int yPos = 1; yPos < 6; yPos++) {									//	]==OO==[
			map[2][yPos].boxHP = 1;		map[3][yPos].boxHP = 1; }				//	]==OO==[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==OO==[
		extraItems = 10 + extraBoxes + gain;									//	]==OO =[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]=    =[
	}
	else if (type == 40) {		// Layers B
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=OO=O=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=OO=O=[
			map[2][i + 1].boxHP = 1;											//	]=OO=O=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=OO=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 41) {		// Layers C
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=O=OO=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=O=OO=[
			map[3][i + 1].boxHP = 1;											//	]=O=OO=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=O=OO=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 42) {		// Layers D
        for ( int i = 0; i < 2; i++ ) {                                                         //	]=OO===[
            map[i + 1][5].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 4][2].boxHP = 1;    //	]=OO===[
            map[i + 1][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 4][1].boxHP = 1; }  //	]==OO==[
        map[0][2].state = 3;    map[2][0].state = 3;                                            //	] =OOOO[
		extraBoxes = 4 + x / 5 + diff * 2;                                                      //	]====OO[
		extraItems = 12 + extraBoxes + gain;                                                    //	]== ===[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 43) {		// Layers E
        for ( int i = 0; i < 2; i++ ) {                                                         // ]=== ==[
            map[i][3].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 3][1].boxHP = 1;        // ]OO====[
            map[i][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 3][0].boxHP = 1; }      // ]OOOO= [
        map[3][5].state = 3;    map[5][3].state = 3;                                            // ]==OO==[
		extraBoxes = 4 + x / 5 + diff * 2;										                // ]===OO=[
		extraItems = 12 + extraBoxes + gain;                                                    // ]===OO=[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
    else if (type == 44) {		// Layers F
        for ( int i = 0; i < 4; i++ ) {                             // ]    ==[
            map[2][i + 1].boxHP = 1;    map[i + 1][0].state = 3;    // ]==OOO=[
            map[3][i + 1].boxHP = 1;    map[i][5].state = 3;        // ]==OOO=[
            map[4][i + 1].boxHP = 1;    }                           // ]==OOO=[
        map[5][0].state = 3;                                        // ]==OOO=[
                                                                    // ]=     [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
    else if (type == 45) {		// Layers G
        for ( int i = 0; i < 3; i++ ) {                                 // ]   ===[
            map[i + 1][4].boxHP = 1;    map[i + 2][2].boxHP = 1;        // ] OOO= [
            map[i + 1][3].boxHP = 1;    map[i + 2][1].boxHP = 1;        // ] OOO= [
            map[0][i + 1].state = 3;    map[i + 3][0].state = 3;        // ] =OOO [
            map[i][5].state = 3;        map[5][i + 1].state = 3;      } // ] =OOO [
        map[0][4].state = 3;    map[5][4].state = 3;                    // ]===   [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
        floorDmgs = rand() % 3 + diff * 1;		// 0-2	// 1-3	// 2-4
	}
	else { generateLevel(0, 100); return; }
    }

    // Add Trapped Items
    if      ( diff == 0 ) { trappedItems = rand() % 2; }        // 0, 1
    else if ( diff >= 1 ) { trappedItems = rand() % 3 + 2; }    // 2, 3, 4
    if ( level >= 75 ) { trappedItems++; }

    // Previous Bosses that entered the next level
    if ( !npcList.empty() ) {
        for ( int i = 0; i < npcList.size(); i++ ) {
            bool placed = false;
            if ( npcList[i].x == 8 ) {
                int xPos = -1, yPos = -1;
                if      ( isTileValid( 5, 1 ) ) { xPos = 5;   yPos = 1; }
                else if ( isTileValid( 4, 1 ) ) { xPos = 4;   yPos = 1; }
                else if ( isTileValid( 5, 2 ) ) { xPos = 5;   yPos = 2; }
                else if ( isTileValid( 4, 2 ) ) { xPos = 4;   yPos = 2; }
                else if ( isTileValid( 5, 3 ) ) { xPos = 5;   yPos = 3; }
                else if ( isTileValid( 4, 3 ) ) { xPos = 4;   yPos = 3; }
                else if ( isTileValid( 5, 4 ) ) { xPos = 5;   yPos = 4; }
                else if ( isTileValid( 4, 4 ) ) { xPos = 4;   yPos = 4; }
                else if ( isTileValid( 4, 5 ) ) { xPos = 4;   yPos = 5; }
                else if ( isTileValid( 3, 1 ) ) { xPos = 3;   yPos = 1; }
                else if ( isTileValid( 3, 2 ) ) { xPos = 3;   yPos = 2; }
                else if ( isTileValid( 3, 3 ) ) { xPos = 3;   yPos = 3; }
                else if ( isTileValid( 3, 4 ) ) { xPos = 3;   yPos = 4; }
                else if ( isTileValid( 3, 5 ) ) { xPos = 3;   yPos = 5; }
                if ( xPos != -1 && yPos != -1 ) {
                    npcList[i].facing = 1;
                    if ( diff != 0 ) npcList[i].hp++;
                    npcList[i].energy += abs( gain ) * 25;
                    npcList[i].x = xPos;
                    npcList[i].y = yPos;
                    placed = true;
                }
                if ( !placed ) { npcList.erase( npcList.begin() + i ); i--; }
            }
            else { npcList.erase( npcList.begin() + i ); i--; }
        }
    }

    // Chance for Boss to appear
    if ( currentGameDiff >= 2 && diff != 0  && level >= 30 ) {
        if ( ( lvlsWithoutBoss >= 14 )
        ||   ( ( diff >= 1 ) && ( rand() % 10 == 0 ) ) )
        {
            int xPos = -1, yPos = -1;
            if      ( isTileValid( 5, 4 ) ) { xPos = 5;   yPos = 4; }
            else if ( isTileValid( 5, 3 ) ) { xPos = 5;   yPos = 3; }
            else if ( isTileValid( 5, 2 ) ) { xPos = 5;   yPos = 2; }
            else if ( isTileValid( 5, 1 ) ) { xPos = 5;   yPos = 1; }
            else if ( isTileValid( 4, 5 ) ) { xPos = 4;   yPos = 5; }
            else if ( isTileValid( 4, 4 ) ) { xPos = 4;   yPos = 4; }
            else if ( isTileValid( 4, 3 ) ) { xPos = 4;   yPos = 3; }
            else if ( isTileValid( 4, 2 ) ) { xPos = 4;   yPos = 2; }
            else if ( isTileValid( 4, 1 ) ) { xPos = 4;   yPos = 1; }
            else if ( isTileValid( 3, 5 ) ) { xPos = 3;   yPos = 5; }
            else if ( isTileValid( 3, 4 ) ) { xPos = 3;   yPos = 4; }
            else if ( isTileValid( 3, 3 ) ) { xPos = 3;   yPos = 3; }
            else if ( isTileValid( 3, 2 ) ) { xPos = 3;   yPos = 2; }
            else if ( isTileValid( 3, 1 ) ) { xPos = 3;   yPos = 1; }
            if ( xPos != -1 && yPos != -1 ) {
                Player boss;
                boss.npc = true;
                boss.type = rand() % 5;
                boss.facing = 1;
                boss.hp = currentGameDiff + diff + 1;
                boss.energy = abs( gain ) * 25;
                boss.x = xPos;
                boss.y = yPos;
                npcList.push_back( boss );
                Mix_PlayChannel( 6, bossAppearSound, 0 );
            }
        }
    }

    if ( npcList.empty() ) { lvlsWithoutBoss++; }
    else                   { lvlsWithoutBoss = 0; }

    int minItems = minItems = 3 * itemWorth;
    if ( extraItems < minItems ) { extraItems = minItems; }

	generateItems(extraItems, type, trappedItems);
	generateBoxes(extraBoxes, type);
	generateFloor(floorDmgs,  type);
}
void App::generateItems(int amt, int type, int trapAmt) {
    // Randomly places Energy on the map based on a template       // Mode 0 = normal, Mode 1 = Trapped items
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
    vector<pair<int,int>> places;
    {
	// Rooms
	if (type == 0) {			// Rooms A
        places.push_back( make_pair( 1, 2 ) );  //	] +====[
        places.push_back( make_pair( 1, 3 ) );  //	] +==  [
        places.push_back( make_pair( 1, 4 ) );  //	] +===+[
        places.push_back( make_pair( 1, 5 ) );  //	] +===+[
        places.push_back( make_pair( 5, 0 ) );  //	]   ==+[
        places.push_back( make_pair( 5, 1 ) );  //	]=====+[
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 1) {		// Rooms B
        places.push_back( make_pair( 0, 3 ) );  //	]++====[
        places.push_back( make_pair( 0, 4 ) );  //	]++==  [
        places.push_back( make_pair( 0, 5 ) );  //	]++==  [
        places.push_back( make_pair( 1, 3 ) );  //	]  ==++[
        places.push_back( make_pair( 1, 4 ) );  //	]  ==++[
        places.push_back( make_pair( 1, 5 ) );  //	]====++[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 2)  {		// Rooms C
        places.push_back( make_pair( 0, 4 ) );  //	]++= ==[
        places.push_back( make_pair( 0, 5 ) );  //	]++====[
        places.push_back( make_pair( 1, 4 ) );  //	]===== [
        places.push_back( make_pair( 1, 5 ) );  //	] =====[
        places.push_back( make_pair( 4, 0 ) );  //	]====++[
        places.push_back( make_pair( 4, 1 ) );  //	]== =++[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 3) {		// Rooms D
        places.push_back( make_pair( 0, 3 ) );  //	]+    =[
        places.push_back( make_pair( 0, 4 ) );  //	]+=====[
        places.push_back( make_pair( 0, 5 ) );  //	]+=++==[
        places.push_back( make_pair( 2, 2 ) );  //	]==++=+[
        places.push_back( make_pair( 2, 3 ) );  //	]=====+[
        places.push_back( make_pair( 3, 2 ) );  //	]=    +[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 4) {		// Rooms E
        places.push_back( make_pair( 0, 4 ) );  //  ]++====[
        places.push_back( make_pair( 0, 5 ) );  //  ]++====[
        places.push_back( make_pair( 1, 4 ) );  //  ]======[
        places.push_back( make_pair( 1, 5 ) );  //  ]======[
        places.push_back( make_pair( 4, 0 ) );  //  ]====++[
        places.push_back( make_pair( 4, 1 ) );  //  ]====++[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    else if (type == 5) {		// Rooms F
        places.push_back( make_pair( 0, 4 ) );  //  ] ++===[
        places.push_back( make_pair( 1, 4 ) );  //  ]++====[
        places.push_back( make_pair( 1, 5 ) );  //  ]======[
        places.push_back( make_pair( 2, 5 ) );  //  ]======[
        places.push_back( make_pair( 3, 0 ) );  //  ]====++[
        places.push_back( make_pair( 4, 0 ) );  //  ]===++ [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	// Plus
	else if (type == 6) {		// Plus A
        places.push_back( make_pair( 2, 2 ) );  //	] =====[
        places.push_back( make_pair( 2, 3 ) );  //	] =+++=[
        places.push_back( make_pair( 2, 4 ) );  //	]==+=+=[
        places.push_back( make_pair( 3, 2 ) );  //	]==+++=[
        places.push_back( make_pair( 3, 4 ) );  //	]======[
        places.push_back( make_pair( 4, 2 ) );  //	]====  [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 7) {		// Plus B
        places.push_back( make_pair( 2, 2 ) );  //	] =====[
        places.push_back( make_pair( 2, 3 ) );  //	] =+++=[
        places.push_back( make_pair( 2, 4 ) );  //	] =+=+=[
        places.push_back( make_pair( 3, 2 ) );  //	]==+++=[
        places.push_back( make_pair( 3, 4 ) );  //	]======[
        places.push_back( make_pair( 4, 2 ) );  //	]===   [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 8) {		// Plus C
        places.push_back( make_pair( 1, 4 ) );  //	] =+===[
        places.push_back( make_pair( 2, 4 ) );  //	] ++++=[
        places.push_back( make_pair( 2, 5 ) );  //	] ===+=[
        places.push_back( make_pair( 3, 4 ) );  //	]====++[
        places.push_back( make_pair( 4, 1 ) );  //	]====+=[
        places.push_back( make_pair( 4, 2 ) );  //	]===   [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 9) {		// Plus D
        places.push_back( make_pair( 1, 3 ) );  //  ] +=+==[
        places.push_back( make_pair( 1, 4 ) );  //  ] +=+==[
        places.push_back( make_pair( 1, 5 ) );  //  ] +=+++[
        places.push_back( make_pair( 3, 1 ) );  //  ]======[
        places.push_back( make_pair( 3, 3 ) );  //  ]===+++[
        places.push_back( make_pair( 3, 4 ) );  //  ]===   [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 10) {		// Plus E
        places.push_back( make_pair( 1, 3 ) );  //	] +++==[
        places.push_back( make_pair( 1, 4 ) );  //	] +=+==[
        places.push_back( make_pair( 1, 5 ) );  //	] +=+++[
        places.push_back( make_pair( 2, 5 ) );  //	]=====+[
        places.push_back( make_pair( 3, 1 ) );  //	]===+++[
        places.push_back( make_pair( 3, 3 ) );  //	]===   [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 11) {		// Plus F
        places.push_back( make_pair( 0, 4 ) );  // ]+== ==[
        places.push_back( make_pair( 0, 5 ) );  // ]+=====[
        places.push_back( make_pair( 1, 3 ) );  // ]=++== [
        places.push_back( make_pair( 2, 3 ) );  // ]===+==[
        places.push_back( make_pair( 3, 1 ) );  // ]===+==[
        places.push_back( make_pair( 3, 2 ) );  // ]====++[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 5, 0 ) );
	}
    else if (type == 12) {		// Plus G
        places.push_back( make_pair( 1, 2 ) );  //	]  +===[
        places.push_back( make_pair( 1, 4 ) );  //	]=++=+=[
        places.push_back( make_pair( 2, 1 ) );  //	]======[
        places.push_back( make_pair( 2, 2 ) );  //	]=++=++[
        places.push_back( make_pair( 2, 4 ) );  //	]= +=+ [
        places.push_back( make_pair( 2, 5 ) );  //	]===== [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	// Paths
	else if (type == 13) {		// Paths A
        places.push_back( make_pair( 1, 3 ) );  //	]=++===[
        places.push_back( make_pair( 1, 4 ) );  //	]=++===[
        places.push_back( make_pair( 1, 5 ) );  //	]=++  =[
        places.push_back( make_pair( 2, 3 ) );  //	]===++=[
        places.push_back( make_pair( 2, 4 ) );  //	]===++=[
        places.push_back( make_pair( 2, 5 ) );  //	]===++=[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
	else if (type == 14) {		// Paths B
        places.push_back( make_pair( 2, 3 ) );  //	]==++==[
        places.push_back( make_pair( 2, 4 ) );  //	]==++==[
        places.push_back( make_pair( 2, 5 ) );  //	]==+  =[
        places.push_back( make_pair( 3, 0 ) );  //	]==  +=[
        places.push_back( make_pair( 3, 1 ) );  //	]===++=[
        places.push_back( make_pair( 3, 4 ) );  //	]===++=[
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
	else if (type == 15) {		// Paths C
        places.push_back( make_pair( 0, 2 ) );  //	] =+===[
        places.push_back( make_pair( 0, 3 ) );  //	]==+===[
        places.push_back( make_pair( 1, 2 ) );  //	]++===+[
        places.push_back( make_pair( 1, 3 ) );  //	]++===+[
        places.push_back( make_pair( 2, 4 ) );  //	]  ==  [
        places.push_back( make_pair( 2, 5 ) );  //	]====  [
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 16) {		// Paths D
        places.push_back( make_pair( 0, 3 ) );  //	]    ==[
        places.push_back( make_pair( 0, 4 ) );  //	]++===+[
        places.push_back( make_pair( 1, 3 ) );  //	]++===+[
        places.push_back( make_pair( 1, 4 ) );  //	]======[
        places.push_back( make_pair( 2, 1 ) );  //	] =++= [
        places.push_back( make_pair( 3, 1 ) );  //	]===== [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 17) {		// Paths E
        places.push_back( make_pair( 2, 2 ) );  //	]  ==+=[
        places.push_back( make_pair( 2, 3 ) );  //	]====+=[
        places.push_back( make_pair( 3, 0 ) );  //	]==+= =[
        places.push_back( make_pair( 3, 1 ) );  //	]==+= =[
        places.push_back( make_pair( 4, 0 ) );  //	]= =++=[
        places.push_back( make_pair( 4, 1 ) );  //	]= =++=[
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
    else if (type == 18) {		// Paths F
        places.push_back( make_pair( 1, 0 ) );  //	]=++= =[
        places.push_back( make_pair( 1, 1 ) );  //	]=++= =[
        places.push_back( make_pair( 1, 4 ) );  //	]= ====[
        places.push_back( make_pair( 1, 5 ) );  //	]= ====[
        places.push_back( make_pair( 2, 0 ) );  //	]=++= =[
        places.push_back( make_pair( 2, 1 ) );  //	]=++= =[
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 2, 5 ) );
	}
    else if (type == 19) {		// Paths G
        places.push_back( make_pair( 0, 2 ) );  //	] =++==[
        places.push_back( make_pair( 0, 3 ) );  //	]======[
        places.push_back( make_pair( 2, 0 ) );  //	]+====+[
        places.push_back( make_pair( 2, 5 ) );  //	]+====+[
        places.push_back( make_pair( 3, 0 ) );  //	]======[
        places.push_back( make_pair( 3, 5 ) );  //	]==++= [
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	// Cross
	else if (type == 20) {		// Cross A
        places.push_back( make_pair( 0, 4 ) );  //	]+++ ==[
        places.push_back( make_pair( 0, 5 ) );  //	]+=====[
        places.push_back( make_pair( 1, 5 ) );  //	]======[
        places.push_back( make_pair( 2, 5 ) );  //	]======[
        places.push_back( make_pair( 4, 0 ) );  //	]=== ++[
        places.push_back( make_pair( 4, 1 ) );  //	]=== ++[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 21) {		// Cross B
        places.push_back( make_pair( 0, 4 ) );  //	]++====[
        places.push_back( make_pair( 0, 5 ) );  //	]++====[
        places.push_back( make_pair( 1, 4 ) );  //	]  === [
        places.push_back( make_pair( 1, 5 ) );  //	]=====+[
        places.push_back( make_pair( 4, 0 ) );  //	]=====+[
        places.push_back( make_pair( 5, 0 ) );  //	]====++[
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 22) {		// Cross C
        places.push_back( make_pair( 0, 4 ) );  //	]++====[
        places.push_back( make_pair( 0, 5 ) );  //	]++====[
        places.push_back( make_pair( 1, 4 ) );  //	] ===  [
        places.push_back( make_pair( 1, 5 ) );  //	] ===  [
        places.push_back( make_pair( 4, 0 ) );  //	]====++[
        places.push_back( make_pair( 4, 1 ) );  //	]====++[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 23) {		// Cross D
        places.push_back( make_pair( 0, 1 ) );  //	]++ ===[
        places.push_back( make_pair( 0, 4 ) );  //	]++=== [
        places.push_back( make_pair( 0, 5 ) );  //	]===== [
        places.push_back( make_pair( 1, 1 ) );  //	]===== [
        places.push_back( make_pair( 1, 4 ) );  //	]++=++ [
        places.push_back( make_pair( 1, 5 ) );  //	]===== [
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 4, 1 ) );
	}
    else if (type == 24) {		// Cross E
        places.push_back( make_pair( 1, 0 ) );  //	]======[
        places.push_back( make_pair( 1, 1 ) );  //	]=+====[
        places.push_back( make_pair( 1, 3 ) );  //	]=+====[
        places.push_back( make_pair( 1, 4 ) );  //	]===== [
        places.push_back( make_pair( 4, 0 ) );  //	]=+==++[
        places.push_back( make_pair( 4, 1 ) );  //	]=+==++[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    else if (type == 25) {		// Cross F
        places.push_back( make_pair( 1, 1 ) );  //	] ==+==[
        places.push_back( make_pair( 1, 3 ) );  //	] = =+=[
        places.push_back( make_pair( 2, 2 ) );  //	]=+=+=+[
        places.push_back( make_pair( 3, 1 ) );  //	]==+= =[
        places.push_back( make_pair( 3, 3 ) );  //	]=+=+==[
        places.push_back( make_pair( 3, 5 ) );  //	]====  [
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 26) {		// Cross G
        places.push_back( make_pair( 1, 3 ) );  //	] +====[
        places.push_back( make_pair( 1, 5 ) );  //	]==+= =[
        places.push_back( make_pair( 2, 4 ) );  //	]=+=+==[
        places.push_back( make_pair( 3, 1 ) );  //	]=  =+=[
        places.push_back( make_pair( 3, 3 ) );  //	]=  +=+[
        places.push_back( make_pair( 4, 2 ) );  //	]===== [
        places.push_back( make_pair( 5, 1 ) );
	}
	// Gallery
	else if (type == 27) {		// Gallery A
        places.push_back( make_pair( 1, 1 ) );  //	]======[
        places.push_back( make_pair( 1, 2 ) );  //	]=++++=[
        places.push_back( make_pair( 1, 3 ) );  //	]=++++=[
        places.push_back( make_pair( 1, 4 ) );  //	]=++++=[
        places.push_back( make_pair( 2, 1 ) );  //	]=++++=[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 28) {		// Gallery B
        places.push_back( make_pair( 0, 1 ) );  //	]======[
        places.push_back( make_pair( 0, 2 ) );  //	]++==++[
        places.push_back( make_pair( 0, 3 ) );  //	]++==++[
        places.push_back( make_pair( 0, 4 ) );  //	]++==++[
        places.push_back( make_pair( 1, 1 ) );  //	]++==++[
        places.push_back( make_pair( 1, 2 ) );  //	]======[
        places.push_back( make_pair( 1, 3 ) );
        places.push_back( make_pair( 1, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 29) {		// Gallery C
        places.push_back( make_pair( 1, 0 ) );  //	]=++++=[
        places.push_back( make_pair( 1, 2 ) );  //	]======[
        places.push_back( make_pair( 1, 3 ) );  //	]=++++=[
        places.push_back( make_pair( 1, 5 ) );  //	]=++++=[
        places.push_back( make_pair( 2, 0 ) );  //	]======[
        places.push_back( make_pair( 2, 2 ) );  //	]=++++=[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 30) {		// Gallery D
        places.push_back( make_pair( 0, 1 ) );  //	]======[
        places.push_back( make_pair( 0, 2 ) );  //	]+=++=+[
        places.push_back( make_pair( 0, 3 ) );  //	]+=++=+[
        places.push_back( make_pair( 0, 4 ) );  //	]+=++=+[
        places.push_back( make_pair( 2, 1 ) );  //	]+=++=+[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 31) {		// Gallery E
        places.push_back( make_pair( 0, 2 ) );  //	]=+++==[
        places.push_back( make_pair( 0, 3 ) );  //	]+=====[
        places.push_back( make_pair( 0, 4 ) );  //	]+=++=+[
        places.push_back( make_pair( 1, 5 ) );  //	]+=++=+[
        places.push_back( make_pair( 2, 0 ) );  //	]=====+[
        places.push_back( make_pair( 2, 2 ) );  //	]==+++=[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 32) {		// Gallery F
        places.push_back( make_pair( 0, 4 ) );  //	]++====[
        places.push_back( make_pair( 0, 5 ) );  //	]++=++=[
        places.push_back( make_pair( 1, 1 ) );  //	]===++=[
        places.push_back( make_pair( 1, 2 ) );  //	]=++===[
        places.push_back( make_pair( 1, 4 ) );  //	]=++=++[
        places.push_back( make_pair( 1, 5 ) );  //	]====++[
        places.push_back( make_pair( 2, 1 ) );
        places.push_back( make_pair( 2, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    // Scattered
	else if (type == 33) {		// Scattered A
        places.push_back( make_pair( 2, 1 ) );  //	]==+===[
        places.push_back( make_pair( 2, 3 ) );  //	]===+==[
        places.push_back( make_pair( 2, 5 ) );  //	]==+=+=[
        places.push_back( make_pair( 3, 0 ) );  //	]===+=+[
        places.push_back( make_pair( 3, 2 ) );  //	]==+=+=[
        places.push_back( make_pair( 3, 4 ) );  //	]===+=+[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 34) {		// Scattered B
        places.push_back( make_pair( 0, 2 ) );  //	]=+=+==[
        places.push_back( make_pair( 0, 4 ) );  //	]+=+===[
        places.push_back( make_pair( 1, 3 ) );  //	]=+=+=+[
        places.push_back( make_pair( 1, 5 ) );  //	]+=+=+=[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 4 ) );  //	]======[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 35) {		// Scattered C
        places.push_back( make_pair( 0, 3 ) );  //	]   ===[
        places.push_back( make_pair( 1, 2 ) );  //	]=+=+==[
        places.push_back( make_pair( 1, 4 ) );  //	]+=+=+=[
        places.push_back( make_pair( 2, 1 ) );  //	]=+=+= [
        places.push_back( make_pair( 2, 3 ) );  //	]==+=+ [
        places.push_back( make_pair( 3, 0 ) );  //	]===+= [
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
	else if (type == 36) {		// Scattered D
        places.push_back( make_pair( 1, 2 ) );  //	] =+===[
        places.push_back( make_pair( 1, 4 ) );  //	] +=+==[
        places.push_back( make_pair( 2, 1 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 3 ) );  //	]=+=+=+[
        places.push_back( make_pair( 2, 5 ) );  //	]==+=+=[
        places.push_back( make_pair( 3, 2 ) );  //	]====  [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
    else if (type == 37) {		// Scattered E
        places.push_back( make_pair( 0, 4 ) );  //	]=+====[
        places.push_back( make_pair( 1, 3 ) );  //	]+=+===[
        places.push_back( make_pair( 1, 5 ) );  //	]=+=+==[
        places.push_back( make_pair( 2, 4 ) );  //	] ===+=[
        places.push_back( make_pair( 3, 1 ) );  //	]===+=+[
        places.push_back( make_pair( 3, 3 ) );  //	]== =+=[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    else if (type == 38) {		// Scattered F
        places.push_back( make_pair( 0, 3 ) );  //	]+=+ ==[
        places.push_back( make_pair( 0, 5 ) );  //	]=+====[
        places.push_back( make_pair( 1, 4 ) );  //	]+=+== [
        places.push_back( make_pair( 2, 3 ) );  //	] ==+=+[
        places.push_back( make_pair( 2, 5 ) );  //	]====+=[
        places.push_back( make_pair( 3, 0 ) );  //	]== +=+[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	// Layers
	else if (type == 39) {		// Layers A
        places.push_back( make_pair( 2, 1 ) );  //	]= ++==[
        places.push_back( make_pair( 2, 2 ) );  //	]==++==[
        places.push_back( make_pair( 2, 3 ) );  //	]==++==[
        places.push_back( make_pair( 2, 4 ) );  //	]==++==[
        places.push_back( make_pair( 2, 5 ) );  //	]==++ =[
        places.push_back( make_pair( 3, 1 ) );  //	]=    =[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
	}
	else if (type == 40) {		// Layers B
        places.push_back( make_pair( 2, 1 ) );  //	]    ==[
        places.push_back( make_pair( 2, 2 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 3 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 4 ) );  //	]==+=+=[
        places.push_back( make_pair( 4, 1 ) );  //	]==+=+=[
        places.push_back( make_pair( 4, 2 ) );  //	]=     [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 41) {		// Layers C
        places.push_back( make_pair( 3, 1 ) );  //	]    ==[
        places.push_back( make_pair( 3, 2 ) );  //	]===++=[
        places.push_back( make_pair( 3, 3 ) );  //	]===++=[
        places.push_back( make_pair( 3, 4 ) );  //	]===++=[
        places.push_back( make_pair( 4, 1 ) );  //	]===++=[
        places.push_back( make_pair( 4, 2 ) );  //	]=     [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 42) {		// Layers D
        places.push_back( make_pair( 1, 4 ) );  //	]=+====[
        places.push_back( make_pair( 1, 5 ) );  //	]=+====[
        places.push_back( make_pair( 2, 2 ) );  //	]==+===[
        places.push_back( make_pair( 2, 3 ) );  //	] =++==[
        places.push_back( make_pair( 3, 2 ) );  //	]====++[
        places.push_back( make_pair( 4, 1 ) );  //	]== ===[
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 43) {		// Layers E
        places.push_back( make_pair( 0, 4 ) );  //	]=== ==[
        places.push_back( make_pair( 1, 4 ) );  //	]++====[
        places.push_back( make_pair( 2, 3 ) );  //	]==++= [
        places.push_back( make_pair( 3, 2 ) );  //	]===+==[
        places.push_back( make_pair( 3, 3 ) );  //	]====+=[
        places.push_back( make_pair( 4, 0 ) );  //	]====+=[
        places.push_back( make_pair( 4, 1 ) );
	}
    else if (type == 44) {		// Layers F
        places.push_back( make_pair( 3, 1 ) );  //	]    ==[
        places.push_back( make_pair( 3, 2 ) );  //	]===++=[
        places.push_back( make_pair( 3, 3 ) );  //	]===++=[
        places.push_back( make_pair( 3, 4 ) );  //	]===++=[
        places.push_back( make_pair( 4, 1 ) );  //	]===++=[
        places.push_back( make_pair( 4, 2 ) );  //	]=     [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 45) {		// Layers G
        places.push_back( make_pair( 1, 3 ) );  //	]   ===[
        places.push_back( make_pair( 1, 4 ) );  //	] ++== [
        places.push_back( make_pair( 2, 3 ) );  //	] ++== [
        places.push_back( make_pair( 2, 4 ) );  //	] ==++ [
        places.push_back( make_pair( 3, 1 ) );  //	] ==++ [
        places.push_back( make_pair( 3, 2 ) );  //	]===   [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
    else { generateItems( amt, 0, 0 ); return; }
    }

    for ( int i = 0; i < amt; i += itemWorth ) {
        if ( !places.empty() ) {
            int randPlace = rand() % places.size();
            xPos = places[randPlace].first;
            yPos = places[randPlace].second;
            if ( xPos != -1 && yPos != -1 && map[xPos][yPos].state <= 1 && map[xPos][yPos].boxHP <= 3 ) {
                map[xPos][yPos].item++;
                if ( map[xPos][yPos].boxHP > 0 ) { map[xPos][yPos].boxType = 2; }
                if ( map[xPos][yPos].item >= 2 ) { places.erase( places.begin() + randPlace ); }
            }
        }
    }

    for ( int i = 0; i < places.size(); i++ ) {
        xPos = places[i].first;
        yPos = places[i].second;
        if ( map[xPos][yPos].item >= 1 ) { places.erase( places.begin() + i ); i--; }
    }

    for ( int i = 0; i < trapAmt; i++ ) {
        if ( !places.empty() ) {
            int randPlace = rand() % places.size();
            xPos = places[randPlace].first;
            yPos = places[randPlace].second;
            if ( xPos != -1 && yPos != -1 && map[xPos][yPos].state <= 1 && map[xPos][yPos].boxHP <= 3 && map[xPos][yPos].item == 0 ) {
                map[xPos][yPos].item = -1;
                if ( map[xPos][yPos].boxHP > 0 ) { map[xPos][yPos].boxType = 2; }
                places.erase( places.begin() + randPlace );
            }
        }
    }
}
void App::generateBoxes(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
    vector<pair<int, int>> places;
    {
	// Rooms
    if ( type == 0 ) {			// Rooms A
        places.push_back( make_pair( 2, 2 ) );  //	] =O=O=[
        places.push_back( make_pair( 2, 3 ) );  //	] =O=  [
        places.push_back( make_pair( 2, 4 ) );  //	] =O=O=[
        places.push_back( make_pair( 2, 5 ) );  //	] =O=O=[
        places.push_back( make_pair( 4, 0 ) );  //	]   =O=[
        places.push_back( make_pair( 4, 1 ) );  //	]====O=[
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
    }
	else if (type == 1) {		// Rooms B
        places.push_back( make_pair( 0, 3 ) );  //	]OO==O=[
        places.push_back( make_pair( 0, 4 ) );  //	]OO==  [
        places.push_back( make_pair( 0, 5 ) );  //	]OO==  [
        places.push_back( make_pair( 1, 3 ) );  //	]  ==OO[
        places.push_back( make_pair( 1, 4 ) );  //	]  ==OO[
        places.push_back( make_pair( 1, 5 ) );  //	]====OO[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 2) {		// Rooms C
        places.push_back( make_pair( 1, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 1, 4 ) );  //	]=OO===[
        places.push_back( make_pair( 2, 2 ) );  //	]=OOO= [
        places.push_back( make_pair( 2, 3 ) );  //	] =OOO=[
        places.push_back( make_pair( 2, 4 ) );  //	]===OO=[
        places.push_back( make_pair( 3, 1 ) );  //	]== ===[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
	else if (type == 3) {		// Rooms D
        places.push_back( make_pair( 0, 2 ) );  //	]=    =[
        places.push_back( make_pair( 0, 3 ) );  //	]=O==O=[
        places.push_back( make_pair( 1, 1 ) );  //	]O=OO=O[
        places.push_back( make_pair( 1, 4 ) );  //	]O=OO=O[
        places.push_back( make_pair( 2, 2 ) );  //	]=O==O=[
        places.push_back( make_pair( 2, 3 ) );  //	]=    =[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 4) {		// Rooms E
        places.push_back( make_pair( 0, 3 ) );  //	]==OO==[
        places.push_back( make_pair( 1, 2 ) );  //	]==O=O=[
        places.push_back( make_pair( 1, 3 ) );  //	]OO===O[
        places.push_back( make_pair( 2, 1 ) );  //	]=OO=OO[
        places.push_back( make_pair( 2, 2 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 4 ) );  //	]===O==[
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 5) {		// Rooms F
        places.push_back( make_pair( 0, 3 ) );  //	] ==O==[
        places.push_back( make_pair( 1, 2 ) );  //	]==OO==[
        places.push_back( make_pair( 1, 3 ) );  //	]OO==O=[
        places.push_back( make_pair( 2, 0 ) );  //	]=O==OO[
        places.push_back( make_pair( 2, 1 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 4 ) );  //	]==O== [
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	// Plus
	else if (type == 6) {		// Plus A
        places.push_back( make_pair( 1, 3 ) );  //	] ==O==[
        places.push_back( make_pair( 2, 2 ) );  //	] =OOO=[
        places.push_back( make_pair( 2, 3 ) );  //	]=OO=OO[
        places.push_back( make_pair( 2, 4 ) );  //	]==OOO=[
        places.push_back( make_pair( 3, 1 ) );  //	]===O==[
        places.push_back( make_pair( 3, 2 ) );  //	]====  [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 7) {		// Plus B
        places.push_back( make_pair( 1, 2 ) );  //	] ===O=[
        places.push_back( make_pair( 2, 1 ) );  //	] =OOOO[
        places.push_back( make_pair( 2, 2 ) );  //	] =O=O=[
        places.push_back( make_pair( 2, 3 ) );  //	]=OOOO=[
        places.push_back( make_pair( 2, 4 ) );  //	]==O===[
        places.push_back( make_pair( 3, 2 ) );  //	]===   [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 8) {		// Plus C
        places.push_back( make_pair( 1, 4 ) );  //	] =O===[
        places.push_back( make_pair( 2, 3 ) );  //	] OOOO=[
        places.push_back( make_pair( 2, 4 ) );  //	] =O=O=[
        places.push_back( make_pair( 2, 5 ) );  //	]===OOO[
        places.push_back( make_pair( 3, 2 ) );  //	]====O=[
        places.push_back( make_pair( 3, 4 ) );  //	]===   [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 9) {		// Plus D
        places.push_back( make_pair( 1, 3 ) );  //	] O=O==[
        places.push_back( make_pair( 1, 4 ) );  //	] O=O =[
        places.push_back( make_pair( 1, 5 ) );  //	] O=OOO[
        places.push_back( make_pair( 3, 1 ) );  //	]======[
        places.push_back( make_pair( 3, 3 ) );  //	]===OOO[
        places.push_back( make_pair( 3, 4 ) );  //	]===   [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 10) {		// Plus E
        places.push_back( make_pair( 1, 3 ) );  // ] OOO==[
        places.push_back( make_pair( 1, 4 ) );  // ] O=O==[
        places.push_back( make_pair( 1, 5 ) );  // ] O=OOO[
        places.push_back( make_pair( 2, 2 ) );  // ]==O==O[
        places.push_back( make_pair( 2, 5 ) );  // ]===OOO[
        places.push_back( make_pair( 3, 1 ) );  // ]===   [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 11) {		// Plus F
        places.push_back( make_pair( 0, 4 ) );  //	]O== ==[
        places.push_back( make_pair( 0, 5 ) );  //	]O==O==[
        places.push_back( make_pair( 1, 3 ) );  //	]=OO=O [
        places.push_back( make_pair( 2, 3 ) );  //	]===O==[
        places.push_back( make_pair( 3, 1 ) );  //	]===O==[
        places.push_back( make_pair( 3, 2 ) );  //	]====OO[
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
	}
    else if (type == 12) {		// Plus G
        places.push_back( make_pair( 1, 2 ) );  //	]  O=O=[
        places.push_back( make_pair( 1, 4 ) );  //	]=OO=OO[
        places.push_back( make_pair( 2, 1 ) );  //	]===O==[
        places.push_back( make_pair( 2, 2 ) );  //	]=OO=OO[
        places.push_back( make_pair( 2, 4 ) );  //	]= O=O [
        places.push_back( make_pair( 2, 5 ) );  //	]===== [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	// Paths
	else if (type == 13) {		// Paths A
        places.push_back( make_pair( 1, 3 ) );  //	]=OO===[
        places.push_back( make_pair( 1, 4 ) );  //	]=OO===[
        places.push_back( make_pair( 1, 5 ) );  //	]=OO  =[
        places.push_back( make_pair( 2, 3 ) );  //	]=  OO=[
        places.push_back( make_pair( 2, 4 ) );  //	]===OO=[
        places.push_back( make_pair( 2, 5 ) );  //	]===OO=[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
	else if (type == 14) {		// Paths B
        places.push_back( make_pair( 2, 0 ) );  //	]==OOO=[
        places.push_back( make_pair( 2, 1 ) );  //	]==OOO=[
        places.push_back( make_pair( 2, 3 ) );  //	]==O  =[
        places.push_back( make_pair( 2, 4 ) );  //	]==  O=[
        places.push_back( make_pair( 2, 5 ) );  //	]==OOO=[
        places.push_back( make_pair( 3, 0 ) );  //	]==OOO=[
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 15) {		// Paths D
        places.push_back( make_pair( 0, 2 ) );  //	] =OO==[
        places.push_back( make_pair( 0, 3 ) );  //	]==OO==[
        places.push_back( make_pair( 1, 2 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 3 ) );  //	]OO==OO[
        places.push_back( make_pair( 2, 4 ) );  //	]  ==  [
        places.push_back( make_pair( 2, 5 ) );  //	]====  [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 16) {		// Paths D
        places.push_back( make_pair( 0, 3 ) );  //	]    ==[
        places.push_back( make_pair( 0, 4 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 3 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 4 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 1 ) );  //	] =OO= [
        places.push_back( make_pair( 2, 2 ) );  //	]===== [
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 17) {		// Paths E
        places.push_back( make_pair( 1, 2 ) );  //	]  =OO=[
        places.push_back( make_pair( 1, 3 ) );  //	]===OO=[
        places.push_back( make_pair( 2, 2 ) );  //	]=OO= =[
        places.push_back( make_pair( 2, 3 ) );  //	]=OO= =[
        places.push_back( make_pair( 3, 0 ) );  //	]= =OO=[
        places.push_back( make_pair( 3, 1 ) );  //	]= =OO=[
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
    else if (type == 18) {		// Paths F
        places.push_back( make_pair( 1, 0 ) );  //	]=OO= =[
        places.push_back( make_pair( 1, 1 ) );  //	]=OO= =[
        places.push_back( make_pair( 1, 4 ) );  //	]= =OO=[
        places.push_back( make_pair( 1, 5 ) );  //	]= =OO=[
        places.push_back( make_pair( 2, 0 ) );  //	]=OO= =[
        places.push_back( make_pair( 2, 1 ) );  //	]=OO= =[
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
    else if (type == 19) {		// Paths G
        places.push_back( make_pair( 0, 2 ) );  //	] =OO==[
        places.push_back( make_pair( 0, 3 ) );  //	]==OO==[
        places.push_back( make_pair( 1, 2 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 3 ) );  //	]OO==OO[
        places.push_back( make_pair( 2, 0 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 1 ) );  //	]==OO= [
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	// Cross
	else if (type == 20) {		// Cross A
        places.push_back( make_pair( 0, 2 ) );  //	]=== ==[
        places.push_back( make_pair( 0, 4 ) );  //	]O=OO=O[
        places.push_back( make_pair( 1, 3 ) );  //	]=O==O=[
        places.push_back( make_pair( 2, 2 ) );  //	]O=OO=O[
        places.push_back( make_pair( 2, 4 ) );  //	]=== ==[
        places.push_back( make_pair( 3, 2 ) );  //	]=== ==[
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 21) {		// Cross B
        places.push_back( make_pair( 2, 0 ) );  //	]==O=O=[
        places.push_back( make_pair( 2, 2 ) );  //	]===O==[
        places.push_back( make_pair( 2, 3 ) );  //	]  O=O [
        places.push_back( make_pair( 2, 5 ) );  //	]==O=O=[
        places.push_back( make_pair( 3, 1 ) );  //	]===O==[
        places.push_back( make_pair( 3, 4 ) );  //	]==O=O=[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 22) {		// Cross C
        places.push_back( make_pair( 1, 0 ) );  //	]=O=O==[
        places.push_back( make_pair( 1, 1 ) );  //	]=O=O==[
        places.push_back( make_pair( 1, 4 ) );  //	] =O=  [
        places.push_back( make_pair( 1, 5 ) );  //	] =O=  [
        places.push_back( make_pair( 2, 2 ) );  //	]=O=O==[
        places.push_back( make_pair( 2, 3 ) );  //	]=O=O==[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
	}
	else if (type == 23) {		// Cross D
        places.push_back( make_pair( 0, 1 ) );  // ]== ===[
        places.push_back( make_pair( 0, 4 ) );  // ]OO=OO [
        places.push_back( make_pair( 1, 1 ) );  // ]==O== [
        places.push_back( make_pair( 1, 4 ) );  // ]==O== [
        places.push_back( make_pair( 2, 2 ) );  // ]OO=OO [
        places.push_back( make_pair( 2, 3 ) );  // ]===== [
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 24) {		// Cross E
        places.push_back( make_pair( 1, 0 ) );  // ]  == =[
        places.push_back( make_pair( 1, 1 ) );  // ]=O==O=[
        places.push_back( make_pair( 1, 3 ) );  // ]=O==O=[
        places.push_back( make_pair( 1, 4 ) );  // ]==OO= [
        places.push_back( make_pair( 2, 2 ) );  // ]=O==O=[
        places.push_back( make_pair( 3, 2 ) );  // ]=O==O=[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 25) {		// Cross F
        places.push_back( make_pair( 1, 1 ) );  //	] ==O=O[
        places.push_back( make_pair( 1, 3 ) );  //	] = =O=[
        places.push_back( make_pair( 2, 2 ) );  //	]=O=O=O[
        places.push_back( make_pair( 3, 1 ) );  //	]==O= =[
        places.push_back( make_pair( 3, 3 ) );  //	]=O=O==[
        places.push_back( make_pair( 3, 5 ) );  //	]====  [
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 5 ) );
	}
    else if (type == 26) {		// Cross G
        places.push_back( make_pair( 1, 3 ) );  //	] O=O==[
        places.push_back( make_pair( 1, 5 ) );  //	]==O= =[
        places.push_back( make_pair( 2, 4 ) );  //	]=O=O=O[
        places.push_back( make_pair( 3, 1 ) );  //	]=  =O=[
        places.push_back( make_pair( 3, 3 ) );  //	]=  O=O[
        places.push_back( make_pair( 3, 5 ) );  //	]===== [
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	// Gallery
	else if (type == 27) {		// Gallery A
        places.push_back( make_pair( 1, 1 ) );  //	]======[
        places.push_back( make_pair( 1, 2 ) );  //	]=OOOO=[
        places.push_back( make_pair( 1, 3 ) );  //	]=OOOO=[
        places.push_back( make_pair( 1, 4 ) );  //	]=OOOO=[
        places.push_back( make_pair( 2, 1 ) );  //	]=OOOO=[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 28) {		// Gallery B
        places.push_back( make_pair( 0, 1 ) );  //	]======[
        places.push_back( make_pair( 0, 2 ) );  //	]OO==OO[
        places.push_back( make_pair( 0, 3 ) );  //	]OO==OO[
        places.push_back( make_pair( 0, 4 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 1 ) );  //	]OO==OO[
        places.push_back( make_pair( 1, 2 ) );  //	]======[
        places.push_back( make_pair( 1, 3 ) );
        places.push_back( make_pair( 1, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 29) {		// Gallery C
        places.push_back( make_pair( 1, 0 ) );  //	]=OOOO=[
        places.push_back( make_pair( 1, 2 ) );  //	]======[
        places.push_back( make_pair( 1, 3 ) );  //	]=OOOO=[
        places.push_back( make_pair( 1, 5 ) );  //	]=OOOO=[
        places.push_back( make_pair( 2, 0 ) );  //	]======[
        places.push_back( make_pair( 2, 2 ) );  //	]=OOOO=[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 30) {		// Gallery D
        places.push_back( make_pair( 0, 1 ) );  //	]======[
        places.push_back( make_pair( 0, 2 ) );  //	]O=OO=O[
        places.push_back( make_pair( 0, 3 ) );  //	]O=OO=O[
        places.push_back( make_pair( 0, 4 ) );  //	]O=OO=O[
        places.push_back( make_pair( 2, 1 ) );  //	]O=OO=O[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 31) {		// Gallery E
        places.push_back( make_pair( 0, 2 ) );  //	]=OOO==[
        places.push_back( make_pair( 0, 3 ) );  //	]O=====[
        places.push_back( make_pair( 0, 4 ) );  //	]O=OO=O[
        places.push_back( make_pair( 1, 5 ) );  //	]O=OO=O[
        places.push_back( make_pair( 2, 0 ) );  //	]=====O[
        places.push_back( make_pair( 2, 2 ) );  //	]==OOO=[
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 32) {		// Gallery F
        places.push_back( make_pair( 0, 4 ) );  //	]OO====[
        places.push_back( make_pair( 0, 5 ) );  //	]OO=OO=[
        places.push_back( make_pair( 1, 1 ) );  //	]===OO=[
        places.push_back( make_pair( 1, 2 ) );  //	]=OO===[
        places.push_back( make_pair( 1, 4 ) );  //	]=OO=OO[
        places.push_back( make_pair( 1, 5 ) );  //	]====OO[
        places.push_back( make_pair( 2, 1 ) );
        places.push_back( make_pair( 2, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	// Scattered
	else if (type == 33) {		// Scattered A
        places.push_back( make_pair( 2, 1 ) );  //	]==O=O=[
        places.push_back( make_pair( 2, 3 ) );  //	]===O=O[
        places.push_back( make_pair( 2, 5 ) );  //	]==O=O=[
        places.push_back( make_pair( 3, 0 ) );  //	]===O=O[
        places.push_back( make_pair( 3, 2 ) );  //	]==O=O=[
        places.push_back( make_pair( 3, 4 ) );  //	]===O=O[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 34) {		// Scattered B
        places.push_back( make_pair( 0, 2 ) );  //	]=O=O=O[
        places.push_back( make_pair( 0, 4 ) );  //	]O=O=O=[
        places.push_back( make_pair( 1, 3 ) );  //	]=O=O=O[
        places.push_back( make_pair( 1, 5 ) );  //	]O=O=O=[
        places.push_back( make_pair( 2, 2 ) );  //	]======[
        places.push_back( make_pair( 2, 4 ) );  //	]======[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 5 ) );
	}
	else if (type == 35) {		// Scattered C
        places.push_back( make_pair( 0, 3 ) );  //	]   =O=[
        places.push_back( make_pair( 1, 2 ) );  //	]=O=O=O[
        places.push_back( make_pair( 1, 4 ) );  //	]O=O=O=[
        places.push_back( make_pair( 2, 1 ) );  //	]=O=O= [
        places.push_back( make_pair( 2, 3 ) );  //	]==O=O [
        places.push_back( make_pair( 3, 0 ) );  //	]===O= [
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 36) {		// Scattered D
        places.push_back( make_pair( 1, 2 ) );  //	] =O=O=[
        places.push_back( make_pair( 1, 4 ) );  //	] O=O=O[
        places.push_back( make_pair( 2, 1 ) );  //	]==O=O=[
        places.push_back( make_pair( 2, 3 ) );  //	]=O=O=O[
        places.push_back( make_pair( 2, 5 ) );  //	]==O=O=[
        places.push_back( make_pair( 3, 2 ) );  //	]====  [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 37) {		// Scattered E
        places.push_back( make_pair( 0, 4 ) );  //	]=O=O==[
        places.push_back( make_pair( 1, 3 ) );  //	]O=O=O=[
        places.push_back( make_pair( 1, 5 ) );  //	]=O=O=O[
        places.push_back( make_pair( 2, 4 ) );  //	] ===O=[
        places.push_back( make_pair( 3, 1 ) );  //	]===O=O[
        places.push_back( make_pair( 3, 3 ) );  //	]== =O=[
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 38) {		// Scattered F
        places.push_back( make_pair( 0, 3 ) );  //	]O=O ==[
        places.push_back( make_pair( 0, 5 ) );  //	]=O=O==[
        places.push_back( make_pair( 1, 4 ) );  //	]O=O=O [
        places.push_back( make_pair( 2, 3 ) );  //	] ==O=O[
        places.push_back( make_pair( 2, 5 ) );  //	]====O=[
        places.push_back( make_pair( 3, 0 ) );  //	]== O=O[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	// Layers
	else if (type == 39) {		// Layers A
        places.push_back( make_pair( 2, 1 ) );  //	]= OO==[
        places.push_back( make_pair( 2, 2 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 3 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 4 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 5 ) );  //	]==OO =[
        places.push_back( make_pair( 3, 1 ) );  //	]=    =[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
	}
	else if (type == 40) {		// Layers B
        places.push_back( make_pair( 1, 1 ) );  //	]    ==[
        places.push_back( make_pair( 1, 2 ) );  //	]=OO=O=[
        places.push_back( make_pair( 1, 3 ) );  //	]=OO=O=[
        places.push_back( make_pair( 1, 4 ) );  //	]=OO=O=[
        places.push_back( make_pair( 2, 1 ) );  //	]=OO=O=[
        places.push_back( make_pair( 2, 2 ) );  //	]=     [
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 2, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 41) {		// Layers C
        places.push_back( make_pair( 1, 1 ) );  //	]    ==[
        places.push_back( make_pair( 1, 2 ) );  //	]=O=OO=[
        places.push_back( make_pair( 1, 3 ) );  //	]=O=OO=[
        places.push_back( make_pair( 1, 4 ) );  //	]=O=OO=[
        places.push_back( make_pair( 3, 1 ) );  //	]=O=OO=[
        places.push_back( make_pair( 3, 2 ) );  //	]=     [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 42) {		// Layers D
        places.push_back( make_pair( 1, 4 ) );  //	]=OO===[
        places.push_back( make_pair( 1, 5 ) );  //	]=OO===[
        places.push_back( make_pair( 2, 2 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 3 ) );  //	] =OOOO[
        places.push_back( make_pair( 2, 4 ) );  //	]====OO[
        places.push_back( make_pair( 2, 5 ) );  //	]== ===[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 43) {		// Layers E
        places.push_back( make_pair( 0, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 0, 4 ) );  //	]OO====[
        places.push_back( make_pair( 1, 3 ) );  //	]OOOO= [
        places.push_back( make_pair( 1, 4 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 2 ) );  //	]===OO=[
        places.push_back( make_pair( 2, 3 ) );  //	]===OO=[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
	}
    else if (type == 44) {		// Layers F
        places.push_back( make_pair( 2, 1 ) );  //	]    ==[
        places.push_back( make_pair( 2, 2 ) );  //	]==OOO=[
        places.push_back( make_pair( 2, 3 ) );  //	]==OOO=[
        places.push_back( make_pair( 2, 4 ) );  //	]==OOO=[
        places.push_back( make_pair( 3, 1 ) );  //	]==OOO=[
        places.push_back( make_pair( 3, 2 ) );  //	]=     [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 45) {		// Layers G
        places.push_back( make_pair( 1, 3 ) );  //	]   ===[
        places.push_back( make_pair( 1, 4 ) );  //	] OOO= [
        places.push_back( make_pair( 2, 1 ) );  //	] OOO= [
        places.push_back( make_pair( 2, 2 ) );  //	] =OOO [
        places.push_back( make_pair( 2, 3 ) );  //	] =OOO [
        places.push_back( make_pair( 2, 4 ) );  //	]===   [
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
    else { generateBoxes( amt, 0 ); return; }
    }

    while ( amt > 0 && !places.empty() ) {
        int randPlace = rand() % places.size();
        xPos = places[randPlace].first;
        yPos = places[randPlace].second;
        if ( xPos != -1 && yPos != -1 ) {
            if ( map[xPos][yPos].item != 0 ) {
                map[xPos][yPos].boxType = 2;
                if ( map[xPos][yPos].boxHP >= 3 ) { places.erase( places.begin() + randPlace ); }
                else {
                    map[xPos][yPos].boxHP++;
                    amt--;
                }
            }
            else {
                if ( map[xPos][yPos].boxHP >= 3 ) { map[xPos][yPos].boxType = 1; }
                if ( map[xPos][yPos].boxHP >= 5 ) { places.erase( places.begin() + randPlace ); }
                else {
                    map[xPos][yPos].boxHP++;
                    amt--;
                }
            }
        }
    }
}
void App::generateFloor(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
    vector<pair<int, int>> places;
    {
	// Rooms
	if (type == 0) {				// Rooms A
        places.push_back( make_pair( 3, 0 ) );  //	] ==X==[
        places.push_back( make_pair( 3, 1 ) );  //	] ==X  [
        places.push_back( make_pair( 3, 2 ) );  //	] ==X==[
        places.push_back( make_pair( 3, 3 ) );  //	] ==X==[
        places.push_back( make_pair( 3, 4 ) );  //	]   X==[
        places.push_back( make_pair( 3, 5 ) );  //	]===X==[
	}
	else if (type == 1) {			// Rooms B
        places.push_back( make_pair( 2, 3 ) );  //	]==X===[
        places.push_back( make_pair( 2, 4 ) );  //	]==X=  [
        places.push_back( make_pair( 2, 5 ) );  //	]==X=  [
        places.push_back( make_pair( 3, 0 ) );  //	]  =X==[
        places.push_back( make_pair( 3, 1 ) );  //	]  =X==[
        places.push_back( make_pair( 3, 2 ) );  //	]===X==[
	}
	else if (type == 2) {			// Rooms C
        places.push_back( make_pair( 0, 5 ) );  //	]X== ==[
        places.push_back( make_pair( 1, 2 ) );  //	]===X==[
        places.push_back( make_pair( 2, 1 ) );  //	]====X [
        places.push_back( make_pair( 3, 4 ) );  //	] X====[
        places.push_back( make_pair( 4, 3 ) );  //	]==X===[
        places.push_back( make_pair( 5, 0 ) );  //	]== ==X[
	}
	else if (type == 3) {			// Rooms D
        places.push_back( make_pair( 1, 2 ) );  //	]=    =[
        places.push_back( make_pair( 1, 3 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 1 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 3, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 4 ) );  //	]=    =[
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
	else if (type == 4) {			// Rooms E
        places.push_back( make_pair( 0, 2 ) );  //	]====X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X=X=X[
        places.push_back( make_pair( 2, 0 ) );  //	]==XXX=[
        places.push_back( make_pair( 2, 3 ) );  //	]X==X==[
        places.push_back( make_pair( 3, 2 ) );  //	]====X=[
        places.push_back( make_pair( 3, 3 ) );  //	]==X===[
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 5) {			// Rooms F
        places.push_back( make_pair( 0, 2 ) );  //	] =X=X=[
        places.push_back( make_pair( 1, 0 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 1 ) );  //	]==XX=X[
        places.push_back( make_pair( 1, 4 ) );  //	]X=XX==[
        places.push_back( make_pair( 2, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 3 ) );  //	]=X=X= [
        places.push_back( make_pair( 2, 5 ) );
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	// Plus
	else if (type == 6) {			// Plus A
        places.push_back( make_pair( 1, 2 ) );  //	] =X=X=[
        places.push_back( make_pair( 1, 4 ) );  //	] X===X[
        places.push_back( make_pair( 2, 1 ) );  //	]===X==[
        places.push_back( make_pair( 2, 5 ) );  //	]=X===X[
        places.push_back( make_pair( 3, 3 ) );  //	]==X=X=[
        places.push_back( make_pair( 4, 1 ) );  //	]====  [
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 7) {			// Plus B
        places.push_back( make_pair( 1, 1 ) );  //	] =XX==[
        places.push_back( make_pair( 1, 3 ) );  //	] X====[
        places.push_back( make_pair( 1, 4 ) );  //	] X=X=X[
        places.push_back( make_pair( 2, 5 ) );  //	]=====X[
        places.push_back( make_pair( 3, 1 ) );  //	]=X=XX=[
        places.push_back( make_pair( 3, 3 ) );  //	]===   [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 8) {			// Plus C
        places.push_back( make_pair( 1, 3 ) );  //	] ==XX=[
        places.push_back( make_pair( 2, 2 ) );  //	] ====X[
        places.push_back( make_pair( 3, 1 ) );  //	] X=X=X[
        places.push_back( make_pair( 3, 3 ) );  //	]==X===[
        places.push_back( make_pair( 3, 5 ) );  //	]===X==[
        places.push_back( make_pair( 4, 5 ) );  //	]===   [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 9) {			// Plus D
        places.push_back( make_pair( 2, 2 ) );  //	] =X===[
        places.push_back( make_pair( 2, 3 ) );  //	] =X===[
        places.push_back( make_pair( 2, 4 ) );  //	] =X===[
        places.push_back( make_pair( 2, 5 ) );  //	]==XXXX[
        places.push_back( make_pair( 3, 2 ) );  //	]======[
        places.push_back( make_pair( 4, 2 ) );  //	]===   [
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 10) {			// Plus E
        places.push_back( make_pair( 1, 2 ) );  // ] =====[
        places.push_back( make_pair( 2, 1 ) );  // ] =X=X=[
        places.push_back( make_pair( 2, 3 ) );  // ] =X===[
        places.push_back( make_pair( 2, 4 ) );  // ]=X=XX=[
        places.push_back( make_pair( 3, 2 ) );  // ]==X===[
        places.push_back( make_pair( 4, 2 ) );  // ]===   [
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 11) {			// Plus F
        places.push_back( make_pair( 0, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 1, 2 ) );  //	]=XX===[
        places.push_back( make_pair( 1, 4 ) );  //	]X==X= [
        places.push_back( make_pair( 2, 1 ) );  //	]=XX=X=[
        places.push_back( make_pair( 2, 2 ) );  //	]==X=X=[
        places.push_back( make_pair( 2, 4 ) );  //	]===X==[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
    else if (type == 12) {			// Plus G
        places.push_back( make_pair( 0, 3 ) );
        places.push_back( make_pair( 1, 3 ) );  //	]  =X==[
        places.push_back( make_pair( 2, 3 ) );  //	]===X==[
        places.push_back( make_pair( 3, 0 ) );  //	]XXX=XX[
        places.push_back( make_pair( 3, 1 ) );  //	]===X==[
        places.push_back( make_pair( 3, 2 ) );  //	]= =X= [
        places.push_back( make_pair( 3, 4 ) );  //	]===X= [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	// Paths
	else if (type == 13) {			// Paths A
        places.push_back( make_pair( 0, 3 ) );  //	]X=====[
        places.push_back( make_pair( 0, 4 ) );  //	]X=====[
        places.push_back( make_pair( 0, 5 ) );  //	]X==  =[
        places.push_back( make_pair( 2, 0 ) );  //	]=  ===[
        places.push_back( make_pair( 2, 1 ) );  //	]==X===[
                                                //	]==X===[
	}
	else if (type == 14) {			// Paths B
        places.push_back( make_pair( 1, 0 ) );  //	]=X====[
        places.push_back( make_pair( 1, 1 ) );  //	]=X====[
        places.push_back( make_pair( 1, 3 ) );  //	]=X=  =[
        places.push_back( make_pair( 1, 4 ) );  //	]==  ==[
        places.push_back( make_pair( 1, 5 ) );  //	]=X====[
                                                //	]=X====[
	}
	else if (type == 15) {			// Paths C
        places.push_back( make_pair( 0, 4 ) );  //	] X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]XX==XX[
        places.push_back( make_pair( 1, 5 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 2 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]  ==  [
        places.push_back( make_pair( 3, 2 ) );  //	]====  [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 16) {			// Paths D
        places.push_back( make_pair( 0, 2 ) );  //	]    ==[
        places.push_back( make_pair( 1, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 1, 2 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]XX==XX[
        places.push_back( make_pair( 2, 4 ) );  //	] X==X [
        places.push_back( make_pair( 3, 3 ) );  //	]===== [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
    else if (type == 17) {		// Paths E
        places.push_back( make_pair( 1, 4 ) );  //	]  X===[
        places.push_back( make_pair( 2, 0 ) );  //	]=XX===[
        places.push_back( make_pair( 2, 1 ) );  //	]===X X[
        places.push_back( make_pair( 2, 4 ) );  //	]===X X[
        places.push_back( make_pair( 2, 5 ) );  //	]= X===[
        places.push_back( make_pair( 3, 2 ) );  //	]= X===[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 18) {		// Paths F
        places.push_back( make_pair( 0, 1 ) );  //	]===X =[
        places.push_back( make_pair( 0, 2 ) );  //	]X==X =[
        places.push_back( make_pair( 0, 3 ) );  //	]X X===[
        places.push_back( make_pair( 0, 4 ) );  //	]X X===[
        places.push_back( make_pair( 2, 2 ) );  //	]X==X =[
        places.push_back( make_pair( 2, 3 ) );  //	]===X =[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 3, 5 ) );
	}
    else if (type == 19) {		// Paths G
        places.push_back( make_pair( 0, 1 ) );  //	] X==X=[
        places.push_back( make_pair( 0, 4 ) );  //	]XX==XX[
        places.push_back( make_pair( 1, 0 ) );  //	]==XX==[
        places.push_back( make_pair( 1, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 1, 4 ) );  //	]XX==XX[
        places.push_back( make_pair( 1, 5 ) );  //	]=X==X [
        places.push_back( make_pair( 2, 2 ) );
        places.push_back( make_pair( 2, 3 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	// Cross
	else if (type == 20) {			// Cross A
        places.push_back( make_pair( 0, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 1, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]X=XX=X[
        places.push_back( make_pair( 2, 3 ) );  //	]=X==X=[
        places.push_back( make_pair( 3, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 4, 2 ) );  //	]=== ==[
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 21) {			// Cross B
        places.push_back( make_pair( 2, 1 ) );  //	]===X==[
        places.push_back( make_pair( 2, 4 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 0 ) );  //	]===X==[
        places.push_back( make_pair( 3, 2 ) );  //	]===X==[
        places.push_back( make_pair( 3, 3 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 5 ) );  //	]===X==[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 22) {			// Cross C
        places.push_back( make_pair( 1, 2 ) );  //	]==X===[
        places.push_back( make_pair( 1, 3 ) );  //	]==X===[
        places.push_back( make_pair( 2, 0 ) );  //	] X=X  [
        places.push_back( make_pair( 2, 1 ) );  //	] X=X  [
        places.push_back( make_pair( 2, 4 ) );  //	]==X===[
        places.push_back( make_pair( 2, 5 ) );  //	]==X===[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
	}
	else if (type == 23) {			// Cross D
        places.push_back( make_pair( 0, 2 ) );  //	]== ===[
        places.push_back( make_pair( 0, 3 ) );  //	]==X== [
        places.push_back( make_pair( 1, 2 ) );  //	]XX=XX [
        places.push_back( make_pair( 1, 3 ) );  //	]XX=XX [
        places.push_back( make_pair( 2, 1 ) );  //	]==X== [
        places.push_back( make_pair( 2, 4 ) );  //	]===== [
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
    else if (type == 24) {			// Cross E
        places.push_back( make_pair( 1, 2 ) );  //	]  == =[
        places.push_back( make_pair( 2, 0 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]=X==X [
        places.push_back( make_pair( 2, 4 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 0 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
    else if (type == 25) {		// Cross F
        places.push_back( make_pair( 1, 2 ) );  //	] =X=X=[
        places.push_back( make_pair( 2, 1 ) );  //	] = X=X[
        places.push_back( make_pair( 2, 3 ) );  //	]==X=X=[
        places.push_back( make_pair( 2, 5 ) );  //	]=X=X =[
        places.push_back( make_pair( 3, 2 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 4 ) );  //	]====  [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 26) {		// Cross G
        places.push_back( make_pair( 1, 4 ) );  //	] =X===[
        places.push_back( make_pair( 2, 3 ) );  //	]=X=X =[
        places.push_back( make_pair( 2, 5 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 2 ) );  //	]=  X=X[
        places.push_back( make_pair( 3, 4 ) );  //	]=  =X=[
        places.push_back( make_pair( 4, 1 ) );  //	]===== [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	// Gallery
	else if (type == 27) {			// Gallery A
        places.push_back( make_pair( 0, 1 ) );  //	]======[
        places.push_back( make_pair( 0, 2 ) );  //	]X====X[
        places.push_back( make_pair( 0, 3 ) );  //	]X====X[
        places.push_back( make_pair( 0, 4 ) );  //	]X====X[
        places.push_back( make_pair( 5, 1 ) );  //	]X====X[
        places.push_back( make_pair( 5, 2 ) );  //	]======[
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 28) {			// Gallery B
        places.push_back( make_pair( 2, 1 ) );  //	]======[
        places.push_back( make_pair( 2, 2 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 4 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 2 ) );  //	]======[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 3, 4 ) );
	}
	else if (type == 29) {			// Gallery C
        places.push_back( make_pair( 1, 1 ) );  //	]======[
        places.push_back( make_pair( 1, 4 ) );  //	]=XXXX=[
        places.push_back( make_pair( 2, 1 ) );  //	]======[
        places.push_back( make_pair( 2, 4 ) );  //	]======[
        places.push_back( make_pair( 3, 1 ) );  //	]=XXXX=[
        places.push_back( make_pair( 3, 4 ) );  //	]======[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 30) {			// Gallery D
        places.push_back( make_pair( 1, 1 ) );  //	]======[
        places.push_back( make_pair( 1, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 3 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 4, 1 ) );  //	]=X==X=[
        places.push_back( make_pair( 4, 2 ) );  //	]======[
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
    else if (type == 31) {		// Gallery E
        places.push_back( make_pair( 1, 2 ) );  //	]======[
        places.push_back( make_pair( 1, 3 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 1 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 3, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 4 ) );  //	]======[
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
    else if (type == 32) {		// Gallery F
        places.push_back( make_pair( 1, 3 ) );  //	]==X===[
        places.push_back( make_pair( 2, 3 ) );  //	]==X===[
        places.push_back( make_pair( 2, 4 ) );  //	]=XX===[
        places.push_back( make_pair( 2, 5 ) );  //	]===XX=[
        places.push_back( make_pair( 3, 0 ) );  //	]===X==[
        places.push_back( make_pair( 3, 1 ) );  //	]===X==[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 4, 2 ) );
	}
	// Scattered
	else if (type == 33) {			// Scattered A
        places.push_back( make_pair( 2, 0 ) );  //	]===X==[
        places.push_back( make_pair( 2, 2 ) );  //	]==X=X=[
        places.push_back( make_pair( 2, 4 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 1 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 3 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 5 ) );  //	]==X=X=[
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 34) {			// Scattered B
        places.push_back( make_pair( 0, 3 ) );  //	]X=X=X=[
        places.push_back( make_pair( 0, 5 ) );  //	]=X=X=X[
        places.push_back( make_pair( 1, 2 ) );  //	]X=X=X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X=X=X[
        places.push_back( make_pair( 2, 3 ) );  //	]======[
        places.push_back( make_pair( 2, 5 ) );  //	]======[
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 35) {			// Scattered C
        places.push_back( make_pair( 0, 4 ) );  //	]   X==[
        places.push_back( make_pair( 1, 1 ) );  //	]X=X=X=[
        places.push_back( make_pair( 2, 2 ) );  //	]=X=X=X[
        places.push_back( make_pair( 2, 4 ) );  //	]==X=X [
        places.push_back( make_pair( 3, 1 ) );  //	]===X= [
        places.push_back( make_pair( 3, 3 ) );  //	]====X [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 36) {			// Scattered D
        places.push_back( make_pair( 1, 3 ) );  //	] X=X==[
        places.push_back( make_pair( 1, 5 ) );  //	] =X=X=[
        places.push_back( make_pair( 2, 2 ) );  //	]=X=X=X[
        places.push_back( make_pair( 2, 4 ) );  //	]==X=X=[
        places.push_back( make_pair( 3, 1 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 3 ) );  //	]====  [
        places.push_back( make_pair( 3, 5 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 37) {			// Scattered E
        places.push_back( make_pair( 0, 5 ) );  //	]X=X=X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X=X=X[
        places.push_back( make_pair( 2, 3 ) );  //	]==X=X=[
        places.push_back( make_pair( 2, 5 ) );  //	] ==X=X[
        places.push_back( make_pair( 3, 2 ) );  //	]====X=[
        places.push_back( make_pair( 3, 4 ) );  //	]== ==X[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 5 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 2 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 38) {		// Scattered F
        places.push_back( make_pair( 0, 4 ) );  //	]=X= ==[
        places.push_back( make_pair( 1, 3 ) );  //	]X=X===[
        places.push_back( make_pair( 1, 5 ) );  //	]=X=X= [
        places.push_back( make_pair( 2, 2 ) );  //	] =X=X=[
        places.push_back( make_pair( 2, 4 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 1 ) );  //	]== =X=[
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	// Layers
	else if (type == 39) {			// Layers A
        places.push_back( make_pair( 1, 1 ) );  //	]= ==X=[
        places.push_back( make_pair( 1, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 3 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 4, 2 ) );  //	]=X== =[
        places.push_back( make_pair( 4, 3 ) );  //	]=    =[
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 40) {			// Layers B
        places.push_back( make_pair( 3, 1 ) );  //	]    ==[
        places.push_back( make_pair( 3, 2 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 3 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 4 ) );  //	]===X=X[
        places.push_back( make_pair( 5, 1 ) );  //	]===X=X[
        places.push_back( make_pair( 5, 2 ) );//	]=     [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 41) {			// Layers C
        places.push_back( make_pair( 2, 1 ) );  //	]    ==[
        places.push_back( make_pair( 2, 2 ) );  //	]==X==X[
        places.push_back( make_pair( 2, 3 ) );  //	]==X==X[
        places.push_back( make_pair( 2, 4 ) );  //	]==X==X[
        places.push_back( make_pair( 5, 1 ) );  //	]==X==X[
        places.push_back( make_pair( 5, 2 ) );  //	]=     [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 42) {			// Layers D
        places.push_back( make_pair( 0, 4 ) );  //	]X=====[
        places.push_back( make_pair( 0, 5 ) );  //	]X=====[
        places.push_back( make_pair( 1, 2 ) );  //	]X=====[
        places.push_back( make_pair( 1, 3 ) );  //	]=X====[
        places.push_back( make_pair( 2, 1 ) );  //	] X====[
        places.push_back( make_pair( 3, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 4, 0 ) );  //	]== =XX[
        places.push_back( make_pair( 5, 0 ) );
	}
	else if (type == 43) {			// Layers E
        places.push_back( make_pair( 0, 5 ) );  //	]XX= ==[
        places.push_back( make_pair( 1, 5 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 4 ) );  //	]====X [
        places.push_back( make_pair( 3, 4 ) );  //	]====X=[
        places.push_back( make_pair( 4, 2 ) );  //	]=====X[
        places.push_back( make_pair( 4, 3 ) );  //	]=====X[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    else if (type == 44) {			// Layers F
        places.push_back( make_pair( 1, 1 ) );  //	]    ==[
        places.push_back( make_pair( 1, 2 ) );  //	]=X===X[
        places.push_back( make_pair( 1, 3 ) );  //	]=X===X[
        places.push_back( make_pair( 1, 4 ) );  //	]=X===X[
        places.push_back( make_pair( 5, 1 ) );  //	]=X===X[
        places.push_back( make_pair( 5, 2 ) );  //	]=     [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
    else if (type == 45) {			// Layers G
        places.push_back( make_pair( 1, 1 ) );  //	]   ===[
        places.push_back( make_pair( 1, 2 ) );  //	] ===X [
        places.push_back( make_pair( 4, 3 ) );  //	] ===X [
        places.push_back( make_pair( 4, 4 ) );  //	] X=== [
                                                //	] X=== [
                                                //	]===   [
	}
    else { generateFloor( amt, 0 ); return; }
    }

    while (amt > 0 && !places.empty()) {
		int randPlace = rand() % places.size();
        xPos = places[randPlace].first;
        yPos = places[randPlace].second;
		if (xPos != -1 && yPos != -1) {
			map[xPos][yPos].state = 1;
            places.erase( places.begin() + randPlace );
			amt--;
        }
	}
}

void App::spawnRandomItem( int xPos, int yPos ) {
    bool spawnedItem = false;
    for ( int i = 0; i < 30; i++ ) {
        int randX = rand() % 3 - 1 + xPos;      if ( randX < 0 ) randX = 0;
                                                if ( randX > 5 ) randX = 5;
        int randY = rand() % 3 - 1 + yPos;      if ( randY < 0 ) randY = 0;
                                                if ( randY > 5 ) randY = 5;
        if ( isTileValid( randX, randY ) && map[randX][randY].item == 0 && !spawnedItem ) {
            map[randX][randY].item = 1;
            spawnedItem = true;
        }
    }
    if ( spawnedItem ) return;

    for ( int i = 0; i < 30; i++ ) {
        int randX = rand() % 5 - 2 + xPos;      if ( randX < 0 ) randX = 0;
                                                if ( randX > 5 ) randX = 5;
        int randY = rand() % 5 - 2 + yPos;      if ( randY < 0 ) randY = 0;
                                                if ( randY > 5 ) randY = 5;
        if ( isTileValid( randX, randY ) && map[randX][randY].item == 0 && !spawnedItem ) {
            map[randX][randY].item = 1;
            spawnedItem = true;
        }
    }
    if ( spawnedItem ) return;

    for ( int i = 0; i < 30; i++ ) {
        int randX = rand() % 7 - 3 + xPos;      if ( randX < 0 ) randX = 0;
                                                if ( randX > 5 ) randX = 5;
        int randY = rand() % 7 - 3 + yPos;      if ( randY < 0 ) randY = 0;
                                                if ( randY > 5 ) randY = 5;
        if ( isTileValid( randX, randY ) && map[randX][randY].item == 0 && !spawnedItem ) {
            map[randX][randY].item = 1;
            spawnedItem = true;
        }
    }
    if ( spawnedItem ) return;

    for ( int i = 0; i < 30; i++ ) {
        int randX = rand() % 9 - 4 + xPos;      if ( randX < 0 ) randX = 0;
                                                if ( randX > 5 ) randX = 5;
        int randY = rand() % 9 - 4 + yPos;      if ( randY < 0 ) randY = 0;
                                                if ( randY > 5 ) randY = 5;
        if ( isTileValid( randX, randY ) && map[randX][randY].item == 0 && !spawnedItem ) {
            map[randX][randY].item = 1;
            spawnedItem = true;
        }
    }
}
void App::upgradeItems() {
    bool playSound = false;
    for ( int i = 0; i < 6; i++ ) {
        for ( int j = 0; j < 6; j++ ) {
            if ( map[i][j].item == -1 ) {
                map[i][j].item = 1;
                map[i][j].upgradeInd = itemUpgradeTime;
                playSound = true;
            }
            else if ( map[i][j].item == 1 ) {
                map[i][j].item = 2;
                map[i][j].upgradeInd = itemUpgradeTime;
                playSound = true;
            }
        }
    }
    if ( playSound ) { Mix_PlayChannel( 1, recoverSound, 0 ); }
}

void App::aiAction( Player &npc ) {      // Generates a random move for the NPCs     // Checks for Attack options before Movement options
    int minEnergy = 0;

    // Megaman NPC
    if ( npc.type == 0 ) {
        minEnergy = 75;
        if ( rand() % 2 ) {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x - player1.x2 == 1 || npc.x - player1.x2 == 2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( lifeAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x2 ) <= 1 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( player1.x2 > npc.x ) && ( rand() % 2 ) ) {}
                    else if ( spinAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( player1.x2 + npc.x == 1 || player1.x2 + npc.x == 2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( lifeAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x2 ) <= 1 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( player1.x2 < npc.x ) && ( rand() % 2 ) ) {}
                    else if ( spinAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
        }
        else {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x - player1.x == 1 || npc.x - player1.x == 2 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( lifeAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x ) <= 1 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( spinAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x == player1.x || npc.x - 2 == player1.x ) ) ) {
                    if ( crossAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x - 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( swordAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x + player1.x == 1 || npc.x + player1.x == 2 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( lifeAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x ) <= 1 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( spinAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x == player1.x || npc.x + 2 == player1.x ) ) ) {
                    if ( crossAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x + 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( swordAtk( npc ) ) return; }
            }
        }
    }
    
    // Protoman NPC
    else if ( npc.type == 1 ) {
        minEnergy = 75;
        if ( rand() % 2 ) {
            if ( npc.facing == 1 ) {
                if ( ( player1.x2 == npc.x - 2 ) && ( player1.y2 == npc.y ) ) {
                    if ( protoAtk( npc ) ) return; }
                if ( (player1.x2 < npc.x) && (player1.x2 >= npc.x - 3) && (npc.y == player1.y2) ) {
                    if ( (player1.x2 != npc.x - 2) && (rand() % 2) ) {}
                    else if ( heroAtk( npc ) ) return; }
                if ( ( npc.x - 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 && ( rand() % 2 ) ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( player1.x2 == npc.x + 2 ) && ( player1.y2 == npc.y ) ) {
                    if ( protoAtk( npc ) ) return; }
                if ( (player1.x2 > npc.x) && (player1.x2 <= npc.x + 3) && (npc.y == player1.y2) ) {
                    if ( (player1.x2 != npc.x + 2) && (rand() % 2) ) {}
                    if ( heroAtk( npc ) ) return; }
                if ( ( npc.x + 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
        }
        else {
            if ( npc.facing == 1 ) {
                if ( (player1.x < npc.x) && (player1.x >= npc.x - 3) && (npc.y == player1.y) ) {
                    if ( heroAtk( npc ) ) return; }
                if ( (npc.x - 2 == player1.x) && (abs(player1.y - npc.y) == 1) ) {
                    if ( protoAtk( npc ) ) return; }
                if ( ( npc.x - 3 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x - 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( swordAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( (player1.x > npc.x) && (player1.x <= npc.x + 3) && (npc.y == player1.y) ) {
                    if ( heroAtk( npc ) ) return; }
                if ( (npc.x + 2 == player1.x) && (abs(player1.y - npc.y) == 1) ) {
                    if ( protoAtk( npc ) ) return; }
                if ( ( npc.x + 3 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( stepAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x + 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( swordAtk( npc ) ) return; }
            }
        }
    }
    
    // Tomahawkman NPC
    else if ( npc.type == 2 ) {
        minEnergy = 75;
        if ( rand() % 2 ) {
            if ( npc.facing == 1 ) {
                if ( ( player1.x2 < npc.x ) && ( npc.y == player1.y2 ) && ( rand() % 2 ) ) {
                    if ( eTomaAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 || npc.x - 2 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( tomaAtkB2( npc ) ) return;
                    else if ( tomaAtkA2( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( tomaAtkB1( npc ) ) return;
                    else if ( tomaAtkA1( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( player1.x > npc.x2 ) && ( npc.y == player1.y2 ) && ( rand() % 2 ) ) {
                    if ( eTomaAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 || npc.x + 2 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( tomaAtkB2( npc ) ) return;
                    else if ( tomaAtkA2( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( tomaAtkB1( npc ) ) return;
                    else if ( tomaAtkA1( npc ) ) return; }
            }
        }
        else {
            if ( npc.facing == 1 ) {
                if ( ( player1.x < npc.x ) && ( npc.y == player1.y ) ) {
                    if ( eTomaAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x || npc.x - 2 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( tomaAtkB2( npc ) ) return;
                    if ( tomaAtkA2( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( tomaAtkB1( npc ) ) return;
                    if ( tomaAtkA1( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( player1.x > npc.x ) && ( npc.y == player1.y ) ) {
                    if ( eTomaAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x || npc.x + 2 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( tomaAtkB2( npc ) ) return;
                    if ( tomaAtkA2( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( tomaAtkB1( npc ) ) return;
                    if ( tomaAtkA1( npc ) ) return; }
            }
        }
    }
    
    // Colonel NPC
    else if ( npc.type == 3 ) {
        minEnergy = 75;
        if ( rand() % 2 ) {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( xDivideAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 3 ) ) {
                    if ( zDivideAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( xDivideAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 3 ) ) {
                    if ( zDivideAtk( npc ) ) return; }
            }
        }
        else {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x ) && ( npc.y == player1.y )
                || ( abs(npc.y - player1.y) == 1 && ( npc.x - 2 == player1.x || npc.x - 4 == player1.x ) ) ) {
                    if ( xDivideAtk( npc ) ) return; }
                if ( ( npc.x == player1.x && npc.y - 1 == player1.y )
                || ( npc.x - 1 == player1.x && npc.y == player1.y )
                || ( npc.x - 2 == player1.x && npc.y + 1 == player1.y ) ) {
                    if ( upDivideAtk( npc ) ) return; }
                if ( ( npc.x == player1.x && npc.y + 1 == player1.y )
                || ( npc.x - 2 == player1.x && npc.y - 1 == player1.y ) ) {
                    if ( downDivideAtk( npc ) )return; }
                if ( ( npc.x == player1.x && npc.y - 1 == player1.y )
                || ( npc.x - 1 == player1.x && npc.y == player1.y )
                || ( npc.x == player1.x && npc.y + 1 == player1.y ) ) {
                    if ( vDivideAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( zDivideAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x ) && ( npc.y == player1.y )
                || ( abs(npc.y - player1.y) == 1 && ( npc.x + 2 == player1.x || npc.x + 4 == player1.x ) ) ) {
                    if ( xDivideAtk( npc ) ) return; }
                if ( ( npc.x == player1.x && npc.y - 1 == player1.y )
                || ( npc.x + 1 == player1.x && npc.y == player1.y )
                || ( npc.x + 2 == player1.x && npc.y + 1 == player1.y ) ) {
                    if ( upDivideAtk( npc ) ) return; }
                if ( ( npc.x == player1.x && npc.y + 1 == player1.y )
                || ( npc.x + 2 == player1.x && npc.y - 1 == player1.y ) ) {
                    if ( downDivideAtk( npc ) )return; }
                if ( ( npc.x == player1.x && npc.y - 1 == player1.y )
                || ( npc.x + 1 == player1.x && npc.y == player1.y )
                || ( npc.x == player1.x && npc.y + 1 == player1.y ) ) {
                    if ( vDivideAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( zDivideAtk( npc ) ) return; }
            }
        }
    }
    
    // Slashman NPC
    else if ( npc.type == 4 ) {
        minEnergy = 75;
        if ( rand() % 2 ) {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( stepCrossAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x2 ) <= 1 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( player1.x2 > npc.x ) && ( rand() % 2 ) ) {}
                    else if ( spinSlashAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) && ( rand() % 2 ) ) {
                    if ( stepCrossAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x2 ) <= 1 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( player1.x2 < npc.x ) && ( rand() % 2 ) ) {}
                    else if ( spinSlashAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x2 ) && ( abs( npc.y - player1.y2 ) <= 1 ) ) {
                    if ( ( npc.y != player1.y2 ) && ( rand() % 2 ) ) {}
                    else if ( wideAtk( npc ) ) return; }
            }
        }
        else {
            if ( npc.facing == 1 ) {
                if ( ( npc.x - 3 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x - 2 == player1.x || npc.x - 4 == player1.x ) ) ) {
                    if ( stepCrossAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x ) <= 1 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( spinSlashAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x == player1.x || npc.x - 2 == player1.x ) ) ) {
                    if ( crossAtk( npc ) ) return; }
                if ( ( npc.x - 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x - 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
            }
            else if ( npc.facing == 3 ) {
                if ( ( npc.x + 3 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x + 2 == player1.x || npc.x + 4 == player1.x ) ) ) {
                    if ( stepCrossAtk( npc ) ) return; }
                if ( ( abs( npc.x - player1.x ) <= 1 ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( spinSlashAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( npc.y == player1.y )
                   || ( abs(npc.y - player1.y) == 1 && ( npc.x == player1.x || npc.x + 2 == player1.x ) ) ) {
                    if ( crossAtk( npc ) ) return; }
                if ( ( npc.x + 1 == player1.x ) && ( abs( npc.y - player1.y ) <= 1 ) ) {
                    if ( wideAtk( npc ) ) return; }
                if ( ( npc.x + 2 == player1.x ) && ( npc.y == player1.y ) ) {
                    if ( longAtk( npc ) ) return; }
            }
        }
    }

    // Movement
    if ( npc.energy < minEnergy ) {
        if      ( npc.x == npc.y ) {
            if ( rand() % 2 ) {
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 2 ) ) return;
                if ( move( npc, 1 ) ) return; }
            else {
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 1 ) ) return;
                if ( move( npc, 2 ) ) return; } }
        else if ( npc.x >  npc.y ) {
            if ( move( npc, 0 ) ) return;
            if ( move( npc, 3 ) ) return;
            if ( move( npc, 2 ) ) return;
            if ( move( npc, 1 ) ) return;
        }
        else if ( npc.x <  npc.y ) {
            if ( move( npc, 3 ) ) return;
            if ( move( npc, 0 ) ) return;
            if ( move( npc, 1 ) ) return;
            if ( move( npc, 2 ) ) return;
        }
    }
    if ( rand() % 2 ) {
        if      ( player1.y > npc.y && player1.x < npc.x ) {    // Up Left
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x - npc.x) <= abs(player1.y - npc.y) ) ) {
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 1 ) ) return; }
            else {
                if ( move( npc, 1 ) ) return;
                if ( move( npc, 0 ) ) return; }
        }
        else if ( player1.y > npc.y && player1.x > npc.x ) {    // Up Right
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x - npc.x) <= abs(player1.y - npc.y) ) ) {
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 3 ) ) return; }
            else {
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 0 ) ) return; }
        }
        else if ( player1.y < npc.y && player1.x < npc.x ) {    // Down Left
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x - npc.x) <= abs(player1.y - npc.y) ) ) {
                if ( move( npc, 2 ) ) return;
                if ( move( npc, 1 ) ) return; }
            else {
                if ( move( npc, 1 ) ) return;
                if ( move( npc, 2 ) ) return; }
        }
        else if ( player1.y < npc.y && player1.x > npc.x ) {    // Down Right
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x - npc.x) <= abs(player1.y - npc.y) ) ) {
                if ( move( npc, 2 ) ) return;
                if ( move( npc, 3 ) ) return; }
            else {
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 2 ) ) return; }
        }
        else if ( npc.facing == 1 ) {
            if      ( player1.x <  npc.x && player1.y == npc.y ) {        // Left
                if ( move( npc, 1 ) ) return;
                if ( npc.x - 1 == player1.x ) return;
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x >  npc.x && player1.y == npc.y ) {   // Right
                if ( move( npc, 3 ) ) return;
                if ( npc.x + 1 == player1.x ) {
                    face( npc, 3 ); return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x == npc.x && player1.y >  npc.y ) {   // Up
                if ( move( npc, 0 ) ) return;
                if ( npc.y + 1 == player1.y ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 2 ) ) return;
                    if ( npc.x == 5 ) { return; }
                    else              { face( npc, 3 ); return; } }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
            else if ( player1.x == npc.x && player1.y <  npc.y ) {   // Down
                if ( move( npc, 2 ) ) return;
                if ( npc.y - 1 == player1.y ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 0 ) ) return;
                    if ( npc.x == 5 ) { return; }
                    else              { face( npc, 3 ); return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
        }
        else if ( npc.facing == 3 ) {
            if      ( player1.x <  npc.x && player1.y == npc.y ) {        // Left
                if ( move( npc, 1 ) ) return;
                if ( npc.x - 1 == player1.x ) {
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x >  npc.x && player1.y == npc.y ) {   // Right
                if ( move( npc, 3 ) ) return;
                if ( npc.x + 1 == player1.x ) return;
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x == npc.x && player1.y >  npc.y ) {   // Up
                if ( move( npc, 0 ) ) return;
                if ( npc.y + 1 == player1.y ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 2 ) ) return;
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
            else if ( player1.x == npc.x && player1.y <  npc.y ) {   // Down
                if ( move( npc, 2 ) ) return;
                if ( npc.y - 1 == player1.y ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 0 ) ) return;
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
        }
    }
    else {
        if      ( player1.y2 > npc.y && player1.x2 < npc.x ) {    // Up Left
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x2 - npc.x) <= abs(player1.y2 - npc.y) ) ) {
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 1 ) ) return; }
            else {
                if ( move( npc, 1 ) ) return;
                if ( move( npc, 0 ) ) return; }
        }
        else if ( player1.y2 > npc.y && player1.x2 > npc.x ) {    // Up Right
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x2 - npc.x) <= abs(player1.y2 - npc.y) ) ) {
                if ( move( npc, 0 ) ) return;
                if ( move( npc, 3 ) ) return; }
            else {
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 0 ) ) return; }
        }
        else if ( player1.y2 < npc.y && player1.x2 < npc.x ) {    // Down Left
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x2 - npc.x) <= abs(player1.y2 - npc.y) ) ) {
                if ( move( npc, 2 ) ) return;
                if ( move( npc, 1 ) ) return; }
            else {
                if ( move( npc, 1 ) ) return;
                if ( move( npc, 2 ) ) return; }
        }
        else if ( player1.y2 < npc.y && player1.x2 > npc.x ) {    // Down Right
            if ( npc.type == 1 || npc.type == 2 || ( abs(player1.x2 - npc.x) <= abs(player1.y2 - npc.y) ) ) {
                if ( move( npc, 2 ) ) return;
                if ( move( npc, 3 ) ) return; }
            else {
                if ( move( npc, 3 ) ) return;
                if ( move( npc, 2 ) ) return; }
        }
        else if ( npc.facing == 1 ) {
            if      ( player1.x2 <  npc.x && player1.y2 == npc.y ) {        // Left
                if ( move( npc, 1 ) ) return;
                if ( npc.x - 1 == player1.x2 ) return;
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x2 >  npc.x && player1.y2 == npc.y ) {   // Right
                if ( move( npc, 3 ) ) return;
                if ( npc.x + 1 == player1.x2 ) {
                    face( npc, 3 ); return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x2 == npc.x && player1.y2 >  npc.y ) {   // Up
                if ( move( npc, 0 ) ) return;
                if ( npc.y + 1 == player1.y2 ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 2 ) ) return;
                    if ( npc.x == 5 ) { return; }
                    else              { face( npc, 3 ); return; } }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
            else if ( player1.x2 == npc.x && player1.y2 <  npc.y ) {   // Down
                if ( move( npc, 2 ) ) return;
                if ( npc.y - 1 == player1.y2 ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 0 ) ) return;
                    if ( npc.x == 5 ) { return; }
                    else              { face( npc, 3 ); return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
        }
        else if ( npc.facing == 3 ) {
            if      ( player1.x2 <  npc.x && player1.y2 == npc.y ) {        // Left
                if ( move( npc, 1 ) ) return;
                if ( npc.x - 1 == player1.x2 ) {
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x2 >  npc.x && player1.y2 == npc.y ) {   // Right
                if ( move( npc, 3 ) ) return;
                if ( npc.x + 1 == player1.x2 ) return;
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 0 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 2 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 0 ) ) return; }
                else {
                    if ( move( npc, 2 ) ) return; }
            }
            else if ( player1.x2 == npc.x && player1.y2 >  npc.y ) {   // Up
                if ( move( npc, 0 ) ) return;
                if ( npc.y + 1 == player1.y2 ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 2 ) ) return;
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y + 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y + 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
            else if ( player1.x2 == npc.x && player1.y2 <  npc.y ) {   // Down
                if ( move( npc, 2 ) ) return;
                if ( npc.y - 1 == player1.y2 ) {
                    if ( move( npc, 3 ) ) return;
                    if ( move( npc, 1 ) ) return;
                    if ( move( npc, 0 ) ) return;
                    if ( npc.x == 5 ) { face( npc, 1 ); return; }
                    else              { return; } }
                if ( !isTileValid( npc.x - 1, npc.y - 1, true ) ) {
                    if ( move( npc, 3 ) ) return; }
                if ( !isTileValid( npc.x + 1, npc.y - 1, true ) ) {
                    if ( move( npc, 1 ) ) return; }
                if ( rand() % 2 ) {
                    if ( move( npc, 1 ) ) return; }
                else {
                    if ( move( npc, 3 ) ) return; }
            }
        }
    }
}
