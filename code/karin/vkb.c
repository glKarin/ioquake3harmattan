#include "vkb.h"

#include <string.h>
#include <X11/keysym.h>
#include "../qcommon/q_shared.h"
#include "../client/keycodes.h"
#include "../client/client.h"

#ifdef _HARMATTAN_RESC
#define RESC _HARMATTAN_RESC
#else
#define RESC "resc/"
#endif

#define VB_S(n) (n * VB_SPACING)
#define VB_W(n) (n * VB_WIDTH)
#define VB_H(n) (n * VB_HEIGHT)
#define VB_S2(n) (VB_S(n) / 3 * 2)
#define VB_W2(n) (VB_W(n) / 3 * 2)
#define VB_H2(n) (VB_H(n) / 3 * 2)
#define TEX_W(n) (n * TEX_WIDTH)
#define TEX_H(n) (n * TEX_HEIGHT)

#define VKB_BUTTON_Z_BASE 2000
#define VKB_SWIPE_Z_BASE 1000
#define VKB_JOYSTICK_Z_BASE 3000 
#define VKB_CURSOR_Z_BASE 4000

#define CIRCLECENTERX 182
#define CIRCLECENTERY 308
#define JOYSTICKOFFSET 70
//small circle 139x139 center point (70,70)
//x offset=69 y offset=69
#define BIGCIRCLEX 0
#define BIGCIRCLEY 0
#define SMALLCIRCLEX 294
#define SMALLCIRCLEY 89
#define BIGCIRCLEWIDTH 283
#define SMALLCIRCLEWIDTH 139
#define MYARG 22.5

#define CIRCLEHALFLENGTH 185.0
#define CIRCLEHALFLENGTHWITHOUTLISTENER 25.0

#define TRANSLUCENTLEVEL 157

/*
typedef struct {
	qboolean	down;
	int			repeats;		// if > 1, it is autorepeating
	char		*binding;
} qkey_t;
*/
extern qkey_t keys[MAX_KEYS];
unsigned client_state = 0;
boolean render_lock = bfalse;

// sort by setting in Q2
static const char *Action_Cmds[Total_Action] = {
	"+attack",
	"weapnext",
	"weapprev",
	"+forward",
	"+back",
	"+left",
	"+right",
	"+speed",
	"+moveleft",
	"+moveright",
	"+strafe",
	"+lookup",
	"+lookdown",
	"centerview",
	"+mlook",
	"+zoom",
	"+moveup",
	"+movedown",
	"+button2",
	"+scores",
	"weapon 1",
	"weapon 2",
	"weapon 3",
	"weapon 4",
	"weapon 5",
	"weapon 6",
	"weapon 7",
	"weapon 8",
	"weapon 9",
	"weapon 10",
	"*attack",
	// NULL
};

static int Key_Defs[Total_Key - Up_Key] = {
	K_UPARROW,
	K_DOWNARROW,
	K_LEFTARROW,
	K_RIGHTARROW,
	K_ESCAPE,
	K_CONSOLE,
	K_PGUP,
	K_PGDN,
	K_TAB,
	K_SHIFT,
	K_ENTER,
	XK_BackSpace & 0xFF, // 8
	K_SPACE,
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'g',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'_',
	'-',
	'.',
	'/',
	'+',
	'=',
	'?',
	'\\',
	K_MOUSE1,
};

static const char *Cmd_Strs[Total_Cmd - God_Cmd] = {
	"god", // GOD mode
	"give all",
	"noclip",
	"notarget",
	"give all weapons",
	"give ammo",
	"give armor",
	"give health",
	"kill",
	"quit",
	//"cg_thirdPerson 1"
};

const char *Tex_Files[VKB_TEX_COUNT] = {
	RESC"anna_buttons.png",
	RESC"circle_joystick.png",
	RESC"A-Z_u.png",
	RESC"0-9.png",
	RESC"a-z_l.png"
};

