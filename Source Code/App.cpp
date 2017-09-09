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

const float iconDisplayTime = 1.25;		// Energy Gain Icon Display time
const float moveAnimationTime = 0.325;
const float preAtkTime = 0.15;			// Slight Backstep before Attacking
const float swordAtkTime = 0.6;
const float longAtkTime = 0.6;
const float wideAtkTime = 0.6;
const float crossAtkTime = 1.0;
const float spinAtkTime = 0.6;
const float stepAtkTime = 0.6;
const float lifeAtkTime = 1;

Player::Player() : x(0), y(0), facing(3), moving(false), turning(false), moveDir(-1), energy(0), energy2(0) {}
Tile::Tile() : state(0), item(-1), boxHP(0), boxType(0) {}
DelayedHpLoss::DelayedHpLoss() : xPos(0), yPos(0), delay(0.0) {}

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
void drawTexture(GLfloat* drawingPlace, GLfloat* texturePlace, GLuint texture) {
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
void drawSpriteSheetSprite(GLfloat* place, GLuint spriteTexture, int index, int spriteCountX, int spriteCountY) {
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat quadUVs[] = { u, v, u, v - spriteHeight, u + spriteWidth, v - spriteHeight, u + spriteWidth, v };
	drawTexture(place, quadUVs, spriteTexture);
}
void drawText(int fontTexture, string text, float size, float spacing, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0) {
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
		vertexData.insert(vertexData.end(), { ((size + spacing) * i) + (-0.5f * size), 0.5f * size, ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		                                      ((size + spacing) * i) + (0.5f * size), -0.5f * size, ((size + spacing) * i) + (0.5f * size),   0.5f * size });
		texCoordData.insert(texCoordData.end(), { texture_x, texture_y, texture_x, texture_y + texture_size, texture_x +
			texture_size, texture_y + texture_size, texture_x + texture_size, texture_y });
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
			 player(Player()), mode(0), menuSel(0), level(0), refreshCount(0),
			 chargeDisplayPlusAmt(0.0), chargeDisplayMinusAmt(0.0), animationType(0), animationDisplayAmt(0.0), selSwordAnimation(0) {
	Init(); }
void App::Init() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Mega Man Puzzle Network", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	glViewport(0, 0, 800, 800);
	glMatrixMode(GL_PROJECTION_MATRIX);
	glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	textSheet1 = loadTexture("Pics\\UI\\texts1.png");
	textSheet2 = loadTexture("Pics\\UI\\texts2.png");
	playerPic = loadTexture("Pics\\Map\\PlayerSheet2.png");
	directionSheet = loadTexture("Pics\\Map\\ArrowSheet.png");
	swordIconSheet = loadTexture("Pics\\Sword Attacks\\0 Icons\\Sword Icon Sheet.png");
	squarePic = loadTexture("Pics\\UI\\Square Blue2.png");
	trianglePic = loadTexture("Pics\\UI\\Triangle Blue2.png");
	boxSheet0 = loadTexture("Pics\\Boxes\\BoxSheet0-1.png");
	boxSheet1 = loadTexture("Pics\\Boxes\\BoxSheet1-2.png");
	boxSheet2 = loadTexture("Pics\\Boxes\\BoxSheet2-3.png");
	goalPic = loadTexture("Pics\\Map\\Next White.png");
	floorSheet = loadTexture("Pics\\Map\\Floor Sheet.png");
	batteryPic = loadTexture("Pics\\Map\\Battery 4-2.png");
	chargePic1 = loadTexture("Pics\\Map\\Battery Charge 3.png");
	chargePic2 = loadTexture("Pics\\Map\\Charge Plus 3.png");
	chargePic3 = loadTexture("Pics\\Map\\Charge Minus.png");

	swordAtkSheet0 = loadTexture("Pics\\Sword Attacks\\1 Sword\\Sword Sheet 0.png");
	swordAtkSheet1 = loadTexture("Pics\\Sword Attacks\\1 Sword\\Sword Sheet 1.png");
	swordAtkSheet2 = loadTexture("Pics\\Sword Attacks\\1 Sword\\Sword Sheet 2.png");
	swordAtkSheet3 = loadTexture("Pics\\Sword Attacks\\1 Sword\\Sword Sheet 3.png");
	longAtkSheet0 = loadTexture( "Pics\\Sword Attacks\\2 Long\\Long Sheet 0.png");
	longAtkSheet1 = loadTexture( "Pics\\Sword Attacks\\2 Long\\Long Sheet 1.png");
	longAtkSheet2 = loadTexture( "Pics\\Sword Attacks\\2 Long\\Long Sheet 2.png");
	longAtkSheet3 = loadTexture( "Pics\\Sword Attacks\\2 Long\\Long Sheet 3.png");
	wideAtkSheet0 = loadTexture( "Pics\\Sword Attacks\\3 Wide\\Wide Sheet 0.png");
	wideAtkSheet1 = loadTexture( "Pics\\Sword Attacks\\3 Wide\\Wide Sheet 1.png");
	wideAtkSheet2 = loadTexture( "Pics\\Sword Attacks\\3 Wide\\Wide Sheet 2.png");
	wideAtkSheet3 = loadTexture( "Pics\\Sword Attacks\\3 Wide\\Wide Sheet 3.png");
	crossAtkSheet0 = loadTexture("Pics\\Sword Attacks\\4 Cross\\Cross Sheet 0.png");
	crossAtkSheet1 = loadTexture("Pics\\Sword Attacks\\4 Cross\\Cross Sheet 1.png");
	crossAtkSheet2 = loadTexture("Pics\\Sword Attacks\\4 Cross\\Cross Sheet 2.png");
	crossAtkSheet3 = loadTexture("Pics\\Sword Attacks\\4 Cross\\Cross Sheet 3.png");
	spinAtkSheet0 = loadTexture( "Pics\\Sword Attacks\\5 Spin\\Spin Sheet 0.png");
	spinAtkSheet1 = loadTexture( "Pics\\Sword Attacks\\5 Spin\\Spin Sheet 1.png");
	spinAtkSheet2 = loadTexture( "Pics\\Sword Attacks\\5 Spin\\Spin Sheet 2.png");
	spinAtkSheet3 = loadTexture( "Pics\\Sword Attacks\\5 Spin\\Spin Sheet 3.png");
	stepAtkSheet0 = loadTexture( "Pics\\Sword Attacks\\6 Step\\Step Sheet 0.png");
	stepAtkSheet1 = loadTexture( "Pics\\Sword Attacks\\6 Step\\Step Sheet 1.png");
	stepAtkSheet2 = loadTexture( "Pics\\Sword Attacks\\6 Step\\Step Sheet 2.png");
	stepAtkSheet3 = loadTexture( "Pics\\Sword Attacks\\6 Step\\Step Sheet 3.png");
	lifeAtkSheet0 = loadTexture( "Pics\\Sword Attacks\\7 Life\\Life Sheet 0-2.png");
	lifeAtkSheet1 = loadTexture( "Pics\\Sword Attacks\\7 Life\\Life Sheet 1-2.png");
	lifeAtkSheet2 = loadTexture( "Pics\\Sword Attacks\\7 Life\\Life Sheet 2-2.png");
	lifeAtkSheet3 = loadTexture( "Pics\\Sword Attacks\\7 Life\\Life Sheet 3-2.png");

	level = 1;
	player.energy = player.energy2 = 300;
	loadLevel(level);
}
App::~App() { SDL_Quit(); }
bool App::UpdateAndRender() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
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
void App::checkKeys() {
	// Menu Keys and Controls
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) { done = true; }
		else if (event.type == SDL_KEYDOWN) {		// Read Single Button presses
			if (mode == 0 && animationDisplayAmt <= 0) {
				if (event.key.keysym.scancode == SDL_SCANCODE_E) { refresh(); }			// Restart current level
				else if (event.key.keysym.scancode == SDL_SCANCODE_R) { reset(); }		// Restart to level 1
				else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) { done = true; }
				else if (event.key.keysym.scancode == SDL_SCANCODE_X) { test(); }
				//else if (event.key.keysym.scancode == SDL_SCANCODE_C) { test2(); }
				// Turn around without moving
				else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (player.facing == 1) { face(3); }
					else if (player.facing == 3) { face(1); }
					player.turning = true; }
	}	}	}


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
	if (mode == -1) { drawMenu(); }
	else if (mode >= 0) {
		drawFloor();
		drawItems();
		drawBoxes();
		drawPlayer();
		drawSwordUI();
		drawTextUI();

		// Render Sword Attacking Animations
		glLoadIdentity();
		glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);
		glScalef(0.6, 0.6, 0.0);
		glTranslatef(1.25, 0.4, 0.0);
		if (animationDisplayAmt > 0 && animationType == 1) {
			if (selSwordAnimation == 0) { swordDisplay(player.facing); }
			else if (selSwordAnimation == 1) { longDisplay(player.facing); }
			else if (selSwordAnimation == 2) { wideDisplay(player.facing); }
			else if (selSwordAnimation == 3) { crossDisplay(player.facing); }
			else if (selSwordAnimation == 4) { spinDisplay(player.facing); }
			else if (selSwordAnimation == 5) { stepDisplay(player.facing); }
			else if (selSwordAnimation == 6) { lifeDisplay(player.facing); } }

		drawBoxesHP();
	}
	SDL_GL_SwapWindow(displayWindow);
}
void App::drawMenu() {

}
void App::drawPlayer() {
	glLoadIdentity();
	glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);
	glScalef(0.6, 0.6, 0.0);
	glTranslatef(1.25, 0.4, 0.0);

	float playerSize = 0.4;
	GLfloat playerPlace[] = { player.x + playerSize, player.y - playerSize, player.x + playerSize, player.y + playerSize,
							  player.x - playerSize, player.y + playerSize, player.x - playerSize, player.y - playerSize };

	float displace = 0.0;
	float displace2 = 0.1;

	int index0a = 14;		// # = Facing direction		// "a" = standing		// "b" = moving
	int index0b = 13;
	int index1a = 0;
	int index1b = 2;
	int index2a = 10;
	int index2b = 11;
	int index3a = 7;
	int index3b = 5;

	// Step Sword Move Animation
	if (animationType == 1 && selSwordAnimation == 5) {
		if		(animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.5) { displace = 1.6; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.4) { displace = 1.2; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.3) { displace = 0.8; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.2) { displace = 0.4; }
		else if (animationDisplayAmt > stepAtkTime + moveAnimationTime * 0.1) { displace = 0.0; }
		if (player.facing == 0) {
			glTranslatef(0, -displace + displace2, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index0b, 8, 2); }
		else if (player.facing == 1) {
			glTranslatef(displace - displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index1b, 8, 2); }
		else if (player.facing == 2) {
			glTranslatef(0.0, displace - displace2, 0.0);
			drawSpriteSheetSprite(playerPlace, playerPic, index2b, 8, 2); }
		else if (player.facing == 3) {
			glTranslatef(-displace + displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index3b, 8, 2); }
	}
	// Slight forward movement if the Player is Attacking
	else if (animationType == 1 && animationDisplayAmt > moveAnimationTime * 0.75) {
		if		(player.facing == 0) {
			glTranslatef(0, displace2, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index0b, 8, 2); }
		else if (player.facing == 1) {
			glTranslatef(-displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index1b, 8, 2); }
		else if (player.facing == 2) {
			glTranslatef(0.0, -displace2, 0.0);
			drawSpriteSheetSprite(playerPlace, playerPic, index2b, 8, 2); }
		else if (player.facing == 3) {
			glTranslatef(displace2, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index3b, 8, 2); }
	}
	// Movement Animation
	else if (animationType == 0 && player.moving) {
		if		(animationDisplayAmt > moveAnimationTime * 0.5) { displace = 0.8; }
		else if (animationDisplayAmt > moveAnimationTime * 0.4) { displace = 0.6; }
		else if (animationDisplayAmt > moveAnimationTime * 0.3) { displace = 0.4; }
		else if (animationDisplayAmt > moveAnimationTime * 0.2) { displace = 0.2; }
		else if (animationDisplayAmt > moveAnimationTime * 0.1) { displace = 0.0; }
		if		(player.moveDir == 0) {
			glTranslatef(0, -displace, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index0b, 8, 2);
		}
		else if (player.moveDir == 1) {
			glTranslatef(displace, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index1b, 8, 2);
		}
		else if (player.moveDir == 2) {
			glTranslatef(0.0, displace, 0.0);
			drawSpriteSheetSprite(playerPlace, playerPic, index2b, 8, 2);
		}
		else if (player.moveDir == 3) {
			glTranslatef(-displace, 0, 0);
			drawSpriteSheetSprite(playerPlace, playerPic, index3b, 8, 2);
		}
	}
	// Player Standing Still
	else {
		int spriteIndex = 0;
		if		(player.facing == 0) { spriteIndex = index0a; }
		else if (player.facing == 1) { spriteIndex = index1a; }
		else if (player.facing == 2) { spriteIndex = index2a; }
		else if (player.facing == 3) { spriteIndex = index3a; }
		drawSpriteSheetSprite(playerPlace, playerPic, spriteIndex, 8, 2);
	}
}
void App::drawSwordUI() {
	glLoadIdentity();
	glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);

	// Draw Sword Icons
	float swordX = -0.45;		float swordY = 3.75;		float swordSize = 0.8;
	GLfloat swordPlace[] = { swordX + 0.25 * swordSize, swordY - 0.25 * swordSize, swordX + 0.25 * swordSize, swordY + 0.25 * swordSize,
							 swordX - 0.25 * swordSize, swordY + 0.25 * swordSize, swordX - 0.25 * swordSize, swordY - 0.25 * swordSize };
	for (int i = 0; i < 7; i++) {
		drawSpriteSheetSprite(swordPlace, swordIconSheet, i + 1, 1, 7);
		glTranslatef(0.0, -0.59, 0.0); }
}
void App::drawFloor() {
	// Draw Tiles
	glLoadIdentity();
	glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);
	glScalef(0.6, 0.6, 0.0);
	glTranslatef(1.25, 0.4, 0.0);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			GLfloat place[] = { i + 0.5, j - 0.5, i + 0.5, j + 0.5,
								i - 0.5, j + 0.5, i - 0.5, j - 0.5 };
			drawSpriteSheetSprite(place, floorSheet, map[i][j].state, 4, 1);
	}	}

	// Draw Goal Icon
	float goalX = 5.8;		float goalY = 5;
	GLfloat goalPlace[] = { goalX + 0.5, goalY - 0.5, goalX + 0.5, goalY + 0.5,
							goalX - 0.5, goalY + 0.5, goalX - 0.5, goalY - 0.5 };
	drawSpriteSheetSprite(goalPlace, goalPic, 0, 1, 1);
}
void App::drawBoxes() {			// Draw the Rock Obstacles on the Floor
	glLoadIdentity();
	glOrtho(-1.0, 5.0, -1.0, 5.0, -1.0, 1.0);
	glScalef(0.6, 0.6, 0.0);
	glTranslatef(1.25, 0.4, 0.0);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (map[i][j].boxHP > 0) {
				GLfloat place[] = { i + 0.4, j - 0.4, i + 0.4, j + 0.4,
									i - 0.4, j + 0.4, i - 0.4, j - 0.4 };
				if (map[i][j].boxType <= 0) {
					int num = map[i][j].boxHP - 1;
					if (num > 3) { num = 3; }
					drawSpriteSheetSprite(place, boxSheet0, num, 3, 1);
				}
				else if (map[i][j].boxType == 1) {
					int num = map[i][j].boxHP - 1;
					if (num > 5) { num = 5; }
					drawSpriteSheetSprite(place, boxSheet1, num, 5, 1);
				}
				else if (map[i][j].boxType == 2) {
					int num = map[i][j].boxHP - 1;
					if (num > 3) { num = 3; }
					drawSpriteSheetSprite(place, boxSheet2, num, 3, 1);
				}
}	}	}	}
void App::drawBoxesHP() {
	glLoadIdentity();
	glTranslatef(-0.355, -0.645, 0.0);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (map[i][j].boxAtkInd > 0) {
				drawText(textSheet2, to_string(map[i][j].boxHP), 0.04, 0, 1, 0, 0);
			}
			else if (map[i][j].boxHP > 0) {
				drawText(textSheet2, to_string(map[i][j].boxHP), 0.04, 0, 1, 1, 0);
			}
			glTranslatef(0, 0.2, 0);
		}
		glTranslatef(0, -0.2 * 6, 0);
		glTranslatef(0.2, 0, 0);
	}
}
void App::drawItems() {		// Draw the Collectable Resources on the Map
	float itemSize = 0.26;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			GLfloat place[] = { i + itemSize, j - itemSize, i + itemSize, j + itemSize,
								i - itemSize, j + itemSize, i - itemSize, j - itemSize };
			if (map[i][j].item > 0) { drawSpriteSheetSprite(place, chargePic1, 0, 1, 1); }
	}	}
}
void App::drawTextUI() {
	glLoadIdentity();

	// Display current Level number
	glTranslatef(-0.9, 0.9, 0.0);
	drawText(textSheet2, "Level " + to_string(level), 0.08, -0.01);

	// Display number of times Refreshed
	if (refreshCount > 0) {
		glTranslatef(1.8, -1.8, 0.0);
		if (refreshCount >= 10) { glTranslatef(-0.038, 0.0, 0.0); }
		if (refreshCount >= 100) { glTranslatef(-0.038, 0.0, 0.0); }
		drawText(textSheet2, to_string(refreshCount), 0.05, -0.01);
	}
	glLoadIdentity();
	glTranslatef(-0.9, 0.9, 0.0);

	// Display Sword Numbers, Icons, and Costs
	glTranslatef( 0.01, -0.3,   0);		drawText(textSheet2, "1", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, " " + to_string(swordCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "2", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, " " + to_string(longCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "3", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, to_string(wideCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "4", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, to_string(crossCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "5", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, to_string(spinCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "6", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, to_string(stepCost), 0.03, -0.005);
	glTranslatef(-0.15, -0.135, 0);		drawText(textSheet2, "7", 0.03, -0.005);
	glTranslatef( 0.15, -0.06,  0);		drawText(textSheet2, to_string(lifeCost), 0.03, -0.005);

	// Draw Battery Icon & Energy Amount
	glTranslatef(-0.01, -0.3, 0.0);
	GLfloat batteryPlace[] = {  0.05, -0.05,  0.05,  0.05,
							   -0.05,  0.05, -0.05, -0.05 };
	drawSpriteSheetSprite(batteryPlace, batteryPic, 0, 1, 1);
	glTranslatef(0.1, 0.0, 0.0);
	drawText(textSheet2, to_string(player.energy), 0.06, -0.01);

	// Display Energy Gain Icon when picking up Energy
	if (chargeDisplayPlusAmt > 0) {
		glTranslatef(-0.09, 0.1, 0.0);
		GLfloat chargePlace[] = {  0.04, -0.04,  0.04,  0.04,
								  -0.04,  0.04, -0.04, -0.04 };
		drawSpriteSheetSprite(chargePlace, chargePic2, 0, 1, 1);
		glTranslatef(0.07, 0.018, 0.0);
		drawText(textSheet2, to_string(energyGainAmt), 0.05, -0.01, (float)30/256, (float)167/256, (float)225/256);
	}

	// Displayer Energy Loss Icon when using Energy
	if (chargeDisplayMinusAmt > 0) {
		glTranslatef(-0.09, 0.1, 0.0);
		GLfloat chargePlace[] = {  0.04, -0.04,  0.04,  0.04,
								  -0.04,  0.04, -0.04, -0.04 };
		drawSpriteSheetSprite(chargePlace, chargePic3, 0, 1, 1);
		glTranslatef(0.07, 0.018, 0.0);
		if		(selSwordAnimation == 0) { drawText(textSheet2, to_string(swordCost), 0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 1) { drawText(textSheet2, to_string(longCost),  0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 2) { drawText(textSheet2, to_string(wideCost),  0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 3) { drawText(textSheet2, to_string(crossCost), 0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 4) { drawText(textSheet2, to_string(spinCost),  0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 5) { drawText(textSheet2, to_string(stepCost),  0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
		else if (selSwordAnimation == 6) { drawText(textSheet2, to_string(lifeCost),  0.05, -0.01, (float)214 / 256, (float)99 / 256, (float)67 / 256); }
	}
}
void App::updateGame(float elapsed) {
	chargeDisplayPlusAmt -= elapsed;
	if (chargeDisplayPlusAmt < 0) { chargeDisplayPlusAmt = 0; }

	chargeDisplayMinusAmt -= elapsed;
	if (chargeDisplayMinusAmt < 0) { chargeDisplayMinusAmt = 0; }

	animationDisplayAmt -= elapsed;
	if (animationDisplayAmt < stepAtkTime && selSwordAnimation == 5 && animationType == 1) { player.moving = false; }
	if (animationDisplayAmt < 0) {
		animationDisplayAmt = 0;
		if (selSwordAnimation != 5 || animationType == 0) { player.moving = false; }
		player.turning = false;
		player.moveDir = -1;
		animationType = 0;
	}

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map[i][j].boxAtkInd -= elapsed;
			if (map[i][j].boxAtkInd < 0) { map[i][j].boxAtkInd = 0; }	}	}

	for (int i = 0; i < delayedHpList.size(); i++) {
		delayedHpList[i].delay -= elapsed;
		if (delayedHpList[i].delay <= 0) {
			hitBox(delayedHpList[i].xPos, delayedHpList[i].yPos);
			delayedHpList.erase(delayedHpList.begin() + i);	}	 }
}

// Sword Attack Animations
void App::swordDisplay(int dir) {
	float atkSize = 0.575;
	int xPos = player.x;		int yPos = player.y;
	if (dir == 0) { yPos++; }
	else if (dir == 1) { xPos--; }
	else if (dir == 2) { yPos--; }
	else if (dir == 3) { xPos++; }
	GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize, xPos + atkSize, yPos + atkSize,
						   xPos - atkSize, yPos + atkSize, xPos - atkSize, yPos - atkSize };
	if (dir == 0) {
		if		(animationDisplayAmt > swordAtkTime) {}
		else if (animationDisplayAmt > swordAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet0, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet0, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						 { drawSpriteSheetSprite(atkPlace, swordAtkSheet0, 2, 3, 1); }
	}
	else if (dir == 1) {
		if		(animationDisplayAmt > swordAtkTime) {}
		else if (animationDisplayAmt > swordAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						 { drawSpriteSheetSprite(atkPlace, swordAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 2) {
		if		(animationDisplayAmt > swordAtkTime) {}
		else if (animationDisplayAmt > swordAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet2, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet2, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						 { drawSpriteSheetSprite(atkPlace, swordAtkSheet2, 2, 3, 1); }
	}
	else if (dir == 3) {
		if		(animationDisplayAmt > swordAtkTime) {}
		else if	(animationDisplayAmt > swordAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > swordAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						 { drawSpriteSheetSprite(atkPlace, swordAtkSheet3, 2, 3, 1); }
	}
}
void App::longDisplay(int dir) {
	float atkSize = 0.6;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		yPos += 1.5;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 2.5, xPos + atkSize, yPos + atkSize * 2.5,
							   xPos - atkSize, yPos + atkSize * 2.5, xPos - atkSize, yPos - atkSize * 2.5 };
		if		(animationDisplayAmt > longAtkTime) {}
		else if (animationDisplayAmt > longAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet0, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet0, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, longAtkSheet0, 2, 3, 1); }
	}
	else if (dir == 1) {
		xPos -= 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 2.5, yPos - atkSize, xPos + atkSize * 2.5, yPos + atkSize,
							   xPos - atkSize * 2.5, yPos + atkSize, xPos - atkSize * 2.5, yPos - atkSize };
		if		(animationDisplayAmt > longAtkTime) {}
		else if (animationDisplayAmt > longAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, longAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 2) {
		yPos -= 1.5;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 2.5, xPos + atkSize, yPos + atkSize * 2.5,
							   xPos - atkSize, yPos + atkSize * 2.5, xPos - atkSize, yPos - atkSize * 2.5 };
		if		(animationDisplayAmt > longAtkTime) {}
		else if (animationDisplayAmt > longAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet2, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet2, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, longAtkSheet2, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos += 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 2.5, yPos - atkSize, xPos + atkSize * 2.5, yPos + atkSize,
							   xPos - atkSize * 2.5, yPos + atkSize, xPos - atkSize * 2.5, yPos - atkSize };
		if		(animationDisplayAmt > longAtkTime) {}
		else if (animationDisplayAmt > longAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > longAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, longAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, longAtkSheet3, 2, 3, 1); }
	}
}
void App::wideDisplay(int dir) {
	float atkSize = 0.5;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		yPos++;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize, xPos + atkSize * 4, yPos + atkSize,
							   xPos - atkSize * 4, yPos + atkSize, xPos - atkSize * 4, yPos - atkSize };
		if		(animationDisplayAmt > wideAtkTime) {}
		else if (animationDisplayAmt > wideAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet0, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet0, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, wideAtkSheet0, 2, 3, 1); }
	}
	else if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 4, xPos + atkSize, yPos + atkSize * 4,
							   xPos - atkSize, yPos + atkSize * 4, xPos - atkSize, yPos - atkSize * 4 };
		if		(animationDisplayAmt > wideAtkTime) {}
		else if (animationDisplayAmt > wideAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, wideAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 2) {
		yPos--;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize, xPos + atkSize * 4, yPos + atkSize,
							   xPos - atkSize * 4, yPos + atkSize, xPos - atkSize * 4, yPos - atkSize };
		if		(animationDisplayAmt > wideAtkTime) {}
		else if (animationDisplayAmt > wideAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet2, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet2, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, wideAtkSheet2, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 4, xPos + atkSize, yPos + atkSize * 4,
							   xPos - atkSize, yPos + atkSize * 4, xPos - atkSize, yPos - atkSize * 4 };
		if		(animationDisplayAmt > wideAtkTime) {}
		else if (animationDisplayAmt > wideAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > wideAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, wideAtkSheet3, 2, 3, 1); }
	}
}
void App::crossDisplay(int dir) {
	float atkSize = 1.5;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		yPos++;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize, xPos + atkSize, yPos + atkSize,
							   xPos - atkSize, yPos + atkSize, xPos - atkSize, yPos - atkSize };
		if		(animationDisplayAmt > crossAtkTime) {}
		else if (animationDisplayAmt > crossAtkTime * 0.8) { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 0, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.6) { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 1, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.5) { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 2, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 3, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 4, 6, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, crossAtkSheet0, 5, 6, 1); }
	}
	else if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize, xPos + atkSize, yPos + atkSize,
							   xPos - atkSize, yPos + atkSize, xPos - atkSize, yPos - atkSize };
		if		(animationDisplayAmt > crossAtkTime) {}
		else if (animationDisplayAmt > crossAtkTime * 0.8) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 0, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.6) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 1, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.5) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 2, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 3, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 4, 6, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, crossAtkSheet1, 5, 6, 1); }
	}
	else if (dir == 2) {
		yPos--;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize, xPos + atkSize, yPos + atkSize,
							   xPos - atkSize, yPos + atkSize, xPos - atkSize, yPos - atkSize };
		if		(animationDisplayAmt > crossAtkTime) {}
		else if (animationDisplayAmt > crossAtkTime * 0.8) { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 0, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.6) { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 1, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.5) { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 2, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 3, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 4, 6, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, crossAtkSheet2, 5, 6, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize, xPos + atkSize, yPos + atkSize,
							   xPos - atkSize, yPos + atkSize, xPos - atkSize, yPos - atkSize };
		if		(animationDisplayAmt > crossAtkTime) {}
		else if (animationDisplayAmt > crossAtkTime * 0.8) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 0, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.6) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 1, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.5) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 2, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 3, 6, 1); }
		else if (animationDisplayAmt > crossAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 4, 6, 1); }
		else if (animationDisplayAmt > 0)				   { drawSpriteSheetSprite(atkPlace, crossAtkSheet3, 5, 6, 1); }
	}
}
void App::spinDisplay(int dir) {
	float atkSize = 0.5;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		GLfloat atkPlace[] = { xPos + atkSize * 3, yPos - atkSize * 3, xPos + atkSize * 3, yPos + atkSize * 3,
							   xPos - atkSize * 3, yPos + atkSize * 3, xPos - atkSize * 3, yPos - atkSize * 3 };
		if		(animationDisplayAmt > spinAtkTime) {}
		else if (animationDisplayAmt > spinAtkTime * 5.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet0, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 3.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet0, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 1.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet0, 2, 4, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, spinAtkSheet0, 3, 4, 1); }
	}
	else if (dir == 1) {
		GLfloat atkPlace[] = { xPos + atkSize * 3, yPos - atkSize * 3, xPos + atkSize * 3, yPos + atkSize * 3,
							   xPos - atkSize * 3, yPos + atkSize * 3, xPos - atkSize * 3, yPos - atkSize * 3 };
		if		(animationDisplayAmt > spinAtkTime) {}
		else if (animationDisplayAmt > spinAtkTime * 5.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 3.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 1.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 2, 4, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, spinAtkSheet1, 3, 4, 1); }
	}
	else if (dir == 2) {
		GLfloat atkPlace[] = { xPos + atkSize * 3, yPos - atkSize * 3, xPos + atkSize * 3, yPos + atkSize * 3,
							   xPos - atkSize * 3, yPos + atkSize * 3, xPos - atkSize * 3, yPos - atkSize * 3 };
		if		(animationDisplayAmt > spinAtkTime) {}
		else if (animationDisplayAmt > spinAtkTime * 5.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet2, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 3.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet2, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 1.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet2, 2, 4, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, spinAtkSheet2, 3, 4, 1); }
	}
	else if (dir == 3) {
		GLfloat atkPlace[] = { xPos + atkSize * 3, yPos - atkSize * 3, xPos + atkSize * 3, yPos + atkSize * 3,
							   xPos - atkSize * 3, yPos + atkSize * 3, xPos - atkSize * 3, yPos - atkSize * 3 };
		if		(animationDisplayAmt > spinAtkTime) {}
		else if (animationDisplayAmt > spinAtkTime * 5.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 0, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 3.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 1, 4, 1); }
		else if (animationDisplayAmt > spinAtkTime * 1.0 / 6.0) { drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 2, 4, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, spinAtkSheet3, 3, 4, 1); }
	}
}
void App::stepDisplay(int dir) {
	float atkSize = 0.5;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		yPos++;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize, xPos + atkSize * 4, yPos + atkSize,
							   xPos - atkSize * 4, yPos + atkSize, xPos - atkSize * 4, yPos - atkSize };
		if		(animationDisplayAmt > stepAtkTime) {}
		else if	(animationDisplayAmt > stepAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet0, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet0, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, stepAtkSheet0, 2, 3, 1); }
	}
	else if (dir == 1) {
		xPos--;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 4, xPos + atkSize, yPos + atkSize * 4,
							   xPos - atkSize, yPos + atkSize * 4, xPos - atkSize, yPos - atkSize * 4 };
		if		(animationDisplayAmt > stepAtkTime) {}
		else if	(animationDisplayAmt > stepAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, stepAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 2) {
		yPos--;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize, xPos + atkSize * 4, yPos + atkSize,
							   xPos - atkSize * 4, yPos + atkSize, xPos - atkSize * 4, yPos - atkSize };
		if		(animationDisplayAmt > stepAtkTime) {}
		else if (animationDisplayAmt > stepAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet2, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet2, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, stepAtkSheet2, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos++;
		GLfloat atkPlace[] = { xPos + atkSize, yPos - atkSize * 4, xPos + atkSize, yPos + atkSize * 4,
							   xPos - atkSize, yPos + atkSize * 4, xPos - atkSize, yPos - atkSize * 4 };
		if		(animationDisplayAmt > stepAtkTime) {}
		else if	(animationDisplayAmt > stepAtkTime * 2.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > stepAtkTime * 1.0 / 3.0) { drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > 0)						{ drawSpriteSheetSprite(atkPlace, stepAtkSheet3, 2, 3, 1); }
	}
}
void App::lifeDisplay(int dir) {
	float atkSize = 0.5;
	float xPos = player.x;		float yPos = player.y;
	if (dir == 0) {
		yPos += 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize * 2, xPos + atkSize * 4, yPos + atkSize * 2,
							   xPos - atkSize * 4, yPos + atkSize * 2, xPos - atkSize * 4, yPos - atkSize * 2 };
		if		(animationDisplayAmt > lifeAtkTime) {}
		else if (animationDisplayAmt > lifeAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet0, 0, 3, 1); }
		else if (animationDisplayAmt > lifeAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet0, 1, 3, 1); }
		else if (animationDisplayAmt > 0)				  { drawSpriteSheetSprite(atkPlace, lifeAtkSheet0, 2, 3, 1); }
	}
	else if (dir == 1) {
		xPos -= 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 2, yPos - atkSize * 4, xPos + atkSize * 2, yPos + atkSize * 4,
							   xPos - atkSize * 2, yPos + atkSize * 4, xPos - atkSize * 2, yPos - atkSize * 4 };
		if		(animationDisplayAmt > lifeAtkTime) {}
		else if (animationDisplayAmt > lifeAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 0, 3, 1); }
		else if (animationDisplayAmt > lifeAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 1, 3, 1); }
		else if (animationDisplayAmt > 0)				  { drawSpriteSheetSprite(atkPlace, lifeAtkSheet1, 2, 3, 1); }
	}
	else if (dir == 2) {
		yPos -= 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 4, yPos - atkSize * 2, xPos + atkSize * 4, yPos + atkSize * 2,
							   xPos - atkSize * 4, yPos + atkSize * 2, xPos - atkSize * 4, yPos - atkSize * 2 };
		if		(animationDisplayAmt > lifeAtkTime) {}
		else if (animationDisplayAmt > lifeAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet2, 0, 3, 1); }
		else if (animationDisplayAmt > lifeAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet2, 1, 3, 1); }
		else if (animationDisplayAmt > 0)				  { drawSpriteSheetSprite(atkPlace, lifeAtkSheet2, 2, 3, 1); }
	}
	else if (dir == 3) {
		xPos += 1.5;
		GLfloat atkPlace[] = { xPos + atkSize * 2, yPos - atkSize * 4, xPos + atkSize * 2, yPos + atkSize * 4,
							   xPos - atkSize * 2, yPos + atkSize * 4, xPos - atkSize * 2, yPos - atkSize * 4 };
		if		(animationDisplayAmt > lifeAtkTime) {}
		else if (animationDisplayAmt > lifeAtkTime * 0.3) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 0, 3, 1); }
		else if (animationDisplayAmt > lifeAtkTime * 0.1) { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 1, 3, 1); }
		else if (animationDisplayAmt > 0)				  { drawSpriteSheetSprite(atkPlace, lifeAtkSheet3, 2, 3, 1); }
	}
}

