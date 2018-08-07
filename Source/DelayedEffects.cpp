#include "DelayedEffects.h"

DelayedDamage::DelayedDamage() : dmg( 0 ), xPos( 0 ), yPos( 0 ), delay( 0.0 ), npc( false ) {}
DelayedDamage::DelayedDamage( int dmgAmt, int x, int y, float delayTime, bool isNpc )
  : dmg(dmgAmt),
    xPos(x),
    yPos(y),
    delay(delayTime),
    npc(isNpc)
{}

DelayedSound::DelayedSound() : soundName( "" ), delay( 0.0 ), npc( false ) {}
DelayedSound::DelayedSound( string name, float delayTime, bool isNpc )
  : soundName(name),
    delay(delayTime),
    npc(isNpc)
{}

DelayedAttackDisplay::DelayedAttackDisplay() : xPos( 0 ), yPos( 0 ), delay( 0.0 ), animationTimer( 0.0 ), dir(-1) {}
DelayedAttackDisplay::DelayedAttackDisplay( string atkType, int x, int y, float delayTime, int facingDir )
  : type(atkType),
    xPos(x),
    yPos(y),
    delay(delayTime),
    dir(facingDir)
{
    if     ( type == "eToma" )     animationTimer = 0.16;
    else if( type == "shockwave" ) animationTimer = 0.28;
}

DelayedEnergyDisplay::DelayedEnergyDisplay() : x(0), y(0), amt(0), type(0), animationTimer(0.0) {}
DelayedEnergyDisplay::DelayedEnergyDisplay( int xPos, int yPos, int energyAmt, int typeNum, float time )
  : x(xPos),
    y(yPos),
    amt(energyAmt),
    type(typeNum),
    animationTimer(time)
{}
