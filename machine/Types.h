/*
  File name : Types.h
  Author : Antony Toron

  Description : Defines common types
 */

enum ROM {INVADERS, BALLOON_BOMBER, GUNFIGHT, SPACE_ENCOUNTERS, SEAWOLF, M4,
          BOWLING};

typedef enum ROM ROM;

enum Control {INSERT_COIN, REGISTER_COIN, P1_START_DOWN, P1_START_UP,
	      P2_START_DOWN, P2_START_UP, P1_SHOT_DOWN, P1_SHOT_UP,
	      P1_LEFT_DOWN, P1_RIGHT_DOWN, P1_LEFT_UP, P1_RIGHT_UP};

typedef enum Control Control;

typedef struct ArcadeMachine * ArcadeMachine_T;

typedef struct DIPSettings * DIPSettings_T;
