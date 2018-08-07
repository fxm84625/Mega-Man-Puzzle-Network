#pragma once
#include "Panel.h"
#include "DelayedEffects.h"
#include <vector>
using namespace std;

class Player {
public:
    Player();
    int x, y;			// Coordinate position
    int x2, y2;         // Previous Coordinate position
    float xDisplay, yDisplay;   // Drawing Character parameters
    int hp;             // HP for NPCs        // Players do not have HP
    int timesHit;       // Records how many times Players hit an NPC
    int facing;			// Facing Direction		// -1 = Left	// 1 = Right
    int moveType;       // 0 = idle     // 1 = turning      // 2 = moving       // 3 = step-swording
    int moveDir;		// Moving Direction		// 0 = Up		// 1 = Left		// 2 = Down		// 3 = Right
    int energy;			// Player Resource - Used for attacking with swords
    int type;           // 0 = Megaman      // 1 = Protoman     // 2 = Tomahawkman      // 3 = Colonel      // 4 = Slashman
    bool npc;           // Whether or not the character is the player or an NPC
    bool onHolyPanel;   // Whether or not the character is on a Holy Panel

    float animationDisplayAmt;	// Used for Animation timing
    float deathDisplayAmt;      // Used for animating defeated Characters
    float currentSwordAtkTime;	// Used for Sword attack display
    float hurtDisplayAmt;       // Used for showing that Characters got hit
    float npcActionTimer;       // NPCs wait a moment after Players act

    int animationType;		// -1 = Trapped    // 0 = Movement    // 1 = Attack    // 2 = Special Invalid movement    // 3,4 = Level Transition     // 5 = Slipping on Ice
    int actionNumber;		// Select which sword attack is being Animated

    int totalEnergyChange;  // Used to help show a smooth transition of Energy loss / gain
    int lastAtkCost;        // Used to display the previous attack's Energy Cost
    int energyDisplayed, energyDisplayed2;              // Shown amount of energy: the current total and current change for the current Level

    void reset();
    virtual string getAtkName() = 0;
    virtual int getAtkCost( int atkNum ) = 0;
    virtual vector< DelayedDamage > attack( int atkNum ) = 0;
};

class MegaMan : public Player {
public:
    MegaMan();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
    vector< DelayedDamage > attack6();
    vector< DelayedDamage > attack7();
};

class ProtoMan : public Player {
public:
    ProtoMan();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
    vector< DelayedDamage > attack6();
};

class TomahawkMan : public Player {
public:
    TomahawkMan();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
};

class Colonel : public Player {
public:
    Colonel();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
};

class SlashMan : public Player {
public:
    SlashMan();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
};

class GutsMan : public Player {
public:
    GutsMan();
    string getAtkName();
    int getAtkCost( int atkNum );
    vector< DelayedDamage > attack( int atkNum );
private:
    vector< DelayedDamage > attack1();
    vector< DelayedDamage > attack2();
    vector< DelayedDamage > attack3();
    vector< DelayedDamage > attack4();
    vector< DelayedDamage > attack5();
};
