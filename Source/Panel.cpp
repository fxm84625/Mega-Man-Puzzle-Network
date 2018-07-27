#include "Panel.h"

Panel::Panel()
  : state( 0 ),
    item( 0 ),
    rockHP( 0 ),
    rockType( 0 ),
    rockAtkInd( 0.0 ),
    upgradeInd( 0.0 ),
    bigRockDeath( false ),
    isPurple( false ),
    prevDmg( 0 )
{}

void Panel::reset() {
    state = 0;
    item = 0;
    rockHP = 0;
    rockType = 0;
    rockAtkInd = 0.0;
    upgradeInd = 0.0;
    bigRockDeath = false;
    isPurple = false;
    prevDmg = 0;
}
