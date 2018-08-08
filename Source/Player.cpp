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

const int gutsPunchCost = 75;
const int gutsHammerCost = 125;
const int gutsDashPunchCost = 125;
const int gutsShockwaveCost = 175;
const int gutsSlamCost = 250;

const float preAtkTime          = 0.10;
const float preAtkTimeToma      = 0.20;
const float preAtkTimeEagleToma = 0.27;
const float preAtkTimeGutsPunch = 0.12;

const float moveAnimationTime   = 0.175;	// Player movement time
const float stepAtkTime         = 0.38;
const float charDeathTime       = 0.2;

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
GutsMan::GutsMan() { type = 5; }

string MegaMan::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "Sword";
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
    default:
    case 1: return swordCost;
    case 2: return discount + longCost;
    case 3: return discount + wideCost;
    case 4: return discount + crossCost;
    case 5: return discount + spinCost;
    case 6: return discount + stepCost;
    case 7: return discount + lifeCost;
    }
}
vector< DelayedDamage > MegaMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    case 6: return attack6();
    case 7: return attack7();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > MegaMan::attack1() {
    // Sword
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 1, x + facing, y, preAtkTime, npc ) );
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack2() {
    // LongSword
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack3() {
    // WideSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack4() {
    // CrossSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedDamage( dmg, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * (i+1);
        int yPos = y - i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack5() {
    // SpinSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            if( i == 0  && j == 0 ) continue;
            int xPos = x + i;
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack6() {
    // StepSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * 3;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > MegaMan::attack7() {
    // LifeSword
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}

string ProtoMan::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "Sword";
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
    default:
    case 1: return swordCost;
    case 2: return discount + longCost;
    case 3: return discount + wideCost2;
    case 4: return discount + stepCost2;
    case 5: return discount + heroCost;
    case 6: return discount + protoCost;
    }
}
vector< DelayedDamage > ProtoMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    case 6: return attack6();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > ProtoMan::attack1() {
    // Sword
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 1, x + facing, y, preAtkTime, npc ) );
    return atkCoords;
}
vector< DelayedDamage > ProtoMan::attack2() {
    // LongSword
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > ProtoMan::attack3() {
    // WideSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > ProtoMan::attack4() {
    // StepSword
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * 3;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > ProtoMan::attack5() {
    // HeroSword
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 3; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > ProtoMan::attack6() {
    // ProtoCross
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 3; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * 2;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}

string TomahawkMan::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "WideSwng";
    case 2: return "WdSwngEX";
    case 3: return "Tomahawk";
    case 4: return "TmhawkEX";
    case 5: return "EaglThwk";
    }
}
int TomahawkMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default:
    case 1: return discount + tomaCostA1;
    case 2: return discount + tomaCostB1;
    case 3: return discount + tomaCostA2;
    case 4: return discount + tomaCostB2;
    case 5: return discount + eTomaCost;
    }
}
vector< DelayedDamage > TomahawkMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > TomahawkMan::attack1() {
    // WideSwing
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTimeToma, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > TomahawkMan::attack2() {
    // WideSwingEX
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTimeToma, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > TomahawkMan::attack3() {
    // TomahawkSwing
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTimeToma, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedDamage > TomahawkMan::attack4() {
    // TomahawkSwingEX
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            int xPos = x + facing * i;
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTimeToma, npc ) );
        }
    }
    return atkCoords;
}
vector< DelayedDamage > TomahawkMan::attack5() {
    // EagleTomahawk
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 5, x + facing, y, preAtkTimeEagleToma - 0.06, npc ) );
    for( int i = 0; i <= 3; i++ ) {
        int xPos = x + facing * (i+2);
        int yPos = y;
        float delay = preAtkTimeEagleToma + 0.04 * (float)i;
        atkCoords.push_back( DelayedDamage( 5, xPos, yPos, delay, npc ) );
    }
    return atkCoords;
}

