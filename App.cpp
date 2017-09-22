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

#include "App.h"
using namespace std;

const float fixed_timestep = 0.0166666f;
const int max_steps = 6;
const float pi = 3.14159265359f;

const int swordCost = 50;
const int longCost = 75;
const int wideCost = 100;
const int crossCost = 125;
const int spinCost = 150;
const int stepCost = 175;
const int lifeCost = 300;

const int energyGainAmt = 100;			// How much Energy the player gains when moving to a Pick-Up
const int itemWorth = 2;				// Item "Worth" is used to generate Levels

const float iconDisplayTime = 0.45;		// Energy Gain Icon Display time
const float boxDmgTime = 0.4;			// Box HP damage indicator time
const float moveAnimationTime = 0.175;	// Player movement time

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

Player::Player() : x(0), y(0), facing(3), moving(false), turning(false), moveDir(-1), energy(0), energy2(0) {}
Tile::Tile() : state(0), item(-1), boxHP(0), boxType(0), boxAtkInd(0.0), prevDmg(0) {}
DelayedHpLoss::DelayedHpLoss() : dmg(0), xPos(0), yPos(0), delay(0.0) {}

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
void drawText(int fontTexture, string text, float sizeX, float sizeY, float spacing, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0) {
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
float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}
float easeIn(float from, float to, float time) {
	float tVal = time*time*time*time*time;
	return (1.0f - tVal)*from + tVal*to;
}
float easeOut(float from, float to, float time) {
	float oneMinusT = 1.0f - time;
	float tVal = 1.0f - (oneMinusT * oneMinusT * oneMinusT *
		oneMinusT * oneMinusT);
	return (1.0f - tVal)*from + tVal*to;
}
float easeInOut(float from, float to, float time) {
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
float easeOutElastic(float from, float to, float time) {
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
			 player(Player()), menuDisplay(false), level(0), currentRefreshCount(0), refreshCount(0), currentEnergyGain(0),
			 animationType(0), selSwordAnimation(0),
			 animationDisplayAmt(0.0), chargeDisplayPlusAmt(0.0), chargeDisplayMinusAmt(0.0), currentSwordAtkTime(0.0),
			 itemAnimationAmt(0.0), bgAnimationAmt(0.0) {
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
	refreshPicA      = loadTexture("Pics\\Refresh A.png");
	refreshPicB      = loadTexture("Pics\\Refresh B.png");

	lvBarPic      = loadTexture("Pics\\Level Bar.png");
	healthBoxPic = loadTexture("Pics\\Health Box.png");
	infoBoxPic   = loadTexture("Pics\\InfoPic.png");

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

	level = 1;
	player.energy = player.energy2 = 300;
	loadLevel(level);
}
App::~App() { SDL_Quit(); }
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
		animationDisplayAmt = moveAnimationTime;
	}
		// Level Transition parameter
	else if (player.x == 6 && player.y == 5 && animationDisplayAmt < 0) {
		animationType = 3;
		animationDisplayAmt = moveAnimationTime * 3;
	}
		// Reload default parameters when nothing is happening or being animated
	else if (animationDisplayAmt < 0) {
		animationDisplayAmt = 0;
		player.moving = false;
		player.turning = false;
		player.moveDir = -1;
		animationType = 0;
	}

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
	} }
}
void App::checkKeys() {
	// Menu Keys and Controls
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { done = true; }
		else if (event.type == SDL_KEYDOWN) {		// Read Single Button presses
			if (!menuDisplay && animationDisplayAmt <= 0) {
				if (event.key.keysym.scancode == SDL_SCANCODE_E) { refresh(); }			// Restart current level
				else if (event.key.keysym.scancode == SDL_SCANCODE_R) { reset(); }		// Restart to level 1
				else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) { done = true; }
				else if (event.key.keysym.scancode == SDL_SCANCODE_X) { test(); }
				//else if (event.key.keysym.scancode == SDL_SCANCODE_C) { test2(); }
				// Turn around without moving
				else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (player.facing == 1) { face(3); }
					else if (player.facing == 3) { face(1); } }
				// Show "Menu"
				else if (event.key.keysym.scancode == SDL_SCANCODE_TAB || event.key.keysym.scancode == SDL_SCANCODE_F) { menuDisplay = true; }
			}
			// While the Menu is displayed, any button will undisplay it
			else if (menuDisplay) { menuDisplay = false; animationDisplayAmt = 0.1; }
	}	}


	const Uint8* keystates = SDL_GetKeyboardState(NULL);		// Read Multiple Button presses simultaneously
	
	// The player can't Move or Attack until after the previous Action is completed
	if (animationDisplayAmt <= 0) {
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
	drawTextUI();

	// Draw game elements, back row first, so the front row is displayed in front
	for (int i = 5; i >= 0; i--) {
		drawItems(i);
		drawBoxes(i);
		if (i == player.y) { drawPlayer(); } }

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

	if (menuDisplay) { drawMenu(); }

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
void App::drawBoxes(int num) {			// Draw the Rock Obstacles on the Floor
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

	if (num > 5) { num = -1; }
	if (num == -1) {
		for (int i = 0; i < 6; i++) {
			for (int j = 5; j >= 0; j--) {
				if (map[i][j].boxHP > 0 || map[i][j].boxAtkInd > 0) {
					GLfloat place[] = { i * scaleX + sizeX, j * scaleY + sizeY, i * scaleX + sizeX, j * scaleY - sizeY,
										i * scaleX - sizeX, j * scaleY - sizeY, i * scaleX - sizeX, j * scaleY + sizeY };

					if      (map[i][j].boxType <= 0) { sheetHeight = 5; textureSheet = rockSheet; }
					else if (map[i][j].boxType == 1) { sheetHeight = 5; textureSheet = rockSheet; }
					else if (map[i][j].boxType == 2) { sheetHeight = 3; textureSheet = rockSheetItem; }

					index = (map[i][j].boxHP - 1) * 3;
					if      (map[i][j].prevDmg == 1 && map[i][j].boxAtkInd > 0) { index += 4; }
					else if (map[i][j].prevDmg == 2 && map[i][j].boxAtkInd > 0) { index += 8; }

					if (map[i][j].boxHP > 0) { drawSpriteSheetSprite(place, textureSheet, index, 3, sheetHeight); }
					else if (map[i][j].boxAtkInd > boxDmgTime * 0.8) { drawSpriteSheetSprite(place, textureSheet, index, 3, sheetHeight); }
					else {
						GLfloat place2[] = { i * scaleX + sizeX2, j * scaleY + sizeY2, i * scaleX + sizeX2, j * scaleY - sizeY2,
											 i * scaleX - sizeX2, j * scaleY - sizeY2, i * scaleX - sizeX2, j * scaleY + sizeY2 };
						if      (map[i][j].boxAtkInd > boxDmgTime * 0.60) { drawSpriteSheetSprite(place2, rockDeathSheet, 0, 4, 1); }
						else if (map[i][j].boxAtkInd > boxDmgTime * 0.40) { drawSpriteSheetSprite(place2, rockDeathSheet, 1, 4, 1); }
						else if (map[i][j].boxAtkInd > boxDmgTime * 0.20) { drawSpriteSheetSprite(place2, rockDeathSheet, 2, 4, 1); }
						else if (map[i][j].boxAtkInd > boxDmgTime * 0.00) { drawSpriteSheetSprite(place2, rockDeathSheet, 3, 4, 1); }
	}	}	}	}	}
	else {
		for (int i = 0; i < 6; i++) {
			if (map[i][num].boxHP > 0 || map[i][num].boxAtkInd > 0) {
					GLfloat place[] = { i * scaleX + sizeX, num * scaleY + sizeY, i * scaleX + sizeX, num * scaleY - sizeY,
										i * scaleX - sizeX, num * scaleY - sizeY, i * scaleX - sizeX, num * scaleY + sizeY };

					if      (map[i][num].boxType <= 0) { sheetHeight = 5; textureSheet = rockSheet; }
					else if (map[i][num].boxType == 1) { sheetHeight = 5; textureSheet = rockSheet; }
					else if (map[i][num].boxType == 2) { sheetHeight = 3; textureSheet = rockSheetItem; }

					index = (map[i][num].boxHP - 1) * 3;
					if      (map[i][num].prevDmg == 1 && map[i][num].boxAtkInd > 0) { index += 4; }
					else if (map[i][num].prevDmg == 2 && map[i][num].boxAtkInd > 0) { index += 8; }

					if (map[i][num].boxHP > 0) { drawSpriteSheetSprite(place, textureSheet, index, 3, sheetHeight); }
					else if (map[i][num].boxAtkInd > boxDmgTime * 0.8) { drawSpriteSheetSprite(place, textureSheet, index, 3, sheetHeight); }
					else {
						GLfloat place2[] = { i * scaleX + sizeX2, num * scaleY + sizeY2, i * scaleX + sizeX2, num * scaleY - sizeY2,
											 i * scaleX - sizeX2, num * scaleY - sizeY2, i * scaleX - sizeX2, num * scaleY + sizeY2 };
						if      (map[i][num].boxAtkInd > boxDmgTime * 0.60) { drawSpriteSheetSprite(place2, rockDeathSheet, 0, 4, 1); }
						else if (map[i][num].boxAtkInd > boxDmgTime * 0.40) { drawSpriteSheetSprite(place2, rockDeathSheet, 1, 4, 1); }
						else if (map[i][num].boxAtkInd > boxDmgTime * 0.20) { drawSpriteSheetSprite(place2, rockDeathSheet, 2, 4, 1); }
						else if (map[i][num].boxAtkInd > boxDmgTime * 0.00) { drawSpriteSheetSprite(place2, rockDeathSheet, 3, 4, 1); }
}	}	}	}	}
void App::drawItems(int num) {		// Draw the Collectable Resources on the Map
	glLoadIdentity();
	glOrtho(orthoX1, orthoX2, orthoY1, orthoY2, -1.0, 1.0);
	//glScalef(0.6, 0.6, 0.0);
	glTranslatef(0.5, 1.11, 0.0);

	float itemSizeX = 0.23 * itemScale / 1.5;
	float itemSizeY = 0.61 * itemScale / 1.5;

	if (num > 5) { num = -1; }
	if (num == -1) {
		for (int i = 0; i < 6; i++) {
			for (int j = 5; j >= 0; j--) {
				GLfloat place[] = { i + itemSizeX * scaleX, j * scaleY + itemSizeY, i + itemSizeX * scaleX, j * scaleY - itemSizeY,
									i - itemSizeX * scaleX, j * scaleY - itemSizeY, i - itemSizeX * scaleX, j * scaleY + itemSizeY };
				if (map[i][j].item > 0 && map[i][j].boxHP <= 0) {
					if      (itemAnimationAmt >= 0 && itemAnimationAmt < 1) { drawSpriteSheetSprite(place, energySheet, 0, 8, 1); }
					else if (itemAnimationAmt >= 1 && itemAnimationAmt < 2) { drawSpriteSheetSprite(place, energySheet, 1, 8, 1); }
					else if (itemAnimationAmt >= 2 && itemAnimationAmt < 3) { drawSpriteSheetSprite(place, energySheet, 2, 8, 1); }
					else if (itemAnimationAmt >= 3 && itemAnimationAmt < 4) { drawSpriteSheetSprite(place, energySheet, 3, 8, 1); }
					else if (itemAnimationAmt >= 4 && itemAnimationAmt < 5) { drawSpriteSheetSprite(place, energySheet, 4, 8, 1); }
					else if (itemAnimationAmt >= 5 && itemAnimationAmt < 6) { drawSpriteSheetSprite(place, energySheet, 5, 8, 1); }
					else if (itemAnimationAmt >= 6 && itemAnimationAmt < 7) { drawSpriteSheetSprite(place, energySheet, 6, 8, 1); }
					else if (itemAnimationAmt >= 7 && itemAnimationAmt < 8) { drawSpriteSheetSprite(place, energySheet, 7, 8, 1); }
	}	}	}	}
	else {
		for (int i = 0; i < 6; i++) {
			GLfloat place[] = { i + itemSizeX * scaleX, num * scaleY + itemSizeY, i + itemSizeX * scaleX, num * scaleY - itemSizeY,
								i - itemSizeX * scaleX, num * scaleY - itemSizeY, i - itemSizeX * scaleX, num * scaleY + itemSizeY };
			if (map[i][num].item > 0 && map[i][num].boxHP <= 0) {
				if      (itemAnimationAmt >= 0 && itemAnimationAmt < 1) { drawSpriteSheetSprite(place, energySheet, 0, 8, 1); }
				else if (itemAnimationAmt >= 1 && itemAnimationAmt < 2) { drawSpriteSheetSprite(place, energySheet, 1, 8, 1); }
				else if (itemAnimationAmt >= 2 && itemAnimationAmt < 3) { drawSpriteSheetSprite(place, energySheet, 2, 8, 1); }
				else if (itemAnimationAmt >= 3 && itemAnimationAmt < 4) { drawSpriteSheetSprite(place, energySheet, 3, 8, 1); }
				else if (itemAnimationAmt >= 4 && itemAnimationAmt < 5) { drawSpriteSheetSprite(place, energySheet, 4, 8, 1); }
				else if (itemAnimationAmt >= 5 && itemAnimationAmt < 6) { drawSpriteSheetSprite(place, energySheet, 5, 8, 1); }
				else if (itemAnimationAmt >= 6 && itemAnimationAmt < 7) { drawSpriteSheetSprite(place, energySheet, 6, 8, 1); }
				else if (itemAnimationAmt >= 7 && itemAnimationAmt < 8) { drawSpriteSheetSprite(place, energySheet, 7, 8, 1); }
}	}	}	}
void App::drawTextUI() {
	// Display current Level number
	glLoadIdentity();
	glTranslatef(0.895, 0.93, 0.0);
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

	// Display number of refreshes left this level
	glLoadIdentity();
	glTranslatef(0.96, -0.85, 0.0);
	float iconSizeX = 0.15 / 2 / 4;
	float iconSizeY = 0.15 / 2 / 2.7;
	GLfloat iconPlace[] = { -iconSizeX, iconSizeY, -iconSizeX, -iconSizeY,      iconSizeX, -iconSizeY, iconSizeX, iconSizeY };
	if (currentRefreshCount <= 2) { drawSpriteSheetSprite(iconPlace, refreshPicA, 0, 1, 1); }
	                         else { drawSpriteSheetSprite(iconPlace, refreshPicB, 0, 1, 1); }
	glTranslatef(-0.16 / 4, 0, 0);
	if (currentRefreshCount <= 1) { drawSpriteSheetSprite(iconPlace, refreshPicA, 0, 1, 1); }
	                         else { drawSpriteSheetSprite(iconPlace, refreshPicB, 0, 1, 1); }
	glTranslatef(-0.16 / 4, 0, 0);
	if (currentRefreshCount <= 0) { drawSpriteSheetSprite(iconPlace, refreshPicA, 0, 1, 1); }
	                         else { drawSpriteSheetSprite(iconPlace, refreshPicB, 0, 1, 1); }
	// Display number of times Refreshed
	if (refreshCount > 0 || menuDisplay) {
		glLoadIdentity();
		glTranslatef(0.957, -0.93, 0.0);
		if (refreshCount >= 10) { glTranslatef(-0.04, 0.0, 0.0); }
		if (refreshCount >= 100) { glTranslatef(-0.04, 0.0, 0.0); }
		if (refreshCount >= 1000) { glTranslatef(-0.04, 0.0, 0.0); }
		drawText(textSheet1A, to_string(refreshCount), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0);
	}

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
	else if (chargeDisplayMinusAmt > 0) { drawText(textSheet2C, to_string(displayedEnergy), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else                                { drawText(textSheet2A, to_string(displayedEnergy), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	// Draw Current Energy gain for this level
	glLoadIdentity();
	glTranslatef(-0.828, 0.8495, 0);
	if (abs(currentEnergyGain) >= 10) { glTranslatef(-0.04, 0, 0); }
	if (abs(currentEnergyGain) >= 100) { glTranslatef(-0.04, 0, 0); }
	if (abs(currentEnergyGain) >= 1000) { glTranslatef(-0.04, 0, 0); }
	if      (currentEnergyGain > 0) { drawText(textSheet1B, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else if (currentEnergyGain < 0) { drawText(textSheet1C, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
	else if (menuDisplay)           { drawText(textSheet1A, to_string(abs(currentEnergyGain)), 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }

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
	bool draw = false;
	if (chargeDisplayPlusAmt > 0) { draw = true; texture = textSheet1B; text = "Energy100"; }
	else if (chargeDisplayMinusAmt > 0) {
		draw = true;
		texture = textSheet1C;
		if      (selSwordAnimation == 0) { text = "Sword50"; }
		else if (selSwordAnimation == 1) { text = "LongSwrd75"; }
		else if (selSwordAnimation == 2) { text = "WideSwrd100"; }
		else if (selSwordAnimation == 3) { text = "CrossSwd125"; }
		else if (selSwordAnimation == 4) { text = "SpinSwrd150"; }
		else if (selSwordAnimation == 5) { text = "StepSwrd175"; }
		else if (selSwordAnimation == 6) { text = "LifeSwrd300"; } }
	if (draw) { drawText(texture, text, 0.08 * 2 / 4, 0.16 * 2 / 2.7, 0); }
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
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y++;
			player.moving = true;
			player.moveDir = 0;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 1, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x--;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 1)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y--;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 3) {	// Move Right
		if (isTileValid(player.x + 1, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x++;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
}
void App::move2(int dir) {		// Move Two Tiles
	if (dir == 0) {			// Move Up			// Can't move out of map, onto a box, or onto a hole in the floor
		if (isTileValid(player.x, player.y + 2)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y += 2;
			player.moving = true;
			player.moveDir = 0;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 2, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x -= 2;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 2)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y -= 2;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 3) {	// Move Right
		if (isTileValid(player.x + 2, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x += 2;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (player.x != -1 && player.x != 6 && map[player.x][player.y].item > 0) {			// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				currentEnergyGain += 100;
				map[player.x][player.y].item = 0; } } }
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
		currentEnergyGain -= 50;
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
		currentEnergyGain -= 75;
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
		currentEnergyGain -= 100;
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
		currentEnergyGain -= 125;
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
		currentEnergyGain -= 150;
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
			currentEnergyGain -= 175;
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
			currentEnergyGain -= 175;
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
		currentEnergyGain -= 300;
	}
}
void App::hitBox(int xPos, int yPos, int dmg) {
	if( xPos >= 0 && xPos <= 5 && yPos >= 0 && yPos <= 5) {
		if (map[xPos][yPos].boxHP > 0) {
			if (map[xPos][yPos].boxHP == 1) { map[xPos][yPos].prevDmg = 1; }
			else { map[xPos][yPos].prevDmg = dmg; }
			map[xPos][yPos].boxHP -= dmg;
			map[xPos][yPos].boxAtkInd = boxDmgTime;
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
			map2[i][j] = map[i][j];
}	}	}
void Player::reset() {
	x = 0;		y = 0;
	moving = false;
	turning = false;
	energy = 0;
	energy2 = 0;
	facing = 3;
}
void App::reset() {
	clearFloor();
	player.reset();
	player.energy = player.energy2 = 400;
	menuDisplay = false;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	animationDisplayAmt = 0;
	delayedHpList.clear();
	level = 1;
	loadLevel(level);
	currentRefreshCount = 0;
	refreshCount = 0;
	currentEnergyGain = 0;
}
void App::refresh() {
	if (currentRefreshCount <= 2) {
		player.x = 0;	player.y = 0;
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 6; j++) {
				map[i][j] = map2[i][j]; } }
		player.facing = 3;
		player.energy = player.energy2;
		chargeDisplayPlusAmt = 0;
		chargeDisplayMinusAmt = 0;
		animationDisplayAmt = 0;
		delayedHpList.clear();
		menuDisplay = false;
		currentRefreshCount++;
		refreshCount++;
		currentEnergyGain = 0;
	}
}
void App::next() {
	level++;
	loadLevel(level);
	player.facing = 3;
	player.energy2 = player.energy;
	menuDisplay = false;
	currentRefreshCount = 0;
	currentEnergyGain = 0;
}
void App::loadLevel(int num) {
	clearFloor();
	player.x = 0;		player.y = 0;
	if (level == 1) { generateLevel(0, 0); }
	else {
		int type = getRand(22);
		generateLevel(type); }		// Generate a Level of Random type

	for (int i = 0; i < 6; i++) {		// Save a copy of the current map		// For if the player wants to redo this level
		for (int j = 0; j < 6; j++) {
			map2[i][j] = map[i][j]; } }
}
void App::generateLevel(int type, int num) {
	if (num <= -1) { num = rand() % 100; }				// Random number between 0 and 99, inclusive - used to determine difficulty
	int x = level;		if (x > 50) { x = 50; }
	int bound2 = 120 - x * 3 / 2;						// Bounds used in determining difficulty - based on current level number
	int bound1 = bound2 - 30;
	if (level >= 100) { bound1 = 10;	bound2 = 30; }
	int diff = 0;				// difficulty 0, 1, 2 = easy, medium, hard			// level		easy	medium	hard
	if (num > bound1 && num <= bound2) { diff = 1; }								// 0			90%		10%		0%
	else if (num > bound2) { diff = 2; }											// 10			75%		25%		0%
	difficulty = diff;																// 20			60%		30%		10%
																					// 30			45%		30%		25%
																					// 40			30%		30%		40%
																					// 50			15%		30%		55%
																					//100			10%		30%		60%
	int gain = - diff * 10;	// Gain determines avg "profit" per level
	int extraBoxes = 0, extraItems = 0, floorDmgs = 0;		// Number of things per floor based on level number and difficulty

	// Map type
	if (type == 0) {			// Rooms A
		map[2][2].boxHP = 1;	map[2][5].boxHP = 1;	map[4][2].boxHP = 1;							//	]==O=O=[
		map[2][3].boxHP = 1;	map[4][0].boxHP = 1;	map[4][3].boxHP = 1;							//	]==O=  [
		map[2][4].boxHP = 1;	map[4][1].boxHP = 1;	map[4][5].boxHP = 1;							//	]==O=O=[
		map[0][1].state = 3;	map[1][1].state = 3;	map[2][1].state = 3;							//	]==O=O=[
		map[4][4].state = 3;	map[5][4].state = 3;													//	]   =O=[
		extraBoxes = 3 + x / 5 + diff * 2;		// Number of Boxes based on Level number				//	]====O=[
		extraItems = 8 + extraBoxes + gain;		// Number of Items based on "gain"
		floorDmgs = rand() % 3 + diff * 2;		// 0-2	// 2-4	// 4-6
												// Number of Damaged Floors based on difficulty
	}
	else if (type == 1) {		// Rooms B
		map[0][4].state = 3;	map[1][0].state = 3;	map[1][1].state = 3;							//	]==== =[
		map[4][0].state = 3;	map[4][1].state = 3;	map[4][2].state = 3;	map[4][5].state = 3;	//	] OOO==[
		map[1][2].boxHP = 1;	map[1][4].boxHP = 1;	map[2][2].boxHP = 1;							//	]==OOO=[
		map[2][3].boxHP = 1;	map[2][4].boxHP = 1;	map[3][2].boxHP = 1;							//	]=OOO =[
		map[3][3].boxHP = 1;	map[3][4].boxHP = 1;	map[4][3].boxHP = 1;							//	]= == =[
		extraBoxes = 3 + x / 5 + diff * 2;																//	]= == =[
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = rand() % 2 + diff * 2;		// 0-1	// 2-3	// 4-5
	}
	else if (type == 2) {		// Rooms C - Change rock spawns
		map[0][2].state = 3;	map[2][0].state = 3;	map[3][0].state = 3;	map[5][2].state = 3;	//	]==  ==[
		map[0][3].state = 3;	map[2][5].state = 3;	map[3][5].state = 3;	map[5][3].state = 3;	//	]==OO==[
		map[1][2].boxHP = 1;	map[2][1].boxHP = 1;	map[3][1].boxHP = 1;	map[4][2].boxHP = 1;	//	] O==O [
		map[1][3].boxHP = 1;	map[2][4].boxHP = 1;	map[3][4].boxHP = 1;	map[4][3].boxHP = 1;	//	] O==O [
		extraBoxes = 3 + x / 5 + diff * 2;																//	]==OO==[
		extraItems = 8 + extraBoxes + gain;																//	]==  ==[
		floorDmgs = rand() % 5 + diff * 4;		// 0-4	// 4-8	// 8-12
	}
	else if (type == 3) {		// Rooms D
		map[0][2].boxHP = 1;	map[1][4].boxHP = 1;	map[3][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]=O==O=[
		map[0][3].boxHP = 1;	map[1][5].boxHP = 1;	map[3][3].boxHP = 1;	map[4][5].boxHP = 1;	//	]=O==O=[
		map[1][0].boxHP = 1;	map[2][2].boxHP = 1;	map[4][0].boxHP = 1;	map[5][2].boxHP = 1;	//	]O=OO=O[
		map[1][1].boxHP = 1;	map[2][3].boxHP = 1;	map[4][1].boxHP = 1;	map[5][3].boxHP = 1;	//	]O=OO=O[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]=O==O=[
		extraItems = 16 + extraBoxes + gain;															//	]=O==O=[
		floorDmgs = rand() % 3 + diff * 3;		// 0-2	// 3-5	// 6-8
	}
	else if (type == 4) {		// Paths A
		map[0][3].state = 1;	map[5][3].state = 1;													//	]===OO=[
		map[1][3].state = 3;	map[2][3].state = 3;	map[3][3].state = 3;	map[4][3].state = 3;	//	]===OO=[
		map[3][4].boxHP = 1;	map[3][5].boxHP = 1;	map[4][4].boxHP = 1;	map[4][5].boxHP = 1;	//	]X    X[
		map[3][0].boxHP = 1;	map[3][1].boxHP = 1;	map[3][2].boxHP = 1;							//	]===OO=[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;	map[4][2].boxHP = 1;							//	]===OO=[
		extraBoxes = 3 + x / 5 + diff * 2;																//	]===OO=[
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 2 + diff;			// 0-1	// 1-2	// 2-3
	}
	else if (type == 5) {		// Plus A
		for (int i = 0; i < 3; i++) {											// ] ==O==[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] =OOO=[
			map[i + 2][2].boxHP = 1;	map[2][i + 2].boxHP = 1;				// ] OO=OO[
			map[i + 2][4].boxHP = 1;	map[4][i + 2].boxHP = 1; }				// ]==OOO=[
		map[1][3].boxHP = 1;	map[3][5].boxHP = 1;							// ]===O==[
		map[3][1].boxHP = 1;	map[5][3].boxHP = 1;							// ]===   [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 3;		// 0-3	// 3-6	// 6-9
	}
	else if (type == 21) {		// Plus B
		for (int i = 0; i < 3; i++) {											// ] ===O=[
			map[0][i + 3].state = 3;	map[i + 3][0].state = 3;				// ] OOOOO[
			map[i + 2][2].boxHP = 1;	map[2][i + 2].boxHP = 1;				// ] =O=O=[
			map[i + 2][4].boxHP = 1;	map[4][i + 2].boxHP = 1; }				// ]==OOO=[
		map[1][4].boxHP = 1;	map[4][5].boxHP = 1;							// ]====O=[
		map[4][1].boxHP = 1;	map[5][4].boxHP = 1;							// ]===   [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 3;		// 0-3	// 3-6	// 6-9
	}
	else if (type == 6) {		// Paths C
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
	else if (type == 7) {		// Paths D
		for (int xPos = 1; xPos < 5; xPos++) {									//	]==== =[
			map[xPos][1].state = 3;												//	]=    X[
			map[xPos][4].state = 3; }											//	]=O=OO=[
		map[4][5].state = 3;	map[5][4].state = 1;							//	]=O=OO=[
		map[1][2].boxHP = 1;	map[3][2].boxHP = 1;	map[4][2].boxHP = 1;	//	]=    =[
		map[1][3].boxHP = 1;	map[3][3].boxHP = 1;	map[4][3].boxHP = 1;	//	]==OOO=[
		map[2][0].boxHP = 1;	map[3][0].boxHP = 1;	map[4][0].boxHP = 1;
		extraBoxes = 3 + x / 5 + diff * 2;
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = diff;						// 0	// 1	// 2
	}
	else if (type == 8) {		// Cross A
		map[0][2].state = 3;	map[2][0].state = 3;	map[3][0].state = 3;	map[5][2].state = 3;
		map[0][3].state = 3;	map[2][5].state = 3;	map[3][5].state = 3;	map[5][3].state = 3;	//	]==  ==[
		map[1][1].boxHP = 1;	map[2][2].boxHP = 1;	map[3][2].boxHP = 1;	map[4][1].boxHP = 1;	//	]=O==O=[
		map[1][4].boxHP = 1;	map[2][3].boxHP = 1;	map[3][3].boxHP = 1;	map[4][4].boxHP = 1;	//	] =OO= [
		extraBoxes = 3 + x / 5 + diff * 2;																//	] =OO= [
		extraItems = 8 + extraBoxes + gain;																//	]=O==O=[
		floorDmgs = rand() % 3 + diff * 2;		// 0-2	// 2-4	// 4-6									//	]==  ==[
	}
	else if (type == 9) {		// Cross B
		for (int i = 0; i < 4; i++ ) {											//	] O===O[
			map[0][i + 2].state = 3;	map[i + 2][0].state = 3;	}			//	] =O=O=[
		map[1][1].boxHP = 1;	map[2][4].boxHP = 1;	map[4][4].boxHP = 1;	//	] ==O==[
		map[1][5].boxHP = 1;	map[3][3].boxHP = 1;	map[5][1].boxHP = 1;	//	] =O=O=[
		map[2][2].boxHP = 1;	map[4][2].boxHP = 1;	map[5][5].boxHP = 1;	//	]=O===O[
		extraBoxes = 3 + x / 5 + diff * 2;										//	]==    [
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = rand() % 3 + 2 + diff * 2;	// 2-4	// 4-6	// 6-8
	}
	else if (type == 10) {		// Cross C
		map[0][3].boxHP = 1;	map[1][2].boxHP = 1;	map[1][3].boxHP = 1;	map[2][1].boxHP = 1;	//	]==OO==[
		map[2][2].boxHP = 1;	map[2][4].boxHP = 1;	map[2][5].boxHP = 1;	map[3][0].boxHP = 1;	//	]==O=O=[
		map[3][1].boxHP = 1;	map[3][5].boxHP = 1;	map[4][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]OO===O[
		map[5][2].boxHP = 1;	map[5][3].boxHP = 1;													//	]=OO=OO[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]==OO==[
		extraItems = 14 + extraBoxes + gain;															//	]===O==[
		floorDmgs = rand() % 4 + 2 + diff * 3;	// 2-5	// 5-8	// 8-11
	}
	else if (type == 11) {		// Cross D
		map[2][2].state = 3;	map[2][3].state = 3;	map[3][2].state = 3;	map[3][3].state = 3;	//	]O=O=O=[
		map[0][3].boxHP = 1;	map[1][2].boxHP = 1;	map[2][1].boxHP = 1;	map[3][0].boxHP = 1;	//	]=O=O=O[
		map[0][5].boxHP = 1;	map[1][4].boxHP = 1;	map[2][5].boxHP = 1;	map[3][4].boxHP = 1;	//	]O=  O=[
		map[4][1].boxHP = 1;	map[4][3].boxHP = 1;	map[4][5].boxHP = 1;							//	]=O  =O[
		map[5][0].boxHP = 1;	map[5][2].boxHP = 1;	map[5][4].boxHP = 1;							//	]==O=O=[
		extraBoxes = 5 + x / 5 + diff * 2;																//	]===O=O[
		extraItems = 14 + extraBoxes + gain;
		floorDmgs = rand() % 4 + diff * 2;		// 0-3	// 2-5	// 4-7
	}
	else if (type == 12) {		// Gallery A
		for (int i = 1; i < 5; i++) {											//	]======[
			map[i][1].boxHP = 1;	map[i][2].boxHP = 1;						//	]=OOOO=[
			map[i][3].boxHP = 1;	map[i][4].boxHP = 1; }						//	]=OOOO=[
		extraBoxes = 5 + x / 5 + diff * 2;										//	]=OOOO=[
		extraItems = 16 + extraBoxes + gain;									//	]=OOOO=[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]======[
	}
	else if (type == 13) {		// Gallery B									// ]======[
		for (int i = 1; i < 5; i++) {											// ]OO==OO[
			map[0][i].boxHP = 1;	map[1][i].boxHP = 1;						// ]OO==OO[
			map[4][i].boxHP = 1;	map[5][i].boxHP = 1; }						// ]OO==OO[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]OO==OO[
		extraItems = 16 + extraBoxes + gain;									// ]======[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 20) {		// Gallery C
		for (int i = 1; i < 5; i++) {											// ]=OOOO=[
			map[i][0].boxHP = 1;	map[i][1].boxHP = 1;						// ]=OOOO=[
			map[i][4].boxHP = 1;	map[i][5].boxHP = 1; }						// ]======[
		extraBoxes = 5 + x / 5 + diff * 2;										// ]======[
		extraItems = 16 + extraBoxes + gain;									// ]=OOOO=[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			// ]=OOOO=[
	}
	else if (type == 14) {		// Scattered A
		for (int i = 0; i < 6; i += 2) {										//	]==O=O=[
			map[2][i + 1].boxHP = 1;											//	]===O=O[
			map[3][i].boxHP = 1;												//	]==O=O=[
			map[4][i + 1].boxHP = 1;											//	]===O=O[
			map[5][i].boxHP = 1; }												//	]==O=O=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]===O=O[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;		// 2-6	// 6-10	// 8-12
	}
	else if (type == 15) {		// Scattered B
		for (int i = 0; i < 4; i++) {
			map[    i % 2][2 + i].boxHP = 1;									//	]=O=O=O[
			map[2 + i % 2][2 + i].boxHP = 1;									//	]O=O=O=[
			map[4 + i % 2][2 + i].boxHP = 1; }									//	]=O=O=O[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]O=O=O=[
		extraItems = 12 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12			//	]======[
	}
	else if (type == 16) {		// Layers A
		map[1][4].state = 3;	map[1][5].state = 3;
		map[4][0].state = 3;	map[4][1].state = 3;							//	]= OO==[
		for (int yPos = 0; yPos < 6; yPos++) {									//	]= OO==[
			map[2][yPos].boxHP = 1;		map[3][yPos].boxHP = 1; }				//	]==OO==[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]==OO==[
		extraItems = 12 + extraBoxes + gain;									//	]==OO =[
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8			//	]==OO =[
	}
	else if (type == 17) {		// Layers B
		for (int i = 1; i < 5; i++) {
			map[i][1].state = 3;		map[i][2].boxHP = 1;
			map[i][3].boxHP = 1;		map[i][4].boxHP = 1;					//	]     =[
			map[i][5].state = 3;	}											//	]=OOOO=[
		map[0][5].state = 3;		map[1][0].state = 3;						//	]=OOOO=[
		extraBoxes = 4 + x / 5 + diff * 2;										//	]=OOOO=[
		extraItems = 12 + extraBoxes + gain;									//	]=    =[
		floorDmgs = rand() % 4 + diff * 3;		// 0-3	// 3-6	// 6-9			//	]= ====[
	}
	else if (type == 18) {		// Layers C
		map[5][0].state = 3;													//	]    ==[
		for (int i = 0; i < 4; i++) {											//	]==O=OO[
			map[2][i + 1].boxHP = 1;	map[4][i + 1].boxHP = 1;				//	]==O=OO[
			map[5][i + 1].boxHP = 1;											//	]==O=OO[
			map[i][5].state = 3;	map[i+1][0].state = 3;	}					//	]==O=OO[
		extraBoxes = 5 + x / 5 + diff * 2;										//	]=     [
		extraItems = 14 + extraBoxes + gain;
		floorDmgs = rand() % 5 + diff * 2;		// 0-4	// 2-6	// 4-8
	}
	else if (type == 19) {		// Layers D
		for (int i = 0; i < 5; i++) {											//	]     =[
			map[i][5].state = 3;	map[i + 1][0].state = 3; }					//	]==OOO=[
		map[2][1].boxHP = 1;	map[3][1].boxHP = 1;	map[4][1].boxHP = 1;	//	]==OOO=[
		map[2][2].boxHP = 1;	map[3][2].boxHP = 1;	map[4][2].boxHP = 1;	//	]==OOO=[
		map[2][3].boxHP = 1;	map[3][3].boxHP = 1;	map[4][3].boxHP = 1;	//	]==OOO=[
		map[2][4].boxHP = 1;	map[3][4].boxHP = 1;	map[4][4].boxHP = 1;	//	]=     [
		extraBoxes = 4 + x / 5 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 5 + 2 + diff * 3;	// 2-6	// 5-9	// 8-12
	}
	else { generateLevel(0, 100); return; }

	if (extraItems <= 0) {
		if (diff == 0) { extraItems = 4; }
		else if (diff == 1) { extraItems = 2; } }
	generateItems(extraItems, type);
	generateBoxes(extraBoxes, type);
	generateFloor(floorDmgs, type);
}
void App::generateItems(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	if (type == 0) {			// Rooms A
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 3; }	//	]+=====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]+===  [
			else if (place[randPlace] == 2) { xPos = 0; yPos = 5; }	//	]+====+[
			else if (place[randPlace] == 3) { xPos = 5; yPos = 0; }	//	]+====+[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]   ==+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 1) {		// Rooms B
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 5; }	//	]++++ =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	] =====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 0; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }	//	]==== =[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 0; }	//	]= =+ =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 1; }	//	]= ++ =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 2) {		// Rooms C
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]++  ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	] ==== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	] ==== [
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]==  ++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
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
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 4; }	//	]+=====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 4) {		// Paths A
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
	else if (type == 5) {		// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 3; }	//	] =====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	] =+++=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	] =+=++[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===++=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 2; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 3; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 21) {		// Plus B
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
	else if (type == 6) {		// Paths C
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
	else if (type == 7) {		// Paths D
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 0; }	//	]==== =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]==+++=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 8) {		// Cross A
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 4; }	//	]++  ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	] ==== [
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	] ==== [
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]==  ++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 9) {		// Cross B
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 1; yPos = 5; }	//	] +====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	] =+=+=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	] ==+==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	] =+=+=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 4; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 10) {		// Cross C
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 4; }	//	]++====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]====++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 11) {		// Cross D
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 5; }	//	]+=+===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 5; }	//	]===++=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 4; }	//	]==  +=[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 3; }	//	]==  =+[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 2; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 12) {		// Gallery A
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
	else if (type == 13) {		// Gallery B
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
	else if (type == 20) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=++++=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=++++=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 0; }	//	]=++++=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 1; }	//	]=++++=[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 14) {		// Scattered A
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
	else if (type == 15) {		// Scattered B
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
	else if (type == 16) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 0; }	//	]= ++==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]= ++==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==++ =[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]==++ =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 17) {		// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 2; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]==+++=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]==+++=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]==+++=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]= ====[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 18) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 4; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 4; yPos = 2; }	//	]====++[
			else if (place[randPlace] == 2) { xPos = 4; yPos = 3; }	//	]====++[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 4; }	//	]====++[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]====++[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 19) {		// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 3; yPos = 1; }	//	]     =[
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
	else { generateItems(amt, 0); }
}
void App::generateBoxes(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	if (type == 0 ) {			// Rooms A
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	]==O=O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]==O=  [
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]==O=O=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }	//	]==O=O=[
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]==== =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	] OOO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==OOO=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=OOO =[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]= == =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]= == =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	]==  ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	] O==O [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] O==O [
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==OO==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]==  ==[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]=O==O=[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]=O==O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 0; }	//	]O=OO=O[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 1; }	//	]O=OO=O[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 4; }	//	]=O==O=[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 5; }	//	]=O==O=[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 3; }
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
	else if (type == 4) {		// Paths A
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
	else if (type == 5) {		// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	] ==O==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	] =OOO=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	] OO=OO[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==OOO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===O==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]===   [
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
	else if (type == 21) {		// Plus B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 4; }	//	] ===O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	] OOOOO[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	] =O=O=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==OOO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]====O=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]===   [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
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
	else if (type == 6) {		// Paths C
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
	else if (type == 7) {		// Paths D
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	]==== =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 0; }	//	]=O=OO=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]=O=OO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]==OOO=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 3; }
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
	else if (type == 8) {		// Cross A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]==  ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=O==O=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] =OO= [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	] =OO= [
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=O==O=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]==  ==[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
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
	else if (type == 9) {		// Cross B
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
	else if (type == 10) {		// Cross C
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
	else if (type == 11) {		// Cross D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }	//	]O=O=O=[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]=O=O=O[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]O=  O=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O  =O[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]==O=O=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]===O=O[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 4; }
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
	else if (type == 12) {		// Gallery A
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
	else if (type == 13) {		// Gallery B
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
	else if (type == 20) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=OOOO=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=OOOO=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 0; }	//	]=OOOO=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 1; }	//	]=OOOO=[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 4; }
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
	else if (type == 14) {		// Scattered A
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
	else if (type == 15) {		// Scattered B
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
	else if (type == 16) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }	//	]= OO==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]= OO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==OO==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==OO =[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]==OO =[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
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
	else if (type == 17) {		// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]=OOOO=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]=OOOO=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	//	]=OOOO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]= ====[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
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
	else if (type == 18) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==O=OO[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==O=OO[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==O=OO[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]==O=OO[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 2; }
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
	else if (type == 19) {		// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==OOO=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==OOO=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==OOO=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==OOO=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
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
	else { generateBoxes(amt, 0); }
}
void App::generateFloor(int amt, int type) {
	if (amt <= 0) { return; }
	int xPos = -1, yPos = -1;
	if (type == 0) {			// Rooms A
		vector<int> place = { 0,1,2,3,4,5 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 3; yPos = 0; }	//	]===X==[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 1; }	//	]===X  [
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	]===X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]   X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]===X==[
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 1) {		// Rooms B
		vector<int> place = { 0,1,2,3,4 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]==== =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	] X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 3; }	//	]=X== =[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]= == =[
			if (xPos != -1 && yPos != -1) {							//	]= == =[
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 2) {			// Rooms C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]=XXXX=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	//	]=XXXX=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 3) {			// Rooms D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 4) {			// Paths A
		vector<int> place = { 0,1,2 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }		//	]======[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }		//	]======[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }		//	]=    =[
			if (xPos != -1 && yPos != -1) {								//	]==X===[
				map[xPos][yPos].state = 1;								//	]==X===[
				remove(place.begin(), place.end(), place[randPlace]);	//	]==X===[
				amt--; }
	}	}
	else if (type == 5) {			// Plus A
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }		//	] ==X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }		//	] ==X==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 1; }		//	] XXXXX[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }		//	]===X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }		//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }		//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 21) {			// Plus B
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }		//	] ==X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }		//	] ==X==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 1; }		//	] XXXXX[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }		//	]===X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }		//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }		//	]===   [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 6) {			// Paths C
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
	else if (type == 7) {			// Paths D
		vector<int> place = { 0,1,2,3 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]==== =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]=    =[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==X===[
			else if (place[randPlace] == 3) { xPos = 5; yPos = 1; }	//	]==X===[
			if (xPos != -1 && yPos != -1) {							//	]X    X[
				map[xPos][yPos].state = 1;							//	]======[
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 8) {			// Cross A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]==  ==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	] X==X [
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] X==X [
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]==  ==[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 9) {			// Cross B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	] X====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 5; }	//	] =X=X=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] ==X==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	] =X=X=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=X===X[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]==    [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 10) {			// Cross C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 3; }	//	]===X==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]===X==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]XXXXXX[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]===X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 11) {			// Cross D
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 5; }	//	]=X=X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 5; }	//	]==  =X[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 2; }	//	]==  X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]=====X[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 1; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 12) {			// Gallery A
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
	else if (type == 13) {			// Gallery B
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
	else if (type == 20) {			// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]=XXXX=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=XXXX=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 14) {			// Scattered A
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
	else if (type == 15) {			// Scattered B
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
	else if (type == 16) {			// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]= ==X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]= ==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 2; }	//	]=X== =[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 3; }	//	]=X== =[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 17) {			// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]==XXX=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]==XXX=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]==XXX=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=    =[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]= ====[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 18) {			// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 3; yPos = 1; }	//	]    ==[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 2; }	//	]===XX=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]===XX=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===XX=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]===XX=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 19) {			// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==XXX=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==XXX=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==XXX=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]==XXX=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else { generateFloor(amt, 0); }
}
void App::test() {
	reset();
	clearFloor();
	level = -1;
	player.energy = player.energy2 = 10000;
	for (int i = 0; i < 6; i++) {
		map[i][5].state = 3;
		map[i][4].item = 1; }
	for (int i = 1; i < 5; i++) {
		for (int j = 1; j < 4; j++) {
			map[i][j].boxHP = 5;
			map[i][j].boxType = 1;
			map2[i][j] = map[i][j]; } }
}
void App::test2() {
	int x = -1, y = -1, z = -1;
	while (x < 0 || x > 21 || y < 0) {
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
	player.energy = player.energy2 = 10000;
	generateLevel(x, z);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map2[i][j] = map[i][j]; } }
}
