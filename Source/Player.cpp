#include "Player.h"

const int swordCost = 50;
const int longCost = 75;
const int wideCost = 100;
const int wideCost2 = 75;
const int crossCost = 125;
const int spinCost = 150;
const int stepCost = 175;
const int stepCost2 = 150;
const int lifeCost = 250;

const int heroCost = 150;
const int protoCost = 225;

const int vDivideCost = 75;
const int upDivideCost = 125;
const int downDivideCost = 125;
const int xDivideCost = 175;
const int zDivideCost = 250;

const int tomaCostA1 = 75;
const int tomaCostB1 = 125;
const int tomaCostA2 = 125;
const int tomaCostB2 = 175;
const int eTomaCost = 250;

const int crossCost2 = 100;
const int stepCrossCost = 175;
const int spinSlashCost = 225;

const float preAtkTime          = 0.10;
const float preAtkTimeToma      = 0.20;
const float preAtkTimeEagleToma = 0.27;

const float moveAnimationTime = 0.175;	// Player movement time
const float charDeathTime       = 0.2;
const float hurtAnimationTime   = 0.5;

Player::Player()
  : x(0), y(0),
    x2(0), y2(0),
    xDisplay(0.0), yDisplay(0.0),
    hp(1),
    timesHit( 0 ),
    facing(1),
    moveType(0),
    moveDir(-1),
    energy(0),
    type(0),
    npc(true),
    onHolyPanel(false),

    animationDisplayAmt(0.0),
    deathDisplayAmt(charDeathTime),
    currentSwordAtkTime(0.0),
    hurtDisplayAmt(0.0),
    npcActionTimer(0.0),

    animationType(-2),
    actionNumber(-1),

    totalEnergyChange(0),
    lastAtkCost(0),
    energyDisplayed(0),
    energyDisplayed2(0)
{}

void Player::reset() {
    x = 0;		        y = 0;
    x2 = 0;             y2 = 0;
    xDisplay = 0.0;     yDisplay = 0.0;
    hp = 1;
    timesHit = 0;
    facing = 1;
    moveType = 0;
    moveDir = -1;
    energy = 0;
    onHolyPanel = false;

    animationDisplayAmt = 0.0;
    deathDisplayAmt = charDeathTime;
    currentSwordAtkTime = 0;
    hurtDisplayAmt = 0;
    npcActionTimer = 0.0;

    animationType = -2;
    actionNumber = -1;

    totalEnergyChange = 0;
    lastAtkCost = 0;
    energyDisplayed = 0;
    energyDisplayed2 = 0;
}

MegaMan::MegaMan() { type = 0; }
ProtoMan::ProtoMan() { type = 1; }
TomahawkMan::TomahawkMan() { type = 2; }
Colonel::Colonel() { type = 3; }
SlashMan::SlashMan() { type = 4; }