string Colonel::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "ArcDvide";
    case 2: return "ScreenDv";
    case 3: return "ScreenDv";
    case 4: return "CrossDiv";
    case 5: return "NeoScrDv";
    }
}
int Colonel::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default:
    case 1: return discount + vDivideCost;
    case 2: return discount + upDivideCost;
    case 3: return discount + downDivideCost;
    case 4: return discount + xDivideCost;
    case 5: return discount + zDivideCost;
    }
}
vector< DelayedDamage > Colonel::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > Colonel::attack1() {
    // Arc Divide
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 1, x + facing, y, preAtkTime, npc ) );
    for( int i = -1; i <= 1; i += 2 ) {
        atkCoords.push_back( DelayedDamage( 1, x, y + i, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > Colonel::attack2() {
    // Screen Divide Up
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > Colonel::attack3() {
    // Screen Divide Down
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y - i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > Colonel::attack4() {
    // Step Cross Divide
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y - i;
        atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > Colonel::attack5() {
    // Z-Saber Cross Divide
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 2, x + facing, y, preAtkTime, npc ) );
    for( int i = 0; i <= 2; i++ ) {
        int xPos = x + facing * i;
        atkCoords.push_back( DelayedDamage( 2, xPos, y + 1, preAtkTime, npc ) );
        atkCoords.push_back( DelayedDamage( 2, xPos, y - 1, preAtkTime, npc ) );
    }
    return atkCoords;
}

string SlashMan::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "LongSlsh";
    case 2: return "WideSlsh";
    case 3: return "CrossSls";
    case 4: return "StepCrss";
    case 5: return "SpinSlsh";
    }
}
int SlashMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default:
    case 1: return discount + longCost;
    case 2: return discount + wideCost;
    case 3: return discount + crossCost2;
    case 4: return discount + stepCrossCost;
    case 5: return discount + spinSlashCost;
    }
}
vector< DelayedDamage > SlashMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > SlashMan::attack1() {
    // LongSlash
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * i;
        int yPos = y;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > SlashMan::attack2() {
    // WideSlash
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing;
        int yPos = y + i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > SlashMan::attack3() {
    // CrossSlash
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedDamage( dmg, xPos, yPos, preAtkTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+1 );
        int yPos = y - i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > SlashMan::attack4() {
    // StepCross
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y + i;
        int dmg = ( i == 0 ? 2 : 1 );
        atkCoords.push_back( DelayedDamage( dmg, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    for( int i = -1; i <= 1; i += 2 ) {
        int xPos = x + facing * ( i+3 );
        int yPos = y - i;
        atkCoords.push_back( DelayedDamage( 1, xPos, yPos, preAtkTime + moveAnimationTime, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > SlashMan::attack5() {
    // Tornado Spin Slash
    vector< DelayedDamage > atkCoords;
    for( int i = -1; i <= 1; i++ ) {
        for( int j = -1; j <= 1; j++ ) {
            if( i == 0  && j == 0 ) continue;
            int xPos = x + i;
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 2, xPos, yPos, preAtkTime, npc ) );
        }
    }
    return atkCoords;
}

string GutsMan::getAtkName() {
    switch( actionNumber ) {
    default:
    case 1: return "GutsPnch";
    case 2: return "GutsHmmr";
    case 3: return "DashPnch";
    case 4: return "Shockwav";
    case 5: return "HamrSlam";
    }
}
int GutsMan::getAtkCost( int atkNum ) {
    int discount = ( onHolyPanel ? -25 : 0 );
    switch( atkNum ) {
    default:
    case 1: return discount + gutsPunchCost;
    case 2: return discount + gutsHammerCost;
    case 3: return discount + gutsDashPunchCost;
    case 4: return discount + gutsShockwaveCost;
    case 5: return discount + gutsSlamCost;
    }
}
vector< DelayedDamage > GutsMan::attack( int actionNumber ) {
    switch( actionNumber ) {
    case 1: return attack1();
    case 2: return attack2();
    case 3: return attack3();
    case 4: return attack4();
    case 5: return attack5();
    default: return vector<DelayedDamage>();
    }
}
vector< DelayedDamage > GutsMan::attack1() {
    // Guts Punch
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 2, x + facing, y, preAtkTimeGutsPunch, npc ) );
    return atkCoords;
}
vector< DelayedDamage > GutsMan::attack2() {
    // Guts Dash Punch
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 5, x + facing, y, preAtkTimeToma, npc ) );
    return atkCoords;
}
vector< DelayedDamage > GutsMan::attack3() {
    // Guts Hammer
    vector< DelayedDamage > atkCoords;
    for( int i = 1; i <= 3; i++ ) {
        int xPos = x + facing * i;
        float delay = moveAnimationTime * 0.40 + moveAnimationTime * 0.25 * (i-1);
        atkCoords.push_back( DelayedDamage( 2, xPos, y, delay, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > GutsMan::attack4() {
    // Guts Hammer Shockwave
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 5, x + facing, y, preAtkTimeToma, npc ) );
    for( int i = 1; i <= 4; i++ ) {
        int xPos = x + facing * (i+1);
        float delay = preAtkTimeToma + 0.06 * i;
        atkCoords.push_back( DelayedDamage( 2, xPos, y, delay, npc ) );
    }
    return atkCoords;
}
vector< DelayedDamage > GutsMan::attack5() {
    // Guts Hammer Shockwave Slam
    vector< DelayedDamage > atkCoords;
    atkCoords.push_back( DelayedDamage( 2, x + facing, y-1, preAtkTimeToma, npc ) );
    atkCoords.push_back( DelayedDamage( 5, x + facing, y, preAtkTimeToma, npc ) );
    atkCoords.push_back( DelayedDamage( 2, x + facing, y+1, preAtkTimeToma, npc ) );
    for( int i = 1; i <= 2; i++ ) {
        int xPos = x + facing * (i+1);
        float delay = preAtkTimeToma + 0.06 * i;
        for( int j = -1; j <= 1; j++ ) {
            int yPos = y + j;
            atkCoords.push_back( DelayedDamage( 2, xPos, yPos, delay, npc ) );
        }
    }
    return atkCoords;
}