// Player Control Functions: Attacking & Movement
void App::face(int dir) {
	if (dir >= 0 && dir <= 3) {
		player.facing = dir;
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
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 1, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x--;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 1)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y--;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 3) {	// Move Right
		if (player.x == 5 && player.y == 5) { next(); return; }
		if (isTileValid(player.x + 1, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x++;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
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
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 1) {	// Move Left
		if (isTileValid(player.x - 2, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x -= 2;
			player.moving = true;
			player.moveDir = 1;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 2) {	// Move Down
		if (isTileValid(player.x, player.y - 2)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.y -= 2;
			player.moving = true;
			player.moveDir = 2;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
	else if (dir == 3) {	// Move Right
		if (player.x == 5 && player.y == 5) { next(); return; }
		if (isTileValid(player.x + 2, player.y)) {
			if (map[player.x][player.y].state == 1) { map[player.x][player.y].state = 2; }		// Walk off Cracked tile -> Cracked tile becomes a Hole
			player.x += 2;
			player.moving = true;
			player.moveDir = 3;
			animationType = 0;
			animationDisplayAmt = moveAnimationTime;
			if (map[player.x][player.y].item > 0) {												// Walk onto Energy resource -> Take Energy
				player.energy += energyGainAmt;
				chargeDisplayPlusAmt = iconDisplayTime;
				chargeDisplayMinusAmt = 0;
				map[player.x][player.y].item = 0; } } }
}
void App::swordAtk(int dir) {			// Does One Dmg to one square in front of the player
	if (player.energy >= swordCost) {
		player.energy -= swordCost;
		if (dir == 0)	   { hitBox(player.x, player.y + 1); }
		else if (dir == 1) { hitBox(player.x - 1, player.y); }
		else if (dir == 2) { hitBox(player.x, player.y - 1); }
		else if (dir == 3) { hitBox(player.x + 1, player.y); }
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = swordAtkTime + preAtkTime;
		selSwordAnimation = 0;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::longAtk(int dir) {		// Does One Dmg to a 2x1 area in front of the player
	if (player.energy >= longCost) {
		player.energy -= longCost;
		if (dir == 0) {						//	  x
			hitBox(player.x, player.y + 1);	//	  x
			hitBox(player.x, player.y + 2);	//	  P
		}
		else if (dir == 1) {				//
			hitBox(player.x - 1, player.y);	//	xxP
			hitBox(player.x - 2, player.y);	//
		}
		else if (dir == 2) {				//	  P
			hitBox(player.x, player.y - 1);	//	  x
			hitBox(player.x, player.y - 2);	//	  x
		}
		else if (dir == 3) {				//
			hitBox(player.x + 1, player.y);	//	  Pxx
			hitBox(player.x + 2, player.y);	//
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = longAtkTime + preAtkTime;
		selSwordAnimation = 1;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::wideAtk(int dir) {			// Does One Dmg to a sweep of 3x1 area in front of the player
	if (player.energy >= wideCost) {
		player.energy -= wideCost;
		if (dir == 0) {
			hitBox(player.x - 1, player.y + 1);	// xxx
			hitBox(player.x,     player.y + 1);	//	P
			hitBox(player.x + 1, player.y + 1);	//
		}
		else if (dir == 1) {
			hitBox(player.x - 1, player.y + 1);	// x
			hitBox(player.x - 1, player.y);		// xP
			hitBox(player.x - 1, player.y - 1);	// x
		}
		else if (dir == 2) {
			hitBox(player.x - 1, player.y - 1);	//
			hitBox(player.x,     player.y - 1);	//	P
			hitBox(player.x + 1, player.y - 1);	// xxx
		}
		else if (dir == 3) {
			hitBox(player.x + 1, player.y + 1);	//	 x
			hitBox(player.x + 1, player.y);		//	Px
			hitBox(player.x + 1, player.y - 1);	//	 x
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = wideAtkTime + preAtkTime;
		selSwordAnimation = 2;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::crossAtk(int dir) {			// Does One Dmg in an X pattern in front of the player		// The Middle of the X cross takes Two Dmg total
	if (player.energy >= crossCost) {
		player.energy -= crossCost;
		if (dir == 0) {
			hitBox(player.x - 1, player.y);		//	x x
			hitBox(player.x,	 player.y + 1);	//	 x
			hitBox(player.x + 1, player.y + 2);	//	xPx
			hitBoxDelay(player.x + 1, player.y,		0.6);
			hitBoxDelay(player.x,	  player.y + 1, 0.6);
			hitBoxDelay(player.x - 1, player.y + 2, 0.6);
		}
		else if (dir == 1) {
			hitBox(player.x,	 player.y - 1);	//	x x
			hitBox(player.x - 1, player.y);		//	 xP
			hitBox(player.x - 2, player.y + 1);	//	x x
			hitBoxDelay(player.x,	  player.y + 1, 0.6);
			hitBoxDelay(player.x - 1, player.y,		0.6);
			hitBoxDelay(player.x - 2, player.y - 1, 0.6);
		}
		else if (dir == 2) {
			hitBox(player.x - 1, player.y);		//	xPx
			hitBox(player.x,	 player.y - 1);	//	 x
			hitBox(player.x + 1, player.y - 2);	//	x x
			hitBoxDelay(player.x + 1, player.y,		0.6);
			hitBoxDelay(player.x,	  player.y - 1, 0.6);
			hitBoxDelay(player.x - 1, player.y - 2, 0.6);
		}
		else if (dir == 3) {
			hitBox(player.x,	 player.y - 1);	//	x x
			hitBox(player.x + 1, player.y);		//	Px
			hitBox(player.x + 2, player.y + 1);	//	x x
			hitBoxDelay(player.x,	  player.y + 1, 0.6);
			hitBoxDelay(player.x + 1, player.y,		0.6);
			hitBoxDelay(player.x + 2, player.y - 1, 0.6);
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = crossAtkTime + preAtkTime;
		selSwordAnimation = 3;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::spinAtk(int dir) {		// Does One Dmg to all squares adjacent to the player (including diagonal adjacents)
	if (player.energy >= spinCost) {
		player.energy -= spinCost;
		hitBox(player.x - 1, player.y + 1);	//	xxx
		hitBox(player.x - 1, player.y);		//	xPx
		hitBox(player.x - 1, player.y - 1);	//	xxx
		hitBox(player.x,	 player.y - 1);
		hitBox(player.x,	 player.y + 1);
		hitBox(player.x + 1, player.y + 1);
		hitBox(player.x + 1, player.y);
		hitBox(player.x + 1, player.y - 1);

		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = stepAtkTime + preAtkTime;
		selSwordAnimation = 4;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::stepAtk(int dir) {			// Moves the player two squares in the facing Direction, then uses WideSword
	if (player.energy >= stepCost) {
		if (dir == 0 && isTileValid(player.x, player.y + 2)) {
			player.energy -= stepCost;
			move2(0);
			hitBoxDelay(player.x - 1, player.y + 1, moveAnimationTime);	// xxx
			hitBoxDelay(player.x, player.y + 1, moveAnimationTime);		//	P
			hitBoxDelay(player.x + 1, player.y + 1, moveAnimationTime);	//
			
			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime + preAtkTime;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
		}
		else if (dir == 1 && isTileValid(player.x - 2, player.y)) {
			player.energy -= stepCost;
			move2(1);
			hitBoxDelay(player.x - 1, player.y + 1, moveAnimationTime);	// x
			hitBoxDelay(player.x - 1, player.y, moveAnimationTime);		// xP
			hitBoxDelay(player.x - 1, player.y - 1, moveAnimationTime);	// x

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime + preAtkTime;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
		}
		else if (dir == 2 && isTileValid(player.x, player.y - 2)) {
			player.energy -= stepCost;
			move2(2);
			hitBoxDelay(player.x - 1, player.y - 1, moveAnimationTime);	//
			hitBoxDelay(player.x,     player.y - 1, moveAnimationTime);	//	P
			hitBoxDelay(player.x + 1, player.y - 1, moveAnimationTime);	// xxx

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime + preAtkTime;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
		}
		else if (dir == 3 && isTileValid(player.x + 2, player.y)) {
			player.energy -= stepCost;
			move2(3);
			hitBoxDelay(player.x + 1, player.y + 1, moveAnimationTime);	//	 x
			hitBoxDelay(player.x + 1, player.y, moveAnimationTime);		//	Px
			hitBoxDelay(player.x + 1, player.y - 1, moveAnimationTime);	//	 x

			animationType = 1;
			animationDisplayAmt = stepAtkTime + moveAnimationTime + preAtkTime;
			selSwordAnimation = 5;
			chargeDisplayMinusAmt = iconDisplayTime;
			chargeDisplayPlusAmt = 0;
		}
	}
}
void App::lifeAtk(int dir) {		// Does Two Dmg to a 2x3 area in front the player
	if (player.energy >= lifeCost) {
		player.energy -= lifeCost;
		if (dir == 0) {
			hitBox(player.x - 1, player.y + 2);	hitBoxDelay(player.x - 1, player.y + 2, 0.75);	// xxx
			hitBox(player.x,	 player.y + 2);	hitBoxDelay(player.x,	  player.y + 2, 0.75);	// xxx
			hitBox(player.x + 1, player.y + 2);	hitBoxDelay(player.x + 1, player.y + 2, 0.75);	//	P
			hitBox(player.x - 1, player.y + 1);	hitBoxDelay(player.x - 1, player.y + 1, 0.75);
			hitBox(player.x,	 player.y + 1);	hitBoxDelay(player.x,	  player.y + 1, 0.75);
			hitBox(player.x + 1, player.y + 1);	hitBoxDelay(player.x + 1, player.y + 1, 0.75);
		}
		else if (dir == 1) {
			hitBox(player.x - 2, player.y + 1);	hitBoxDelay(player.x - 2, player.y + 1, 0.75);	// xx
			hitBox(player.x - 2, player.y);		hitBoxDelay(player.x - 2, player.y,		0.75);	// xxP
			hitBox(player.x - 2, player.y - 1);	hitBoxDelay(player.x - 2, player.y - 1, 0.75);	// xx
			hitBox(player.x - 1, player.y + 1);	hitBoxDelay(player.x - 1, player.y + 1, 0.75);
			hitBox(player.x - 1, player.y);		hitBoxDelay(player.x - 1, player.y,		0.75);
			hitBox(player.x - 1, player.y - 1);	hitBoxDelay(player.x - 1, player.y - 1, 0.75);
		}
		else if (dir == 2) {
			hitBox(player.x - 1, player.y - 2); hitBoxDelay(player.x - 1, player.y - 2, 0.75);	//  P
			hitBox(player.x,	 player.y - 2);	hitBoxDelay(player.x, player.y - 2,		0.75);	// xxx
			hitBox(player.x + 1, player.y - 2);	hitBoxDelay(player.x + 1, player.y - 2, 0.75);	// xxx
			hitBox(player.x - 1, player.y - 1);	hitBoxDelay(player.x - 1, player.y - 1, 0.75);
			hitBox(player.x,	 player.y - 1);	hitBoxDelay(player.x, player.y - 1,		0.75);
			hitBox(player.x + 1, player.y - 1);	hitBoxDelay(player.x + 1, player.y - 1, 0.75);
		}
		else if (dir == 3) {
			hitBox(player.x + 2, player.y + 1);	hitBoxDelay(player.x + 2, player.y + 1, 0.75);	//  xx
			hitBox(player.x + 2, player.y);		hitBoxDelay(player.x + 2, player.y,		0.75);	// Pxx
			hitBox(player.x + 2, player.y - 1);	hitBoxDelay(player.x + 2, player.y - 1, 0.75);	//  xx
			hitBox(player.x + 1, player.y + 1);	hitBoxDelay(player.x + 1, player.y + 1, 0.75);
			hitBox(player.x + 1, player.y);		hitBoxDelay(player.x + 1, player.y,		0.75);
			hitBox(player.x + 1, player.y - 1);	hitBoxDelay(player.x + 1, player.y - 1, 0.75);
		}
		// Start Sword Attack Animation
		animationType = 1;
		animationDisplayAmt = lifeAtkTime + preAtkTime;
		selSwordAnimation = 6;
		chargeDisplayMinusAmt = iconDisplayTime;
		chargeDisplayPlusAmt = 0;
	}
}
void App::hitBox(int xPos, int yPos) {
	if( xPos >= 0 && xPos <= 5 && yPos >= 0 && yPos <= 5) {
		if (map[xPos][yPos].boxHP > 0) {
			map[xPos][yPos].boxHP--;
			map[xPos][yPos].boxAtkInd = 0.25;
		}
}	}
void App::hitBoxDelay(int xPos, int yPos, float delay) {
	DelayedHpLoss one;
	one.xPos = xPos;
	one.yPos = yPos;
	one.delay = delay;
	delayedHpList.push_back(one);
}

// Map Functions
bool App::isTileValid(int xPos, int yPos) {		// Checks if a specific tile allows the Player to walk on it
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
	player.energy = player.energy2 = 300;
	mode = 0;
	chargeDisplayPlusAmt = 0;
	chargeDisplayMinusAmt = 0;
	animationDisplayAmt = 0;
	delayedHpList.clear();
	menuSel = 0;
	level = 1;
	loadLevel(level);
	refreshCount = 0;
}
void App::refresh() {
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
	menuSel = 0;
	refreshCount++;
}
void App::next() {
	level++;
	loadLevel(level);
	player.facing = 3;
	player.energy2 = player.energy;
	menuSel = 0;
	animationDisplayAmt = 0.55;
}
void App::loadLevel(int num) {		// Negative levels are tutorial levels
	clearFloor();
	player.x = 0;		player.y = 0;
	if (level == 1) { generateLevel(0, 0); }
	else {
		int type = getRand(21);		// Random number between 0 & 20
		generateLevel(type); }		// Generate a Level of Random type

	for (int i = 0; i < 6; i++) {		// Save a copy of the current map		// For if the player wants to redo this level
		for (int j = 0; j < 6; j++) {
			map2[i][j] = map[i][j]; } }
}
void App::generateLevel(int type, int num) {
	if (num == -1) { num = rand() % 100 + 1; }			// Random number between 1 and 100, inclusive
	int x = level;		if (x > 28) { x = 28; }			// Random number used in determining difficulty ("diff")
	int bound1 = 90 - x * 3;							// Bounds used in determining difficulty
	int bound2 = 100 - x * 2;							// Bounds are based on current level number
	int diff = 0;				// difficulty 0, 1, 2 = easy, normal, hard			// level		easy	normal	hard
	if (num > bound1 && num <= bound2) { diff = 1; }								// 0			90%		10%		 0%
	else if (num > bound2) { diff = 2; }											// 10			60%		20%		20%
	int gain = -4 - diff * 6;	// Gain determines avg "profit" per level			// 20			30%		30%		40%
	//if (num % 2) { gain += 4; }													// 28+			 6%		38%		56%
	int extraBoxes, extraItems, floorDmgs;		// Number of things per floor based on level number and difficulty

	// 20 Types of Maps, Indexed 0-19
	if (type == 0) {			// Rooms A
		map[1][3].boxHP = 1;	map[4][0].boxHP = 1;	map[4][4].boxHP = 1;							//	]=O==O=[
		map[1][4].boxHP = 1;	map[4][1].boxHP = 1;	map[4][5].boxHP = 1;							//	]=O==O=[
		map[1][5].boxHP = 1;	map[4][2].boxHP = 1;													//	]=O==  [
		map[0][2].state = 3;	map[1][2].state = 3;	map[4][3].state = 3;	map[5][3].state = 3;	//	]  ==O=[
		extraBoxes = 3 + x / 3 + diff * 2;		// Number of Boxes based on Level number				//	]====O=[
		extraItems = 8 + extraBoxes + gain;		// Number of Items based on "gain"						//	]====O=[
		floorDmgs = rand() % 3 + 1 + diff * 3;	// 1-3	// 4-6	// 7-9
												// Number of Damaged Floors based on difficulty
	}
	else if (type == 1) {		// Rooms B
		map[0][4].state = 3;	map[1][4].state = 3;	map[2][0].state = 3;	map[4][5].state = 3;	//	]  == =[
		map[0][5].state = 3;	map[1][5].state = 3;	map[2][1].state = 3;							//	]  OOO=[
		map[5][0].state = 3;	map[5][1].state = 3;	map[5][2].state = 3;							//	]==O=O=[
		map[2][2].boxHP = 1;	map[2][4].boxHP = 1;	map[3][4].boxHP = 1;	map[4][3].boxHP = 1;	//	]==OOO [
		map[2][3].boxHP = 1;	map[3][2].boxHP = 1;	map[4][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]== == [
		extraBoxes = 3 + x / 3 + diff * 2;																//	]== == [
		extraItems = 8 + extraBoxes + gain;
		floorDmgs = rand() % 2 + diff * 2;		// 0-1	// 2-3	// 4-5
	}
	else if (type == 2) {		// Rooms C
		map[0][2].boxHP = 1;	map[2][0].boxHP = 1;	map[3][0].boxHP = 1;	map[4][2].boxHP = 1;	//	]==OO==[
		map[0][3].boxHP = 1;	map[2][1].boxHP = 1;	map[3][1].boxHP = 1;	map[4][3].boxHP = 1;	//	]==OO==[
		map[1][2].boxHP = 1;	map[2][4].boxHP = 1;	map[3][4].boxHP = 1;	map[5][2].boxHP = 1;	//	]OO==OO[
		map[1][3].boxHP = 1;	map[2][5].boxHP = 1;	map[3][5].boxHP = 1;	map[5][3].boxHP = 1;	//	]OO==OO[
		extraBoxes = 5 + x / 3 + diff * 2;																//	]==OO==[
		extraItems = 16 + extraBoxes + gain;															//	]==OO==[
		floorDmgs = rand() % 3 + diff;			// 0-2	// 1-3	// 2-4
	}
	else if (type == 3) {		// Rooms D
		map[0][2].boxHP = 1;	map[1][4].boxHP = 1;	map[3][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]=O==O=[
		map[0][3].boxHP = 1;	map[1][5].boxHP = 1;	map[3][3].boxHP = 1;	map[4][5].boxHP = 1;	//	]=O==O=[
		map[1][0].boxHP = 1;	map[2][2].boxHP = 1;	map[4][0].boxHP = 1;	map[5][2].boxHP = 1;	//	]O=OO=O[
		map[1][1].boxHP = 1;	map[2][3].boxHP = 1;	map[4][1].boxHP = 1;	map[5][3].boxHP = 1;	//	]O=OO=O[
		extraBoxes = 5 + x / 3 + diff * 2;																//	]=O==O=[
		extraItems = 16 + extraBoxes + gain;															//	]=O==O=[
		floorDmgs = rand() % 3 + diff * 3;		// 0-2	// 3-5	// 6-8
	}
	else if (type == 4) {		// Paths A
		map[0][3].state = 1;	map[5][3].state = 1;													//	]===OO=[
		map[1][3].state = 3;	map[2][3].state = 3;	map[3][3].state = 3;	map[4][3].state = 3;	//	]===OO=[
		map[3][4].boxHP = 1;	map[3][5].boxHP = 1;	map[4][4].boxHP = 1;	map[4][5].boxHP = 1;	//	]X    X[
		map[3][0].boxHP = 1;	map[3][1].boxHP = 1;	map[3][2].boxHP = 1;							//	]===OO=[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;	map[4][2].boxHP = 1;							//	]===OO=[
		extraBoxes = 3 + x / 3 + diff * 2;																//	]===OO=[
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 2 + diff;			// 0-1	// 1-2	// 2-3
	}
	else if (type == 5) {		// Paths B
		map[0][2].state = 3;	map[1][2].state = 3;	map[2][2].state = 3;	//	]==OO==[
		map[3][2].state = 3;	map[4][2].state = 3;							//	]==OO==[
		map[3][0].boxHP = 1;	map[3][1].boxHP = 1;							//	]==OO==[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;							//	]     =[
		map[2][3].boxHP = 1;	map[2][4].boxHP = 1;	map[2][5].boxHP = 1;	//	]===OO=[
		map[3][3].boxHP = 1;	map[3][4].boxHP = 1;	map[3][5].boxHP = 1;	//	]===OO=[
		extraBoxes = 3 + x / 3 + diff * 2;
		extraItems = 10 + extraBoxes + gain;
		floorDmgs = rand() % 2 + diff;			// 0-1	// 1-2	// 2-3
	}
	else if (type == 6) {		// Paths C
		map[1][1].state = 3;	map[1][2].state = 3;	map[2][1].state = 3;	map[2][2].state = 3;	//	]=OO===[
		map[3][3].state = 3;	map[3][4].state = 3;	map[4][3].state = 3;	map[4][4].state = 3;	//	]=OO  =[
		map[1][3].boxHP = 1;	map[1][4].boxHP = 1;	map[1][5].boxHP = 1;							//	]=OO  =[
		map[2][3].boxHP = 1;	map[2][4].boxHP = 1;	map[2][5].boxHP = 1;							//	]=  OO=[
		map[3][0].boxHP = 1;	map[3][1].boxHP = 1;	map[3][2].boxHP = 1;							//	]=  OO=[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;	map[4][2].boxHP = 1;							//	]===OO=[
		extraBoxes = 4 + x / 3 + diff * 2;
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 2 + 1 + diff;		// 1-2	// 2-3	// 3-4
	}
	else if (type == 7) {		// Paths D
		for (int xPos = 1; xPos < 5; xPos++) {									//	]==== =[
			map[xPos][1].state = 3;												//	]=    X[
			map[xPos][4].state = 3; }											//	]=O=OO=[
		map[4][5].state = 3;	map[5][4].state = 1;							//	]=O=OO=[
		map[1][2].boxHP = 1;	map[3][2].boxHP = 1;	map[4][2].boxHP = 1;	//	]=    =[
		map[1][3].boxHP = 1;	map[3][3].boxHP = 1;	map[4][3].boxHP = 1;	//	]==OOO=[
		map[2][0].boxHP = 1;	map[3][0].boxHP = 1;	map[4][0].boxHP = 1;
		extraBoxes = 3 + x / 3 + diff * 2;
		extraItems = 9 + extraBoxes + gain;
		floorDmgs = diff;						// 0	// 1	// 2
	}
	else if (type == 8) {		// Cross A
		map[0][3].state = 3;	map[2][0].state = 3;	map[3][5].state = 3;	map[5][2].state = 3;	//	]=== ==[
		map[1][3].boxHP = 1;	map[2][1].boxHP = 1;	map[2][2].boxHP = 1;	map[2][3].boxHP = 1;	//	]===O==[
		map[3][2].boxHP = 1;	map[3][3].boxHP = 1;	map[3][4].boxHP = 1;	map[4][2].boxHP = 1;	//	] OOO==[
		extraBoxes = 3 + x / 3 + diff * 2;																//	]==OOO [
		extraItems = 10 + extraBoxes + gain;															//	]==O===[
		floorDmgs = rand() % 2 + 1 + diff;		// 1-2	// 2-3	// 3-4									//	]== ===[
	}
	else if (type == 9) {		// Cross B
		map[2][2].state = 1;	map[2][3].state = 1;	map[3][2].state = 1;	map[3][3].state = 1;
		map[0][4].boxHP = 1;	map[0][5].boxHP = 1;	map[1][4].boxHP = 1;	map[1][5].boxHP = 1;	//	]OO===O[
		map[1][1].boxHP = 1;																			//	]OO==O=[
		map[2][2].boxHP = 1;	map[2][3].boxHP = 1;	map[3][2].boxHP = 1;	map[3][3].boxHP = 1;	//	]==OO==[
		map[4][0].boxHP = 1;	map[4][1].boxHP = 1;	map[5][0].boxHP = 1;	map[5][1].boxHP = 1;	//	]==OO==[
		map[4][4].boxHP = 1;	map[5][5].boxHP = 1;													//	]=O==OO[
		extraBoxes = 3 + x / 3 + diff * 2;																//	]====OO[
		extraItems = 15 + extraBoxes + gain;
		floorDmgs = rand() % 3 + diff * 3;		// 0-2	// 3-5	// 6-8
	}
	else if (type == 10) {		// Cross C
		map[0][3].boxHP = 1;	map[1][2].boxHP = 1;	map[1][3].boxHP = 1;	map[2][1].boxHP = 1;	//	]==OO==[
		map[2][2].boxHP = 1;	map[2][4].boxHP = 1;	map[2][5].boxHP = 1;	map[3][0].boxHP = 1;	//	]==O=O=[
		map[3][1].boxHP = 1;	map[3][5].boxHP = 1;	map[4][2].boxHP = 1;	map[4][4].boxHP = 1;	//	]OO===O[
		map[5][2].boxHP = 1;	map[5][3].boxHP = 1;													//	]=OO=OO[
		extraBoxes = 5 + x / 3 + diff * 2;																//	]==OO==[
		extraItems = 14 + extraBoxes + gain;															//	]===O==[
		floorDmgs = rand() % 2 + 1 + diff;		// 1-2	// 2-3	// 3-4
	}
	else if (type == 11) {		// Cross D
		map[1][1].state = 3;	map[1][5].state = 3;	map[2][2].state = 3;							//	]O ====[
		map[3][3].state = 3;	map[4][4].state = 3;	map[4][0].state = 3;							//	]=O=O =[
		map[0][5].boxHP = 1;	map[1][2].boxHP = 1;	map[1][3].boxHP = 1;	map[1][4].boxHP = 1;	//	]=OO O=[
		map[2][1].boxHP = 1;	map[2][3].boxHP = 1;	map[3][2].boxHP = 1;	map[3][4].boxHP = 1;	//	]=O OO=[
		map[4][1].boxHP = 1;	map[4][2].boxHP = 1;	map[4][3].boxHP = 1;	map[5][0].boxHP = 1;	//	]= O=O=[
		extraBoxes = 4 + x / 3 + diff * 2;																//	]==== O[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 2 + 1 + diff;		// 1-2	// 2-3	// 3-4
	}
	else if (type == 12) {		// Gallery A
		for (int i = 1; i < 5; i++) {											//	]======[
			map[i][1].boxHP = 1;	map[i][4].boxHP = 1;						//	]=OOOO=[
			map[1][i].boxHP = 1;	map[4][i].boxHP = 1; }						//	]=O==O=[
		extraBoxes = 3 + x / 3 + diff * 2;										//	]=O==O=[
		extraItems = 9 + extraBoxes + gain;										//	]=OOOO=[
		floorDmgs = rand() % 4 + 1 + diff * 2;	// 1-4	// 3-6	// 5-8			//	]======[
	}
	else if (type == 13) {		// Gallery B									//	]======[
		for (int i = 1; i < 5; i++) {											//	]=O=O=O[
			map[1][i].boxHP = 1;	map[3][i].boxHP = 1;						//	]=O=O=O[
			map[5][i].boxHP = 1; }												//	]=O=O=O[
		extraBoxes = 5 + x / 3 + diff * 2;										//	]=O=O=O[
		extraItems = 16 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 4 + diff * 3;		// 0-4	// 3-7	// 6-10
	}
	else if (type == 14) {		// Gallery C
		for (int i = 0; i < 5; i += 2) {										//	]=O=O==[
			map[1][i + 1].boxHP = 1;											//	]==O=O=[
			map[2][i].boxHP = 1;												//	]=O=O==[
			map[3][i + 1].boxHP = 1;											//	]==O=O=[
			map[4][i].boxHP = 1; }												//	]=O=O==[
		extraBoxes = 4 + x / 3 + diff * 2;										//	]==O=O=[
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 9 + diff * 4;		// 0-8	// 4-12	// 8-16
	}
	else if (type == 15) {		// Gallery D
		for (int xPos = 1; xPos <= 5; xPos += 2) {								//	]=O=O=O[
			for (int yPos = 1; yPos <= 5; yPos += 2) {							//	]======[
				map[xPos][yPos].boxHP = 1;	}	}								//	]=O=O=O[
		map[3][3].state = 1;													//	]======[
		extraBoxes = 3 + x / 3 + diff * 2;										//	]=O=O=O[
		extraItems = 8 + extraBoxes + gain;										//	]======[
		floorDmgs = rand() % 3 + 1 + diff * 3;	// 1-3	// 4-6	// 7-9
	}
	else if (type == 16) {		// Layers A										//	]==OO==[
		for (int yPos = 0; yPos < 6; yPos++) {									//	]==OO==[
			map[2][yPos].boxHP = 1;		map[3][yPos].boxHP = 1; }				//	]==OO==[
		extraBoxes = 4 + x / 3 + diff * 2;										//	]==OO==[
		extraItems = 12 + extraBoxes + gain;									//	]==OO==[
		floorDmgs = rand() % 5 + diff * 4;		// 0-4	// 4-8	// 8-12			//	]==OO==[
	}
	else if (type == 17) {		// Layers B
		for (int i = 0; i < 5; i++) {											//	]     =[
			map[i][5].state = 3;	map[i + 1][0].state = 3; }					//	]=O=O=O[
		for (int yPos = 1; yPos < 5; yPos++) {									//	]=O=O=O[
			map[1][yPos].boxHP = 1;		map[3][yPos].boxHP = 1;					//	]=O=O=O[
			map[5][yPos].boxHP = 1; }											//	]=O=O=O[
		extraBoxes = 4 + x / 3 + diff * 2;										//	]=     [
		extraItems = 12 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 1 + diff * 2;	// 1-4	// 3-6	// 5-8
	}
	else if (type == 18) {		// Layers C
		map[1][5].state = 1;	map[5][1].state = 1;							//	]=XO=O=[
		for (int i = 0; i < 4; i++) {											//	]=O=O=O[
			map[i + 1][4 - i].boxHP = 1;	map[2 + i][5 - i].boxHP = 1; }		//	]==O=O=[
		map[4][5].boxHP = 1;	map[5][4].boxHP = 1;							//	]===O=O[
		extraBoxes = 3 + x / 3 + diff * 2;										//	]====OX[
		extraItems = 10 + extraBoxes + gain;									//	]======[
		floorDmgs = rand() % 4 + diff * 2;		// 0-3	// 2-5	// 4-7
	}
	else if (type == 19) {		// Layers D
		map[2][2].state = 3;	map[2][3].state = 3;							//	]=O=OO=[
		for (int yPos = 0; yPos < 6; yPos++) {									//	]=O=OO=[
			map[1][yPos].boxHP = 1;		map[4][yPos].boxHP = 1; }				//	]=O =OO[
		map[3][0].boxHP = 1;		map[3][1].boxHP = 1;						//	]=O =OO[
		map[3][4].boxHP = 1;		map[3][5].boxHP = 1;						//	]=O=OO=[
		map[5][2].boxHP = 1;		map[5][3].boxHP = 1;						//	]=O=OO=[
		extraBoxes = 6 + x / 3 + diff * 2;
		extraItems = 18 + extraBoxes + gain;
		floorDmgs = rand() % 4 + 1 + diff * 2;	// 1-4	// 3-6	// 5-8
	}
	else { generateLevel(0, num); }

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
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]+====+[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 5; }	//	]+===  [
			else if (place[randPlace] == 3) { xPos = 5; yPos = 0; }	//	]  ===+[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 4; }
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
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 4; }	//	]  ++ =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 5; }	//	]  +===[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 5; }	//	]===== [
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]== =+ [
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]== ++ [
			else if (place[randPlace] == 5) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 2) {		// Rooms C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]++===+[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]===+==[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 5; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 3) {		// Rooms D
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 4; }	//	]+====+[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==++==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]=====+[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 5; }
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
	else if (type == 5) {		// Paths B
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 3; }	//	]++====[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 4; }	//	]++====[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]     =[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 5; }	//	]====+=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 0; }	//	]====+=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 6) {		// Paths C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=++  =[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]=++  =[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]=  ++=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]=  ++=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
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
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 4; }	//	]++= ==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	] ====+[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 0; }	//	]===== [
			else if (place[randPlace] == 4) { xPos = 5; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 1; }	//	]== =++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 9) {		// Cross B
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
	else if (type == 10) {		// Cross C
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 4; }	//	]++===+[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]+=====[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]===+==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 0; }	//	]=====+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]====++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 11) {		// Cross D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 0; yPos = 5; }	//	]+ ====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]===+ =[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 1; }	//	]==+ +=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=+ +==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]= +===[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 4; }	//	]==== +[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 0; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 12) {		// Gallery A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=++++=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=+==+=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=+==+=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]=++++=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 13) {		// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=+=+=+[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=+=+=+[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=+=+=+[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]=+=+=+[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 14) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]=+=+==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==+=+=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=+=+==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 0; }	//	]==+=+=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]=+=+==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]==+=+=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 15) {		// Gallery D
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 1; }	//	]=+=+=+[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=+=+=+[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=+=+=+[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 16) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 1; }	//	]===+==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==++==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 0; }	//	]===+==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]===+==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==++==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]===+==[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 17) {		// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 3; yPos = 1; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 2; }	//	]===+=+[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]===+=+[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===+=+[
			else if (place[randPlace] == 4) { xPos = 5; yPos = 1; }	//	]===+=+[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 5; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 18) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 2; yPos = 5; }	//	]==+=+=[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 4; }	//	]===+++[
			else if (place[randPlace] == 2) { xPos = 4; yPos = 3; }	//	]====+=[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 4; }	//	]=====+[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 5; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1 && map[xPos][yPos].state != 2) {
				map[xPos][yPos].item = 1;
				if (map[xPos][yPos].boxHP > 0) { map[xPos][yPos].boxType = 2; }
				amt -= itemWorth;
				remove(place.begin(), place.end(), place[randPlace]); }
	}	}
	else if (type == 19) {		// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0)	{ xPos = 1; yPos = 2; }	//	]====+=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]====+=[
			else if (place[randPlace] == 2) { xPos = 4; yPos = 0; }	//	]=+ ==+[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 1; }	//	]=+ ==+[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]====++[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 5; }	//	]====++[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 3; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }	//	]=O==O=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=O==O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=O==  [
			else if (place[randPlace] == 3) { xPos = 4; yPos = 0; }	//	]  ==O=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]====O=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]====O=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 5; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	]  == =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]  OOO=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]==O=O=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]==OOO [
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]== == [
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]== == [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
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
	else if (type == 2) {		// Rooms C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if (place[randPlace] == 0)		{ xPos = 0; yPos = 2; }	//	]==OO==[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]OO==OO[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]OO==OO[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 0; }	//	]==OO==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 1; }	//	]==OO==[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 3; }
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
	else if (type == 5) {		// Paths B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 2; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==OO==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]     =[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===OO=[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]===OO=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 1; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if		(place[randPlace] == 0)	{ xPos = 1; yPos = 3; }	//	]=OO===[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 4; }	//	]=OO  =[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=OO  =[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]=  OO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]=  OO=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]===OO=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 2; }
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
			if      (place[randPlace] == 0) { xPos = 1; yPos = 3; }	//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]===O==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	] OOO==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==OOO [
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==O===[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]== ===[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 2; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 4; }	//	]OO===O[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]OO==O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 1; }	//	]==OO==[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]==OO==[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 5; }	//	]=O==OO[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 2; }	//	]====OO[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 13) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 14) { xPos = 5; yPos = 5; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 5; }	//	]O ====[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=O=O =[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=OO O=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O OO=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]= O=O=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 3; }	//	]==== O[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 0; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=OOOO=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=O==O=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O==O=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 1; }	//	]=OOOO=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
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
	else if (type == 13) {		// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=O=O=O[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=O=O=O[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O=O=O[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]=O=O=O[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
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
	else if (type == 14) {		// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]=O=O==[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]==O=O=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=O=O==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 0; }	//	]==O=O=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]=O=O==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 4; }	//	]==O=O=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 2; }
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
	else if (type == 15) {		// Gallery D
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]=O=O=O[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 3; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]=O=O=O[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 1; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 3; }	//	]=O=O=O[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 5; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 5; yPos = 3; }
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
	else if (type == 16) {		// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }	//	]==OO==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]==OO==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==OO==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==OO==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==OO==[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]==OO==[
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
			if      (place[randPlace] == 0) { xPos = 1; yPos = 1; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 2; }	//	]=O=O=O[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 3; }	//	]=O=O=O[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]=O=O=O[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]=O=O=O[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
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
	else if (type == 18) {		// Layers C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 4; }	//	]==O=O=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]=O=O=O[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==O=O=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 2; }	//	]===O=O[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 4; }	//	]====O=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 1; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 4; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=O=OO=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=O=OO=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]=O =OO[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]=O =OO[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 4; }	//	]=O=OO=[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 5; }	//	]=O=OO=[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 5; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 15) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 16) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 17) { xPos = 5; yPos = 3; }
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
		vector<int> place = { 0,1,2,3,4,5,6,7,8 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 3; }	//	]==XX==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==XX==[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 5; }	//	]==XX  [
			else if (place[randPlace] == 3) { xPos = 3; yPos = 0; }	//	]  =X==[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 1; }	//	]===X==[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 3; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 1) {		// Rooms B
		vector<int> place = { 0,1,2,3,4 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	]  == =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]  X=X=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]===X==[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 2; }	//	]==X=X [
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]== ===[
			if (xPos != -1 && yPos != -1) {							//	]== ===[
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 2) {			// Rooms C
		vector<int> place = { 0,1,2,3 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 2; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]======[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 3; }	//	]==XX==[
			if (xPos != -1 && yPos != -1) {							//	]======[
				map[xPos][yPos].state = 1;							//	]======[
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
	else if (type == 5) {			// Paths B
		vector<int> place = { 0,1,2 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 4; yPos = 3; }		//	]====X=[
			else if (place[randPlace] == 1) { xPos = 4; yPos = 4; }		//	]====X=[
			else if (place[randPlace] == 2) { xPos = 4; yPos = 5; }		//	]====X=[
			if (xPos != -1 && yPos != -1) {								//	]     =[
				map[xPos][yPos].state = 1;								//	]======[
				remove(place.begin(), place.end(), place[randPlace]);	//	]======[
				amt--; }
	}	}
	else if (type == 6) {			// Paths C
		vector<int> place = { 0,1,2,3 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 0; }	//	]===  =[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 5; }	//	]===  X[
			else if (place[randPlace] == 3) { xPos = 5; yPos = 3; }	//	]X  ===[
			if (xPos != -1 && yPos != -1) {							//	]=  ===[
				map[xPos][yPos].state = 1;							//	]==X===[
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
		vector<int> place = { 0,1,2,3 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 2; }	//	]=== ==[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 4; }	//	]==X===[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 1; }	//	] ===X=[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 3; }	//	]=X=== [
			if (xPos != -1 && yPos != -1) {							//	]===X==[
				map[xPos][yPos].state = 1;							//	]== ===[
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 9) {			// Cross B
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 5; }	//	]X====X[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 4; }	//	]======[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 1; }	//	]======[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 0; }	//	]=====X[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 10) {			// Cross C
		vector<int> place = { 0,1,2,3,4 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 3; }	//	]======[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 2; }	//	]===X==[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 3; }	//	]==XXX=[
			else if (place[randPlace] == 3) { xPos = 3; yPos = 4; }	//	]===X==[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 3; }	//	]======[
			if (xPos != -1 && yPos != -1) {							//	]======[
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 11) {			// Cross D
		vector<int> place = { 0,1,2,3 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 4; }	//	]= ====[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 3; }	//	]=X== =[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 2; }	//	]==X ==[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 1; }	//	]== X==[
			if (xPos != -1 && yPos != -1) {							//	]= ==X=[
				map[xPos][yPos].state = 1;							//	]==== =[
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 12) {			// Gallery A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 0; }	//	]X====X[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 5; }	//	]=X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 1; }	//	]==XX==[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 4; }	//	]==XX==[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 2; }	//	]=X==X=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 3; }	//	]X====X[
			else if (place[randPlace] == 6) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 3; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 10) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 11) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 13) {			// Gallery B
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }	//	]==X=X=[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 2; }	//	]==X=X=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 3; }	//	]==X=X=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]==X=X=[
			else if (place[randPlace] == 5) { xPos = 2; yPos = 5; }	//	]==X=X=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 14) {			// Gallery C
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 1; }	//	]X=X=X=[
			else if (place[randPlace] == 1) { xPos = 0; yPos = 3; }	//	]=X=X=X[
			else if (place[randPlace] == 2) { xPos = 0; yPos = 5; }	//	]X=X=X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 0; }	//	]=X=X=X[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 2; }	//	]X=X=X=[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 4; }	//	]=X=X=X[
			else if (place[randPlace] == 6) { xPos = 2; yPos = 1; }
			else if (place[randPlace] == 7) { xPos = 2; yPos = 3; }
			else if (place[randPlace] == 8) { xPos = 2; yPos = 5; }
			else if (place[randPlace] == 9) { xPos = 3; yPos = 0; }
			else if (place[randPlace] == 10) { xPos = 3; yPos = 2; }
			else if (place[randPlace] == 11) { xPos = 3; yPos = 4; }
			else if (place[randPlace] == 12) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 13) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 14) { xPos = 4; yPos = 5; }
			else if (place[randPlace] == 15) { xPos = 5; yPos = 0; }
			else if (place[randPlace] == 16) { xPos = 5; yPos = 2; }
			else if (place[randPlace] == 17) { xPos = 5; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 15) {			// Gallery D
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 0; yPos = 0; }	//	]=X===X[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 5; }	//	]===X==[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 2; }	//	]==X=X=[
			else if (place[randPlace] == 4) { xPos = 2; yPos = 4; }	//	]=X===X[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]X=====[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 8) { xPos = 5; yPos = 1; }
			else if (place[randPlace] == 9) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 16) {			// Layers A
		vector<int> place = { 0,1,2,3,4,5,6,7,8,9,10,11 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 1; yPos = 0; }	//	]=X==X=[
			else if (place[randPlace] == 1) { xPos = 1; yPos = 1; }	//	]=X==X=[
			else if (place[randPlace] == 2) { xPos = 1; yPos = 2; }	//	]=X==X=[
			else if (place[randPlace] == 3) { xPos = 1; yPos = 3; }	//	]=X==X=[
			else if (place[randPlace] == 4) { xPos = 1; yPos = 4; }	//	]=X==X=[
			else if (place[randPlace] == 5) { xPos = 1; yPos = 5; }	//	]=X==X=[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 0; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 1; }
			else if (place[randPlace] == 8) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 9) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 10) { xPos = 4; yPos = 4; }
			else if (place[randPlace] == 11) { xPos = 4; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 17) {			// Layers B
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 1; }	//	]     =[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 2; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 3; }	//	]==X=X=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 4; }	//	]==X=X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 1; }	//	]==X=X=[
			else if (place[randPlace] == 5) { xPos = 4; yPos = 2; }	//	]=     [
			else if (place[randPlace] == 6) { xPos = 4; yPos = 3; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 4; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 18) {			// Layers C
		vector<int> place = { 0,1,2,3,4,5,6 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 4; }	//	]===X=X[
			else if (place[randPlace] == 1) { xPos = 3; yPos = 3; }	//	]==X=X=[
			else if (place[randPlace] == 2) { xPos = 3; yPos = 5; }	//	]===X=X[
			else if (place[randPlace] == 3) { xPos = 4; yPos = 2; }	//	]====X=[
			else if (place[randPlace] == 4) { xPos = 4; yPos = 4; }	//	]======[
			else if (place[randPlace] == 5) { xPos = 5; yPos = 3; }	//	]======[
			else if (place[randPlace] == 6) { xPos = 5; yPos = 5; }
			if (xPos != -1 && yPos != -1) {
				map[xPos][yPos].state = 1;
				remove(place.begin(), place.end(), place[randPlace]);
				amt--; }
	}	}
	else if (type == 19) {			// Layers D
		vector<int> place = { 0,1,2,3,4,5,6,7 };
		while (amt > 0 && !place.empty()) {
			int randPlace = rand() % place.size();
			if      (place[randPlace] == 0) { xPos = 2; yPos = 0; }	//	]==X===[
			else if (place[randPlace] == 1) { xPos = 2; yPos = 1; }	//	]==X===[
			else if (place[randPlace] == 2) { xPos = 2; yPos = 4; }	//	]== XX=[
			else if (place[randPlace] == 3) { xPos = 2; yPos = 5; }	//	]== XX=[
			else if (place[randPlace] == 4) { xPos = 3; yPos = 2; }	//	]==X===[
			else if (place[randPlace] == 5) { xPos = 3; yPos = 3; }	//	]==X===[
			else if (place[randPlace] == 6) { xPos = 4; yPos = 2; }
			else if (place[randPlace] == 7) { xPos = 4; yPos = 3; }
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
	level = 50;
	player.energy = player.energy2 = 20000;
	for (int i = 1; i < 5; i++) {
		for (int j = 1; j < 5; j++) {
			map[i][j].boxHP = 5;
			map[i][j].boxType = 1;
			map2[i][j] = map[i][j]; } }
}
/*
void App::test2() {
	int x = -1;
	while (x < 0 || x > 21) {
		cout << "Level Type: ";
		cin >> x;
	}
	reset();
	clearFloor();
	level = 50;
	player.energy = player.energy2 = 20000;
	generateLevel(x);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			map2[i][j] = map[i][j]; } }
}
*/