struct vkb_cursor VKB_Cursor[CURSOR_COUNT] = {
	{VB_S(0) + VB_W(6), VB_S(2), VB_W(2) + VB_S(1), 
		0.5, 3.0, btrue, 1, 0.0,
		BIGCIRCLEX, BIGCIRCLEY + BIGCIRCLEWIDTH, BIGCIRCLEWIDTH,
		0.5, SMALLCIRCLEX, SMALLCIRCLEY + SMALLCIRCLEWIDTH, SMALLCIRCLEWIDTH,
		SMALLCIRCLEX, SMALLCIRCLEY + SMALLCIRCLEWIDTH, SMALLCIRCLEWIDTH,
		opengl_e_base, opengl_s_base, btrue, VKB_CURSOR_Z_BASE + 3, VKB_In_Game,
		Total_Action, TurnUp_Action, TurnDown_Action, TurnLeft_Action, TurnRight_Action}, // ???
	{VB_S(7) + VB_W(2), VB_S(5) + VB_W(1), VB_W(1) + VB_S(2),
		1.0, 5.0, bfalse, 0, 0.0,
		0, 0, 0,
		1.0, TEX_W(2), TEX_W(3), TEX_W(1),
		TEX_W(2), TEX_W(4), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_CURSOR_Z_BASE + 2, VKB_In_Game,
		Attack_Action, 
		// TurnUp_Action, TurnDown_Action, TurnLeft_Action, TurnRight_Action
		Mouse1_Button, Mouse1_Button, Mouse1_Button, Mouse1_Button
	},
	{VB_W(2) + VB_S(4), VB_H(2) + VB_S(2), VB_W(1), 
		1.0, 6.0, bfalse, 0, 0.0,
		0, 0, 0,
		1.0, TEX_W(0), TEX_W(3), TEX_W(1),
		TEX_W(0), TEX_W(4), TEX_W(1),
		opengl_e_base, opengl_e_base, btrue, VKB_CURSOR_Z_BASE + 1, VKB_In_Game,
		Zoom_Action,
		Mouse1_Button, Mouse1_Button, Mouse1_Button, Mouse1_Button
	},
};

struct vkb_swipe VKB_Swipe[SWIPE_COUNT] = {
	{HARMATTAN_WIDTH / 4 * 3, 0, HARMATTAN_WIDTH, HARMATTAN_HEIGHT,
		HARMATTAN_WIDTH / 4 * 3, 0, HARMATTAN_WIDTH, HARMATTAN_HEIGHT, bfalse,
		BIGCIRCLEX, BIGCIRCLEY + BIGCIRCLEWIDTH, BIGCIRCLEWIDTH, BIGCIRCLEWIDTH,
		opengl_e_base, opengl_s_base, btrue, VKB_SWIPE_Z_BASE + 1, VKB_In_Game, 
		//TurnUp_Action, TurnDown_Action, TurnLeft_Action, TurnRight_Action
		MouseLook_Action,
		Mouse1_Button, Mouse1_Button, Mouse1_Button, Mouse1_Button
		}
};

struct vkb_joystick VKB_Joystick[JOYSTICK_COUNT] = {
	{VB_S(1), VB_S(1), VB_W(3) + VB_S(2), 
		0.0f, 1.8f,
		BIGCIRCLEX, BIGCIRCLEY + BIGCIRCLEWIDTH, BIGCIRCLEWIDTH,
		0.5, SMALLCIRCLEX, SMALLCIRCLEY + SMALLCIRCLEWIDTH, SMALLCIRCLEWIDTH,
		opengl_s_base, opengl_s_base, btrue, VKB_JOYSTICK_Z_BASE + 1, VKB_In_Game, 
		Forward_Action, Backward_Action, MoveLeft_Action, MoveRight_Action},
	{VB_S(1), VB_S(1), VB_W(3) + VB_S(2), 
		0.0f, 1.2f,
		BIGCIRCLEX, BIGCIRCLEY + BIGCIRCLEWIDTH, BIGCIRCLEWIDTH,
		0.5, SMALLCIRCLEX, SMALLCIRCLEY + SMALLCIRCLEWIDTH, SMALLCIRCLEWIDTH,
		opengl_s_base, opengl_s_base, btrue, VKB_CURSOR_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 
		Up_Key, Down_Key, Left_Key, Right_Key},
	{VB_S(0), VB_W(1) + VB_S(1) / 2, VB_W(2) + VB_S(1), //107 
		0.0f, 1.0f,
		BIGCIRCLEX, BIGCIRCLEY + BIGCIRCLEWIDTH, BIGCIRCLEWIDTH,
		0.5, SMALLCIRCLEX, SMALLCIRCLEY + SMALLCIRCLEWIDTH, SMALLCIRCLEWIDTH,
		opengl_s_base, opengl_mb_base, btrue, VKB_CURSOR_Z_BASE + 100, VKB_In_Menu, 
		Up_Key, Down_Key, Left_Key, Right_Key}
};

