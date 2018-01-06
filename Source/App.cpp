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
const int stepCrossCost  = 150;
const int spinSlashCost  = 225;

const int startingEnergy = 300;
const int energyGainAmt  = 100;			    // How much Energy the player gains when moving to a Pick-Up
const int energyGainAmt2 = 150;
const int energyLossAmt  =  50;
const int itemWorth = energyGainAmt / 50;	// Item "Worth" is used to generate Levels

const float iconDisplayTime = 0.45;		// Energy Gain Icon Display time
const float boxDmgTime = 0.4;			// Box HP damage indicator time
const float moveAnimationTime = 0.175;	// Player movement time
const float pauseAnimationTime = moveAnimationTime * 2.5;
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

Player::Player() : x(0), y(0), facing(3), moving(false), turning(false), moveDir(-1), energy(0), type(0) {}
Tile::Tile() : state(0), item(-1), boxHP(0), boxType(0), boxAtkInd(0.0), prevDmg(0) {}
DelayedHpLoss::DelayedHpLoss() : dmg(0), xPos(0), yPos(0), delay(0.0) {}
DelayedHpLoss::DelayedHpLoss( int dmgAmt, int x, int y, float delayTime ) {
    dmg = dmgAmt;
    xPos = x;
    yPos = y;
    delay = delayTime;
}
DelayedSound::DelayedSound() : soundName(""), delay(0.0) {}
DelayedSound::DelayedSound( string name, float delayTime ) {
    soundName = name;
    delay = delayTime;
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
             player(Player()), energyDisplayed(0), level(0), currentEnergyGain(0), gameDiffSel(2), currentGameDiff(2),
			 animationType(0), selSwordAnimation(-1), musicSel(1), musicMuted(true),
			 animationDisplayAmt(0.0), chargeDisplayPlusAmt(0.0), chargeDisplayMinusAmt(0.0), currentSwordAtkTime(0.0),
			 itemAnimationAmt(0.0), bgAnimationAmt(0.0), musicSwitchDisplayAmt(0.0) {
	Init(); }
void App::Init() {
	SDL_Init(SDL_INIT_VIDEO);
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
    tmanHurtSheet     = loadTexture("Pics\\Characters\\Toma Hurt Sheet.png");
    slashmanMoveSheet = loadTexture("Pics\\Characters\\Slash Move Sheet.png");
    slashmanAtkSheet  = loadTexture("Pics\\Characters\\Slash Atk Sheet.png");
    slashmanHurtSheet = loadTexture("Pics\\Characters\\Slash Hurt Sheet.png");

	rockSheet        = loadTexture("Pics\\Rock Sheet 1.png");
	rockSheetItem    = loadTexture("Pics\\Rock Sheet 2.png");
    rockSheetItem2   = loadTexture("Pics\\Rock Sheet 2-2.png");
    rockSheetTrappedItem = loadTexture("Pics\\Rock Sheet 2-3.png");
	rockDeathSheet   = loadTexture("Pics\\Death Sheet.png");
	floorSheet       = loadTexture("Pics\\Tile Sheet 2.png");
	floorMoveSheet   = loadTexture("Pics\\Move Tile Sheet 2.png");
	floorBottomPic1  = loadTexture("Pics\\Tile Bottom 1.png");
	floorBottomPic2  = loadTexture("Pics\\Tile Bottom 2.png");
	energySheet      = loadTexture("Pics\\Energy Sheet.png");
    energySheet2     = loadTexture("Pics\\Energy Sheet 2.png");
    trappedEnergySheet = loadTexture("Pics\\Energy Sheet 3.png");
	bgA              = loadTexture("Pics\\Background A.png");
	bgB              = loadTexture("Pics\\Background B.png");
	bgC              = loadTexture("Pics\\Background C.png");
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

	Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 4, 4096 );
	Mix_Volume( 0, 48 );
    Mix_Volume( 1, 32 );
    Mix_Volume( 2, 32 );
    Mix_Volume( 3, 07 );
	
	swordSound      = Mix_LoadWAV( "Sounds\\SwordSwing.ogg" );
	lifeSwordSound  = Mix_LoadWAV( "Sounds\\LifeSword.ogg" );
    screenDivSound  = Mix_LoadWAV( "Sounds\\ScreenDivide.ogg" );
    tomahawkSound   = Mix_LoadWAV( "Sounds\\Tomahawk.ogg" );
    eTomaSound      = Mix_LoadWAV( "Sounds\\EToma.ogg" );
    spinSlashSound  = Mix_LoadWAV( "Sounds\\SpinSlash.ogg");

	itemSound       = Mix_LoadWAV( "Sounds\\GotItem.ogg" );
    itemSound2      = Mix_LoadWAV( "Sounds\\GotItem2.ogg" );
    trapItemSound   = Mix_LoadWAV( "Sounds\\TrappedItem.ogg" );
	rockBreakSound  = Mix_LoadWAV( "Sounds\\AreaGrabHit.ogg" );
	panelBreakSound = Mix_LoadWAV( "Sounds\\PanelCrack.ogg" );

	menuOpenSound   = Mix_LoadWAV( "Sounds\\ChipDesc.ogg" );
	menuCloseSound  = Mix_LoadWAV( "Sounds\\ChipDescClose.ogg" );

    quitCancelSound = Mix_LoadWAV( "Sounds\\QuitCancel.ogg" );
    quitChooseSound = Mix_LoadWAV( "Sounds\\QuitChoose.ogg" );
    quitOpenSound   = Mix_LoadWAV( "Sounds\\QuitOpen.ogg" );

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

	reset();
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
    if ( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) { energyDisplayed = player.energy; }
    else if ( energyDisplayed > player.energy ) { energyDisplayed -= 500 * elapsed; }
    else if ( energyDisplayed < player.energy ) { energyDisplayed += 500 * elapsed; }
    if ( abs( energyDisplayed - player.energy ) <= 500 * fixed_timestep ) { energyDisplayed = player.energy; }

    if ( chargeDisplayMinusAmt <= 0 && chargeDisplayPlusAmt <= 0 ) { energyDisplayed2 = currentEnergyGain; }
    else if ( energyDisplayed2 > currentEnergyGain ) { energyDisplayed2 -= 500 * elapsed; }
    else if ( energyDisplayed2 < currentEnergyGain ) { energyDisplayed2 += 500 * elapsed; }
    if ( abs( energyDisplayed2 - currentEnergyGain ) <= 500 * fixed_timestep ) { energyDisplayed2 = currentEnergyGain; }

	// Display parameter for Energy gain
	chargeDisplayPlusAmt -= elapsed;
	if (chargeDisplayPlusAmt < 0) { chargeDisplayPlusAmt = 0; }

	// Display parameter for Energy usage
	chargeDisplayMinusAmt -= elapsed;
	if (chargeDisplayMinusAmt < 0) { chargeDisplayMinusAmt = 0; }
    
	// Display parameter for attack, movement, and level transition animations
	animationDisplayAmt -= elapsed;
	// Invalid movement when trying to move back onto the Starting Tile
	if (player.x == -1 && player.y == 0 && animationDisplayAmt < 0) {
		animationType = 2;
		player.facing = 1;
		player.x = 0;
		animationDisplayAmt = moveAnimationTime; }
	// Level Transition parameter
	else if (player.x == 6 && player.y == 5 && animationDisplayAmt < 0) {
		animationType = 3;
		animationDisplayAmt = moveAnimationTime * 3; }
	// Reload default parameters when nothing is happening or being animated
	else if (animationDisplayAmt <= 0) {
		animationDisplayAmt = 0;
		player.moving = false;
		player.turning = false;
		player.moveDir = -1;
		animationType = 0; }

	// Handles Level Transition
	if (animationType == 3 && animationDisplayAmt > 0 && animationDisplayAmt <= moveAnimationTime * 2) {
		next();
		animationType = 4;
	}
	
	// Item Display parameter
	itemAnimationAmt += elapsed * 10;
	if (itemAnimationAmt > 8.0) { itemAnimationAmt = 0; }
	bgAnimationAmt += elapsed * 3.5;
	if (bgAnimationAmt > 8.0) { bgAnimationAmt = 0; }

	// Rock HP Indicators
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map[i][j].boxAtkInd -= elapsed;
			if (map[i][j].boxAtkInd < 0) { map[i][j].boxAtkInd = 0; }
			if (map[i][j].boxHP < 0) { map[i][j].boxHP = 0; }
	} }

	// Handles delayed Damage to Rocks
	for (int i = 0; i < delayedHpList.size(); i++) {
		delayedHpList[i].delay -= elapsed;
		if (delayedHpList[i].delay <= 0) {
			hitBox(delayedHpList[i].xPos, delayedHpList[i].yPos, delayedHpList[i].dmg);
			delayedHpList.erase(delayedHpList.begin() + i);
	}	}

	// Handles delayed Sounds
	for ( int i = 0; i < delayedSoundList.size(); i++ ) {
		delayedSoundList[i].delay -= elapsed;
		if ( delayedSoundList[i].delay <= 0 ) {
			if      ( delayedSoundList[i].soundName == "sword" )  { Mix_PlayChannel( 0, swordSound, 0 ); }
			else if ( delayedSoundList[i].soundName == "life" )   { Mix_PlayChannel( 0, lifeSwordSound, 0 ); }
            else if ( delayedSoundList[i].soundName == "divide" ) { Mix_PlayChannel( 0, screenDivSound, 0 ); }
            else if ( delayedSoundList[i].soundName == "toma" )   { Mix_PlayChannel( 0, tomahawkSound, 0 ); }
            else if ( delayedSoundList[i].soundName == "eToma" )  { Mix_PlayChannel( 0, eTomaSound, 0 ); }
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
	// Menu Controls
	while (SDL_PollEvent(&event)) {
		if      ( event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE ) { done = true; }
		else if ( event.type == SDL_KEYDOWN && animationDisplayAmt <= 0 ) {		// Read Single Button presses
            if      ( event.key.keysym.scancode == SDL_SCANCODE_C ) { changeMusic(); }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_V ) { toggleMusic(); }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) {
                if      ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else                      { quitMenuOn    = true;  Mix_PlayChannel( 1, quitOpenSound, 0 ); menuSel = true; }
                animationDisplayAmt = menuExitTime;
            }
            else if ( animationDisplayAmt <= 0 && (event.key.keysym.scancode == SDL_SCANCODE_TAB || event.key.keysym.scancode == SDL_SCANCODE_F) ) {
                if ( menuDisplay ) { menuDisplay = false; Mix_PlayChannel( 1, menuCloseSound, 0 ); }
                else               { menuDisplay = true;  Mix_PlayChannel( 1, menuOpenSound, 0 ); }
                animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_R ) {
                if      ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else                      { resetMenuOn   = true;  Mix_PlayChannel( 1, quitOpenSound, 0 ); menuSel = true; }
                animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_X ) {
                if      ( trainMenuOn )   { trainMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( quitMenuOn )    { quitMenuOn    = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( resetMenuOn )   { resetMenuOn   = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( diffSelMenuOn ) { diffSelMenuOn = false; Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else                      { trainMenuOn   = true;  Mix_PlayChannel( 1, quitOpenSound, 0 ); menuSel = true; }
                animationDisplayAmt = menuExitTime;
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_W || event.key.keysym.scancode == SDL_SCANCODE_UP ) {
                if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel > 1 ) {       charSel -= 2;      Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_S || event.key.keysym.scancode == SDL_SCANCODE_DOWN ) {
                if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel < 3 ) {       charSel += 2;      Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_A || event.key.keysym.scancode == SDL_SCANCODE_LEFT ) {
                if ( quitMenuOn ) {
                    if ( !menuSel ) {          menuSel = true;    Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
                else if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel > 0 && charSel % 2 == 1) {
                                               charSel--;         Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
                else if ( diffSelMenuOn ) {
                    if ( gameDiffSel > 0 ) {   gameDiffSel--;     Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
            }
            else if ( event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_RIGHT ) {
                if ( quitMenuOn ) {
                    if ( menuSel ) {           menuSel = false;   Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
                else if ( resetMenuOn || trainMenuOn ) {
                    if ( charSel < 3 && charSel % 2 == 0 ) {
                                               charSel++;         Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
                else if ( diffSelMenuOn ) {
                    if ( gameDiffSel < 4 ) {   gameDiffSel++;     Mix_PlayChannel( 1, quitChooseSound, 0 );   }   }
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
                        Mix_PlayChannel( 1, quitCancelSound, 0 );
                        animationDisplayAmt = menuExitTime; }
                }
                else if ( resetMenuOn ) { 
                    if ( menuSel ) {
                        resetMenuOn = false;
                        diffSelMenuOn = true;
                        gameDiffSel = currentGameDiff;
                        Mix_PlayChannel( 1, quitChooseSound, 0 ); }
                    else {
                        resetMenuOn = false;
                        Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                    animationDisplayAmt = menuExitTime;
                }
                else if ( diffSelMenuOn ) {
                    diffSelMenuOn = false;
                    player.type = charSel;
                    currentGameDiff = gameDiffSel;
                    reset();
                    Mix_PlayChannel( 1, quitChooseSound, 0 );
                    animationDisplayAmt = menuExitTime;
                }
                else if ( trainMenuOn ) {
                    trainMenuOn = false;
                    player.type = charSel;
                    test();
                    Mix_PlayChannel( 1, quitChooseSound, 0 );
                    animationDisplayAmt = menuExitTime;
                }
            }
            else if ( ( event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_SPACE ) && 
                        !quitMenuOn && !resetMenuOn && !diffSelMenuOn && !trainMenuOn && animationDisplayAmt <= 0 ) {
                if      ( player.facing == 1 ) { face( 3 ); }
                else if ( player.facing == 3 ) { face( 1 ); }
            }
	    }
    }

    // Player Movement and Attacks
    if ( animationDisplayAmt <= 0 && !quitMenuOn && !resetMenuOn && !diffSelMenuOn && !trainMenuOn ) {
        // The player can't Move or Attack until after the previous Action is completed, or if a Menu is open

	    const Uint8* keystates = SDL_GetKeyboardState(NULL);		// Read Multiple Button presses simultaneously

		animationType = 0;
		// Move Up
		if		(keystates[SDL_SCANCODE_W] || keystates[SDL_SCANCODE_UP]) { move(0); }
		// Move Left
		else if (keystates[SDL_SCANCODE_A] || keystates[SDL_SCANCODE_LEFT]) {
			if (player.facing != 1) {
				player.turning = true;
				face(1); }
			move(1);
		}
		// Move Down
		else if (keystates[SDL_SCANCODE_S] || keystates[SDL_SCANCODE_DOWN]) { move(2); }
		// Move Right
		else if (keystates[SDL_SCANCODE_D] || keystates[SDL_SCANCODE_RIGHT]) {
			if (player.facing != 3) {
				player.turning = true;
				face(3); }
			move(3);
		}

		// Sword Attacks
		else if (keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1]) {
            if      ( player.type == 0 || player.type == 1 ) { swordAtk( player.facing ); }
            else if ( player.type == 2 )                     { tomaAtkA1( player.facing ); }
            else if ( player.type == 3 )                     { vDivideAtk( player.facing ); }
            else if ( player.type == 4 )                     { longAtk( player.facing ); }
        }
		else if (keystates[SDL_SCANCODE_2] || keystates[SDL_SCANCODE_KP_2]) {
            if      ( player.type == 0 || player.type == 1 ) { longAtk( player.facing ); }
            else if ( player.type == 2 )                     { tomaAtkB1( player.facing); }
            else if ( player.type == 3 )                     { upDivideAtk( player.facing ); }
            else if ( player.type == 4 )                     { wideAtk( player.facing ); }
        }
		else if (keystates[SDL_SCANCODE_3] || keystates[SDL_SCANCODE_KP_3]) {
            if      ( player.type == 0 || player.type == 1 ) { wideAtk( player.facing ); }
            else if ( player.type == 2 )                     { tomaAtkA2( player.facing ); }
            else if ( player.type == 3 )                     { downDivideAtk( player.facing ); }
            else if ( player.type == 4 )                     { crossAtk( player.facing ); }
        }
		else if (keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4]) { 
            if      ( player.type == 0 ) { crossAtk( player.facing );   }
            else if ( player.type == 1 ) { stepAtk( player.facing );    }
            else if ( player.type == 2 ) { tomaAtkB2( player.facing ); }
            else if ( player.type == 3 ) { xDivideAtk( player.facing ); }
            else if ( player.type == 4 ) { stepCrossAtk( player.facing ); }
        }
		else if (keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5]) {
            if      ( player.type == 0 ) { spinAtk( player.facing );    }
            else if ( player.type == 1 ) { heroAtk( player.facing );    }
            else if ( player.type == 2 ) { eTomaAtk( player.facing ); }
            else if ( player.type == 3 ) { zDivideAtk( player.facing ); }
            else if ( player.type == 4 ) { spinSlashAtk( player.facing ); }
        }
		else if (keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6]) {
            if      ( player.type == 0 ) { stepAtk(player.facing); }
            else if ( player.type == 1 ) { protoAtk( player.facing ); } }
		else if (keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7]) {
            if (player.type == 0) { lifeAtk(player.facing); } }
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
	for (int i = 5; i >= 0; i--) {
		drawItems(i);
		drawBoxes(i);
        if ( i == player.y ) { drawPlayer(); }
    }

    drawSwordAtks();

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
    if      ( player.type == 0 ) { drawSpriteSheetSprite( place, menuPic0, 0, 1, 1 ); }
    else if ( player.type == 1 ) { drawSpriteSheetSprite( place, menuPic1, 0, 1, 1 ); }
    else if ( player.type == 2 ) { drawSpriteSheetSprite( place, menuPic2, 0, 1, 1 ); }
    else if ( player.type == 3 ) { drawSpriteSheetSprite( place, menuPic3, 0, 1, 1 ); }
    else if ( player.type == 4 ) { drawSpriteSheetSprite( place, menuPic4, 0, 1, 1 ); }
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

void App::drawPlayer() {
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);
    if ( player.type == 3 ) { glTranslatef( 0, -0.05 * playerScale, 0 ); }
	
    float playerSizeX, playerSizeY;
	int picIndex = 0;
	float displace = 0.0;
	float displace2 = 0.3;
    GLuint texture;

	// Step Sword Move Animation
	if (animationType == 1 && (selSwordAnimation == 5 || selSwordAnimation == 12 || selSwordAnimation == 21)) {
        int textureSheetWidth = 8;
        if ( player.type == 0 ) {
		    playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            displace2 = 0.3;
            texture = megamanAtkSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.78 * playerScale;
		    playerSizeY = 0.64 * playerScale; 
            glTranslatef( 0, 0.08 * playerScale, 0 );
            displace2 -= 0.08;
            texture = protoAtkSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            glTranslatef( 0, 0.2 * playerScale, 0 );
            texture = colonelAtkSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.94 * playerScale;
		    playerSizeY = 0.88 * playerScale; 
            glTranslatef( 0, 0.065 * playerScale, 0 );
            displace2 = 0.20;
            textureSheetWidth = 9;
            texture = slashmanAtkSheet; }

		if		(animationDisplayAmt > stepAtkTime + moveAnimationTime * 1.0) { displace = 2.000 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.9) { displace = 1.975 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.8) { displace = 1.950 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.7) { displace = 1.925 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.6) { displace = 1.900 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.5) { displace = 1.520 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.4) { displace = 1.140 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.3) { displace = 0.760 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.2) { displace = 0.380 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.1) { displace = 0.000 * scaleX; }

        if ( player.type == 4 ) {
            if ( ( player.energy / 100 ) % 2 ) {
                if      (animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 0; }
		        else if (animationDisplayAmt > stepAtkTime - 0.18) { picIndex = 1; }
		        else if (animationDisplayAmt > 0)                  { picIndex = 2; } }
            else {
                if      (animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 2; }
		        else if (animationDisplayAmt > stepAtkTime - 0.18) { picIndex = 3; }
		        else if (animationDisplayAmt > 0)                  { picIndex = 4; } }
        }
        else {
		    if      (animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 1; }
		    else if (animationDisplayAmt > stepAtkTime - 0.10) { picIndex = 2; }
		    else if (animationDisplayAmt > stepAtkTime - 0.14) { picIndex = 3; }
		    else if (animationDisplayAmt > stepAtkTime - 0.20) { picIndex = 4; }
		    else if (animationDisplayAmt > stepAtkTime - 0.26) { picIndex = 5; }
		    else if (animationDisplayAmt > stepAtkTime - 0.32) { picIndex = 6; }
		    else if (animationDisplayAmt > 0)                  { picIndex = 7; } }

		if (player.facing == 1) {
            glTranslatef(  displace - displace2, 0.02, 0 );
            picIndex += textureSheetWidth; }
		else if (player.facing == 3) {
            glTranslatef( -displace + displace2, 0.02, 0 ); }

        GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
        drawSpriteSheetSprite( playerPlace, texture, picIndex, textureSheetWidth, 2 );
	}
	// Attack Animation
	else if (animationType == 1 && animationDisplayAmt > 0) {
        int textureSheetWidth = 8;
		if ( player.type == 0 ) {
		    playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            texture = megamanAtkSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.78 * playerScale;
		    playerSizeY = 0.64 * playerScale;
            glTranslatef( 0, 0.08 * playerScale, 0 );
            displace2 -= 0.08;
            texture = protoAtkSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 1.00 * playerScale;
            playerSizeY = 0.76 * playerScale;
            glTranslatef( 0, 0.2 * playerScale, 0 );
            texture = colonelAtkSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.94 * playerScale;
		    playerSizeY = 0.88 * playerScale; 
            glTranslatef( 0, 0.065 * playerScale, 0 );
            displace2 = 0.20;
            textureSheetWidth = 9;
            texture = slashmanAtkSheet; }

        // Altered animation for Tomahawkman's regular attacks
        if ( selSwordAnimation >= 14 && selSwordAnimation <= 17 ) {
            playerSizeX = 0.80 * playerScale;
            playerSizeY = 0.65 * playerScale;
            displace2 = 0.22;
            glTranslatef( 0, 0.1 * playerScale, 0 );
            texture = tmanAtkSheet1;
            textureSheetWidth = 4;
            if      (animationDisplayAmt > currentSwordAtkTime - 0.20) { picIndex = 0; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.24) { picIndex = 1; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.28) { picIndex = 2; }
			else if (animationDisplayAmt > 0)                          { picIndex = 3; } }
        // Tomahawkman's Eagle Tomahawk attack
        else if ( selSwordAnimation == 18 ) {
            playerSizeX = 1.10 * playerScale;
            playerSizeY = 0.99  * playerScale;
            glTranslatef( 0, 0.43 * playerScale, 0 );
            displace2 = 0.17;
            texture = tmanAtkSheet2;
            textureSheetWidth = 5;
            if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 0; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.18) { picIndex = 1; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 2; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.24) { picIndex = 3; }
            else if (animationDisplayAmt > 0)                          { picIndex = 4; } }
        // Slashman's LongSlash and CrossSlash
        else if ( selSwordAnimation == 19 ) {
            if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 2; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 3; }
			else if (animationDisplayAmt > 0)                          { picIndex = 4; }
        }
        // Slashman's WideSlash
        else if ( selSwordAnimation == 20 ) {
            if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 0; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 1; }
			else if (animationDisplayAmt > 0)                          { picIndex = 2; }
        }
        // Slashman's CrossSlash
        else if ( selSwordAnimation == 3 && player.type == 4 ) {
            if ( ( player.energy / 100 ) % 2 ) {
                if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 0; }
			    else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 1; }
			    else if (animationDisplayAmt > 0)                          { picIndex = 2; } }
            else {
                if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 2; }
			    else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 3; }
			    else if (animationDisplayAmt > 0)                          { picIndex = 4; } }
        }
        // Slashman's SpinSlash
        else if ( selSwordAnimation == 22 ) {
            if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 5; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.16) { picIndex = 6; }
            else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 7; }
            else if (animationDisplayAmt > currentSwordAtkTime - 0.28) { picIndex = 6; }
            else if (animationDisplayAmt > currentSwordAtkTime - 0.34) { picIndex = 8; }
            else if (animationDisplayAmt > currentSwordAtkTime - 0.40) { picIndex = 6; }
            else if (animationDisplayAmt > currentSwordAtkTime - 0.46) { picIndex = 7; }
			else if (animationDisplayAmt > 0)                          { picIndex = 6; }
        }
		// Animation for the rest of the Attacks
		else {
			if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 1; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.14) { picIndex = 2; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.18) { picIndex = 3; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 4; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.26) { picIndex = 5; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.30) { picIndex = 6; }
			else if (animationDisplayAmt > 0)                          { picIndex = 7; } }

		if (player.facing == 1) {
			glTranslatef(-displace2, 0.02, 0);
			picIndex += textureSheetWidth; }
		else if (player.facing == 3) {
			glTranslatef( displace2, 0.02, 0); }

        GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
        drawSpriteSheetSprite(playerPlace, texture, picIndex, textureSheetWidth, 2);
	}
	// Movement Animation
    else if ( animationType == 0 && player.moving
              || ( animationType == -1 && animationDisplayAmt > pauseAnimationTime - moveAnimationTime / 2.0 ) ) {
        if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

		if		(animationDisplayAmt > (moveAnimationTime * 0.70 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 1; picIndex = 0; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.60 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 1; picIndex = 1; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.50 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 1; picIndex = 2; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.45 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 0; picIndex = 3; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.35 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 0; picIndex = 2; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.25 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 0; picIndex = 1; }
		else if (animationDisplayAmt > (moveAnimationTime * 0.00 + (animationType == -1 ? moveAnimationTime * 1.5 : 0))) { displace = 0; picIndex = 0; }
		if      (player.moveDir == 0) {
			glTranslatef(0, -displace * scaleY, 0);
			if      (player.facing == 1) {
                glTranslatef( -displace2, 0, 0 );
                drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2); }
			else if (player.facing == 3) {
                glTranslatef(  displace2, 0, 0 );
                drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);     }
		}
		else if (player.moveDir == 1) {
			glTranslatef(displace * scaleX - displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2);
		}
		else if (player.moveDir == 2) {
			glTranslatef(0.0, displace * scaleY, 0.0);
			if      (player.facing == 1) {
                glTranslatef( -displace2, 0, 0 );
                drawSpriteSheetSprite(playerPlace, texture, picIndex + 4, 4, 2);  }
			else if (player.facing == 3) {
                glTranslatef( displace2, 0, 0 );
                drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);      }
		}
		else if (player.moveDir == 3) {
			glTranslatef(-displace * scaleX + displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);
		}
	}
    // Trapped Energy Hurt Animation
    else if (animationType == -1 && animationDisplayAmt > 0) {
        if ( player.type == 0 ) {
		    playerSizeX = 0.40 * playerScale;
		    playerSizeY = 0.48 * playerScale;
            glTranslatef( 0, -0.06 * playerScale, 0 );
            displace2 = 0.014;
            texture = megamanHurtSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.70 * playerScale;
		    playerSizeY = 0.49 * playerScale;
            glTranslatef( 0, -0.05 * playerScale, 0 );
            displace2 = -0.01 * playerScale;
            texture = protoHurtSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.59 * playerScale;
            playerSizeY = 0.51 * playerScale;
            glTranslatef( 0, -0.02 * playerScale, 0 );
            displace2 = 0.17 * playerScale;
            texture = tmanHurtSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.65 * playerScale;
		    playerSizeY = 0.67 * playerScale;
            glTranslatef( 0, 0.088 * playerScale, 0 );
            displace2 = -0.184 * playerScale;
            texture = colonelHurtSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.52 * playerScale;
            glTranslatef( 0, -0.04 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanHurtSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

		if		( animationDisplayAmt > moveAnimationTime * 0.75 ) { picIndex = 0; }
        else if ( animationDisplayAmt > moveAnimationTime * 0.00 ) { picIndex = 1; }
		if (player.facing == 1) {
			glTranslatef(-displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex + 2, 2, 2);
		}
		else if (player.facing == 3) {
			glTranslatef(displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, texture, picIndex, 2, 2);
		}
    }
	// Turning Animation
	else if (animationType == 0 && player.turning) {
		bool reverse = true;
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };

		if		(animationDisplayAmt > moveAnimationTime * 0.70) { picIndex = 0; reverse = true; }
		else if (animationDisplayAmt > moveAnimationTime * 0.60) { picIndex = 1; reverse = true; }
		else if (animationDisplayAmt > moveAnimationTime * 0.50) { picIndex = 2; reverse = true; }
		else if (animationDisplayAmt > moveAnimationTime * 0.45) { picIndex = 3; reverse = false; }
		else if (animationDisplayAmt > moveAnimationTime * 0.35) { picIndex = 2; reverse = false; }
		else if (animationDisplayAmt > moveAnimationTime * 0.25) { picIndex = 1; reverse = false; }
		else if (animationDisplayAmt > moveAnimationTime * 0.00) { picIndex = 0; reverse = false; }

		if      (player.facing == 1) {
            picIndex += (reverse ? 0 : 4);
            reverse ? glTranslatef(  displace, 0, 0 ) : glTranslatef( -displace, 0, 0 ); }
		else if (player.facing == 3) {
            picIndex += (reverse ? 4 : 0);
            reverse ? glTranslatef( -displace, 0, 0 ) : glTranslatef(  displace, 0, 0 ); }
		drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);
	}
	// Special Invalid Movement animation for Start Tile
	else if (animationType == 2 && animationDisplayAmt > 0) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace - displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 0, 4, 2);
	}
	// Level Transition pt. 1
	else if (animationType == 3 && animationDisplayAmt > moveAnimationTime * 2) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = 1 - (animationDisplayAmt - moveAnimationTime * 2) / moveAnimationTime;
		glTranslatef(displace + displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 4, 4, 2);
	}
	// Level Transition pt. 2
	else if (animationType == 4 && animationDisplayAmt > 0 && animationDisplayAmt <= moveAnimationTime * 2) {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            displace2 = 0;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace2 = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace2 = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		displace = animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace + displace2, 0, 0);
		drawSpriteSheetSprite(playerPlace, texture, 4, 4, 2);
	}
	// Player Standing Still
	else {
		if ( player.type == 0 ) {
		    playerSizeX = 0.35 * playerScale;
		    playerSizeY = 0.54 * playerScale;
            texture = megamanMoveSheet; }
        else if ( player.type == 1 ) {
            playerSizeX = 0.53 * playerScale;
		    playerSizeY = 0.59 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.08 * playerScale;
            texture = protoMoveSheet; }
        else if ( player.type == 2 ) {
            playerSizeX = 0.52 * playerScale;
            playerSizeY = 0.60 * playerScale;
            glTranslatef( 0, 0.05 * playerScale, 0 );
            displace = 0.2 * playerScale;
            texture = tmanMoveSheet; }
        else if ( player.type == 3 ) {
            playerSizeX = 0.61 * playerScale;
		    playerSizeY = 0.63 * playerScale;
            glTranslatef( 0, 0.09 * playerScale, 0 );
            displace = -0.1 * playerScale;
            texture = colonelMoveSheet; }
        else if ( player.type == 4 ) {
            playerSizeX = 0.66 * playerScale;
		    playerSizeY = 0.56 * playerScale;
            glTranslatef( 0, 0.00 * playerScale, 0 );
            displace = 0.2 * playerScale;
            texture = slashmanMoveSheet; }

		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };

		if      (player.facing == 1) { picIndex = 0; glTranslatef( -displace, 0, 0 ); }
		else if (player.facing == 3) { picIndex = 4; glTranslatef(  displace, 0, 0 ); }
		drawSpriteSheetSprite(playerPlace, texture, picIndex, 4, 2);
	}
}
void App::drawSwordAtks() {
    glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	glTranslatef(0.5, 1.2, 0.0);
	if (animationDisplayAmt > 0 && animationType == 1) {
		if      (selSwordAnimation == 0) { swordDisplay(player.facing); }
		else if (selSwordAnimation == 1) { longDisplay(player.facing); }
		else if (selSwordAnimation == 2) { wideDisplay(player.facing); }
		else if (selSwordAnimation == 3) { crossDisplay(player.facing); }
		else if (selSwordAnimation == 4) { spinDisplay(player.facing); }
		else if (selSwordAnimation == 5) { stepDisplay(player.facing); }
		else if (selSwordAnimation == 6) { lifeDisplay(player.facing); }
        else if (selSwordAnimation == 7) { heroDisplay(player.facing); }
        else if (selSwordAnimation == 8) { protoDisplay(player.facing); }
        else if (selSwordAnimation == 9)  { vDivideDisplay(player.facing); }
        else if (selSwordAnimation == 10) { upDivideDisplay(player.facing); }
        else if (selSwordAnimation == 11) { downDivideDisplay(player.facing); }
        else if (selSwordAnimation == 12) { xDivideDisplay(player.facing); }
        else if (selSwordAnimation == 13) { zDivideDisplay(player.facing); }
        else if (selSwordAnimation == 14) { tomaDisplayA1(player.facing); }
        else if (selSwordAnimation == 15) { tomaDisplayB1(player.facing); }
        else if (selSwordAnimation == 16) { tomaDisplayA2(player.facing); }
        else if (selSwordAnimation == 17) { tomaDisplayB2(player.facing); }
        else if (selSwordAnimation == 18) {
            for ( int i = 0; i < delayedETomaDisplayList.size(); i++ ) {
                if ( delayedETomaDisplayList[i].delay <= 0 ) {
                    eTomaDisplay( delayedETomaDisplayList[i].xPos, delayedETomaDisplayList[i].yPos, delayedETomaDisplayList[i].animationTimer );
                }
            }
        }
        else if (selSwordAnimation == 19) { longSlashDisplay(player.facing); }
        else if (selSwordAnimation == 20) { wideSlashDisplay(player.facing); }
        else if (selSwordAnimation == 21) { stepCrossDisplay(player.facing); }
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
				GLfloat place[] = { i * scaleX + sizeX2, row * scaleY + sizeY2, i * scaleX + sizeX2, row * scaleY - sizeY2,
									i * scaleX - sizeX2, row * scaleY - sizeY2, i * scaleX - sizeX2, row * scaleY + sizeY2 };
				if      (map[i][row].boxAtkInd > boxDmgTime * 0.60) { drawSpriteSheetSprite(place, rockDeathSheet, 0, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.40) { drawSpriteSheetSprite(place, rockDeathSheet, 1, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.20) { drawSpriteSheetSprite(place, rockDeathSheet, 2, 4, 1); }
				else if (map[i][row].boxAtkInd > boxDmgTime * 0.00) { drawSpriteSheetSprite(place, rockDeathSheet, 3, 4, 1); }
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
	if ( energyDisplayed >= 10 )   { glTranslatef(-0.04, 0, 0); }
	if ( energyDisplayed >= 100 )  { glTranslatef(-0.04, 0, 0); }
	if ( energyDisplayed >= 1000 ) { glTranslatef(-0.04, 0, 0); }
    int energyToDisplay = energyDisplayed;
	if      ( energyToDisplay < 1 )    { energyToDisplay = 1; }
	else if ( energyToDisplay > 9999 ) { energyToDisplay = 9999; }
	if      (chargeDisplayPlusAmt  > 0) { drawText(textSheet2B, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else if (chargeDisplayMinusAmt > 0 || energyToDisplay == 1) {
	                                      drawText(textSheet2C, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else                                { drawText(textSheet2A, to_string( energyToDisplay ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	// Draw Current Energy gain for this level
	glLoadIdentity();
	glTranslatef(-0.828, 0.8495, 0);
	if (abs( energyDisplayed2 ) >= 10)   { glTranslatef(-0.04, 0, 0); }
	if (abs( energyDisplayed2 ) >= 100)  { glTranslatef(-0.04, 0, 0); }
	if (abs( energyDisplayed2 ) >= 1000) { glTranslatef(-0.04, 0, 0); }
	if (level > 0) {
        if      ( energyDisplayed2 > 0 ) { drawText( textSheet1B, to_string( abs( energyDisplayed2 ) ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 ); }
        else if ( energyDisplayed2 < 0 ) { drawText( textSheet1C, to_string( abs( energyDisplayed2 ) ), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 ); }
	}

	// Display Sword Name and Cost
	glLoadIdentity();
	glTranslatef(-0.97, -0.93, 0);
	string text = "";
	GLuint texture;

    if ( selSwordAnimation != -1 ) {
        if ( selSwordAnimation == -2 ) {
            texture = textSheet1B;
            text = "Energy" + to_string( energyGainAmt ); }
        else if ( selSwordAnimation == -3 ) {
            texture = textSheet1B;
            text = "Energy" + to_string( energyGainAmt2 ); }
        else if ( selSwordAnimation == -4 ) {
            texture = textSheet1C;
            text = "Energy" + to_string( energyLossAmt ); }
        else {
            texture = textSheet1C;
            if      ( selSwordAnimation == 0 )  { text = "Sword"    + to_string( swordCost ); }
            else if ( selSwordAnimation == 1 )  { text = "LongSwrd" + to_string( longCost ); }
            else if ( selSwordAnimation == 2 ) {
                if      ( player.type == 0 )    { text = "WideSwrd" + to_string( wideCost ); }
                else if ( player.type == 1 )    { text = "WideSwrd" + to_string( wideCost2 ); } }
            else if ( selSwordAnimation == 3 ) {
                if      ( player.type == 0 )    { text = "CrossSrd" + to_string( crossCost ); }
                else if ( player.type == 4 )    { text = "CrossSls" + to_string( crossCost2 ); } }
            else if ( selSwordAnimation == 4 )  { text = "SpinSwrd" + to_string( spinCost ); }
            else if ( selSwordAnimation == 5 ) {
                if ( player.type == 0 )         { text = "StepSwrd" + to_string( stepCost ); }
                else if ( player.type == 1 )    { text = "StepSwrd" + to_string( stepCost2 ); } }
            else if ( selSwordAnimation == 6 )  { text = "LifeSwrd" + to_string( lifeCost ); } 
            else if ( selSwordAnimation == 7 )  { text = "HeroSwrd" + to_string( heroCost ); }
            else if ( selSwordAnimation == 8 )  { text = "ProtoCrs" + to_string( protoCost ); }
            else if ( selSwordAnimation == 9 )  { text = "ScrnDivV" + to_string( vDivideCost ); }
            else if ( selSwordAnimation == 10 ) { text = "ScreenDv" + to_string( upDivideCost ); }
            else if ( selSwordAnimation == 11 ) { text = "ScreenDv" + to_string( downDivideCost ); }
            else if ( selSwordAnimation == 12 ) { text = "CrossDiv" + to_string( xDivideCost ); }
            else if ( selSwordAnimation == 13 ) { text = "NeoSnDiv" + to_string( zDivideCost ); }
            else if ( selSwordAnimation == 14 ) { text = "WideSwng" + to_string( tomaCostA1 ); }
            else if ( selSwordAnimation == 15 ) { text = "WdSwngEX" + to_string( tomaCostB1 ); }
            else if ( selSwordAnimation == 16 ) { text = "Tomahawk" + to_string( tomaCostA2 ); }
            else if ( selSwordAnimation == 17 ) { text = "TomahkEX" + to_string( tomaCostB2 ); }
            else if ( selSwordAnimation == 18 ) { text = "ETomahwk" + to_string( eTomaCost ); }
            else if ( selSwordAnimation == 19 ) { text = "LongSlsh" + to_string( longCost ); }
            else if ( selSwordAnimation == 20 ) { text = "WideSlsh" + to_string( wideCost ); }
            else if ( selSwordAnimation == 21 ) { text = "StepCrss" + to_string( stepCrossCost ); }
            else if ( selSwordAnimation == 22 ) { text = "SpinSlsh" + to_string( spinSlashCost ); }
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

// Sword Attack Animations
void App::swordDisplay(int dir) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.22 * playerScale;
	int xPos = player.x;
	int yPos = player.y;
    GLuint texture;

	if ( dir == 1 ) {
        xPos--;
        texture = swordAtkSheet1; }
    else if ( dir == 3 ) {
        xPos++;
        texture = swordAtkSheet3; }

	GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };

	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::longDisplay(int dir) {
	float sizeX = 0.70 * playerScale;
	float sizeY = 0.37 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos -= 1.5;
        texture = longAtkSheet1; }
	else if (dir == 3) {
		xPos += 1.5;
        texture = longAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideDisplay(int dir) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (dir == 3) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::crossDisplay(int dir) {
	float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (dir == 3) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::spinDisplay(int dir) {
	float sizeX = 1.10 * 1.25 * playerScale;
	float sizeY = 0.72 * 1.25 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
        texture = spinAtkSheet1; }
	else if (dir == 3) {
        texture = spinAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (animationDisplayAmt > 0)                   { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::stepDisplay(int dir) {
	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos--;
        texture = wideAtkSheet1; }
	else if (dir == 3) {
		xPos++;
        texture = wideAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::lifeDisplay(int dir) {
	float sizeX = 0.95 * playerScale;
	float sizeY = 0.80 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos -= 1.5;
        texture = lifeAtkSheet1; }
	else if (dir == 3) {
		xPos += 1.5;
        texture = lifeAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::heroDisplay( int dir ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.45 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 2;
        texture = heroAtkSheet1; }
    else if ( dir == 3 ) {
        xPos += 2;
        texture = heroAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::protoDisplay( int dir ) {
    float sizeX = 1.17 * playerScale;
    float sizeY = 0.71 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 2;
        texture = protoAtkSheet1; }
    else if ( dir == 3 ) {
        xPos += 2;
        texture = protoAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::vDivideDisplay( int dir ) {
    float sizeX = 0.68 * playerScale;
    float sizeY = 0.53 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 0.75;
        texture = screenDivVSheet3; }
    else if ( dir == 3 ) {
        xPos += 0.75;
        texture = screenDivVSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::upDivideDisplay( int dir ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1;
        texture = screenDivUpSheet3; }
    else if ( dir == 3 ) {
        xPos += 1;
        texture = screenDivUpSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::downDivideDisplay( int dir ) {
    float sizeX = 0.85 * playerScale;
    float sizeY = 0.78 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1;
        texture = screenDivDownSheet3; }
    else if ( dir == 3 ) {
        xPos += 1;
        texture = screenDivDownSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if	(animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::xDivideDisplay( int dir ) {
    float sizeX = 0.94 * playerScale;
    float sizeY = 1.03 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1;
        texture = screenDivXSheet3; }
    else if ( dir == 3 ) {
        xPos += 1;
        texture = screenDivXSheet1; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}
void App::zDivideDisplay(int dir) {
	float sizeX = 1.32 * playerScale;
	float sizeY = 0.60 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos -= 1; }
	else if (dir == 3) {
		xPos += 1; }
    texture = screenDivZSheet;

    GLfloat atkPlace[] = { xPos * scaleX - sizeX, yPos * scaleY + sizeY, xPos * scaleX - sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX + sizeX, yPos * scaleY - sizeY, xPos * scaleX + sizeX, yPos * scaleY + sizeY };
    if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
	else if (animationDisplayAmt > lifeAtkTime - 0.16) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.28) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
	else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA1(int dir) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetA1; }
    else if ( dir == 3 ) {
        xPos += 1;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB1(int dir) {
    float sizeX = 0.95 / 2 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1;
        texture = tomahawkAtkSheetB1; }
    else if ( dir == 3 ) {
        xPos += 1;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayA2(int dir) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetA1; }
    else if ( dir == 3 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetA3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
}
void App::tomaDisplayB2(int dir) {
    float sizeX = 0.95 * playerScale;
    float sizeY = 0.80 * playerScale;
    float xPos = player.x;
    float yPos = player.y;
    GLuint texture;

    if ( dir == 1 ) {
        xPos -= 1.5;
        texture = tomahawkAtkSheetB1; }
    else if ( dir == 3 ) {
        xPos += 1.5;
        texture = tomahawkAtkSheetB3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > swordAtkTime - 0.20 ) {}
    else if ( animationDisplayAmt > swordAtkTime - 0.26 ) { drawSpriteSheetSprite(atkPlace, texture, 0, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.28 ) { drawSpriteSheetSprite(atkPlace, texture, 1, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.30 ) { drawSpriteSheetSprite(atkPlace, texture, 2, 4, 1); }
    else if ( animationDisplayAmt > swordAtkTime - 0.32 ) { drawSpriteSheetSprite(atkPlace, texture, 3, 4, 1); }
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
void App::longSlashDisplay(int dir) {
    float sizeX = 0.84 * playerScale;
	float sizeY = 0.54 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos -= 1.5;
        texture = longSlashSheet1; }
	else if (dir == 3) {
		xPos += 1.5;
        texture = longSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::wideSlashDisplay(int dir) {
    float sizeX = 0.39 * playerScale;
	float sizeY = 0.95 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos--;
        texture = wideSlashSheet1; }
	else if (dir == 3) {
		xPos++;
        texture = wideSlashSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if		(animationDisplayAmt > swordAtkTime - 0.12) {}
	else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, texture, 0, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, texture, 1, 3, 1); }
	else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, texture, 2, 3, 1); }
}
void App::stepCrossDisplay(int dir) {
    float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
    GLuint texture;

	if (dir == 1) {
		xPos--;
        texture = crossAtkSheet1; }
	else if (dir == 3) {
		xPos++;
        texture = crossAtkSheet3; }

    GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
                           xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
    if      ( animationDisplayAmt > stepAtkTime - 0.18 ) {}
    else if ( animationDisplayAmt > stepAtkTime - 0.26 ) { drawSpriteSheetSprite( atkPlace, texture, 0, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.32 ) { drawSpriteSheetSprite( atkPlace, texture, 1, 3, 1 ); }
    else if ( animationDisplayAmt > stepAtkTime - 0.38 ) { drawSpriteSheetSprite( atkPlace, texture, 2, 3, 1 ); }
}

// Player Control Functions: Attacking & Movement
void App::face(int dir) {
	if (dir >= 0 && dir <= 3) {
		player.turning = true;
		player.facing = dir;
		animationType = 0;
		animationDisplayAmt = moveAnimationTime;
}	}
void App::move(int dir) {
	if (dir == 0) {			// Move Up			// Can't move out of map, onto a box, or onto a hole in the floor
		if (isTileValid(player.x, player.y + 1)) {
			if (map[player.x][player.y].state == 1) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.y++;
			player.moving = true;
			player.moveDir = 0;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
                chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    animationType = -1;
                    animationDisplayAmt = pauseAnimationTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 1, player.y)) {
			if (map[player.x][player.y].state == 1 && !(player.x == 0 && player.y == 0)) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.x--;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    animationType = -1;
                    animationDisplayAmt = pauseAnimationTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 1)) {
			if (map[player.x][player.y].state == 1) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.y--;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    animationType = -1;
                    animationDisplayAmt = pauseAnimationTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 3) {	// Move Right
		if (isTileValid(player.x + 1, player.y)) {
			if ( map[player.x][player.y].state == 1 ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.x++;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    animationType = -1;
                    animationDisplayAmt = pauseAnimationTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
}
void App::move2(int dir) {		// Move Two Tiles
	if (dir == 0) {			// Move Up			// Can't move out of map, onto a box, or onto a hole in the floor
		if (isTileValid(player.x, player.y + 2)) {
			if ( map[player.x][player.y].state == 1 ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.y += 2;
			player.moving = true;
			player.moveDir = 0;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 2, player.y)) {
			if ( map[player.x][player.y].state == 1 ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.x -= 2;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
                chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 2)) {
			if ( map[player.x][player.y].state == 1 ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.y -= 2;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
	else if (dir == 3) {	// Move Right
		if (isTileValid(player.x + 2, player.y)) {
			if ( map[player.x][player.y].state == 1 ) { 		// Walk off Cracked tile -> Cracked tile becomes a Hole
				map[player.x][player.y].state = 2;
				Mix_PlayChannel( 2, panelBreakSound, 0 ); }
			player.x += 2;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item != 0) {  // Walk onto Energy resource -> Take Energy
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
                if ( map[player.x][player.y].item == 2 ) {
                    Mix_PlayChannel( 1, itemSound2, 0 );
                    selSwordAnimation = -3;
                    player.energy += energyGainAmt2;
                    currentEnergyGain += energyGainAmt2; }
                else if ( map[player.x][player.y].item == -1 ) {
                    Mix_PlayChannel( 1, trapItemSound, 0 );
                    chargeDisplayPlusAmt = 0;
                    chargeDisplayMinusAmt = iconDisplayTime;
                    selSwordAnimation = -4;
                    player.energy -= energyLossAmt;
                    if ( player.energy < 0 ) player.energy = 0;
                    currentEnergyGain -= energyLossAmt; }
                else {
                    Mix_PlayChannel( 1, itemSound, 0 );
                    selSwordAnimation = -2;
                    player.energy += energyGainAmt;
                    currentEnergyGain += energyGainAmt; }
				map[player.x][player.y].item = 0;
	} } }
}

void App::swordAtk(int dir) {			// Does One Dmg to one square in front of the player
	if (player.energy >= swordCost) {
		player.energy -= swordCost;
		if      (dir == 1) { hitBoxDelay(player.x - 1, player.y, preAtkTime); }
		else if (dir == 3) { hitBoxDelay(player.x + 1, player.y, preAtkTime); }
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 0;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= swordCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::longAtk(int dir) {		// Does One Dmg to a 2x1 area in front of the player
	if (player.energy >= longCost) {
		player.energy -= longCost;
		if (dir == 1) {											//
			hitBoxDelay(player.x - 1, player.y, preAtkTime);	//	xxP
			hitBoxDelay(player.x - 2, player.y, preAtkTime);	//
		}
		else if (dir == 3) {									//
			hitBoxDelay(player.x + 1, player.y, preAtkTime);	//	  Pxx
			hitBoxDelay(player.x + 2, player.y, preAtkTime);	//
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
        if      ( player.type == 0 || player.type == 1 ) { selSwordAnimation = 1; }
        else if ( player.type == 4 )                     { selSwordAnimation = 19; }
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= longCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::wideAtk(int dir) {			// Does One Dmg to a 1x3 area in front of the player
	if ( player.type == 0 && player.energy >= wideCost ||
         player.type == 4 && player.energy >= wideCost ||
         player.type == 1 && player.energy >= wideCost2 ) {
        if      ( player.type == 0 || player.type == 4) { player.energy -= wideCost;  }
        else if ( player.type == 1 )                    { player.energy -= wideCost2; }
		if (dir == 1) {
			hitBoxDelay(player.x - 1, player.y + 1, preAtkTime);	// x
			hitBoxDelay(player.x - 1, player.y,     preAtkTime);	// xP
			hitBoxDelay(player.x - 1, player.y - 1, preAtkTime);	// x
		}
		else if (dir == 3) {
			hitBoxDelay(player.x + 1, player.y + 1, preAtkTime);	//	 x
			hitBoxDelay(player.x + 1, player.y,     preAtkTime);	//	Px
			hitBoxDelay(player.x + 1, player.y - 1, preAtkTime);	//	 x
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		if      ( player.type == 0 || player.type == 1) { selSwordAnimation = 2; }
        else if ( player.type == 4  )                   { selSwordAnimation = 20; }
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
        if      ( player.type == 0 || player.type == 4 ) { currentEnergyGain -= wideCost;  }
        else if ( player.type == 1 )                     { currentEnergyGain -= wideCost2; }
        Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::crossAtk(int dir) {			// Does One Dmg in an X pattern in front of the player		// The Middle of the X cross takes Two Dmg total
	if ( player.type == 0 && player.energy >= crossCost ||
         player.type == 4 && player.energy >= crossCost2 ) {
        if      ( player.type == 0 ) { player.energy -= crossCost; }
        else if ( player.type == 4 ) { player.energy -= crossCost2; }
		if (dir == 1) {
			hitBoxDelay(player.x,	  player.y - 1, preAtkTime);	//	x x
			hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);	//	 xP
			hitBoxDelay(player.x - 2, player.y + 1, preAtkTime);	//	x x
			hitBoxDelay(player.x,	  player.y + 1, preAtkTime);
			hitBoxDelay(player.x - 2, player.y - 1, preAtkTime);
		}
		else if (dir == 3) {
			hitBoxDelay(player.x,	  player.y - 1, preAtkTime);	//	x x
			hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);	//	Px
			hitBoxDelay(player.x + 2, player.y + 1, preAtkTime);	//	x x
			hitBoxDelay(player.x,	  player.y + 1, preAtkTime);
			hitBoxDelay(player.x + 2, player.y - 1, preAtkTime);
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 3;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
        if      ( player.type == 0 ) { currentEnergyGain -= crossCost; }
        else if ( player.type == 4 ) { currentEnergyGain -= crossCost2; }
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::spinAtk(int dir) {		// Does One Dmg to all squares adjacent to the player (including diagonal adjacents)
	if (player.energy >= spinCost) {
		player.energy -= spinCost;
		hitBoxDelay(player.x - 1, player.y + 1, preAtkTime);	//	xxx
		hitBoxDelay(player.x - 1, player.y,     preAtkTime);	//	xPx
		hitBoxDelay(player.x - 1, player.y - 1, preAtkTime);	//	xxx
		hitBoxDelay(player.x,	  player.y - 1, preAtkTime);
		hitBoxDelay(player.x,	  player.y + 1, preAtkTime);
		hitBoxDelay(player.x + 1, player.y + 1, preAtkTime);
		hitBoxDelay(player.x + 1, player.y,     preAtkTime);
		hitBoxDelay(player.x + 1, player.y - 1, preAtkTime);

		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 4;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= spinCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::stepAtk(int dir) {			// Moves the player two squares in the facing Direction, then uses WideSword
	if ( player.type == 0 && player.energy >= stepCost || player.type == 1 && player.energy >= stepCost2 ) {
		if (dir == 1 && isTileValid(player.x - 2, player.y)) {
			if      ( player.type == 0 ) { player.energy -= stepCost; }
            else if ( player.type == 1 ) { player.energy -= stepCost2; }

			move2(1);
			hitBoxDelay(player.x - 1, player.y + 1, moveAnimationTime + preAtkTime);	// x
			hitBoxDelay(player.x - 1, player.y,     moveAnimationTime + preAtkTime);	// xP
			hitBoxDelay(player.x - 1, player.y - 1, moveAnimationTime + preAtkTime);	// x

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
            if      ( player.type == 0 ) { currentEnergyGain -= stepCost; }
            else if ( player.type == 1 ) { currentEnergyGain -= stepCost2; }
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
		}
		else if (dir == 3 && isTileValid(player.x + 2, player.y)) {
			if      ( player.type == 0 ) { player.energy -= stepCost; }
            else if ( player.type == 1 ) { player.energy -= stepCost2; }

			move2(3);
			hitBoxDelay(player.x + 1, player.y + 1, moveAnimationTime + preAtkTime);	//	 x
			hitBoxDelay(player.x + 1, player.y,     moveAnimationTime + preAtkTime);	//	Px
			hitBoxDelay(player.x + 1, player.y - 1, moveAnimationTime + preAtkTime);	//	 x

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
			if      ( player.type == 0 ) { currentEnergyGain -= stepCost; }
            else if ( player.type == 1 ) { currentEnergyGain -= stepCost2; }
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
		}
	}
}
void App::lifeAtk(int dir) {		// Does Two Dmg to a 2x3 area in front the player
	if (player.energy >= lifeCost) {
		player.energy -= lifeCost;
		if (dir == 1) {
			hitBoxDelay(player.x - 2, player.y + 1, preAtkTime, 2);		// xx
			hitBoxDelay(player.x - 2, player.y,     preAtkTime, 2);		// xxP
			hitBoxDelay(player.x - 2, player.y - 1, preAtkTime, 2);		// xx
			hitBoxDelay(player.x - 1, player.y + 1, preAtkTime, 2);
			hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);
			hitBoxDelay(player.x - 1, player.y - 1, preAtkTime, 2);
		}
		else if (dir == 3) {
			hitBoxDelay(player.x + 2, player.y + 1, preAtkTime, 2);		//  xx
			hitBoxDelay(player.x + 2, player.y,     preAtkTime, 2);		// Pxx
			hitBoxDelay(player.x + 2, player.y - 1, preAtkTime, 2);		//  xx
			hitBoxDelay(player.x + 1, player.y + 1, preAtkTime, 2);
			hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);
			hitBoxDelay(player.x + 1, player.y - 1, preAtkTime, 2);
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 6;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= lifeCost;
		Mix_PlayChannel( 0, lifeSwordSound, 0 );
	}
}
void App::heroAtk( int dir ) {      // Does Two Dmg to a 3x1 area in front of the player
    if ( player.energy >= heroCost ) {
        player.energy -= heroCost;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y, preAtkTime, 2);
            hitBoxDelay( player.x - 2, player.y, preAtkTime, 2);   // XXXP
            hitBoxDelay( player.x - 3, player.y, preAtkTime, 2);
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y, preAtkTime, 2);
            hitBoxDelay( player.x + 2, player.y, preAtkTime, 2);   // PXXX
            hitBoxDelay( player.x + 3, player.y, preAtkTime, 2);
        }
        // Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 7;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= heroCost;
		//Mix_PlayChannel( 0, lifeSwordSound, 0 );
        Mix_PlayChannel( 0, swordSound, 0 );
    }
}
void App::protoAtk( int dir ) {     // Does Two Dmg in a Plus pattern in front of the player - HeroSword + WideSword
    if ( player.energy >= protoCost ) {
        player.energy -= protoCost;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y, preAtkTime, 2 );   //  x
            hitBoxDelay( player.x - 2, player.y, preAtkTime, 2 );   // xxxP
            hitBoxDelay( player.x - 3, player.y, preAtkTime, 2 );   //  x
            hitBoxDelay( player.x - 2, player.y + 1, preAtkTime, 2 );
            hitBoxDelay( player.x - 2, player.y - 1, preAtkTime, 2 );
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y, preAtkTime, 2 );   //   x
            hitBoxDelay( player.x + 2, player.y, preAtkTime, 2 );   // Pxxx
            hitBoxDelay( player.x + 3, player.y, preAtkTime, 2 );   //   x
            hitBoxDelay( player.x + 2, player.y + 1, preAtkTime, 2 );
            hitBoxDelay( player.x + 2, player.y - 1, preAtkTime, 2 );
        }
        // Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 8;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= protoCost;
		//Mix_PlayChannel( 0, lifeSwordSound, 0 );
        Mix_PlayChannel( 0, swordSound, 0 );
    }
}
void App::vDivideAtk( int dir ) {   // Attacks in a V pattern   // Does 2 dmg at the tip of the V
    if (player.energy >= vDivideCost) {
		player.energy -= vDivideCost;
		if (dir == 1) {
            hitBoxDelay(player.x,     player.y + 1, preAtkTime );       //   x
            hitBoxDelay(player.x - 1, player.y,     preAtkTime );   //  XP
            hitBoxDelay(player.x,     player.y - 1, preAtkTime );       //   x
        }
		else if (dir == 3) {
            hitBoxDelay(player.x,     player.y + 1, preAtkTime );       //  x
            hitBoxDelay(player.x + 1, player.y,     preAtkTime );       //  PX
            hitBoxDelay(player.x,     player.y - 1, preAtkTime );       //  x
        }
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 9;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= vDivideCost;
		Mix_PlayChannel( 0, screenDivSound, 0 );
	}
}
void App::upDivideAtk( int dir ) {  // Attacks Diagonally Upwards       Does 2 dmg
    if (player.energy >= upDivideCost) {
		player.energy -= upDivideCost;
		if (dir == 1) {
            hitBoxDelay(player.x - 2, player.y + 1, preAtkTime, 2 );   //  X
            hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);    //   XP
            hitBoxDelay(player.x,     player.y - 1, preAtkTime, 2 );   //    X
        }
		else if (dir == 3) {
            hitBoxDelay(player.x + 2, player.y + 1, preAtkTime, 2 );   //    X
            hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);    //  PX
            hitBoxDelay(player.x,     player.y - 1, preAtkTime, 2 );   //  X
        }
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 10;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= upDivideCost;
		Mix_PlayChannel( 0, screenDivSound, 0 );
	}
}
void App::downDivideAtk( int dir ) {    // Attacks Diagonally Downward      Does 2 dmg
    if (player.energy >= downDivideCost) {
		player.energy -= downDivideCost;
		if (dir == 1) {
            hitBoxDelay(player.x,     player.y + 1, preAtkTime, 2 );   //    X
            hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);    //   XP
            hitBoxDelay(player.x - 2, player.y - 1, preAtkTime, 2 );   //  X
        }
		else if (dir == 3) {
            hitBoxDelay(player.x,     player.y + 1, preAtkTime, 2 );   //  X
            hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);    //  PX
            hitBoxDelay(player.x + 2, player.y - 1, preAtkTime, 2 );   //    X
        }
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 11;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= downDivideCost;
		Mix_PlayChannel( 0, screenDivSound, 0 );
	}
}
void App::xDivideAtk( int dir ) {			// Moves the player two squares in the facing Direction, then uses CrossSword
    if ( player.energy >= xDivideCost ) {
		if (dir == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= xDivideCost;

			move2(1);
			hitBoxDelay(player.x,	  player.y - 1, moveAnimationTime + preAtkTime, 2);	//	X X
			hitBoxDelay(player.x - 1, player.y,     moveAnimationTime + preAtkTime, 2);	//	 XP
			hitBoxDelay(player.x - 2, player.y + 1, moveAnimationTime + preAtkTime, 2);	//	X X
			hitBoxDelay(player.x,	  player.y + 1, moveAnimationTime + preAtkTime, 2);
			hitBoxDelay(player.x - 2, player.y - 1, moveAnimationTime + preAtkTime, 2);

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 12;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
            currentEnergyGain -= xDivideCost;
            delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime ) );
		}
		else if (dir == 3 && isTileValid(player.x + 2, player.y)) {
			player.energy -= xDivideCost;

			move2(3);
			hitBoxDelay(player.x,	  player.y - 1, moveAnimationTime + preAtkTime, 2);	//	X X
			hitBoxDelay(player.x + 1, player.y,     moveAnimationTime + preAtkTime, 2);	//	PX
			hitBoxDelay(player.x + 2, player.y + 1, moveAnimationTime + preAtkTime, 2);	//	X X
			hitBoxDelay(player.x,	  player.y + 1, moveAnimationTime + preAtkTime, 2);
			hitBoxDelay(player.x + 2, player.y - 1, moveAnimationTime + preAtkTime, 2);

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 12;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
			currentEnergyGain -= xDivideCost;
            delayedSoundList.push_back( DelayedSound( "divide", moveAnimationTime ) );
		}
	}
}
void App::zDivideAtk(int dir) {		// Does Two Dmg in a Z pattern
	if (player.energy >= zDivideCost) {
		player.energy -= zDivideCost;
		if (dir == 1) {
			hitBoxDelay(player.x - 2, player.y + 1, preAtkTime, 2);		// XXX
			hitBoxDelay(player.x - 2, player.y - 1, preAtkTime, 2);		//  XP
			hitBoxDelay(player.x - 1, player.y + 1, preAtkTime, 2);		// XXX
			hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);
			hitBoxDelay(player.x - 1, player.y - 1, preAtkTime, 2);
			hitBoxDelay(player.x,     player.y + 1, preAtkTime, 2);
            hitBoxDelay(player.x,     player.y - 1, preAtkTime, 2);
		}
		else if (dir == 3) {
			hitBoxDelay(player.x + 2, player.y + 1, preAtkTime, 2);		// XXX
			hitBoxDelay(player.x + 2, player.y - 1, preAtkTime, 2);		// PX
			hitBoxDelay(player.x + 1, player.y + 1, preAtkTime, 2);		// XXX
			hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);
			hitBoxDelay(player.x + 1, player.y - 1, preAtkTime, 2);
			hitBoxDelay(player.x,     player.y + 1, preAtkTime, 2);
            hitBoxDelay(player.x,     player.y - 1, preAtkTime, 2);
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 13;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= zDivideCost;
		Mix_PlayChannel( 0, screenDivSound, 0 );
	}
}
void App::tomaAtkA1(int dir) {
    if ( player.energy >= tomaCostA1 ) { 
        player.energy -= tomaCostA1;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y + 1, preAtkTimeToma );	// x
            hitBoxDelay( player.x - 1, player.y,     preAtkTimeToma );  // xP
            hitBoxDelay( player.x - 1, player.y - 1, preAtkTimeToma );	// x
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y + 1, preAtkTimeToma );	//	 x
            hitBoxDelay( player.x + 1, player.y,     preAtkTimeToma );  //	Px
            hitBoxDelay( player.x + 1, player.y - 1, preAtkTimeToma );	//	 x
        }
        animationType = 1;
        animationDisplayAmt = lifeAtkTime;
        currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 14;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= tomaCostA1;
        delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
    }
}
void App::tomaAtkB1(int dir) {
    if ( player.energy >= tomaCostA2 ) { 
        player.energy -= tomaCostA2;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y + 1, preAtkTimeToma, 2 );  // X
            hitBoxDelay( player.x - 1, player.y,     preAtkTimeToma, 2 );  // XP
            hitBoxDelay( player.x - 1, player.y - 1, preAtkTimeToma, 2 );  // X
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y + 1, preAtkTimeToma, 2 );  //	 X
            hitBoxDelay( player.x + 1, player.y,     preAtkTimeToma, 2 );  //	PX
            hitBoxDelay( player.x + 1, player.y - 1, preAtkTimeToma, 2 );  //	 X
        }
        animationType = 1;
        animationDisplayAmt = lifeAtkTime;
        currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 15;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= tomaCostB1;
        delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
    }
}
void App::tomaAtkA2(int dir) {
    if ( player.energy >= tomaCostB1 ) { 
        player.energy -= tomaCostB1;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y + 1, preAtkTimeToma );
            hitBoxDelay( player.x - 2, player.y + 1, preAtkTimeToma );	// xx
            hitBoxDelay( player.x - 1, player.y,     preAtkTimeToma );  // xxP
            hitBoxDelay( player.x - 2, player.y,     preAtkTimeToma );  // xx
            hitBoxDelay( player.x - 1, player.y - 1, preAtkTimeToma );
            hitBoxDelay( player.x - 2, player.y - 1, preAtkTimeToma );
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y + 1, preAtkTimeToma );
            hitBoxDelay( player.x + 2, player.y + 1, preAtkTimeToma );	//	 xx
            hitBoxDelay( player.x + 1, player.y,     preAtkTimeToma );  //	Pxx
            hitBoxDelay( player.x + 2, player.y,     preAtkTimeToma );  //	 xx
            hitBoxDelay( player.x + 1, player.y - 1, preAtkTimeToma );
            hitBoxDelay( player.x + 2, player.y - 1, preAtkTimeToma );
        }
        animationType = 1;
        animationDisplayAmt = lifeAtkTime;
        currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 16;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= tomaCostA2;
        delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
    }
}
void App::tomaAtkB2(int dir) {
    if ( player.energy >= tomaCostB2 ) { 
        player.energy -= tomaCostB2;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y + 1, preAtkTimeToma, 2 );
            hitBoxDelay( player.x - 2, player.y + 1, preAtkTimeToma, 2 );  // XX
            hitBoxDelay( player.x - 1, player.y,     preAtkTimeToma, 2 );  // XXP
            hitBoxDelay( player.x - 2, player.y,     preAtkTimeToma, 2 );  // XX
            hitBoxDelay( player.x - 1, player.y - 1, preAtkTimeToma, 2 );
            hitBoxDelay( player.x - 2, player.y - 1, preAtkTimeToma, 2 );
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y + 1, preAtkTimeToma, 2 );
            hitBoxDelay( player.x + 2, player.y + 1, preAtkTimeToma, 2 );  //  XX
            hitBoxDelay( player.x + 1, player.y,     preAtkTimeToma, 2 );  // PXX
            hitBoxDelay( player.x + 2, player.y,     preAtkTimeToma, 2 );  //  XX
            hitBoxDelay( player.x + 1, player.y - 1, preAtkTimeToma, 2 );
            hitBoxDelay( player.x + 2, player.y - 1, preAtkTimeToma, 2 );
        }
        animationType = 1;
        animationDisplayAmt = lifeAtkTime;
        currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 17;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= tomaCostB2;
        delayedSoundList.push_back( DelayedSound( "toma", preAtkTimeToma ) );
    }
}
void App::eTomaAtk(int dir) {
    if ( player.energy >= eTomaCost ) {
        player.energy -= eTomaCost;
        if ( dir == 1 ) {
            hitBoxDelay( player.x - 1, player.y, preAtkTimeEagleToma - 0.06, 5 );          // XXXXXP
            hitBoxDelay( player.x - 2, player.y, preAtkTimeEagleToma,        5 );
            hitBoxDelay( player.x - 3, player.y, preAtkTimeEagleToma + 0.04, 5 );
            hitBoxDelay( player.x - 4, player.y, preAtkTimeEagleToma + 0.08, 5 );
            hitBoxDelay( player.x - 5, player.y, preAtkTimeEagleToma + 0.12, 5 );
            
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 2, player.y, preAtkTimeEagleToma ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 3, player.y, preAtkTimeEagleToma + 0.04 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 4, player.y, preAtkTimeEagleToma + 0.08 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x - 5, player.y, preAtkTimeEagleToma + 0.12 ) );
        }
        else if ( dir == 3 ) {
            hitBoxDelay( player.x + 1, player.y, preAtkTimeEagleToma - 0.06, 5 );          // PXXXXX
            hitBoxDelay( player.x + 2, player.y, preAtkTimeEagleToma       , 5 );
            hitBoxDelay( player.x + 3, player.y, preAtkTimeEagleToma + 0.04, 5 );
            hitBoxDelay( player.x + 4, player.y, preAtkTimeEagleToma + 0.08, 5 );
            hitBoxDelay( player.x + 5, player.y, preAtkTimeEagleToma + 0.12, 5 );

            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 2, player.y, preAtkTimeEagleToma ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 3, player.y, preAtkTimeEagleToma + 0.04 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 4, player.y, preAtkTimeEagleToma + 0.08 ) );
            delayedETomaDisplayList.push_back( DelayedETomaDisplay( player.x + 5, player.y, preAtkTimeEagleToma + 0.12 ) );
        }
        animationType = 1;
        animationDisplayAmt = eTomaAtkTime;
        currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 18;
        chargeDisplayMinusAmt = iconDisplayTime;
        chargeDisplayPlusAmt = 0;
        currentEnergyGain -= eTomaCost;
        delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma ) );
        //delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma + 0.04 ) );
        //delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma + 0.08 ) );
        //delayedSoundList.push_back( DelayedSound( "eToma", preAtkTimeEagleToma + 0.12 ) );
    }
}
void App::stepCrossAtk(int dir) {
    if ( player.energy >= stepCrossCost ) {
		if (dir == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= stepCrossCost;

			move2(1);
            hitBoxDelay(player.x - 2, player.y + 1, moveAnimationTime + preAtkTime);
			hitBoxDelay(player.x - 2, player.y - 1, moveAnimationTime + preAtkTime);	    // x x
			hitBoxDelay(player.x - 1, player.y,     moveAnimationTime + preAtkTime, 2); 	//  XP
			hitBoxDelay(player.x,     player.y + 1, moveAnimationTime + preAtkTime);	    // x x
            hitBoxDelay(player.x,     player.y - 1, moveAnimationTime + preAtkTime);

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 21;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
            currentEnergyGain -= stepCrossCost;
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
		}
		else if (dir == 3 && isTileValid(player.x + 2, player.y)) {
            player.energy -= stepCrossCost;

			move2(3);
            hitBoxDelay(player.x,     player.y + 1, moveAnimationTime + preAtkTime);
			hitBoxDelay(player.x,     player.y - 1, moveAnimationTime + preAtkTime);        //	x x
			hitBoxDelay(player.x + 1, player.y,     moveAnimationTime + preAtkTime, 2);	    //	PX
			hitBoxDelay(player.x + 2, player.y + 1, moveAnimationTime + preAtkTime);	    //	x x
            hitBoxDelay(player.x + 2, player.y - 1, moveAnimationTime + preAtkTime);

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime;
			currentSwordAtkTime = animationDisplayAmt;
			selSwordAnimation = 21;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
			currentEnergyGain -= stepCrossCost;
            delayedSoundList.push_back( DelayedSound( "sword", moveAnimationTime ) );
		}
	}
}
void App::spinSlashAtk(int dir) {
    if (player.energy >= spinSlashCost) {
		player.energy -= spinSlashCost;
		hitBoxDelay(player.x - 1, player.y + 1, preAtkTime, 2);	    //	XXX
		hitBoxDelay(player.x - 1, player.y,     preAtkTime, 2);	    //	XPX
		hitBoxDelay(player.x - 1, player.y - 1, preAtkTime, 2);	    //	XXX
		hitBoxDelay(player.x,	  player.y - 1, preAtkTime, 2);
		hitBoxDelay(player.x,	  player.y + 1, preAtkTime, 2);
		hitBoxDelay(player.x + 1, player.y + 1, preAtkTime, 2);
		hitBoxDelay(player.x + 1, player.y,     preAtkTime, 2);
		hitBoxDelay(player.x + 1, player.y - 1, preAtkTime, 2);

		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
        selSwordAnimation = 22;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= spinSlashCost;
		Mix_PlayChannel( 0, spinSlashSound, 0 );
	}
}

void App::hitBox(int xPos, int yPos, int dmg) {
	if( xPos >= 0 && xPos <= 5 && yPos >= 0 && yPos <= 5) {
		if (map[xPos][yPos].boxHP > 0) {
			if (map[xPos][yPos].boxHP == 1) { map[xPos][yPos].prevDmg = 1; }
			else { map[xPos][yPos].prevDmg = dmg; }
			map[xPos][yPos].boxHP -= dmg;
			map[xPos][yPos].boxAtkInd = boxDmgTime;
			if ( map[xPos][yPos].boxHP <= 0 ) { Mix_PlayChannel( 2, rockBreakSound, 0 ); }
		}
}	}
void App::hitBoxDelay(int xPos, int yPos, float delay, int dmg) {
	delayedHpList.push_back( DelayedHpLoss( dmg, xPos, yPos, delay) );
}

// Map Functions
bool App::isTileValid(int xPos, int yPos) {		// Checks if a specific tile allows the Player to walk on it
	if (xPos == -1 && yPos == 0) return true;			// Start Tile
	if (xPos == 6 && yPos == 5) return true;			// Goal Tile
	if (xPos < 0 || xPos > 5 || yPos < 0 || yPos > 5) return false;		// Map boundaries are 0 to 5 on both X and Y axes
	if (map[xPos][yPos].boxHP > 0) return false;		// Players cannot walk into a Rock
	if (map[xPos][yPos].state >= 2) return false;		// Players cannot walk onto a Hole
	return true;
}
void App::clearFloor() {		// Clears all tiles
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map[i][j].state = 0;
			map[i][j].item = 0;
			map[i][j].boxHP = 0;
			map[i][j].boxType = 0;
			map[i][j].prevDmg = 0;
			map[i][j].boxAtkInd = 0;
}	}	}
void Player::reset() {
	x = 0;		y = 0;
	moving = false;
	turning = false;
	energy = 0;
	facing = 3;
}
void App::reset() {
    srand( time( 0 ) );
	clearFloor();
	player.reset();
	player.energy = energyDisplayed = startingEnergy;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	animationDisplayAmt = 0;
	delayedHpList.clear();
	level = 1;
	loadLevel(level);
	currentEnergyGain = 0;
    selSwordAnimation = -1;
    quitMenuOn = resetMenuOn = diffSelMenuOn = trainMenuOn = false;
    menuSel = true;
}
void App::next() {
	level++;
	loadLevel(level);
	player.facing = 3;
	currentEnergyGain = 0;
    selSwordAnimation = -1;
    quitMenuOn = resetMenuOn = trainMenuOn = false;
    menuSel = true;
}
void App::loadLevel(int num) {
	clearFloor();
	player.x = 0;		player.y = 0;
	if (level == 1) { generateLevel(0, 0); }
	else {
		int type = getRand(44);
		generateLevel(type); }		// Generate a Level of Random type
}
void App::generateLevel(int type, int num) {        // (levelType, difficulty)          // difficulty -1 = randomly generated difficulty
	if (num <= -1) { num = rand() % 100; }				// Random number between 0 and 99, inclusive - used to determine difficulty
	int x = level;		if ( x > 50 ) { x = 50; }
	int bound2 = 120 - x * 3 / 2;						// Bounds used in determining difficulty - based on current level number
	if (level >=  75) { bound2 = 40; }
	if (level >= 100) { bound2 = 35; }
	int bound1 = bound2 - 30;
	int diff = 0;				// difficulty 0, 1, 2 = easy, medium, hard			// level		easy	medium	hard
	if (num > bound1 && num <= bound2) { diff = 1; }								//   0			90%		10%		0%
	else if (num > bound2) { diff = 2; }											//  10			75%		25%		0%
    lvlDiff = diff;																    //  20			60%		30%		10%
																					//  30			45%		30%		25%
																					//  40			30%		30%		40%
																					//  50			15%		30%		55%
																					//  75			10%		30%		60%
																					// 100			 5%		30%		65%
    int gain = 0;       // Gain determines avg "profit" per level
    if      ( currentGameDiff == 0 ) { gain =  2 - diff * 9; }      //  2  -07  -16
    else if ( currentGameDiff == 1 ) { gain =  0 - diff * 9; }      //  0  -09  -18
    else if ( currentGameDiff == 2 ) { gain = -2 - diff * 9; }      // -2  -11  -20
    else if ( currentGameDiff == 3 ) { gain = -4 - diff * 9; }      // -4  -13  -22
    else if ( currentGameDiff == 4 ) { gain = -6 - diff * 9; }      // -6  -15  -24
	int extraBoxes = 0, extraItems = 0, floorDmgs = 0, trappedItems = 0;		// Number of things per floor based on level number and difficulty

	// Map types
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
		map[4][4].boxHP = 1;													//	]=OO=O=[
		for (int i = 0; i < 2; i++) {											//	]=OO== [
			map[1][i + 3].boxHP = 1;		map[2][i + 3].boxHP = 1;			//	] ==OO=[
			map[3][i + 1].boxHP = 1;		map[4][i + 1].boxHP = 1; }			//	]===OO=[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]== ===[
		extraItems = 9 + extraBoxes + gain;
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
		floorDmgs = rand() % 4 + 1 + diff * 4;		// 1-4	// 5-8	// 9-12
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
		floorDmgs = rand() % 5 + diff * 2;		    // 0-4	// 2-6	// 4-8
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
		for ( int i = 0; i < 2; i++ ) {														//	]===OO=[
			map[1][i + 2].boxHP = 1;	map[3][i].boxHP = 1;	map[3][i + 4].boxHP = 1;	//	]===OO=[
			map[2][i + 2].boxHP = 1;	map[4][i].boxHP = 1;	map[4][i + 4].boxHP = 1;	//	]=OO= =[
			map[1][i].state = 3;	    map[4][i + 2].state = 3; }	                        //	]=OO= =[
		                    																//	]= =OO=[
															                                //	]= =OO=[
        extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 2;	    // 0-3	// 2-5	// 4-7
	}
    else if (type == 18) {		// Paths F
		for ( int i = 0; i < 2; i++ ) {														//	] OO= =[
			map[1][i].boxHP = 1;	map[1][i + 4].boxHP = 1;	map[3][i + 2].boxHP = 1;	//	] OO= =[
			map[2][i].boxHP = 1;	map[2][i + 4].boxHP = 1;	map[4][i + 2].boxHP = 1;	//	]===OO=[
			map[0][i + 4].state = 3;	map[4][i].state = 3;    map[4][i + 4].state = 3; }  //	]===OO=[
		                    																//	]=OO= =[
															                                //	]=OO= =[
        extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 3 + diff * 2;	    // 0-2	// 2-4	// 4-6
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
		floorDmgs = rand() % 5 + diff * 2;	    // 0-4	// 2-6	// 4-8
	}
	// Cross
	else if (type == 20) {		// Cross A
		for (int i = 0; i < 4; i++ ) {											//	] O===O[
			map[0][i + 2].state = 3;	map[i + 2][0].state = 3;	}			//	] =O=O=[
		map[1][1].boxHP = 1;	map[2][4].boxHP = 1;	map[4][4].boxHP = 1;	//	] ==O==[
		map[1][5].boxHP = 1;	map[3][3].boxHP = 1;	map[5][1].boxHP = 1;	//	] =O=O=[
		map[2][2].boxHP = 1;	map[4][2].boxHP = 1;	map[5][5].boxHP = 1;	//	]=O===O[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==    [
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8
	}
	else if (type == 21) {		// Cross B
        for ( int i = 0; i < 5; i++ ) {                                                             //	]     =[
            map[i][5].state = 3;    map[i + 1][i].boxHP = 1;    map[i + 1][4 - i].boxHP = 1; }      //	]=O===O[
        map[0][2].state = 3;                                                                        //	]==O=O=[
		extraBoxes = 3 + x / 5 + diff * 2;															//	] ==O==[
		extraItems = 9 + extraBoxes + gain;														    //	]==O=O=[
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8							    //	]=O===O[
	}
	else if (type == 22) {		// Cross C
        for ( int i = 0; i < 5; i++ ) {                                                         //	]O===O=[
            map[i][i + 1].boxHP = 1;    map[i][5 - i].boxHP = 1;    map[5][i].state = 3; }      //	]=O=O= [
        map[2][0].state = 3;    map[3][0].state = 3;    map[4][0].state = 3;                    //	]==O== [
		extraBoxes = 3 + x / 5 + diff * 2;                                                      //	]=O=O= [
		extraItems = 9 + extraBoxes + gain;                                                     //	]O===O [
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8                          //	]==    [
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
        for ( int i = 0; i < 3; i++ ) {                                         // ]   O=O[
            map[i][5].state = 3;    map[5][i].state = 3;                        // ]  ==O=[
            map[i + 1][i + 1].boxHP = 1;    map[3 - i][i + 1].boxHP = 1;        // ]=O=O=O[
            map[i + 3][i + 3].boxHP = 1;    map[5 - i][i + 3].boxHP = 1; }      // ]==O== [
        map[0][4].state = 3;    map[4][0].state = 3;                            // ]=O=O  [
        map[1][4].state = 3;    map[4][1].state = 3;                            // ]====  [
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
	// Scattered
	else if (type == 32) {		// Scattered A
		for (int i = 0; i < 6; i += 2) {										//	]==O=O=[
			map[2][i + 1].boxHP = 1;											//	]===O=O[
			map[3][i].boxHP = 1;												//	]==O=O=[
			map[4][i + 1].boxHP = 1;											//	]===O=O[
			map[5][i].boxHP = 1; }												//	]==O=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]===O=O[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;		// 2-6	// 6-10	// 8-12
	}
	else if (type == 33) {		// Scattered B
		for (int i = 0; i < 4; i++) {
			map[    i % 2][2 + i].boxHP = 1;									//	]=O=O=O[
			map[2 + i % 2][2 + i].boxHP = 1;									//	]O=O=O=[
			map[4 + i % 2][2 + i].boxHP = 1; }									//	]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]O=O=O=[
		extraItems = 12 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12			//	]======[
	}
	else if (type == 34) {		// Scattered C
        for ( int i = 0; i < 3; i++ ) {                                         // ]   =O=[
            map[i][5].state = 3;    map[5][i].state = 3; }                      // ]=O=O=O[
        for ( int i = 0; i < 4; i++ ) {                                         // ]O=O=O=[
            map[i    ][3 - i].boxHP = 1;    map[i + 1][4 - i].boxHP = 1;        // ]=O=O= [
            map[i + 1][i + 2].boxHP = 1;    map[i + 2][i + 1].boxHP = 1; }      // ]==O=O [		
		extraBoxes = 4 + x / 5 + diff * 2;										// ]===O= [			
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
	else if (type == 35) {		// Scattered D
        for ( int i = 1; i <= 5; i += 2 ) {
            map[i][2].boxHP = 1;    map[i][4].boxHP = 1;                    // ] =O=O=[
            map[2][i].boxHP = 1;    map[4][i].boxHP = 1; }                  // ] O=O=O[
        map[0][4].state = 3;    map[0][5].state = 3;                        // ]==O=O=[
        map[4][0].state = 3;    map[5][0].state = 3;                        // ]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;                                  // ]==O=O=[
		extraItems = 12 + extraBoxes + gain;                                // ]====  [
        floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
    else if (type == 36) {		// Scattered E
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
    else if (type == 37) {		// Scattered F
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
	else if (type == 38) {		// Layers A
        map[1][0].state = 3;    map[2][0].state = 3;    map[4][0].state = 3;
        map[1][5].state = 3;    map[3][0].state = 3;    map[4][1].state = 3;    //	]= OO==[
		for (int yPos = 1; yPos < 6; yPos++) {									//	]==OO==[
			map[2][yPos].boxHP = 1;		map[3][yPos].boxHP = 1; }				//	]==OO==[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==OO==[
		extraItems = 10 + extraBoxes + gain;									//	]==OO =[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]=    =[
	}
	else if (type == 39) {		// Layers B
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=OO=O=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=OO=O=[
			map[2][i + 1].boxHP = 1;											//	]=OO=O=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=OO=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 40) {		// Layers C
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=O=OO=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=O=OO=[
			map[3][i + 1].boxHP = 1;											//	]=O=OO=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=O=OO=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 41) {		// Layers D
        for ( int i = 0; i < 2; i++ ) {                                                         //	]=OO===[
            map[i + 1][5].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 4][2].boxHP = 1;    //	]=OO===[
            map[i + 1][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 4][1].boxHP = 1; }  //	]==OO==[
        map[0][2].state = 3;    map[2][0].state = 3;                                            //	] =OOOO[
		extraBoxes = 4 + x / 5 + diff * 2;                                                      //	]====OO[
		extraItems = 12 + extraBoxes + gain;                                                    //	]== ===[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 42) {		// Layers E
        for ( int i = 0; i < 2; i++ ) {                                                         // ]=== ==[
            map[i][3].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 3][1].boxHP = 1;        // ]OO====[
            map[i][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 3][0].boxHP = 1; }      // ]OOOO= [
        map[3][5].state = 3;    map[5][3].state = 3;                                            // ]==OO==[
		extraBoxes = 4 + x / 5 + diff * 2;										                // ]===OO=[
		extraItems = 12 + extraBoxes + gain;                                                    // ]===OO=[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
    else if (type == 43) {		// Layers F
        for ( int i = 0; i < 2; i++ ) {                                                         // ] OO===[
            map[1][i + 4].boxHP = 1;    map[2][i + 2].boxHP = 1;    map[3][i].boxHP = 1;        // ] OO===[
            map[2][i + 4].boxHP = 1;    map[3][i + 2].boxHP = 1;    map[4][i].boxHP = 1;        // ]==OO==[
            map[0][i + 4].state = 3;    map[5][i].state = 3;                             }      // ]==OO==[
                                                                                                // ]===OO [
                                                                                                // ]===OO [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else { generateLevel(0, 100); return; }

    if      ( diff == 0 ) { trappedItems = rand() % 2; }        // 0, 1
    else if ( diff >= 1 ) { trappedItems = rand() % 3 + 2; }    // 2, 3, 4
    if ( level >= 75 ) { trappedItems++; }

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
        places.push_back( make_pair( 0, 3 ) );  //	]=== ==[
        places.push_back( make_pair( 0, 4 ) );  //	]++====[
        places.push_back( make_pair( 1, 3 ) );  //	]++=== [
        places.push_back( make_pair( 1, 4 ) );  //	] ===++[
        places.push_back( make_pair( 4, 1 ) );  //	]====++[
        places.push_back( make_pair( 4, 2 ) );  //	]== ===[
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 2 ) );
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
        places.push_back( make_pair( 2, 2 ) );  //	]====+=[
        places.push_back( make_pair( 2, 3 ) );  //	]====+=[
        places.push_back( make_pair( 3, 0 ) );  //	]==+= =[
        places.push_back( make_pair( 3, 1 ) );  //	]==+= =[
        places.push_back( make_pair( 4, 0 ) );  //	]= =++=[
        places.push_back( make_pair( 4, 1 ) );  //	]= =++=[
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
    else if (type == 18) {		// Paths F
        places.push_back( make_pair( 1, 4 ) );  //	] ++= =[
        places.push_back( make_pair( 1, 5 ) );  //	] ++= =[
        places.push_back( make_pair( 2, 0 ) );  //	]====+=[
        places.push_back( make_pair( 2, 1 ) );  //	]====+=[
        places.push_back( make_pair( 2, 4 ) );  //	]==+= =[
        places.push_back( make_pair( 2, 5 ) );  //	]==+= =[
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
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
        places.push_back( make_pair( 1, 1 ) );  //	] +====[
        places.push_back( make_pair( 1, 5 ) );  //	] =+=+=[
        places.push_back( make_pair( 2, 2 ) );  //	] ==+==[
        places.push_back( make_pair( 2, 4 ) );  //	] =+=+=[
        places.push_back( make_pair( 3, 3 ) );  //	]=+===+[
        places.push_back( make_pair( 4, 2 ) );  //	]==    [
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 21) {		// Cross B
        places.push_back( make_pair( 1, 0 ) );  //	]     =[
        places.push_back( make_pair( 1, 4 ) );  //	]=+====[
        places.push_back( make_pair( 2, 1 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 3 ) );  //	] ==+==[
        places.push_back( make_pair( 3, 2 ) );  //	]==+=+=[
        places.push_back( make_pair( 4, 1 ) );  //	]=+===+[
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
	}
	else if (type == 22) {		// Cross C
        places.push_back( make_pair( 0, 1 ) );  //	]+=====[
        places.push_back( make_pair( 0, 5 ) );  //	]=+=+= [
        places.push_back( make_pair( 1, 2 ) );  //	]==+== [
        places.push_back( make_pair( 1, 4 ) );  //	]=+=+= [
        places.push_back( make_pair( 2, 3 ) );  //	]+===+ [
        places.push_back( make_pair( 3, 2 ) );  //	]==    [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
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
        places.push_back( make_pair( 1, 1 ) );  //	]   +==[
        places.push_back( make_pair( 1, 3 ) );  //	]  ==+=[
        places.push_back( make_pair( 2, 2 ) );  //	]=+=+=+[
        places.push_back( make_pair( 3, 1 ) );  //	]==+== [
        places.push_back( make_pair( 3, 3 ) );  //	]=+=+  [
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
    // Scattered
	else if (type == 32) {		// Scattered A
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
	else if (type == 33) {		// Scattered B
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
	else if (type == 34) {		// Scattered C
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
	else if (type == 35) {		// Scattered D
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
    else if (type == 36) {		// Scattered E
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
    else if (type == 37) {		// Scattered F
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
	else if (type == 38) {		// Layers A
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
	else if (type == 39) {		// Layers B
        places.push_back( make_pair( 2, 1 ) );  //	]    ==[
        places.push_back( make_pair( 2, 2 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 3 ) );  //	]==+=+=[
        places.push_back( make_pair( 2, 4 ) );  //	]==+=+=[
        places.push_back( make_pair( 4, 1 ) );  //	]==+=+=[
        places.push_back( make_pair( 4, 2 ) );  //	]=     [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 40) {		// Layers C
        places.push_back( make_pair( 3, 1 ) );  //	]    ==[
        places.push_back( make_pair( 3, 2 ) );  //	]===++=[
        places.push_back( make_pair( 3, 3 ) );  //	]===++=[
        places.push_back( make_pair( 3, 4 ) );  //	]===++=[
        places.push_back( make_pair( 4, 1 ) );  //	]===++=[
        places.push_back( make_pair( 4, 2 ) );  //	]=     [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	else if (type == 41) {		// Layers D
        places.push_back( make_pair( 1, 4 ) );  //	]=+====[
        places.push_back( make_pair( 1, 5 ) );  //	]=+====[
        places.push_back( make_pair( 2, 2 ) );  //	]==+===[
        places.push_back( make_pair( 2, 3 ) );  //	] =++==[
        places.push_back( make_pair( 3, 2 ) );  //	]====++[
        places.push_back( make_pair( 4, 1 ) );  //	]== ===[
        places.push_back( make_pair( 5, 1 ) );
	}
	else if (type == 42) {		// Layers E
        places.push_back( make_pair( 0, 4 ) );  //	]=== ==[
        places.push_back( make_pair( 1, 4 ) );  //	]++====[
        places.push_back( make_pair( 2, 3 ) );  //	]==++= [
        places.push_back( make_pair( 3, 2 ) );  //	]===+==[
        places.push_back( make_pair( 3, 3 ) );  //	]====+=[
        places.push_back( make_pair( 4, 0 ) );  //	]====+=[
        places.push_back( make_pair( 4, 1 ) );
	}
    else if (type == 43) {		// Layers F
        places.push_back( make_pair( 1, 4 ) );  //	] ++===[
        places.push_back( make_pair( 1, 5 ) );  //	] +====[
        places.push_back( make_pair( 2, 3 ) );  //	]==+===[
        places.push_back( make_pair( 2, 5 ) );  //	]===+==[
        places.push_back( make_pair( 3, 0 ) );  //	]====+ [
        places.push_back( make_pair( 3, 2 ) );  //	]===++ [
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
	}
    else { generateItems( amt, 0, 0 ); return; }

    for ( int i = 0; i < amt; i += itemWorth ) {
        if ( !places.empty() ) {
            int randPlace = rand() % places.size();
            xPos = places[randPlace].first;
            yPos = places[randPlace].second;
            if ( xPos != -1 && yPos != -1 && map[xPos][yPos].state <= 1 && map[xPos][yPos].boxHP <= 3 ) {
                map[xPos][yPos].item++;
                if ( map[xPos][yPos].boxHP > 0 ) { map[xPos][yPos].boxType = 2; }
                if ( map[xPos][yPos].item >= 2 ) { remove( places.begin(), places.end(), places[randPlace] ); }
            }
        }
    }

    for ( int i = 0; i < places.size(); i++ ) {
        xPos = places[i].first;
        yPos = places[i].second;
        if ( map[xPos][yPos].item >= 1 ) { remove( places.begin(), places.end(), places[i] ); }
    }

    for ( int i = 0; i < trapAmt; i++ ) {
        if ( !places.empty() ) {
            int randPlace = rand() % places.size();
            xPos = places[randPlace].first;
            yPos = places[randPlace].second;
            if ( xPos != -1 && yPos != -1 && map[xPos][yPos].state <= 1 && map[xPos][yPos].boxHP <= 3 && map[xPos][yPos].item == 0 ) {
                map[xPos][yPos].item = -1;
                if ( map[xPos][yPos].boxHP > 0 ) { map[xPos][yPos].boxType = 2; }
                remove( places.begin(), places.end(), places[randPlace] );
            }
        }
    }
}
void App::generateBoxes(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
    vector<pair<int, int>> places;
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
        places.push_back( make_pair( 1, 4 ) );  //	]=OO=O=[
        places.push_back( make_pair( 2, 3 ) );  //	]=OO== [
        places.push_back( make_pair( 2, 4 ) );  //	] ==OO=[
        places.push_back( make_pair( 3, 1 ) );  //	]===OO=[
        places.push_back( make_pair( 3, 2 ) );  //	]== ===[
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 4 ) );
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
        places.push_back( make_pair( 1, 2 ) );  //	]===OO=[
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
        places.push_back( make_pair( 1, 0 ) );  //	] OO= =[
        places.push_back( make_pair( 1, 1 ) );  //	] OO= =[
        places.push_back( make_pair( 1, 4 ) );  //	]===OO=[
        places.push_back( make_pair( 1, 5 ) );  //	]===OO=[
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
        places.push_back( make_pair( 1, 1 ) );  //	] O===O[
        places.push_back( make_pair( 1, 5 ) );  //	] =O=O=[
        places.push_back( make_pair( 2, 2 ) );  //	] ==O==[
        places.push_back( make_pair( 2, 4 ) );  //	] =O=O=[
        places.push_back( make_pair( 3, 3 ) );  //	]=O===O[
        places.push_back( make_pair( 4, 2 ) );  //	]==    [
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 5, 1 ) );
        places.push_back( make_pair( 5, 5 ) );
	}
	else if (type == 21) {		// Cross B
        places.push_back( make_pair( 1, 0 ) );  //	]     =[
        places.push_back( make_pair( 1, 4 ) );  //	]=O===O[
        places.push_back( make_pair( 2, 1 ) );  //	]==O=O=[
        places.push_back( make_pair( 2, 3 ) );  //	] ==O==[
        places.push_back( make_pair( 3, 2 ) );  //	]==O=O=[
        places.push_back( make_pair( 4, 1 ) );  //	]=O===O[
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 22) {		// Cross C
        places.push_back( make_pair( 0, 1 ) );  //	]O===O=[
        places.push_back( make_pair( 0, 5 ) );  //	]=O=O= [
        places.push_back( make_pair( 1, 2 ) );  //	]==O== [
        places.push_back( make_pair( 1, 4 ) );  //	]=O=O= [
        places.push_back( make_pair( 2, 3 ) );  //	]O===O [
        places.push_back( make_pair( 3, 2 ) );  //	]==    [
        places.push_back( make_pair( 3, 4 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 5 ) );
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
        places.push_back( make_pair( 1, 1 ) );  //	]   O=O[
        places.push_back( make_pair( 1, 3 ) );  //	]  ==O=[
        places.push_back( make_pair( 2, 2 ) );  //	]=O=O=O[
        places.push_back( make_pair( 3, 1 ) );  //	]==O== [
        places.push_back( make_pair( 3, 3 ) );  //	]=O=O  [
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
	// Scattered
	else if (type == 32) {		// Scattered A
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
	else if (type == 33) {		// Scattered B
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
	else if (type == 34) {		// Scattered C
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
	else if (type == 35) {		// Scattered D
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
    else if (type == 36) {		// Scattered E
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
    else if (type == 37) {		// Scattered F
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
	else if (type == 38) {		// Layers A
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
	else if (type == 39) {		// Layers B
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
	else if (type == 40) {		// Layers C
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
	else if (type == 41) {		// Layers D
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
	else if (type == 42) {		// Layers E
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
    else if (type == 43) {		// Layers F
        places.push_back( make_pair( 1, 4 ) );  //	] OO===[
        places.push_back( make_pair( 1, 5 ) );  //	] OO===[
        places.push_back( make_pair( 2, 2 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 3 ) );  //	]==OO==[
        places.push_back( make_pair( 2, 4 ) );  //	]===OO [
        places.push_back( make_pair( 2, 5 ) );  //	]===OO [
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 1 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 0 ) );
        places.push_back( make_pair( 4, 1 ) );
	}
    else { generateBoxes( amt, 0 ); return; }

    while ( amt > 0 && !places.empty() ) {
        int randPlace = rand() % places.size();
        xPos = places[randPlace].first;
        yPos = places[randPlace].second;
        if ( xPos != -1 && yPos != -1 ) {
            if ( map[xPos][yPos].item != 0 ) {
                map[xPos][yPos].boxType = 2;
                if ( map[xPos][yPos].boxHP >= 3 ) { remove( places.begin(), places.end(), places[randPlace] ); }
                else {
                    map[xPos][yPos].boxHP++;
                    amt--;
                }
            }
            else {
                if ( map[xPos][yPos].boxHP >= 3 ) { map[xPos][yPos].boxType = 1; }
                if ( map[xPos][yPos].boxHP >= 5 ) { remove( places.begin(), places.end(), places[randPlace] ); }
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
        places.push_back( make_pair( 1, 2 ) );  //	]=== ==[
        places.push_back( make_pair( 2, 1 ) );  //	]===X==[
        places.push_back( make_pair( 2, 2 ) );  //	]===XX [
        places.push_back( make_pair( 3, 3 ) );  //	] XX===[
        places.push_back( make_pair( 3, 4 ) );  //	]==X===[
        places.push_back( make_pair( 4, 3 ) );  //	]== ===[
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
        places.push_back( make_pair( 0, 2 ) );  //	]==X===[
        places.push_back( make_pair( 1, 1 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]==XX=X[
        places.push_back( make_pair( 2, 2 ) );  //	]X=XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 5 ) );  //	]===X==[
        places.push_back( make_pair( 3, 0 ) );
        places.push_back( make_pair( 3, 2 ) );
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
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
        places.push_back( make_pair( 1, 3 ) );  //	]  =x==[
        places.push_back( make_pair( 2, 3 ) );  //	]===x==[
        places.push_back( make_pair( 3, 1 ) );  //	]=xx=xx[
        places.push_back( make_pair( 3, 2 ) );  //	]===x==[
        places.push_back( make_pair( 3, 4 ) );  //	]= =x= [
        places.push_back( make_pair( 3, 5 ) );  //	]===== [
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
        places.push_back( make_pair( 1, 4 ) );  //	]==X===[
        places.push_back( make_pair( 2, 4 ) );  //	]=XX===[
        places.push_back( make_pair( 2, 5 ) );  //	]===X X[
        places.push_back( make_pair( 3, 2 ) );  //	]===X X[
        places.push_back( make_pair( 3, 3 ) );  //	]= ====[
        places.push_back( make_pair( 5, 2 ) );  //	]= ====[
        places.push_back( make_pair( 5, 3 ) );
	}
    else if (type == 18) {		// Paths F
        places.push_back( make_pair( 1, 2 ) );  //	] === =[
        places.push_back( make_pair( 1, 3 ) );  //	] ==X =[
        places.push_back( make_pair( 2, 2 ) );  //	]=XX===[
        places.push_back( make_pair( 2, 3 ) );  //	]=XX===[
        places.push_back( make_pair( 3, 1 ) );  //	]===X =[
        places.push_back( make_pair( 3, 4 ) );  //	]==== =[
	}
    else if (type == 19) {		// Paths G
        places.push_back( make_pair( 1, 1 ) );  //	] =====[
        places.push_back( make_pair( 1, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 2 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 3 ) );  //	]==XX==[
        places.push_back( make_pair( 3, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 3, 3 ) );  //	]===== [
        places.push_back( make_pair( 4, 1 ) );
        places.push_back( make_pair( 4, 4 ) );
	}
	// Cross
	else if (type == 20) {			// Cross A
        places.push_back( make_pair( 1, 3 ) );  //	] ==X==[
        places.push_back( make_pair( 2, 3 ) );  //	] ==X==[
        places.push_back( make_pair( 3, 1 ) );  //	] XX=XX[
        places.push_back( make_pair( 3, 2 ) );  //	] ==X==[
        places.push_back( make_pair( 3, 4 ) );  //	]===X==[
        places.push_back( make_pair( 3, 5 ) );  //	]==    [
        places.push_back( make_pair( 4, 3 ) );
        places.push_back( make_pair( 5, 3 ) );
	}
	else if (type == 21) {			// Cross B
        places.push_back( make_pair( 1, 2 ) );  //	]     =[
        places.push_back( make_pair( 2, 2 ) );  //	]===X==[
        places.push_back( make_pair( 3, 0 ) );  //	]===X==[
        places.push_back( make_pair( 3, 1 ) );  //	] XX=XX[
        places.push_back( make_pair( 3, 3 ) );  //	]===X==[
        places.push_back( make_pair( 3, 4 ) );  //	]===X==[
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 5, 2 ) );
	}
	else if (type == 22) {			// Cross C
        places.push_back( make_pair( 0, 3 ) );  //	]==X===[
        places.push_back( make_pair( 1, 3 ) );  //	]==X== [
        places.push_back( make_pair( 2, 1 ) );  //	]XX=XX [
        places.push_back( make_pair( 2, 2 ) );  //	]==X== [
        places.push_back( make_pair( 2, 4 ) );  //	]==X== [
        places.push_back( make_pair( 2, 5 ) );  //	]==    [
        places.push_back( make_pair( 3, 3 ) );
        places.push_back( make_pair( 4, 3 ) );
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
        places.push_back( make_pair( 1, 2 ) );  //	]   =X=[
        places.push_back( make_pair( 2, 1 ) );  //	]  XX=X[
        places.push_back( make_pair( 2, 3 ) );  //	]==X=X=[
        places.push_back( make_pair( 2, 4 ) );  //	]=X=XX [
        places.push_back( make_pair( 3, 2 ) );  //	]==X=  [
        places.push_back( make_pair( 3, 4 ) );  //	]====  [
        places.push_back( make_pair( 4, 2 ) );
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
	// Scattered
	else if (type == 32) {			// Scattered A
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
	else if (type == 33) {			// Scattered B
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
	else if (type == 34) {			// Scattered C
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
	else if (type == 35) {			// Scattered D
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
    else if (type == 36) {			// Scattered E
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
    else if (type == 37) {		// Scattered F
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
	else if (type == 38) {			// Layers A
        places.push_back( make_pair( 1, 1 ) );  //	]= ==X=[
        places.push_back( make_pair( 1, 2 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 3 ) );  //	]=X==X=[
        places.push_back( make_pair( 1, 4 ) );  //	]=X==X=[
        places.push_back( make_pair( 4, 2 ) );  //	]=X== =[
        places.push_back( make_pair( 4, 3 ) );  //	]=    =[
        places.push_back( make_pair( 4, 4 ) );
        places.push_back( make_pair( 4, 5 ) );
	}
	else if (type == 39) {			// Layers B
        places.push_back( make_pair( 3, 1 ) );  //	]    ==[
        places.push_back( make_pair( 3, 2 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 3 ) );  //	]===X=X[
        places.push_back( make_pair( 3, 4 ) );  //	]===X=X[
        places.push_back( make_pair( 5, 1 ) );  //	]===X=X[
        places.push_back( make_pair( 5, 2 ) );//	]=     [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 40) {			// Layers C
        places.push_back( make_pair( 2, 1 ) );  //	]    ==[
        places.push_back( make_pair( 2, 2 ) );  //	]==X==X[
        places.push_back( make_pair( 2, 3 ) );  //	]==X==X[
        places.push_back( make_pair( 2, 4 ) );  //	]==X==X[
        places.push_back( make_pair( 5, 1 ) );  //	]==X==X[
        places.push_back( make_pair( 5, 2 ) );  //	]=     [
        places.push_back( make_pair( 5, 3 ) );
        places.push_back( make_pair( 5, 4 ) );
	}
	else if (type == 41) {			// Layers D
        places.push_back( make_pair( 0, 4 ) );  //	]X=====[
        places.push_back( make_pair( 0, 5 ) );  //	]X=====[
        places.push_back( make_pair( 1, 2 ) );  //	]X=====[
        places.push_back( make_pair( 1, 3 ) );  //	]=X====[
        places.push_back( make_pair( 2, 1 ) );  //	] X====[
        places.push_back( make_pair( 3, 1 ) );  //	]==XX==[
        places.push_back( make_pair( 4, 0 ) );  //	]== =XX[
        places.push_back( make_pair( 5, 0 ) );
	}
	else if (type == 42) {			// Layers E
        places.push_back( make_pair( 0, 5 ) );  //	]XX= ==[
        places.push_back( make_pair( 1, 5 ) );  //	]==XX==[
        places.push_back( make_pair( 2, 4 ) );  //	]====X [
        places.push_back( make_pair( 3, 4 ) );  //	]====X=[
        places.push_back( make_pair( 4, 2 ) );  //	]=====X[
        places.push_back( make_pair( 4, 3 ) );  //	]=====X[
        places.push_back( make_pair( 5, 0 ) );
        places.push_back( make_pair( 5, 1 ) );
	}
    else if (type == 43) {			// Layers F
        places.push_back( make_pair( 1, 2 ) );  //	] ==X==[
        places.push_back( make_pair( 1, 3 ) );  //	] ==X==[
        places.push_back( make_pair( 2, 0 ) );  //	]=X==X=[
        places.push_back( make_pair( 2, 1 ) );  //	]=X==X=[
        places.push_back( make_pair( 3, 4 ) );  //	]==X== [
        places.push_back( make_pair( 3, 5 ) );  //	]==X== [
        places.push_back( make_pair( 4, 2 ) );
        places.push_back( make_pair( 4, 3 ) );
	}
    else { generateFloor( amt, 0 ); return; }

    while (amt > 0 && !places.empty()) {
		int randPlace = rand() % places.size();
        xPos = places[randPlace].first;
        yPos = places[randPlace].second;
		if (xPos != -1 && yPos != -1) {
			map[xPos][yPos].state = 1;
            remove( places.begin(), places.end(), places[randPlace] );
			amt--;
        }
	}
}

void App::test() {
	reset();
	clearFloor();
	level = -1;
	player.energy = energyDisplayed = 10000;
	
    for ( int i = 0; i < 6; i++ ) { map[i][5].state = 3; }
    for ( int i = 1; i < 5; i++ ) {
        map[0][i].boxHP = 5;
        map[1][i].boxHP = 5;
        map[2][i].boxHP = 5;    map[2][i].state = 1;
        map[4][i].boxHP = 3;    map[4][i].item = 1;     map[4][i].boxType = 2;
        map[5][i].boxHP = 3;    map[5][i].item = 2;     map[5][i].boxType = 2;
        map[i][0].item = -1;
    }
    map[4][0].item = -1;    map[5][0].item = -1;
    map[2][4].state = 0;
}
