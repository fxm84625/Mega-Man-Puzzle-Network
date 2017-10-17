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
const int crossCost = 125;
const int spinCost  = 150;
const int stepCost  = 175;
const int lifeCost  = 300;

const int startingEnergy = 300;
const int energyGainAmt = 100;			    // How much Energy the player gains when moving to a Pick-Up
const int itemWorth = energyGainAmt / 50;	// Item "Worth" is used to generate Levels

const float iconDisplayTime = 0.45;		// Energy Gain Icon Display time
const float boxDmgTime = 0.4;			// Box HP damage indicator time
const float moveAnimationTime = 0.175;	// Player movement time
const float menuExitTime = 0.2;         // Delay after coming out of a Menu

const float preAtkTime   = 0.10;
const float swordAtkTime = 0.42;
const float longAtkTime  = 0.42;
const float wideAtkTime  = 0.42;
const float crossAtkTime = 0.42;
const float spinAtkTime  = 0.42;
const float stepAtkTime  = 0.38;
const float lifeAtkTime  = 0.52;

const float orthoX1 = -1.0;
const float orthoX2 = 7.0;
const float orthoY1 = 0.0;
const float orthoY2 = 5.4;

const float overallScale = 1.25;
const float scaleX = 0.8 * overallScale;
const float scaleY = 0.48 * overallScale;
const float playerScale = 1.2;
const float itemScale = 1.1;

Player::Player() : x(0), y(0), facing(3), moving(false), turning(false), moveDir(-1), energy(0) {}
Tile::Tile() : state(0), item(-1), boxHP(0), boxType(0), boxAtkInd(0.0), prevDmg(0) {}
DelayedHpLoss::DelayedHpLoss() : dmg(0), xPos(0), yPos(0), delay(0.0) {}
DelayedSound::DelayedSound() : soundName(""), delay(0.0) {}

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

long getRand(long num) {	// Returns pseudo-random number between 0 and (num-1)
	srand(time(NULL));				// Gets a new set of random numbers based on time
	return rand() % num;			// Returns a Random number from the generated string
}

App::App() : done(false), timeLeftOver(0.0), fixedElapsed(0.0), lastFrameTicks(0.0),
			 menuDisplay(false), quitMenuOn(false), resetMenuOn(false), trainMenuOn(false), menuSel(true),
             player(Player()), level(0), currentEnergyGain(0),
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
	megamanMoveSheet = loadTexture("Pics\\Mega Man Move Sheet.png");
	megamanAtkSheet  = loadTexture("Pics\\Mega Man Atk Sheet.png");
	rockSheet        = loadTexture("Pics\\Rock Sheet 1.png");
	rockSheetItem    = loadTexture("Pics\\Rock Sheet 2.png");
	rockDeathSheet   = loadTexture("Pics\\Death Sheet.png");
	floorSheet       = loadTexture("Pics\\Tile Sheet 2.png");
	floorMoveSheet   = loadTexture("Pics\\Move Tile Sheet 2.png");
	floorBottomPic1  = loadTexture("Pics\\Tile Bottom 1.png");
	floorBottomPic2  = loadTexture("Pics\\Tile Bottom 2.png");
	energySheet      = loadTexture("Pics\\Energy Sheet.png");
	energyGetPic     = loadTexture("Pics\\Energy Get.png");
	bgA              = loadTexture("Pics\\Background A.png");
	bgB              = loadTexture("Pics\\Background B.png");
	bgC              = loadTexture("Pics\\Background C.png");
	menuPic          = loadTexture("Pics\\Menu.png");
	lvBarPic         = loadTexture("Pics\\Level Bar.png");
	healthBoxPic     = loadTexture("Pics\\Health Box.png");
	infoBoxPic       = loadTexture("Pics\\InfoPic.png");
    musicDisplayPic  = loadTexture("Pics\\MusicDisplay.png");
    quitPicY         = loadTexture("Pics\\QuitY.png");
    quitPicN         = loadTexture("Pics\\QuitN.png");
    resetPicY        = loadTexture("Pics\\ResetY.png");
    resetPicN        = loadTexture("Pics\\ResetN.png");
    trainPicY        = loadTexture("Pics\\TrainY.png");
    trainPicN        = loadTexture("Pics\\TrainN.png");

	swordAtkSheet1 = loadTexture("Pics\\Sword Attacks 2\\Sword Sheet 1.png");
	swordAtkSheet3 = loadTexture("Pics\\Sword Attacks 2\\Sword Sheet 3.png");
	longAtkSheet1  = loadTexture("Pics\\Sword Attacks 2\\Long Sheet 1.png");
	longAtkSheet3  = loadTexture("Pics\\Sword Attacks 2\\Long Sheet 3.png");
	wideAtkSheet1  = loadTexture("Pics\\Sword Attacks 2\\Wide Sheet 1.png");
	wideAtkSheet3  = loadTexture("Pics\\Sword Attacks 2\\Wide Sheet 3.png");
	crossAtkSheet1 = loadTexture("Pics\\Sword Attacks 2\\Cross Sheet 1-3.png");
	crossAtkSheet3 = loadTexture("Pics\\Sword Attacks 2\\Cross Sheet 3-3.png");
	spinAtkSheet1  = loadTexture("Pics\\Sword Attacks 2\\Spin Sheet 1.png");
	spinAtkSheet3  = loadTexture("Pics\\Sword Attacks 2\\Spin Sheet 3.png");
	stepAtkSheet1  = loadTexture("Pics\\Sword Attacks 2\\Wide Sheet 1.png");
	stepAtkSheet3  = loadTexture("Pics\\Sword Attacks 2\\Wide Sheet 3.png");
	lifeAtkSheet1  = loadTexture("Pics\\Sword Attacks 2\\Life Sheet 1.png");
	lifeAtkSheet3  = loadTexture("Pics\\Sword Attacks 2\\Life Sheet 3.png");

	Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 4, 4096 );
	Mix_Volume( -1, 32 );
	Mix_Volume(  0, 48 );
    Mix_Volume(  3, 12 );
	
	swordSound      = Mix_LoadWAV( "Sounds\\SwordSwing.wav" );
	lifeSwordSound  = Mix_LoadWAV( "Sounds\\LifeSword.wav" );
	itemSound       = Mix_LoadWAV( "Sounds\\GotItem2.wav" );
	rockBreakSound  = Mix_LoadWAV( "Sounds\\AreaGrabHit2.wav" );
	panelBreakSound = Mix_LoadWAV( "Sounds\\PanelCrack3.wav" );

	menuOpenSound   = Mix_LoadWAV( "Sounds\\ChipDesc2.wav" );
	menuCloseSound  = Mix_LoadWAV( "Sounds\\ChipDescClose2.wav" );

    quitCancelSound = Mix_LoadWAV( "Sounds\\QuitCancel2.wav" );
    quitChooseSound = Mix_LoadWAV( "Sounds\\QuitChoose2.wav" );
    quitOpenSound   = Mix_LoadWAV( "Sounds\\QuitOpen2.wav" );

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
}
App::~App() {
	Mix_FreeChunk( swordSound );
	Mix_FreeChunk( lifeSwordSound );
	Mix_FreeChunk( itemSound );
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

    if ( animationDisplayAmt > 0 ) { return false; }
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
	// Display parameter for Energy gain
	chargeDisplayPlusAmt -= elapsed;
	if (chargeDisplayPlusAmt < 0) { chargeDisplayPlusAmt = 0; }

	// Display parameter for Energy usage
	chargeDisplayMinusAmt -= elapsed;
	if (chargeDisplayMinusAmt < 0) { chargeDisplayMinusAmt = 0; }

	// Display parameter for attack, movement, and level transition animations
	animationDisplayAmt -= elapsed;
	// Step sword parameter
	if (animationDisplayAmt < stepAtkTime && selSwordAnimation == 5 && animationType == 1) { player.moving = false; }
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
	else if (animationDisplayAmt < 0) {
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
			if      ( delayedSoundList[i].soundName == "sword" ) { Mix_PlayChannel( 0, swordSound, 0 ); }
			else if ( delayedSoundList[i].soundName == "life" )  { Mix_PlayChannel( 0, lifeSwordSound, 0 ); }
			delayedSoundList.erase( delayedSoundList.begin() + i);
	}	}

    // Handles display time of Music track Number and Name
    musicSwitchDisplayAmt -= elapsed;
    if ( musicSwitchDisplayAmt < 0 ) { musicSwitchDisplayAmt = 0; }
}
void App::checkKeys() {
	// Menu Keys and Controls
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { done = true; }
		else if (event.type == SDL_KEYDOWN) {		// Read Single Button presses

            if ( !menuDisplay && !quitMenuOn && !resetMenuOn && !trainMenuOn && animationDisplayAmt <= 0 ) {
                // Show Reset confirmation Menu
				if (event.key.keysym.scancode == SDL_SCANCODE_R) {
                    resetMenuOn = true;
                    menuSel = true;
                    Mix_PlayChannel( 1, quitOpenSound, 0 ); }

                // Show Quit confirmation Menu
				else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    quitMenuOn = true;
                    menuSel = true;
                    Mix_PlayChannel( 1, quitOpenSound, 0 ); }

                // Show Training confirmation Menu
				else if (event.key.keysym.scancode == SDL_SCANCODE_X) {
                    trainMenuOn = true;
                    menuSel = true;
                    Mix_PlayChannel( 1, quitOpenSound, 0 ); }

                // Music Controls
				else if (event.key.keysym.scancode == SDL_SCANCODE_C) { changeMusic(); }
                else if (event.key.keysym.scancode == SDL_SCANCODE_V) { toggleMusic(); }
				//else if (event.key.keysym.scancode == SDL_SCANCODE_Z) { test2(); }

				// Turn around without moving
				else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (player.facing == 1) { face(3); }
					else if (player.facing == 3) { face(1); } }

				// Show Help Info Menu
				else if (event.key.keysym.scancode == SDL_SCANCODE_TAB || event.key.keysym.scancode == SDL_SCANCODE_F) {
					menuDisplay = true;
					Mix_PlayChannel( 1, menuOpenSound, 0 ); }
			}
			// If the Help Menu is displayed, any button except for Music controls will close the menu
			else if ( menuDisplay ) {
                if      (event.key.keysym.scancode == SDL_SCANCODE_C) { changeMusic(); }
                else if (event.key.keysym.scancode == SDL_SCANCODE_V) { toggleMusic(); }
                else {
				    menuDisplay = false; animationDisplayAmt = menuExitTime;
				    Mix_PlayChannel( 1, menuCloseSound, 0 ); } }

            // Menu Controls
            else if ( quitMenuOn || resetMenuOn || trainMenuOn ) {
                if ( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE || event.key.keysym.scancode == SDL_SCANCODE_TAB )  {
                    quitMenuOn = false;     resetMenuOn = false;    trainMenuOn = false;
                    animationDisplayAmt = menuExitTime;
                    Mix_PlayChannel( 1, quitCancelSound, 0 ); }
                else if ( event.key.keysym.scancode == SDL_SCANCODE_A || event.key.keysym.scancode == SDL_SCANCODE_LEFT )  {
                    if ( !menuSel ) { Mix_PlayChannel( 1, quitChooseSound, 0 ); }
                    menuSel = true;
                }
                else if ( event.key.keysym.scancode == SDL_SCANCODE_D || event.key.keysym.scancode == SDL_SCANCODE_RIGHT ) {
                    if (  menuSel ) { Mix_PlayChannel( 1, quitChooseSound, 0 ); }
                    menuSel = false;
                }
                else if ( event.key.keysym.scancode == SDL_SCANCODE_1      || event.key.keysym.scancode == SDL_SCANCODE_KP_1 ||
                          event.key.keysym.scancode == SDL_SCANCODE_2      || event.key.keysym.scancode == SDL_SCANCODE_KP_2 ||
                          event.key.keysym.scancode == SDL_SCANCODE_3      || event.key.keysym.scancode == SDL_SCANCODE_KP_3 ||
                          event.key.keysym.scancode == SDL_SCANCODE_4      || event.key.keysym.scancode == SDL_SCANCODE_KP_4 ||
                          event.key.keysym.scancode == SDL_SCANCODE_5      || event.key.keysym.scancode == SDL_SCANCODE_KP_5 ||
                          event.key.keysym.scancode == SDL_SCANCODE_6      || event.key.keysym.scancode == SDL_SCANCODE_KP_6 ||
                          event.key.keysym.scancode == SDL_SCANCODE_7      || event.key.keysym.scancode == SDL_SCANCODE_KP_7 ||
                          event.key.keysym.scancode == SDL_SCANCODE_RETURN || event.key.keysym.scancode == SDL_SCANCODE_KP_ENTER ) {
                    if ( menuSel ) {
                        if ( quitMenuOn ) { done = true; }
                        else if ( resetMenuOn ) { reset(); }
                        else if ( trainMenuOn ) { test(); }
                        animationDisplayAmt = menuExitTime;
                        Mix_PlayChannel( 1, quitChooseSound, 0 );
                    }
                    else {
                        quitMenuOn = false;     resetMenuOn = false;    trainMenuOn = false;
                        animationDisplayAmt = menuExitTime;
                        Mix_PlayChannel( 1, quitCancelSound, 0 );
            } } }
	} }

    if ( !quitMenuOn && !resetMenuOn && !trainMenuOn && animationDisplayAmt <= 0 ) {
        // The player can't Move or Attack until after the previous Action is completed

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
		else if (keystates[SDL_SCANCODE_1] || keystates[SDL_SCANCODE_KP_1]) { swordAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_2] || keystates[SDL_SCANCODE_KP_2]) { longAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_3] || keystates[SDL_SCANCODE_KP_3]) { wideAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_4] || keystates[SDL_SCANCODE_KP_4]) { crossAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_5] || keystates[SDL_SCANCODE_KP_5]) { spinAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_6] || keystates[SDL_SCANCODE_KP_6]) { stepAtk(player.facing); }
		else if (keystates[SDL_SCANCODE_7] || keystates[SDL_SCANCODE_KP_7]) { lifeAtk(player.facing); }
    }
}