struct vkb_button VKB_Button[VKB_COUNT] = {
	// game
	{VB_S(1) + VB_W(1), VB_S(3) + VB_H(2), VB_W(1), VB_H(1), 
		VB_S(1) + VB_W(1), VB_S(3) + VB_H(2), VB_W(1), VB_H(1),  
		TEX_W(4), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Jump_Action},
	{VB_S(1) + VB_W(1), VB_S(2) + VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) + VB_W(1), VB_S(2) + VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(4), TEX_W(2), TEX_W(1), -TEX_W(1),
		TEX_W(4), TEX_W(3), TEX_W(1), -TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Crouch_Action},
	{VB_S(1) + VB_W(1), VB_S(1), VB_W(1), VB_H(1), 
		VB_S(1) + VB_W(1), VB_S(1), VB_W(1), VB_H(1),  
		TEX_W(2), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Run_Action},
	{VB_S(-1) + VB_W(2), VB_S(0), VB_W(1), VB_H(1), 
		VB_S(-1) + VB_W(2), VB_S(0), VB_W(1), VB_H(1),  
		TEX_W(2), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		PrevWeapon_Action},
	{VB_S(-2) + VB_W(1), VB_S(0), VB_W(1), VB_H(1), 
		VB_S(-2) + VB_W(1), VB_S(0), VB_W(1), VB_H(1),  
		TEX_W(4), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		NextWeapon_Action},
	{VB_W(1) + VB_S(1), VB_H(2) + VB_S(2), VB_W(1), VB_H(1), 
		VB_W(1) + VB_S(1), VB_H(2) + VB_S(2), VB_W(1), VB_H(1),  
		TEX_W(3), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Score_Action},
	{VB_W(2) + VB_S(4), VB_S(2) + VB_W(2), VB_W(1), VB_H(1), 
		VB_W(2) + VB_S(4), VB_S(2) + VB_W(2), VB_W(1), VB_H(1),  
		TEX_W(3), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_s_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		MouseLook_Action},
	{VB_S(3) + VB_W(3), VB_S(1), VB_W(1), VB_H(1), 
		VB_S(3) + VB_W(3), VB_S(1), VB_W(1), VB_H(1),  
		TEX_W(5), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		UseItem_Action},
	{VB_S(2) + VB_W(2), VB_S(1), VB_W(1), VB_H(1), 
		VB_S(2) + VB_W(2), VB_S(1), VB_W(1), VB_H(1),  
		TEX_W(0), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		ViewCenter_Action},
	{VB_W(1) + VB_S(3), VB_S(2) + VB_W(2), VB_W(1), VB_H(1), 
		VB_W(1) + VB_S(3), VB_S(2) + VB_W(2), VB_W(1), VB_H(1),  
		TEX_W(4), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_s_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		SideStep_Action},

	{VB_S(1) / 4 * 9 + VB_W(5), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 9 + VB_W(5), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(6), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon1_Action},
	{VB_S(1) / 4 * 7 + VB_W(4), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 7 + VB_W(4), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(7), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon2_Action},
	{VB_S(1) / 4 * 5 + VB_W(3), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 5 + VB_W(3), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(7), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon3_Action},
	{VB_S(1) / 4 * 3 + VB_W(2), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 3 + VB_W(2), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(3), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon4_Action},
	{VB_S(1) / 4 + VB_W(1), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 + VB_W(1), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(1), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mb_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon5_Action},
	{VB_S(1) / 4, VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4, VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(2), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mf_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon6_Action},
	{VB_S(1) / 4 * 3 + VB_W(1), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 3 + VB_W(1), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(3), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mf_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon7_Action},
	{VB_S(1) / 4 * 5 + VB_W(2), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 5 + VB_W(2), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(0), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_mf_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon8_Action},
	{VB_S(1) / 4 * 7 + VB_W(3), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 7 + VB_W(3), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(1), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_mf_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon9_Action},
	{VB_S(1) / 4 * 9 + VB_W(4), VB_H(1), VB_W(1), VB_H(1), 
		VB_S(1) / 4 * 9 + VB_W(4), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(6), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_mf_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		Weapon10_Action},
	{VB_W(1) + VB_S(1), VB_H(3) + VB_S(3), VB_W(1), VB_H(1), 
		VB_W(1) + VB_S(1), VB_H(3) + VB_S(3), VB_W(1), VB_H(1),  
		TEX_W(2), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 1, VKB_In_Game, 0,
		AutoAimAttack_Action},

	// menu
	{VB_S(1) + VB_W(1), VB_S(1) / 2, VB_W(1), VB_H(1),
		VB_S(1) + VB_W(1), VB_S(1) / 2, VB_W(1), VB_H(1),
		TEX_W(2), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_mf_base, btrue, VKB_CURSOR_Z_BASE + 100, VKB_In_Menu, 0,
		Enter_Key},
	{VB_S(1) + VB_W(1), VB_H(1) + VB_S(1) / 2, VB_W(1), VB_H(1),
		VB_S(1) + VB_W(1), VB_H(1) + VB_S(1) / 2, VB_W(1), VB_H(1),
		TEX_W(3), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_mb_base, btrue, VKB_CURSOR_Z_BASE + 100, VKB_In_Menu, 0,
		Space_Key}, // space key for stop resfrsh on multiplayer server menu.
	
	// general
	{0, VB_H(1), VB_W(1), VB_H(1), 
		0, VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(7), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_s_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Menu | VKB_In_Console | VKB_In_Game, 0,
		Console_Key},
	{VB_W(1), VB_H(1), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(1), VB_W(1), VB_H(1),  
		TEX_W(1), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 500, VKB_In_Menu | VKB_In_Console | VKB_In_Game | VKB_In_Loading, 0,
		Escape_Key},

	// second
	{VB_S(2), VB_S(2) + VB_H(2), VB_W(1), VB_H(1), 
		VB_S(2), VB_S(2) + VB_H(2), VB_W(1), VB_H(1),  
		TEX_W(2), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_s_base, opengl_e_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Game, 0,
		Attack_Action},

	// 1 - 9 - 0
	{VB_W2(1) + VB_S2(1) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(1) + VB_S2(1) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num0_Key},
	{VB_W2(2) + VB_S2(2) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(2) + VB_S2(2) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num9_Key},
	{VB_W2(3) + VB_S2(3) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(3) + VB_S2(3) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(7), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num8_Key},
	{VB_W2(4) + VB_S2(4) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(4) + VB_S2(4) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(6), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num7_Key},
	{VB_W2(5) + VB_S2(5) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(5) + VB_S2(5) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(5), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num6_Key},
	{VB_W2(6) + VB_S2(6) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(6) + VB_S2(6) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(4), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num5_Key},
	{VB_W2(7) + VB_S2(7) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(7) + VB_S2(7) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num4_Key},
	{VB_W2(8) + VB_S2(8) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(8) + VB_S2(8) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num3_Key},
	{VB_W2(9) + VB_S2(9) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(9) + VB_S2(9) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num2_Key},
	{VB_W2(10) + VB_S2(10) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1), 
		VB_W2(10) + VB_S2(10) + VB_W(1), VB_H2(4) + VB_S2(4), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Num1_Key},

		// p - q
	{VB_W2(1) + VB_S2(1) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(1) + VB_S2(1) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		P_Key},
	{VB_W2(2) + VB_S2(2) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(2) + VB_S2(2) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		O_Key},
	{VB_W2(3) + VB_S2(3) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(3) + VB_S2(3) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(7), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		I_Key},
	{VB_W2(4) + VB_S2(4) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(4) + VB_S2(4) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(6), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		U_Key},
	{VB_W2(5) + VB_S2(5) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(5) + VB_S2(5) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(5), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Y_Key},
	{VB_W2(6) + VB_S2(6) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(6) + VB_S2(6) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(4), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		T_Key},
	{VB_W2(7) + VB_S2(7) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(7) + VB_S2(7) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		R_Key},
	{VB_W2(8) + VB_S2(8) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(8) + VB_S2(8) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		E_Key},
	{VB_W2(9) + VB_S2(9) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(9) + VB_S2(9) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		W_Key},
	{VB_W2(10) + VB_S2(10) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1), 
		VB_W2(10) + VB_S2(10) + VB_W(1), VB_H2(3) + VB_S2(3), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Q_Key},

	// l - a
	{VB_W2(1) + VB_S2(1) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(1) + VB_S2(1) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		L_Key},
	{VB_W2(2) + VB_S2(2) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(2) + VB_S2(2) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		K_Key},
	{VB_W2(3) + VB_S2(3) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(3) + VB_S2(3) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		J_Key},
	{VB_W2(4) + VB_S2(4) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(4) + VB_S2(4) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(7), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		H_Key},
	{VB_W2(5) + VB_S2(5) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(5) + VB_S2(5) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(6), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		G_Key},
	{VB_W2(6) + VB_S2(6) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(6) + VB_S2(6) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(5), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		F_Key},
	{VB_W2(7) + VB_S2(7) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(7) + VB_S2(7) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(4), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		D_Key},
	{VB_W2(8) + VB_S2(8) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(8) + VB_S2(8) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		S_Key},
	{VB_W2(9) + VB_S2(9) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1), 
		VB_W2(9) + VB_S2(9) + VB_W(1) + VB_W2(1) / 2 + VB_S2(1) / 2, VB_H2(2) + VB_S2(2), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		A_Key},

	// m - z
	{VB_W2(1) + VB_S2(1) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(1) + VB_S2(1) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(1), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(1), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		M_Key},
	{VB_W2(2) + VB_S2(2) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(2) + VB_S2(2) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		N_Key},
	{VB_W2(3) + VB_S2(3) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(3) + VB_S2(3) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(7), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		B_Key},
	{VB_W2(4) + VB_S2(4) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(4) + VB_S2(4) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(6), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		V_Key},
	{VB_W2(5) + VB_S2(5) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(5) + VB_S2(5) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(5), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		C_Key},
	{VB_W2(6) + VB_S2(6) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(6) + VB_S2(6) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(4), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		X_Key},
	{VB_W2(7) + VB_S2(7) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1), 
		VB_W2(7) + VB_S2(7) + VB_W(1) + VB_W2(3) / 2 + VB_S2(3) / 2, VB_H2(1) + VB_S2(1), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Z_Key},
	
	// _ - . / + = ! ?
	{VB_W2(1) + VB_S2(1) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(1) + VB_S2(1) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(0), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(0), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Question_Key},
	{VB_W2(2) + VB_S2(2) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(2) + VB_S2(2) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(6), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Period_Key},
	{VB_W2(3) + VB_S2(3) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(3) + VB_S2(3) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		BackSlash_Key},
	{VB_W2(4) + VB_S2(4) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(4) + VB_S2(4) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Slash_Key},

	{VB_W2(6) + VB_S2(6) + VB_W(1), VB_S2(0), VB_W2(2) + VB_S2(1), VB_H2(1), 
		VB_W2(6) + VB_S2(6) + VB_W(1), VB_S2(0), VB_W2(2) + VB_S2(1), VB_H2(1),  
		TEX_W(6), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(6), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 4,
		Space_Key},

	{VB_W2(7) + VB_S2(7) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(7) + VB_S2(7) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Underscore_Key},
	{VB_W2(8) + VB_S2(8) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(8) + VB_S2(8) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(2), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(2), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 2,
		Minus_Key},
	{VB_W2(9) + VB_S2(9) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(9) + VB_S2(9) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(4), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Equal_Key},
	{VB_W2(10) + VB_S2(10) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1), 
		VB_W2(10) + VB_S2(10) + VB_W(1), VB_S2(0), VB_W2(1), VB_H2(1),  
		TEX_W(3), TEX_W(3), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(4), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 3,
		Plus_Key},


	// special
	{VB_W(1), VB_H(4) + VB_S(5), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(4) + VB_S(5), VB_W(1), VB_H(1),  
		TEX_W(3), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(3), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 0,
		PageUp_Key},
	{VB_W(1), VB_H(3) + VB_S(4), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(3) + VB_S(4), VB_W(1), VB_H(1),  
		TEX_W(5), TEX_W(1), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(2), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 0,
		PageDown_Key},
	
	{VB_W(1), VB_H(2) + VB_S(3), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(2) + VB_S(3), VB_W(1), VB_H(1),  
		TEX_W(5), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(5), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 4,
		Backspace_Key},

	{VB_W(1), VB_H(1) + VB_S(2), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(1) + VB_S(2), VB_W(1), VB_H(1),  
		TEX_W(6), TEX_W(6), TEX_W(1), -TEX_W(1),
		TEX_W(6), TEX_W(7), TEX_W(1), -TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 4,
		Tab_Key},
	{VB_W(1), VB_H(0) + VB_S(1), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(0) + VB_S(1), VB_W(1), VB_H(1),  
		TEX_W(7), TEX_W(7), TEX_W(1), TEX_W(1),
		TEX_W(7), TEX_W(8), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 4,
		Enter_Key},
	/*
	{VB_W(1), VB_H(5) + VB_S(6), VB_W(1), VB_H(1), 
		VB_W(1), VB_H(5) + VB_S(6), VB_W(1), VB_H(1),  
		TEX_W(4), TEX_W(5), TEX_W(1), TEX_W(1),
		TEX_W(4), TEX_W(6), TEX_W(1), TEX_W(1),
		opengl_e_base, opengl_s_base, btrue, VKB_BUTTON_Z_BASE + 200, VKB_In_Console | VKB_In_Message, 0,
		Tab_Key},
		*/
};