string MegaMan::getAtkName() {
    switch( actionNumber ) {
    default: case 1: return "Sword";
    case 2: return "LongSwrd";
    case 3: return "WideSwrd";
    case 4: return "CrossSwd";
    case 5: return "SpinSwrd";
    case 6: return "StepSwrd";
    case 7: return "LifeSwrd";
    }
}
int MegaMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default: case 1: return swordCost;
    case 2: return discount + longCost;
    case 3: return discount + wideCost;
    case 4: return discount + crossCost;
    case 5: return discount + spinCost;
    case 6: return discount + stepCost;
    case 7: return discount + lifeCost;
    }
}
vector< DelayedHpLoss > MegaMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    case 6: return attack6();
    case 7: return attack7();
    default: return vector<DelayedHpLoss>();
    }
}
vector< DelayedHpLoss > MegaMan::attack1() {
    // Sword
    vector< DelayedHpLoss > atkCoords;
    atkCoords.push_back( DelayedHpLoss( 1, x + facing, y, preAtkTime, npc ) );
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack2() {
    // LongSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack3() {
    // WideSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack4() {
    // CrossSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedHpLoss( dmg, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * (i+1);
        int yPos = y - i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack5() {
    // SpinSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            if( i == 0  && j == 0 ) continue;
            int xPos = x + i;
            int yPos = y + j;
            atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack6() {
    // StepSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * 3;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > MegaMan::attack7() {
    // LifeSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}

string ProtoMan::getAtkName() {
    switch( actionNumber ) {
    default: case 1: return "Sword";
    case 2: return "LongSwrd";
    case 3: return "WideSwrd";
    case 4: return "StepSwrd";
    case 5: return "HeroSwrd";
    case 6: return "ProtoCrs";
    }
}
int ProtoMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default: case 1: return swordCost;
    case 2: return discount + longCost;
    case 3: return discount + wideCost2;
    case 4: return discount + stepCost2;
    case 5: return discount + heroCost;
    case 6: return discount + protoCost;
    }
}
vector< DelayedHpLoss > ProtoMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    case 6: return attack6();
    default: return vector<DelayedHpLoss>();
    }
}
vector< DelayedHpLoss > ProtoMan::attack1() {
    // Sword
    vector< DelayedHpLoss > atkCoords;
    atkCoords.push_back( DelayedHpLoss( 1, x + facing, y, preAtkTime, npc ) );
    return atkCoords;
}
vector< DelayedHpLoss > ProtoMan::attack2() {
    // LongSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > ProtoMan::attack3() {
    // WideSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > ProtoMan::attack4() {
    // StepSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * 3;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > ProtoMan::attack5() {
    // HeroSword
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 3; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > ProtoMan::attack6() {
    // ProtoCross
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 3; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * 2;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}

string TomahawkMan::getAtkName() {
    switch( actionNumber ) {
    default: case 1: return "WideSwng";
    case 2: return "WdSwngEX";
    case 3: return "Tomahawk";
    case 4: return "TmhawkEX";
    case 5: return "EaglThwk";
    }
}
int TomahawkMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default: case 1: return tomaCostA1;
    case 2: return discount + tomaCostB1;
    case 3: return discount + tomaCostA2;
    case 4: return discount + tomaCostB2;
    case 5: return discount + eTomaCost;
    }
}
vector< DelayedHpLoss > TomahawkMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedHpLoss>();
    }
}
vector< DelayedHpLoss > TomahawkMan::attack1() {
    // WideSwing
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTimeToma, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > TomahawkMan::attack2() {
    // WideSwingEX
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTimeToma, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > TomahawkMan::attack3() {
    // TomahawkSwing
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTimeToma, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedHpLoss > TomahawkMan::attack4() {
    // TomahawkSwingEX
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTimeToma, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedHpLoss > TomahawkMan::attack5() {
    // EagleTomahawk
    vector< DelayedHpLoss > atkCoords;
    atkCoords.push_back( DelayedHpLoss( 5, x + facing, y, preAtkTimeEagleToma - 0.06, npc ) );
    for( int i = 2; i <= 5; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        float delay = preAtkTimeEagleToma - 0.08 + 0.04 * (float)i;
        atkCoords.push_back( DelayedHpLoss( 5, xPos, yPos, delay, npc ) );
    }
    return atkCoords;
}

string Colonel::getAtkName() {
    switch( actionNumber ) {
    default: case 1: return "ArcDvide";
    case 2: return "ScreenDv";
    case 3: return "ScreenDv";
    case 4: return "CrossDiv";
    case 5: return "NeoScrDv";
    }
}
int Colonel::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default: case 1: return vDivideCost;
    case 2: return discount + upDivideCost;
    case 3: return discount + downDivideCost;
    case 4: return discount + xDivideCost;
    case 5: return discount + zDivideCost;
    }
}
vector< DelayedHpLoss > Colonel::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedHpLoss>();
    }
}
vector< DelayedHpLoss > Colonel::attack1() {
    // Arc Divide
    vector< DelayedHpLoss > atkCoords;
    atkCoords.push_back( DelayedHpLoss( 1, x + facing, y, preAtkTime, npc ) );
    for( int i = -1; i <= 1; i += 2 ) {
        atkCoords.push_back( DelayedHpLoss( 1, x, y + i, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > Colonel::attack2() {
    // Screen Divide Up
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > Colonel::attack3() {
    // Screen Divide Down
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y - i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > Colonel::attack4() {
    // Step Cross Divide
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y - i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > Colonel::attack5() {
    // Z-Saber Cross Divide
    vector< DelayedHpLoss > atkCoords;
    atkCoords.push_back( DelayedHpLoss( 2, x + facing, y, preAtkTime, npc ) );
    for( int i = 0; i <= 2; i++ ) {
        int xPos = x + facing * i;
        atkCoords.push_back( DelayedHpLoss( 2, xPos, y + 1, preAtkTime, npc ) );
        atkCoords.push_back( DelayedHpLoss( 2, xPos, y - 1, preAtkTime, npc ) );
    }
    return atkCoords;
}

string SlashMan::getAtkName() {
    switch( actionNumber ) {
    default: case 1: return "LongSlsh";
    case 2: return "WideSlsh";
    case 3: return "CrossSls";
    case 4: return "StepCrss";
    case 5: return "SpinSlsh";
    }
}
int SlashMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default: case 1: return longCost;
    case 2: return discount + wideCost;
    case 3: return discount + crossCost2;
    case 4: return discount + stepCrossCost;
    case 5: return discount + spinSlashCost;
    }
}
vector< DelayedHpLoss > SlashMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedHpLoss>();
    }
}
vector< DelayedHpLoss > SlashMan::attack1() {
    // LongSlash
    vector< DelayedHpLoss > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > SlashMan::attack2() {
    // WideSlash
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > SlashMan::attack3() {
    // CrossSlash
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedHpLoss( dmg, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y - i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > SlashMan::attack4() {
    // StepCross
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedHpLoss( dmg, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y - i;
        atkCoords.push_back( DelayedHpLoss( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedHpLoss > SlashMan::attack5() {
    // Tornado Spin Slash
    vector< DelayedHpLoss > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            if( i == 0  && j == 0 ) continue;
            int xPos = x + i;
            int yPos = y + j;
            atkCoords.push_back( DelayedHpLoss( 2, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}
