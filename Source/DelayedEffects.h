#pragma once
#include <string>
using namespace std;

struct DelayedDamage {		// Used for displaying a Delayed HP Loss of a Rock or Character
    DelayedDamage();
    DelayedDamage( int dmgAmt, int x, int y, float delayTime, bool isNpc = false );
    int dmg;
    int xPos, yPos;
    float delay;
    bool npc;        // Whether or not the damage was from an NPC
};

struct DelayedSound {
    DelayedSound();
    DelayedSound( string name, float delayTime, bool isNpc = false );
    bool npc;
    string soundName;
    float delay;
};

struct DelayedAttackDisplay {
    DelayedAttackDisplay();
    DelayedAttackDisplay( string atkType, int x, int y, float time, int facingDir = -1 );
    string type;
    int xPos, yPos;
    float delay;
    float animationTimer;
    int dir;
};

struct DelayedEnergyDisplay {
    DelayedEnergyDisplay();
    DelayedEnergyDisplay( int xPos, int yPos, int energyAmt, int typeNum, float time );
    int x, y;
    int amt;
    int type;       // 0 = Attack cost   // 1 = Energy   // 2 = Poison   // 3 = NPC Attack Damage
    float animationTimer;
};