// Display Functions
void App::Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	drawBg();
	drawFloor();

	// Draw game elements, back row first, so the front row is displayed in front
	for (int i = 5; i >= 0; i--) {
		drawItems(i);
		drawBoxes(i);
		
        if ( i == 0 ) {
            if ( menuDisplay ) { drawMenu(); }
            drawTextUI();
        }

        if ( i == player.y ) { drawPlayer(); }
    }

	// Render Sword Attacking Animations
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	//glScalef(0.6, 0.6, 0.0);
	glTranslatef(0.5, 1.2, 0.0);
	if (animationDisplayAmt > 0 && animationType == 1) {
		if      (selSwordAnimation == 0) { swordDisplay(player.facing); }
		else if (selSwordAnimation == 1) { longDisplay(player.facing); }
		else if (selSwordAnimation == 2) { wideDisplay(player.facing); }
		else if (selSwordAnimation == 3) { crossDisplay(player.facing); }
		else if (selSwordAnimation == 4) { spinDisplay(player.facing); }
		else if (selSwordAnimation == 5) { stepDisplay(player.facing); }
		else if (selSwordAnimation == 6) { lifeDisplay(player.facing); } }

    if ( musicSwitchDisplayAmt || menuDisplay ) { displayMusic(); }

    if ( quitMenuOn ) { drawQuitMenu(); }
    else if ( resetMenuOn ) { drawResetMenu(); }
    else if ( trainMenuOn ) { drawTrainMenu(); }

	SDL_GL_SwapWindow(displayWindow);
}
void App::drawBg() {
	GLuint texture;
	float bgSizeX = 0.768 * 8 / 4;
	float bgSizeY = 0.768 * 8 / 2.7;
	if (difficulty == 0) { texture = bgA; }
	else if (difficulty == 1) { texture = bgB; }
	else if (difficulty == 2) { texture = bgC; }
	glTranslatef(0.064 * 2 / 4 * bgAnimationAmt, -0.064 * 2 / 2.7 * bgAnimationAmt, 0);
	GLfloat place[] = { -bgSizeX, bgSizeY, -bgSizeX, -bgSizeY,     bgSizeX, -bgSizeY, bgSizeX, bgSizeY };
	drawSpriteSheetSprite(place, texture, 0, 1, 1);
}
void App::drawMenu() {
	glLoadIdentity();
	float menuSizeX = 1;
	float menuSizeY = 1;
	GLfloat place[] = { -menuSizeX, menuSizeY, -menuSizeX, -menuSizeY,     menuSizeX, -menuSizeY, menuSizeX, menuSizeY };
	drawSpriteSheetSprite(place, menuPic, 0, 1, 1);
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
    if ( menuSel ) { drawSpriteSheetSprite( place, resetPicY, 0, 1, 1 ); }
    else           { drawSpriteSheetSprite( place, resetPicN, 0, 1, 1 ); }
}
void App::drawTrainMenu() {
    glLoadIdentity();
    GLfloat place[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
    if ( menuSel ) { drawSpriteSheetSprite( place, trainPicY, 0, 1, 1 ); }
    else           { drawSpriteSheetSprite( place, trainPicN, 0, 1, 1 ); }
}
void App::drawPlayer() {
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	//glScalef(0.6, 0.6, 0.0);
	glTranslatef(0.5, 1.2, 0.0);
	
	int picIndex = 0;
	float displace = 0.0;
	float displace2 = 0.3;

	// Step Sword Move Animation
	if (animationType == 1 && selSwordAnimation == 5) {
		float playerSizeX = 0.66 * playerScale;
		float playerSizeY = 0.56 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
		if		(animationDisplayAmt > stepAtkTime + moveAnimationTime * 1.0) { displace = 2.00 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.9) { displace = 1.975 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.8) { displace = 1.95 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.7) { displace = 1.925 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.6) { displace = 1.90 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.5) { displace = 1.52 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.4) { displace = 1.14 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.3) { displace = 0.76 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.2) { displace = 0.38 * scaleX; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.1) { displace = 0.00 * scaleX; }
		if      (animationDisplayAmt > stepAtkTime - 0.06) { picIndex = 1; }
		else if (animationDisplayAmt > stepAtkTime - 0.10) { picIndex = 2; }
		else if (animationDisplayAmt > stepAtkTime - 0.14) { picIndex = 3; }
		else if (animationDisplayAmt > stepAtkTime - 0.20) { picIndex = 4; }
		else if (animationDisplayAmt > stepAtkTime - 0.26) { picIndex = 5; }
		else if (animationDisplayAmt > stepAtkTime - 0.32) { picIndex = 6; }
		else if (animationDisplayAmt > 0)                  { picIndex = 7; }
		if (player.facing == 1) {
			glTranslatef( displace - displace2, 0.02, 0);
			drawSpriteSheetSprite(playerPlace, megamanAtkSheet, picIndex + 8, 8, 2); }
		else if (player.facing == 3) {
			glTranslatef(-displace + displace2, 0.02, 0);
			drawSpriteSheetSprite(playerPlace, megamanAtkSheet, picIndex, 8, 2); }
	}
	// Attack Animation
	else if (animationType == 1 && animationDisplayAmt > 0) {
		float playerSizeX = 0.66 * playerScale;
		float playerSizeY = 0.56 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
		// Altered animation for LifeSword
		if (selSwordAnimation == 6) {
			if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 1; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.14) { picIndex = 2; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.18) { picIndex = 3; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 4; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.26) { picIndex = 5; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.30) { picIndex = 6; }
			else if (animationDisplayAmt > 0)                          { picIndex = 7; } }
		// Animation for the rest of the Attacks
		else {
			if      (animationDisplayAmt > currentSwordAtkTime - 0.10) { picIndex = 1; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.14) { picIndex = 2; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.18) { picIndex = 3; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.22) { picIndex = 4; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.26) { picIndex = 5; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.30) { picIndex = 6; }
			else if (animationDisplayAmt > currentSwordAtkTime - 0.42) { picIndex = 7; } }
		if (player.facing == 1) {
			glTranslatef(-displace2, 0.02, 0);
			drawSpriteSheetSprite(playerPlace, megamanAtkSheet, picIndex + 8, 8, 2); }
		else if (player.facing == 3) {
			glTranslatef( displace2, 0.02, 0);
			drawSpriteSheetSprite(playerPlace, megamanAtkSheet, picIndex, 8, 2); }
	}
	// Movement Animation
	else if (animationType == 0 && player.moving) {
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY };
		if		(animationDisplayAmt > moveAnimationTime * 0.70) { displace = 1; picIndex = 0; }
		else if (animationDisplayAmt > moveAnimationTime * 0.60) { displace = 1; picIndex = 1; }
		else if (animationDisplayAmt > moveAnimationTime * 0.50) { displace = 1; picIndex = 2; }
		else if (animationDisplayAmt > moveAnimationTime * 0.45) { displace = 0; picIndex = 3; }
		else if (animationDisplayAmt > moveAnimationTime * 0.35) { displace = 0; picIndex = 2; }
		else if (animationDisplayAmt > moveAnimationTime * 0.25) { displace = 0; picIndex = 1; }
		else if (animationDisplayAmt > moveAnimationTime * 0.00) { displace = 0; picIndex = 0; }
		if      (player.moveDir == 0) {
			glTranslatef(0, -displace * scaleY, 0);
			if      (player.facing == 1) { drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex + 4, 4, 2); }
			else if (player.facing == 3) { drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex, 4, 2); }
		}
		else if (player.moveDir == 1) {
			glTranslatef(displace * scaleX, 0, 0);
			drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex + 4, 4, 2);
		}
		else if (player.moveDir == 2) {
			glTranslatef(0.0, displace * scaleY, 0.0);
			if      (player.facing == 1) { drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex + 4, 4, 2); }
			else if (player.facing == 3) { drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex, 4, 2); }
		}
		else if (player.moveDir == 3) {
			glTranslatef(-displace * scaleX, 0, 0);
			drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex, 4, 2);
		}
	}
	// Turning Animation
	else if (animationType == 0 && player.turning) {
		bool reverse = true;
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
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
		if      (player.facing == 1) { picIndex += (reverse ? 0 : 4); }
		else if (player.facing == 3) { picIndex += (reverse ? 4 : 0); }
		drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex, 4, 2);
	}
	// Special Invalid Movement animation for Start Tile
	else if (animationType == 2 && animationDisplayAmt > 0) {
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };
		displace = animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace, 0, 0);
		drawSpriteSheetSprite(playerPlace, megamanMoveSheet, 0, 4, 2);
	}
	// Level Transition pt. 1
	else if (animationType == 3 && animationDisplayAmt > moveAnimationTime * 2) {
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };
		displace = 1 - (animationDisplayAmt - moveAnimationTime * 2) / moveAnimationTime;
		glTranslatef(displace, 0, 0);
		drawSpriteSheetSprite(playerPlace, megamanMoveSheet, 4, 4, 2);
	}
	// Level Transition pt. 2
	else if (animationType == 4 && animationDisplayAmt > 0 && animationDisplayAmt <= moveAnimationTime * 2) {
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };
		displace = animationDisplayAmt / moveAnimationTime;
		glTranslatef(-displace, 0, 0);
		drawSpriteSheetSprite(playerPlace, megamanMoveSheet, 4, 4, 2);
	}
	// Player Standing Still
	else {
		float playerSizeX = 0.35 * playerScale;
		float playerSizeY = 0.54 * playerScale;
		GLfloat playerPlace[] = { player.x  * scaleX + playerSizeX, player.y * scaleY + playerSizeY,
								  player.x  * scaleX + playerSizeX, player.y * scaleY - playerSizeY,
		                          player.x  * scaleX - playerSizeX, player.y * scaleY - playerSizeY,
								  player.x  * scaleX - playerSizeX, player.y * scaleY + playerSizeY };
		if      (player.facing == 1) { picIndex = 0; }
		else if (player.facing == 3) { picIndex = 4; }
		drawSpriteSheetSprite(playerPlace, megamanMoveSheet, picIndex, 4, 2);
	}
}
void App::drawFloor() {
	// Draw Tiles
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	//glScalef(0.6, 0.6, 0.0);
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
	//glScalef(0.6, 0.6, 0.0);
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
                bool perlinOn = false;

				if      (map[i][row].boxType <= 0) { sheetHeight = 5; textureSheet = rockSheet; }
				else if (map[i][row].boxType == 1) { sheetHeight = 5; textureSheet = rockSheet; }
				else if (map[i][row].boxType == 2) { sheetHeight = 3; textureSheet = rockSheetItem; }

				index = (map[i][row].boxHP - 1) * 3;
				if      (map[i][row].prevDmg == 1 && map[i][row].boxAtkInd > 0) { index += 4; perlinOn = true; }
				else if (map[i][row].prevDmg == 2 && map[i][row].boxAtkInd > 0) { index += 8; perlinOn = true; }

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
	//glScalef(0.6, 0.6, 0.0);
	glTranslatef(0.5, 1.11, 0.0);

	float itemSizeX = 0.23 * itemScale / 1.5;
	float itemSizeY = 0.61 * itemScale / 1.5;

	for (int i = 0; i < 6; i++) {
		GLfloat place[] = { i + itemSizeX * scaleX, row * scaleY + itemSizeY, i + itemSizeX * scaleX, row * scaleY - itemSizeY,
							i - itemSizeX * scaleX, row * scaleY - itemSizeY, i - itemSizeX * scaleX, row * scaleY + itemSizeY };
		if (map[i][row].item > 0 && map[i][row].boxHP <= 0) {
			if      (itemAnimationAmt >= 0 && itemAnimationAmt < 1) { drawSpriteSheetSprite(place, energySheet, 0, 8, 1); }
			else if (itemAnimationAmt >= 1 && itemAnimationAmt < 2) { drawSpriteSheetSprite(place, energySheet, 1, 8, 1); }
			else if (itemAnimationAmt >= 2 && itemAnimationAmt < 3) { drawSpriteSheetSprite(place, energySheet, 2, 8, 1); }
			else if (itemAnimationAmt >= 3 && itemAnimationAmt < 4) { drawSpriteSheetSprite(place, energySheet, 3, 8, 1); }
			else if (itemAnimationAmt >= 4 && itemAnimationAmt < 5) { drawSpriteSheetSprite(place, energySheet, 4, 8, 1); }
			else if (itemAnimationAmt >= 5 && itemAnimationAmt < 6) { drawSpriteSheetSprite(place, energySheet, 5, 8, 1); }
			else if (itemAnimationAmt >= 6 && itemAnimationAmt < 7) { drawSpriteSheetSprite(place, energySheet, 6, 8, 1); }
			else if (itemAnimationAmt >= 7 && itemAnimationAmt < 8) { drawSpriteSheetSprite(place, energySheet, 7, 8, 1); }
}	}	}
void App::drawTextUI() {
	// Display current Level number
	glLoadIdentity();
	glTranslatef(0.895, 0.95, 0.0);
	if (level >= 10) { glTranslatef(-0.04, 0.0, 0.0); }
	if (level >= 100) { glTranslatef(-0.04, 0.0, 0.0); }
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
	if (player.energy >= 10) { glTranslatef(-0.04, 0, 0); }
	if (player.energy >= 100) { glTranslatef(-0.04, 0, 0); }
	if (player.energy >= 1000) { glTranslatef(-0.04, 0, 0); }
	int displayedEnergy = player.energy;
	if (displayedEnergy < 1) { displayedEnergy = 1; }
	if (displayedEnergy > 9999) { displayedEnergy = 9999; }
	if      (chargeDisplayPlusAmt  > 0) { drawText(textSheet2B, to_string(displayedEnergy), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else if (chargeDisplayMinusAmt > 0 || displayedEnergy == 1) {
	                                      drawText(textSheet2C, to_string(displayedEnergy), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else                                { drawText(textSheet2A, to_string(displayedEnergy), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	// Draw Current Energy gain for this level
	glLoadIdentity();
	glTranslatef(-0.828, 0.8495, 0);
	if (abs(currentEnergyGain) >= 10) { glTranslatef(-0.04, 0, 0); }
	if (abs(currentEnergyGain) >= 100) { glTranslatef(-0.04, 0, 0); }
	if (abs(currentEnergyGain) >= 1000) { glTranslatef(-0.04, 0, 0); }
	if (level > 0) {
		if      (currentEnergyGain > 0) { drawText(textSheet1B, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
		else if (currentEnergyGain < 0) { drawText(textSheet1C, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
		else if (menuDisplay)           { drawText(textSheet1A, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	}

	// Draw Info Box
	glLoadIdentity();
	glTranslatef(0, 0.02, 0);
	GLfloat UIPlace[] = { -1.0, 0.475, -1.0, -0.595,    -0.75, -0.595, -0.75, 0.475 };
	drawSpriteSheetSprite(UIPlace, infoBoxPic, 0, 1, 1);

	// Display Sword Name and Cost
	glLoadIdentity();
	glTranslatef(-0.97, -0.93, 0);
	string text = "";
	GLuint texture;

    if ( selSwordAnimation != -1 ) {
        if ( selSwordAnimation == -2 ) {
            texture = textSheet1B;
            text = "Energy" + to_string( energyGainAmt ); }
        else {
            texture = textSheet1C;
            if      ( selSwordAnimation == 0 ) { text = "Sword"    + to_string( swordCost ); }
            else if ( selSwordAnimation == 1 ) { text = "LongSwrd" + to_string( longCost ); }
            else if ( selSwordAnimation == 2 ) { text = "WideSwrd" + to_string( wideCost ); }
            else if ( selSwordAnimation == 3 ) { text = "CrossSwd" + to_string( crossCost ); }
            else if ( selSwordAnimation == 4 ) { text = "SpinSwrd" + to_string( spinCost ); }
            else if ( selSwordAnimation == 5 ) { text = "StepSwrd" + to_string( stepCost ); }
            else if ( selSwordAnimation == 6 ) { text = "LifeSwrd" + to_string( lifeCost ); } }
        drawText( texture, text, 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0 );
    }
}

// Sword Attack Animations
void App::swordDisplay(int dir) {

	float sizeX = 0.37 * playerScale;
	float sizeY = 0.22 * playerScale;
	int xPos = player.x;
	int yPos = player.y;
	if (dir == 1) { xPos--; }
	else if (dir == 3) { xPos++; }
	GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
						   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
	if (dir == 1) {
		if		(animationDisplayAmt > swordAtkTime - 0.12) {}
		else if (animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 3) {
		if		(animationDisplayAmt > swordAtkTime - 0.12) {}
		else if	(animationDisplayAmt > swordAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 2, 3, 1); }
	}
}
void App::longDisplay(int dir) {

	float sizeX = 0.70 * playerScale;
	float sizeY = 0.37 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		xPos -= 1.5;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > longAtkTime - 0.12) {}
		else if (animationDisplayAmt > longAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, longAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, longAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > longAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, longAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos += 1.5;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > longAtkTime - 0.12) {}
		else if (animationDisplayAmt > longAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, longAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, longAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > longAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, longAtkSheet3, 2, 3, 1); }
	}
}
void App::wideDisplay(int dir) {

	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > wideAtkTime - 0.12) {}
		else if (animationDisplayAmt > wideAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > wideAtkTime - 0.12) {}
		else if (animationDisplayAmt > wideAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 2, 3, 1); }
	}
}
void App::crossDisplay(int dir) {

	float sizeX = 1.2 * playerScale;
	float sizeY = 1.2 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > crossAtkTime - 0.12) {}
		else if (animationDisplayAmt > crossAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > crossAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > crossAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 2, 3, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.40) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 3, 6, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.46) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 4, 6, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.52) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 5, 6, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > crossAtkTime - 0.12) {}
		else if (animationDisplayAmt > crossAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > crossAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > crossAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 2, 3, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.40) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 3, 6, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.46) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 4, 6, 1); }
		//else if (animationDisplayAmt > crossAtkTime - 0.52) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 5, 6, 1); }
	}
}
void App::spinDisplay(int dir) {

	float sizeX = 1.10 * 1.25 * playerScale;
	float sizeY = 0.72 * 1.25 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > spinAtkTime - 0.12) {}
		else if (animationDisplayAmt > spinAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 2, 4, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 3, 4, 1); }
	}
	else if (dir == 3) {
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > spinAtkTime - 0.12) {}
		else if (animationDisplayAmt > spinAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 2, 4, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 3, 4, 1); }
	}
}
void App::stepDisplay(int dir) {

	float sizeX = 0.37 * playerScale;
	float sizeY = 0.71 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if      (animationDisplayAmt > stepAtkTime - 0.18) {}
		else if	(animationDisplayAmt > stepAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if      (animationDisplayAmt > stepAtkTime - 0.18) {}
		else if	(animationDisplayAmt > stepAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 2, 3, 1); }
	}
}
void App::lifeDisplay(int dir) {

	float sizeX = 0.95 * playerScale;
	float sizeY = 0.80 * playerScale;
	float xPos = player.x;
	float yPos = player.y;
	if (dir == 1) {
		xPos -= 1.5;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
		else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 0, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 1, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 2, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 3, 4, 1); }
	}
	else if (dir == 3) {
		xPos += 1.5;
		GLfloat atkPlace[] = { xPos * scaleX + sizeX, yPos * scaleY + sizeY, xPos * scaleX + sizeX, yPos * scaleY - sizeY,
							   xPos * scaleX - sizeX, yPos * scaleY - sizeY, xPos * scaleX - sizeX, yPos * scaleY + sizeY };
		if		(animationDisplayAmt > lifeAtkTime - 0.12) {}
		else if (animationDisplayAmt > lifeAtkTime - 0.20) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 0, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.26) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 1, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.32) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 2, 4, 1); }
		else if (animationDisplayAmt > lifeAtkTime - 0.38) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 3, 4, 1); }
	}
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += energyGainAmt;
				map[player.x][player.y].item = 0;
				Mix_PlayChannel( 1, itemSound, 0 );
                selSwordAnimation = -2;
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
		animationDisplayAmt = longAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 1;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= longCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::wideAtk(int dir) {			// Does One Dmg to a sweep of 3x1 area in front of the player
	if (player.energy >= wideCost) {
		player.energy -= wideCost;
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
		animationDisplayAmt = wideAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 2;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= wideCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::crossAtk(int dir) {			// Does One Dmg in an X pattern in front of the player		// The Middle of the X cross takes Two Dmg total
	if (player.energy >= crossCost) {
		player.energy -= crossCost;
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
		animationDisplayAmt = crossAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 3;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= crossCost;
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
		animationDisplayAmt = stepAtkTime;
		currentSwordAtkTime = animationDisplayAmt;
		selSwordAnimation = 4;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
		currentEnergyGain -= spinCost;
		Mix_PlayChannel( 0, swordSound, 0 );
	}
}
void App::stepAtk(int dir) {			// Moves the player two squares in the facing Direction, then uses WideSword
	if (player.energy >= stepCost) {
		if (dir == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= stepCost;
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
			currentEnergyGain -= stepCost;
			DelayedSound sound;
			sound.delay = moveAnimationTime;
			sound.soundName = "sword";
			delayedSoundList.push_back( sound );
		}
		else if (dir == 3 && isTileValid(player.x + 2, player.y)) {
			player.energy -= stepCost;
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
			currentEnergyGain -= stepCost;
			DelayedSound sound;
			sound.delay = moveAnimationTime;
			sound.soundName = "sword";
			delayedSoundList.push_back( sound );
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
	DelayedHpLoss one;
	one.xPos = xPos;
	one.yPos = yPos;
	one.delay = delay;
	one.dmg = dmg;
	delayedHpList.push_back(one);
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
	clearFloor();
	player.reset();
	player.energy = startingEnergy;
	menuDisplay = false;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	animationDisplayAmt = 0;
	delayedHpList.clear();
	level = 1;
	loadLevel(level);
	currentEnergyGain = 0;
    selSwordAnimation = -1;
    quitMenuOn = resetMenuOn = trainMenuOn = false;
    menuSel = true;
}
void App::next() {
	level++;
	loadLevel(level);
	player.facing = 3;
	menuDisplay = false;
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
		int type = getRand(32);
		generateLevel(type); }		// Generate a Level of Random type
}
void App::generateLevel(int type, int num) {
	if (num <= -1) { num = rand() % 100; }				// Random number between 0 and 99, inclusive - used to determine difficulty
	int x = level;		if (x > 50) { x = 50; }
	int bound2 = 120 - x * 3 / 2;						// Bounds used in determining difficulty - based on current level number
	if (level >=  75) { bound2 = 40; }
	if (level >= 100) { bound2 = 35; }
	int bound1 = bound2 - 30;
	int diff = 0;				// difficulty 0, 1, 2 = easy, medium, hard			// level		easy	medium	hard
	if (num > bound1 && num <= bound2) { diff = 1; }								//   0			90%		10%		0%
	else if (num > bound2) { diff = 2; }											//  10			75%		25%		0%
	difficulty = diff;																//  20			60%		30%		10%
																					//  30			45%		30%		25%
																					//  40			30%		30%		40%
																					//  50			15%		30%		55%
																					//  75			10%		30%		60%
																					// 100			 5%		30%		65%
																					// 150			 1%		30%		69%
	int gain = 0 - diff * 9;	// Gain determines avg "profit" per level
	int extraBoxes = 0, extraItems = 0, floorDmgs = 0;		// Number of things per floor based on level number and difficulty

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
	// Plus
	else if (type == 5) {		// Plus A
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
	else if (type == 6) {		// Plus B
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
	else if (type == 7) {		// Plus C
		map[4][4].state = 3;
		for (int i = 0; i < 3; i++) {											// ] O=O==[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] O=O =[
			map[i + 3][1].boxHP = 1;	map[1][i + 3].boxHP = 1;				// ] O=OOO[
			map[i + 3][3].boxHP = 1;	map[3][i + 3].boxHP = 1; }				// ]======[
		extraBoxes = 4 + x / 5 + diff * 2;										// ]===OOO[
		extraItems = 11 + extraBoxes + gain;									// ]===   [
		floorDmgs = rand() % 3 + 1 + diff * 2;		// 1-3	// 3-5	// 5-7
	}
	else if (type == 8) {		// Plus D
		for (int i = 0; i < 3; i++) {											// ] OOO==[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] O=O==[
			map[i + 3][1].boxHP = 1;	map[1][i + 3].boxHP = 1;				// ] O=OOO[
			map[i + 3][3].boxHP = 1;	map[3][i + 3].boxHP = 1; }				// ]==O==O[
		map[2][2].boxHP = 1;	map[2][5].boxHP = 1;	map[5][2].boxHP = 1;	// ]===OOO[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]===   [
		extraItems = 14 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 2;			// 0-3	// 2-5	// 4-7
	}
	else if (type == 9) {		// Plus E
		for ( int i = 0; i < 2; i++ ) {											// ]O== ==[
			map[0][i + 4].boxHP = 1;	map[i + 1][3].boxHP = 1;				// ]O==O==[
			map[3][i + 1].boxHP = 1;	map[i + 4][0].boxHP = 1; }				// ]=OO=O [
		map[3][4].boxHP = 1;	map[3][5].state = 3;							// ]===O==[
		map[4][3].boxHP = 1;	map[5][3].state = 3;							// ]===O==[
		extraBoxes = 3 + x / 5 + diff * 2;										// ]====OO[
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 5 + 2 + diff * 2;		// 2-6	// 4-8	// 6-10
	}
	// Paths
	else if (type == 10) {		// Paths A
		map[5][3].state = 1;																			//	]===OO=[
		map[1][3].state = 3;	map[2][3].state = 3;	map[3][3].state = 3;	map[4][3].state = 3;	//	]===OO=[
		map[3][4].boxHP = 1;	map[3][5].boxHP = 1;	map[4][4].boxHP = 1;	map[4][5].boxHP = 1;	//	]=    X[
		map[3][0].boxHP = 1;	map[3][1].boxHP = 1;	map[3][2].boxHP = 1;							//	]===OO=[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;	map[4][2].boxHP = 1;							//	]===OO=[
		extraBoxes = 3 + x / 5 + diff * 2;																//	]===OO=[
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff;			// 2-4	// 3-5	// 4-6
	}
	else if (type == 11) {		// Paths B
		map[5][3].state = 1;
		map[2][2].state = 3;	map[3][2].state = 3;	map[3][3].state = 3;	map[4][3].state = 3;	//	]==OOO=[
		map[2][3].boxHP = 1;	map[2][5].boxHP = 1;	map[3][4].boxHP = 1;	map[4][4].boxHP = 1;	//	]==OOO=[
		map[2][4].boxHP = 1;							map[3][5].boxHP = 1;	map[4][5].boxHP = 1;	//	]==O  X[
		map[2][0].boxHP = 1;	map[3][0].boxHP = 1;	map[4][0].boxHP = 1;	map[4][2].boxHP = 1;	//	]==  O=[
		map[2][1].boxHP = 1;	map[3][1].boxHP = 1;	map[4][1].boxHP = 1;							//	]==OOO=[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]==OOO=[
		extraItems = 14 + extraBoxes + gain;
		if (diff == 0) { floorDmgs = 0; }
		floorDmgs = rand() % 3 + 1 + diff;		// 1-3	// 2-4	// 3-5
	}
	else if (type == 12) {		// Paths C
        for ( int i = 0; i < 2; i++ ) {                                                     //	] =OO==[
            map[i][1].state = 3;    map[i + 2][4].boxHP = 1;    map[i + 4][1].state = 3;    //	]==OO==[
            map[i][2].boxHP = 1;    map[i + 2][5].boxHP = 1;    map[i + 4][2].boxHP = 1;    //	]OO==OO[
            map[i][3].boxHP = 1;    map[i + 4][0].state = 3;    map[i + 4][3].boxHP = 1; }  //	]OO==OO[
        map[0][5].state = 3;                                                                //	]  ==  [
		extraBoxes = 4 + x / 5 + diff * 2;                                                  //	]====  [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;		// 0-4	// 3-7	// 6-10
	}
	else if (type == 13) {		// Paths D
		for ( int i = 0; i < 2; i++ ) {														//	]    ==[
			map[i][3].boxHP = 1;	map[i + 2][1].boxHP = 1;	map[5][i].state = 3;		//	]OO==OO[
			map[i][4].boxHP = 1;	map[i + 2][2].boxHP = 1;	map[i + 4][3].boxHP = 1;	//	]OO==OO[
			map[i][5].state = 3;	map[i + 2][5].state = 3;	map[i + 4][4].boxHP = 1; }	//	]==OO==[
		map[0][1].state = 3;																//	] =OO= [
		extraBoxes = 4 + x / 5 + diff * 2;													//	]===== [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
	// Cross
	else if (type == 14) {		// Cross A
		for (int i = 0; i < 4; i++ ) {											//	] O===O[
			map[0][i + 2].state = 3;	map[i + 2][0].state = 3;	}			//	] =O=O=[
		map[1][1].boxHP = 1;	map[2][4].boxHP = 1;	map[4][4].boxHP = 1;	//	] ==O==[
		map[1][5].boxHP = 1;	map[3][3].boxHP = 1;	map[5][1].boxHP = 1;	//	] =O=O=[
		map[2][2].boxHP = 1;	map[4][2].boxHP = 1;	map[5][5].boxHP = 1;	//	]=O===O[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==    [
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8
	}
	else if (type == 15) {		// Cross B
        for ( int i = 0; i < 5; i++ ) {                                                             //	]     =[
            map[i][5].state = 3;    map[i + 1][i].boxHP = 1;    map[i + 1][4 - i].boxHP = 1; }      //	]=O===O[
        map[0][2].state = 3;                                                                        //	]==O=O=[
		extraBoxes = 3 + x / 5 + diff * 2;															//	] ==O==[
		extraItems = 9 + extraBoxes + gain;														    //	]==O=O=[
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8							    //	]=O===O[
	}
	else if (type == 16) {		// Cross C
        for ( int i = 0; i < 5; i++ ) {                                                         //	]O===O=[
            map[i][i + 1].boxHP = 1;    map[i][5 - i].boxHP = 1;    map[5][i].state = 3; }      //	]=O=O= [
        map[2][0].state = 3;    map[3][0].state = 3;    map[4][0].state = 3;                    //	]==O== [
		extraBoxes = 3 + x / 5 + diff * 2;                                                      //	]=O=O= [
		extraItems = 9 + extraBoxes + gain;                                                     //	]O===O [
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8                          //	]==    [
	}
	else if (type == 17) {		// Cross D
        for ( int i = 0; i < 2; i++ ) {                                                         // ]== ===[
            map[i][1].boxHP = 1;    map[2][i + 2].boxHP = 1;    map[i + 3][1].boxHP = 1;        // ]OO=OO [
            map[i][4].boxHP = 1;                                map[i + 3][4].boxHP = 1; }      // ]==O== [
        for ( int i = 0; i < 5; i++ ) { map[5][i].state = 3; }                                  // ]==O== [
        map[2][5].state = 3;                                                                    // ]OO=OO [
		extraBoxes = 3 + x / 5 + diff * 2;                                                      // ]===== [
		extraItems = 10 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
    else if (type == 18) {		// Cross E
        for ( int i = 0; i < 2; i++ ) {                                                         // ]  == =[
            map[1][i].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[1][i + 3].boxHP = 1;        // ]=O==O=[
            map[4][i].boxHP = 1;                                map[4][i + 3].boxHP = 1; }      // ]=O==O=[
        map[0][5].state = 3;    map[1][5].state = 3;    map[4][5].state = 3;                    // ]==OO= [
        map[5][2].state = 3;                                                                    // ]=O==O=[
		extraBoxes = 3 + x / 5 + diff * 2;                                                      // ]=O==O=[
		extraItems = 10 + extraBoxes + gain;
        floorDmgs = rand() % 5 + diff * 3;	    // 0-4	// 3-7	// 6-10
	}
	// Gallery
	else if (type == 19) {		// Gallery A
		for (int i = 1; i < 5; i++) {											//	]======[
			map[i][1].boxHP = 1;	map[i][2].boxHP = 1;						//	]=OOOO=[
			map[i][3].boxHP = 1;	map[i][4].boxHP = 1; }						//	]=OOOO=[
		extraBoxes = 5 + x / 5 + diff * 2;										//	]=OOOO=[
		extraItems = 16 + extraBoxes + gain;									//	]=OOOO=[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]======[
	}
	else if (type == 20) {		// Gallery B									// ]======[
		for (int i = 1; i < 5; i++) {											// ]OO==OO[
			map[0][i].boxHP = 1;	map[1][i].boxHP = 1;						// ]OO==OO[
			map[4][i].boxHP = 1;	map[5][i].boxHP = 1; }						// ]OO==OO[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]OO==OO[
		extraItems = 16 + extraBoxes + gain;									// ]======[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 21) {		// Gallery C
		for (int i = 1; i < 5; i++) {											// ]=OOOO=[
			map[i][0].boxHP = 1;	map[i][3].boxHP = 1;						// ]======[
			map[i][2].boxHP = 1;	map[i][5].boxHP = 1; }						// ]=OOOO=[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]=OOOO=[
		extraItems = 16 + extraBoxes + gain;									// ]======[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			// ]=OOOO=[
	}
	else if (type == 22) {		// Gallery D
        for (int i = 1; i < 5; i++) {                                           // ]======[
			map[0][i].boxHP = 1;	map[3][i].boxHP = 1;                        // ]O=OO=O[
			map[2][i].boxHP = 1;	map[5][i].boxHP = 1; }                      // ]O=OO=O[
		extraBoxes = 5 + x / 5 + diff * 2;                                      // ]O=OO=O[
		extraItems = 16 + extraBoxes + gain;                                    // ]O=OO=O[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8          // ]======[
	}
	// Scattered
	else if (type == 23) {		// Scattered A
		for (int i = 0; i < 6; i += 2) {										//	]==O=O=[
			map[2][i + 1].boxHP = 1;											//	]===O=O[
			map[3][i].boxHP = 1;												//	]==O=O=[
			map[4][i + 1].boxHP = 1;											//	]===O=O[
			map[5][i].boxHP = 1; }												//	]==O=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]===O=O[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;		// 2-6	// 6-10	// 8-12
	}
	else if (type == 24) {		// Scattered B
		for (int i = 0; i < 4; i++) {
			map[    i % 2][2 + i].boxHP = 1;									//	]=O=O=O[
			map[2 + i % 2][2 + i].boxHP = 1;									//	]O=O=O=[
			map[4 + i % 2][2 + i].boxHP = 1; }									//	]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]O=O=O=[
		extraItems = 12 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12			//	]======[
	}
	else if (type == 25) {		// Scattered C
        for ( int i = 0; i < 3; i++ ) {                                         // ]   =O=[
            map[i][5].state = 3;    map[5][i].state = 3; }                      // ]=O=O=O[
        for ( int i = 0; i < 4; i++ ) {                                         // ]O=O=O=[
            map[i    ][3 - i].boxHP = 1;    map[i + 1][4 - i].boxHP = 1;        // ]=O=O= [
            map[i + 1][i + 2].boxHP = 1;    map[i + 2][i + 1].boxHP = 1; }      // ]==O=O [		
		extraBoxes = 4 + x / 5 + diff * 2;										// ]===O= [			
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
	else if (type == 26) {		// Scattered D
        for ( int i = 1; i <= 5; i += 2 ) {
            map[i][2].boxHP = 1;    map[i][4].boxHP = 1;                    // ] =O=O=[
            map[2][i].boxHP = 1;    map[4][i].boxHP = 1; }                  // ] O=O=O[
        map[0][4].state = 3;    map[0][5].state = 3;                        // ]==O=O=[
        map[4][0].state = 3;    map[5][0].state = 3;                        // ]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;                                  // ]==O=O=[
		extraItems = 12 + extraBoxes + gain;                                // ]====  [
        floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
	// Layers
	else if (type == 27) {		// Layers A
        map[1][0].state = 3;    map[2][0].state = 3;    map[4][0].state = 3;
        map[1][5].state = 3;    map[3][0].state = 3;    map[4][1].state = 3;    //	]= OO==[
		for (int yPos = 1; yPos < 6; yPos++) {									//	]==OO==[
			map[2][yPos].boxHP = 1;		map[3][yPos].boxHP = 1; }				//	]==OO==[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==OO==[
		extraItems = 10 + extraBoxes + gain;									//	]==OO =[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]=    =[
	}
	else if (type == 28) {		// Layers B
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=OO=O=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=OO=O=[
			map[2][i + 1].boxHP = 1;											//	]=OO=O=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=OO=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 29) {		// Layers C
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]=O=OO=[
			map[1][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]=O=OO=[
			map[3][i + 1].boxHP = 1;											//	]=O=OO=[
			map[i][5].state = 3;	map[i + 1][0].state = 3;	}				//	]=O=OO=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 30) {		// Layers D
        for ( int i = 0; i < 2; i++ ) {                                                         //	]=OO===[
            map[i + 1][5].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 4][2].boxHP = 1;    //	]=OO===[
            map[i + 1][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 4][1].boxHP = 1; }  //	]==OO==[
        map[0][2].state = 3;    map[2][0].state = 3;                                            //	] =OOOO[
		extraBoxes = 4 + x / 5 + diff * 2;                                                      //	]====OO[
		extraItems = 12 + extraBoxes + gain;                                                    //	]== ===[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 31) {		// Layers E
        for ( int i = 0; i < 2; i++ ) {                                                         // ]=== ==[
            map[i][3].boxHP = 1;    map[i + 2][3].boxHP = 1;    map[i + 3][1].boxHP = 1;        // ]OO====[
            map[i][4].boxHP = 1;    map[i + 2][2].boxHP = 1;    map[i + 3][0].boxHP = 1; }      // ]OOOO= [
        map[3][5].state = 3;    map[5][3].state = 3;                                            // ]==OO==[
		extraBoxes = 4 + x / 5 + diff * 2;										                // ]===OO=[
		extraItems = 12 + extraBoxes + gain;                                                    // ]===OO=[
        floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else { generateLevel(0, 100); return; }

    if      ( diff == 0 && extraItems < 6 ) { extraItems = 6; }
    else if ( diff == 1 && extraItems < 4 ) { extraItems = 4; }
    else if ( diff == 2 && extraItems < 4 ) { extraItems = 4; }
	generateItems(extraItems, type);
	generateBoxes(extraBoxes, type);
	generateFloor(floorDmgs,  type);
}
void App::generateItems(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	// Rooms
	if (type == 0) {			// Rooms A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	] +====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	] +==  [
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	] +===+[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	] +===+[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 0; }	//	]   ==+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 1; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 1) {		// Rooms B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      ( place[randPlace] == 0 ) { xPos = 0; yPos = 3; }	//	]++====[
			else if ( place[randPlace] == 1 ) { xPos = 0; yPos = 4; }	//	]++==  [
			else if ( place[randPlace] == 2 ) { xPos = 0; yPos = 5; }	//	]++==  [
			else if ( place[randPlace] == 3 ) { xPos = 1; yPos = 3; }	//	]  ==++[
			else if ( place[randPlace] == 4 ) { xPos = 1; yPos = 4; }	//	]  ==++[
			else if ( place[randPlace] == 5 ) { xPos = 1; yPos = 5; }	//	]====++[
			else if ( place[randPlace] == 6 ) { xPos = 4; yPos = 0; }
			else if ( place[randPlace] == 7 ) { xPos = 4; yPos = 1; }
			else if ( place[randPlace] == 8 ) { xPos = 4; yPos = 2; }
			else if ( place[randPlace] == 9 ) { xPos = 5; yPos = 0; }
			else if ( place[randPlace] == 10 ) { xPos = 5; yPos = 1; }
			else if ( place[randPlace] == 11 ) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 2) {		// Rooms C
		vector<int> place = { 0,1,2,3,4,5,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]++= ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]===== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	] =====[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]====++[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]== =++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 3) {		// Rooms D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 4; }	//	]+    =[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]=    +[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 4) {		// Rooms E
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]++====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]====++[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]====++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]);
	}	}	}
	// Plus
	else if (type == 5) {		// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 2; }	//	] =====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	] =+++=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]==+=+=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]==+++=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 6) {		// Plus B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 2; }	//	] =====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	] =+++=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	] =+=+=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]==+++=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 7) {		// Plus C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	] +=+==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	] +=+ =[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	] +=+++[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]===+++[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 8) {		// Plus D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	] +++==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	] +=+==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	] +=+++[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }	//	]=====+[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===+++[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 9) {		// Plus E
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 4; }	// ]+== ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	// ]+=====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	// ]=++== [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	// ]===+==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	// ]===+==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	// ]====++[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 0; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	// Paths
	else if (type == 10) {		// Paths A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 3; yPos = 0; }	//	]===++=[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 1; }	//	]===++=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===++=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 5; }	//	]===++=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 0; }	//	]===++=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 11) {		// Paths C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==++==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==+  =[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]==  +=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===++=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===++=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 12) {		// Paths C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 2; }	//	] =+===[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]==+===[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]++===+[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]++===+[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]  ==  [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 13) {		// Paths D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 3; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]++===+[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]++===+[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	] =++= [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	//	]===== [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	// Cross
	else if (type == 14) {		// Cross A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)      { xPos = 1; yPos = 1; }	//	] +====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	] =+=+=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] ==+==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] =+=+=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=+===+[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 15) {		// Cross B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)      { xPos = 1; yPos = 0; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=+====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]==+=+=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	] ==+==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==+=+=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]=+===+[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 0; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 16) {		// Cross C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)      { xPos = 0; yPos = 1; }	//	]+=====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]=+=+= [
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]==+== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=+=+= [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]+===+ [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 17) {		// Cross D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)      { xPos = 0; yPos = 1; }	//	]++ ===[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]++=== [
			else if (place[randPlace] == 2) { xPos = 0; yPos = 5; }	//	]===== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 1; }	//	]===== [
			else if (place[randPlace] == 4) { xPos = 1; yPos = 4; }	//	]++=++ [
			else if (place[randPlace] == 5) { xPos = 1; yPos = 5; }	//	]===== [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
    else if (type == 18) {		// Cross E
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)      { xPos = 1; yPos = 0; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=+====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=+====[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]===== [
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]=+==++[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]=+==++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	// Gallery
	else if (type == 19) {		// Gallery A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=++++=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=++++=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=++++=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]=++++=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 20) {		// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 2; }	//	]++==++[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 3; }	//	]++==++[
			else if (place[randPlace] == 3) { xPos = 0; yPos = 4; }	//	]++==++[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 1; }	//	]++==++[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 1; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 1; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 21) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=++++=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=++++=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]=++++=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 0; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]=++++=[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 22) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 2; }	//	]+=++=+[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 3; }	//	]+=++=+[
			else if (place[randPlace] == 3) { xPos = 0; yPos = 4; }	//	]+=++=+[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]+=++=+[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
    // Scattered
	else if (type == 23) {		// Scattered A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]==+===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]===+==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==+=+=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]===+=+[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==+=+=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===+=+[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 24) {		// Scattered B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 2; }	//	]=+=+==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]+=+===[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=+=+=+[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]+=+=+=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 25) {		// Scattered C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 3; }	//	]   ===[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=+=+==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]+=+=+=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 1; }	//	]=+=+= [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]==+=+ [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 0; }	//	]===+= [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 26) {		// Scattered D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	] =+===[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	] +=+==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]==+=+=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=+=+=+[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 5; }	//	]==+=+=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
            else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	// Layers
	else if (type == 27) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 1; }	//	]= ++==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 5; }	//	]==++ =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	//	]=    =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 28) {		// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==+=+=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==+=+=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==+=+=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]==+=+=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 29) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 3; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 2; }	//	]===++=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]===++=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===++=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]===++=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 30) {		// Layers D
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 4; }	//	]=+====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	]=+====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==+===[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	] =++==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]====++[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]== ===[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 31) {		// Layers E
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 4; }	//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==++= [
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]===+==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]====+=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 0; }	//	]====+=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else { generateItems(amt, 0); }
}
void App::generateBoxes(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	// Rooms
	if (type == 0 ) {			// Rooms A
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	] =O=O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	] =O=  [
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	] =O=O=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }	//	] =O=O=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]   =O=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]====O=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 1) {		// Rooms B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      ( place[randPlace] == 0 ) { xPos = 0; yPos = 3; }	//	]OO==O=[
			else if ( place[randPlace] == 1 ) { xPos = 0; yPos = 4; }	//	]OO==  [
			else if ( place[randPlace] == 2 ) { xPos = 0; yPos = 5; }	//	]OO==  [
			else if ( place[randPlace] == 3 ) { xPos = 1; yPos = 3; }	//	]  ==OO[
			else if ( place[randPlace] == 4 ) { xPos = 1; yPos = 4; }	//	]  ==OO[
			else if ( place[randPlace] == 5 ) { xPos = 1; yPos = 5; }	//	]====OO[
			else if ( place[randPlace] == 6 ) { xPos = 4; yPos = 0; }
			else if ( place[randPlace] == 7 ) { xPos = 4; yPos = 1; }
			else if ( place[randPlace] == 8 ) { xPos = 4; yPos = 2; }
			else if ( place[randPlace] == 9 ) { xPos = 4; yPos = 5; }
			else if ( place[randPlace] == 10 ) { xPos = 5; yPos = 0; }
			else if ( place[randPlace] == 11 ) { xPos = 5; yPos = 1; }
			else if ( place[randPlace] == 12 ) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 2) {		// Rooms C
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=OO=O=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]=OO== [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] ==OO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===OO=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]== ===[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 3) {		// Rooms D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]=O==O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 1; }	//	]O=OO=O[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]O=OO=O[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]=O==O=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 4) {		// Rooms E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 0; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]==O=O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]OO===O[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 1; }	//	]=OO=OO[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]==OO==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]===O==[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Plus
	else if (type == 5) {		// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	] ==O==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	] =OOO=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]=OO=OO[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==OOO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===O==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 6) {		// Plus B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	] ===O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	] =OOOO[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] =O=O=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=OOOO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==O===[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 7) {		// Plus C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	] O=O==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	] O=O =[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	] O=OOO[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]===OOO[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 8) {		// Plus D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }	// ] OOO==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	// ] O=O==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	// ] O=OOO[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	// ]==O==O[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 5; }	// ]===OOO[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	// ]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 9) {		// Plus E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 0; yPos = 4; }	//	]O== ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]O==O==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=OO=O [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]===O==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===O==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]====OO[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 0; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Paths
	else if (type == 10) {		// Paths A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 3; yPos = 0; }	//	]===OO=[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 1; }	//	]===OO=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===OO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 5; }	//	]===OO=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 0; }	//	]===OO=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 11) {		// Paths C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 2; yPos = 0; }	//	]==OOO=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]==OOO=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==O  =[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==  O=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 5; }	//	]==OOO=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 0; }	//	]==OOO=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 12) {		// Paths D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 0; yPos = 2; }	//	] =OO==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]OO==OO[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]OO==OO[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]  ==  [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 13) {		// Paths D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]OO==OO[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]OO==OO[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]==OO==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	] =OO= [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]===== [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Cross
	else if (type == 14) {		// Cross A
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	] O===O[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	] =O=O=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] ==O==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] =O=O=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=O===O[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 15) {		// Cross B
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=O===O[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]==O=O=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	] ==O==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==O=O=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]=O===O[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 16) {		// Cross C
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]O===O=[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]=O=O= [
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]==O== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O=O= [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]O===O [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 17) {		// Cross D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	// ]== ===[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	// ]OO=OO [
			else if (place[randPlace] == 2) { xPos = 1; yPos = 1; }	// ]==O== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	// ]==O== [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	// ]OO=OO [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 3; }	// ]===== [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
    else if (type == 18) {		// Cross E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	// ]  == =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	// ]=O==O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	// ]=O==O=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	// ]==OO= [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	// ]=O==O=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	// ]=O==O=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Gallery
	else if (type == 19) {		// Gallery A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=OOOO=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=OOOO=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=OOOO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]=OOOO=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 20) {		// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 2; }	//	]OO==OO[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 3; }	//	]OO==OO[
			else if (place[randPlace] == 3) { xPos = 0; yPos = 4; }	//	]OO==OO[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 1; }	//	]OO==OO[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 1; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 1; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 21) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=OOOO=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=OOOO=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]=OOOO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 0; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]=OOOO=[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 22) {		// Gallery D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 2; }	//	]O=OO=O[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 3; }	//	]O=OO=O[
			else if (place[randPlace] == 3) { xPos = 0; yPos = 4; }	//	]O=OO=O[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]O=OO=O[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Scattered
	else if (type == 23) {		// Scattered A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]==O=O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]===O=O[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==O=O=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]===O=O[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==O=O=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===O=O[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 24) {		// Scattered B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]=O=O=O[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]O=O=O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=O=O=O[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]O=O=O=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 25) {		// Scattered C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 0; yPos = 3; }	//	]   =O=[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 2; }	//	]=O=O=O[
			else if (place[randPlace] == 2)  { xPos = 1; yPos = 4; }	//	]O=O=O=[
			else if (place[randPlace] == 3)  { xPos = 2; yPos = 1; }	//	]=O=O= [
			else if (place[randPlace] == 4)  { xPos = 2; yPos = 3; }	//	]==O=O [
			else if (place[randPlace] == 5)  { xPos = 3; yPos = 0; }	//	]===O= [
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7)  { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 26) {		// Scattered D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 1; yPos = 2; } //	] =O=O=[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 4; } //	] O=O=O[
			else if (place[randPlace] == 2)  { xPos = 2; yPos = 1; } //	]==O=O=[
			else if (place[randPlace] == 3)  { xPos = 2; yPos = 3; } //	]=O=O=O[
			else if (place[randPlace] == 4)  { xPos = 2; yPos = 5; } //	]==O=O=[
			else if (place[randPlace] == 5)  { xPos = 3; yPos = 2; } //	]====  [
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7)  { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 2; }
            else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	// Layers
	else if (type == 27) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]= OO==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==OO==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 5; }	//	]==OO =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	//	]=    =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 28) {		// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 1; yPos = 1; } //	]    ==[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 2; } //	]=OO=O=[
			else if (place[randPlace] == 2)  { xPos = 1; yPos = 3; } //	]=OO=O=[
			else if (place[randPlace] == 3)  { xPos = 1; yPos = 4; } //	]=OO=O=[
			else if (place[randPlace] == 4)  { xPos = 2; yPos = 1; } //	]=OO=O=[
			else if (place[randPlace] == 5)  { xPos = 2; yPos = 2; } //	]=     [
			else if (place[randPlace] == 6)  { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7)  { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 29) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 1; yPos = 1; } //	]    ==[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 2; } //	]=O=OO=[
			else if (place[randPlace] == 2)  { xPos = 1; yPos = 3; } //	]=O=OO=[
			else if (place[randPlace] == 3)  { xPos = 1; yPos = 4; } //	]=O=OO=[
			else if (place[randPlace] == 4)  { xPos = 3; yPos = 1; } //	]=O=OO=[
			else if (place[randPlace] == 5)  { xPos = 3; yPos = 2; } //	]=     [
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7)  { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 30) {		// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 1; yPos = 4; } //	]=OO===[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 5; } //	]=OO===[
			else if (place[randPlace] == 2)  { xPos = 2; yPos = 2; } //	]==OO==[
			else if (place[randPlace] == 3)  { xPos = 2; yPos = 3; } //	] =OOOO[
			else if (place[randPlace] == 4)  { xPos = 2; yPos = 4; } //	]====OO[
			else if (place[randPlace] == 5)  { xPos = 2; yPos = 5; } //	]== ===[
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7)  { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else if (type == 31) {		// Layers E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; } //	]=== ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; } //	]OO====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; } //	]OOOO= [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; } //	]==OO==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; } //	]===OO=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 3; } //	]===OO=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 1; }
			if (xPos != -1 && yPos != -1) {
				if (map[xPos][yPos].item > 0) {
					map[xPos][yPos].boxType = 2;
					if (map[xPos][yPos].boxHP >= 3) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
				}	}
				else {
					if (map[xPos][yPos].boxHP >= 3) { map[xPos][yPos].boxType = 1; }
					if (map[xPos][yPos].boxHP >= 5) { remove(place.begin(), place.end(), place[randPlace]); }
					else {
						map[xPos][yPos].boxHP++;
						amt--;
	}	}	}	}	}
	else { generateBoxes(amt, 0); }
}
void App::generateFloor(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	// Rooms
	if (type == 0) {				// Rooms A
		vector<int> place = { 0,1,2,3,4,5 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 3; yPos = 0; }	//	] ==X==[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 1; }	//	] ==X  [
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	] ==X==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	] ==X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]   X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]===X==[
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 1) {			// Rooms B
		vector<int> place = { 0,1,2,3,4,5 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 3; }	//	]==X===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==X=  [
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==X=  [
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]  =X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]  =X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]===X==[
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 2) {			// Rooms C
		vector<int> place = { 0,1,2,3,4,5 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]===X==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]===XX [
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	] XX===[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]==X===[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 3; }	//	]== ===[
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 3) {			// Rooms D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]=    =[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 4) {			// Rooms E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]====X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=X=X=X[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 0; }	//	]==XXX=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]X==X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]====X=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]==X===[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Plus
	else if (type == 5) {			// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }		//	] =X=X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }		//	] X===X[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }		//	]===X==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }		//	]=X===X[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }		//	]==X=X=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }		//	]====  [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 6) {			// Plus B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }		//	] =XX==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }		//	] X====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }		//	] X=X=X[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }		//	]=====X[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }		//	]=X=XX=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }		//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 7) {			// Plus C
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }		//	] =X===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }		//	] =X===[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }		//	] =X===[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }		//	]==XXXX[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }		//	]======[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }		//	]===   [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 8) {			// Plus D
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	// ] =====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	// ] =X=X=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	// ] =X===[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	// ]=X=XX=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	// ]==X===[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; } // ]===   [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 9) {			// Plus E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }		//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }		//	]=XX===[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }		//	]X==X= [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 1; }		//	]=XX=X=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }		//	]==X=X=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }		//	]===X==[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Paths
	else if (type == 10) {			// Paths A
		vector<int> place = { 0,1,2,3,4,5 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }		//	]==X===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 0; }		//	]==X===[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }		//	]X    =[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }		//	]==X===[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }		//	]==X===[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }		//	]==X===[
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 11) {			// Paths B
		vector<int> place = { 0,1,2,3,4 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=X====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=X====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=X=  =[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]==  ==[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 5; }	//	]=X====[
			if (xPos != -1 && yPos != -1) {							//	]=X====[
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 12) {			// Paths C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	] X==X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]XX==XX[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	//	]==XX==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]  ==  [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]====  [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 13) {			// Paths D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]XX==XX[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	] X==X [
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]===== [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
            else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
            else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
            else if (place[randPlace] == 9) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Cross
	else if (type == 14) {			// Cross A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }	//	] ==X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	] ==X==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 1; }	//	] XX=XX[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	] ==X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 15) {			// Cross B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 0; }	//	]===X==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	] XX=XX[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===X==[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 16) {			// Cross C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }	//	]==X===[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==X== [
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]XX=XX [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	//	]==X== [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==X== [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 17) {			// Cross D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]== ===[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]==X== [
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]XX=XX [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]XX=XX [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]==X== [
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]===== [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
            else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
    else if (type == 18) {			// Cross E
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]  == =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 0; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=X==X [
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 0; }	//	]==XX==[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
            else if (place[randPlace] == 8) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Gallery
	else if (type == 19) {			// Gallery A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 2; }	//	]X====X[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 3; }	//	]X====X[
			else if (place[randPlace] == 3) { xPos = 0; yPos = 4; }	//	]X====X[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]X====X[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 20) {			// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==XX==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 21) {			// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=XXXX=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]=XXXX=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 22) {			// Gallery D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]=X==X=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Scattered
	else if (type == 23) {			// Scattered A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }	//	]===X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]===X=X[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]==X=X=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]===X=X[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]==X=X=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 24) {			// Scattered B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }	//	]X=X=X=[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]=X=X=X[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]X=X=X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=X=X=X[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 25) {			// Scattered C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 0; yPos = 4; }	//	]   X==[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 1; }	//	]X=X=X=[
			else if (place[randPlace] == 2)  { xPos = 2; yPos = 2; }	//	]=X=X=X[
			else if (place[randPlace] == 3)  { xPos = 2; yPos = 4; }	//	]==X=X [
			else if (place[randPlace] == 4)  { xPos = 3; yPos = 1; }	//	]===X= [
			else if (place[randPlace] == 5)  { xPos = 3; yPos = 3; }	//	]====X [
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7)  { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9)  { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 26) {			// Scattered D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)  { xPos = 1; yPos = 3; } //	] X=X==[
			else if (place[randPlace] == 1)  { xPos = 1; yPos = 5; } //	] =X=X=[
			else if (place[randPlace] == 2)  { xPos = 2; yPos = 2; } //	]=X=X=X[
			else if (place[randPlace] == 3)  { xPos = 2; yPos = 4; } //	]==X=X=[
			else if (place[randPlace] == 4)  { xPos = 3; yPos = 1; } //	]===X=X[
			else if (place[randPlace] == 5)  { xPos = 3; yPos = 3; } //	]====  [
			else if (place[randPlace] == 6)  { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7)  { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8)  { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 9)  { xPos = 5; yPos = 1; }
            else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	// Layers
	else if (type == 27) {			// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]= ==X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 2; }	//	]=X== =[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 28) {			// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 3; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 2; }	//	]===X=X[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]===X=X[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===X=X[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]===X=X[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 29) {			// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==X==X[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==X==X[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==X==X[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]==X==X[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 30) {			// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]X=====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]X=====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]=X====[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	] X====[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	//	]== =XX[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 0; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 31) {			// Layers E
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 5; }	//	]XX= ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]====X [
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]====X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 2; }	//	]=====X[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 3; }	//	]=====X[
            else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
            else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else { generateFloor(amt, 0); }
}

void App::changeMusic( int track ) {
    musicSwitchDisplayAmt = 1.5;

    if (track == -1 ) {
        if ( !musicMuted ) { musicSel++; }
	    if ( musicSel > 18 ) { musicSel = 1; } }
    else { musicSel == track; }

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
void App::displayMusic() {
    string text;
    if ( musicMuted ) { text = "Music Paused"; }
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

void App::test() {
	reset();
	clearFloor();
	level = -1;
	player.energy = 10000;
	for (int i = 0; i < 6; i++) {
		map[i][5].state = 3;
		map[i][4].item = 1;}

	for (int i = 1; i < 5; i++) {
        map[5][i].state = 1;
		for (int j = 1; j < 4; j++) {
			map[i][j].boxHP = 5;
			map[i][j].boxType = 1; } }
}
void App::test2() {
	int x = -1, y = -1, z = -1;
	while (x < 0 || x > 31 || y < 0) {
		cout << "Level Type: ";
		cin >> x;
		cout << "Level: ";
		cin >> y;
		cout << "Difficulty: ";
		cin >> z;
		cout << endl;
	}
	reset();
	clearFloor();
	level = y;
	player.energy = 10000;
	generateLevel(x, z);
}