int karinGetActionData(unsigned action, int *ikeys, unsigned int key_max, unsigned int *key_count, char *cmd, unsigned int cmd_max)
{
	if(!ikeys || key_max == 0 || !key_count || !cmd || cmd_max == 0)
		return -1; // data is null
	if(action >= Total_Cmd)
		return -2; // action is invalid

	int r = Invalid_Data;
	const char *command = NULL;
	int l = 0;
	int j;

	if(action < Total_Action)
	{
		command = Action_Cmds[action];
		if(command)
		{
			l = strlen(command);
			if(cmd_max >= l + 1)
			{
				strncpy(cmd, command, l);
				cmd[l] = '\0';
				r = Cmd_Data;
			}

			// src/client/menu.c M_FindKeysForCommand
			const char *b = NULL;
			for (j=0 ; j<256 ; j++)
			{
				b = keys[j].binding;
				if (!b)
					continue;
				if ((command[0] != '*' && !strncmp (b, command, l))
						|| (command[0] == '*' && !strncmp (b + 1, command + 1, l - 1)))
				{
					//printf("%d %s %d\n", action, command, j);
					ikeys[0] = j;
					*key_count = 1;
					if(r == Invalid_Data)
						r = Key_Data;
					return r;
				}
			}
			if(r == Invalid_Data)
				r = Key_Data;
			ikeys[0] = K_LAST_KEY;
			*key_count = 1;
			return r;
		}
	}
	else if(action > Total_Action && action < Total_Key)
	{
		ikeys[0] = Key_Defs[action - Up_Key];
		*key_count = 1;
		r = (action >= Mouse1_Button ? Button_Data : (action <= Enter_Key ? Key_Data : Char_Data));
	}
	else if(action > Total_Key && action < Total_Cmd)
	{
		command = Cmd_Strs[action - God_Cmd];
		if(command)
		{
			l = strlen(command);
			if(l > key_max)
				l = key_max - 1;
			for(j = 0; j < l; j++)
			{
				ikeys[j] = command[j];
			}
			ikeys[j] = K_ENTER;
			*key_count = j + 1;
			r = Char_Data;
		}
		else
		{
			*key_count = 0;
		}
	}
	return r;
}

float karinFormatAngle(float angle)
{
	int i = (int)angle;
	float f = angle - i;
	float r = 0.0;
	if(angle > 360)
	{
		r = i % 360 + f;
	}
	else if(angle < 0)
	{
		r = 360 - abs(i % 360 + f);
	}
	else
		r = angle;
	if(r == 360.0)
		r = 0.0;
	return r;
}

unsigned karinGetClientState(void)
{
	unsigned loading = !!(cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE);
	if(loading)
		client_state = VKB_In_Loading;
	else
	{
		unsigned state = Key_GetCatcher();
		if(state == 0)
			state = KEYCATCH_CGAME;
		client_state = state;
	}
	return client_state;
}

