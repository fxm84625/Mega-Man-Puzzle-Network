#include "DelayedEffects.h"

DelayedHpLoss::DelayedHpLoss() : dmg( 0 ), xPos( 0 ), yPos( 0 ), delay( 0.0 ), npc( false ) {}
DelayedHpLoss::DelayedHpLoss( int dmgAmt, int x, int y, float delayTime, bool isNpc )
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

DelayedETomaDisplay::DelayedETomaDisplay() : xPos( 0 ), yPos( 0 ), delay( 0.0 ), animationTimer( 0.1 ) {}
DelayedETomaDisplay::DelayedETomaDisplay( int x, int y, float delayTime )
  : animationTimer( 0.1 ),
    xPos(x),
    yPos(y),
    delay(delayTime)
{}

DelayedEnergyDisplay::DelayedEnergyDisplay() : x(0), y(0), amt(0), type(0), animationTimer(0.0) {}
DelayedEnergyDisplay::DelayedEnergyDisplay( int xPos, int yPos, int energyAmt, int typeNum, float time )
  : x(xPos),
    y(yPos),
    amt(energyAmt),
    type(typeNum),
    animationTimer(time)
{}
