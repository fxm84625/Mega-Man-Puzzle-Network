#pragma once

struct Panel {
    Panel();
    int state;		// 0 = Solid Ground		// -1 = Cracked Floor	// -2 = Cracked Hole		// -3 = Hole     // 1 = Ice     // 2 = Poison      // 3 = Holy
    int item;		// 0 = No Energy (Resource)			// 1 = Green Energy       // 2 = Blue Energy       // -1 = Trapped Red Energy
    int rockHP;		// 0 = No Rock
    int rockType;	// 0 = Rock		        // 1 = Ice Rock w/ Item
    float rockAtkInd;	// Used for Indicating that a Rock got Attacked and damaged
    float upgradeInd;   // Used for Indicating that an Item got Upgraded
    bool bigRockDeath;   // Used for special Rock death animation
    bool isPurple;      // Used to determine which color sprite sheet to use
    int prevDmg;

    void reset();
};
