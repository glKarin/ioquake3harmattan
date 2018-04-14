#ifndef _KARIN_EGLVKB_H
#define _KARIN_EGLVKB_H

enum
{
	fire_vkb = 0,
	escape_vkb,
	turnup_vkb, turndown_vkb, turnleft_vkb, turnright_vkb, turncenter_vkb,
	forward_vkb, backward_vkb, moveleft_vkb, moveright_vkb,
	jump_vkb, down_vkb,
	weapon1_vkb, weapon2_vkb, weapon3_vkb, weapon4_vkb, weapon5_vkb, weapon6_vkb, weapon7_vkb, weapon8_vkb, weapon9_vkb,
	prevweapon_vkb, nextweapon_vkb,
	useitem_vkb,
	score_vkb,
	walk_vkb,
	zoom_vkb,
	VB_TotalFunction
};

typedef struct{
	int backgroundX;
	int backgroundY;
	int textX;
	int textY;
	const char *text;
}VirtualButton;

//for general
#define VBW 64 //On-screen button width
#define VBS 20 //On-screen button spacing

//for game 854x480
#define N950SW 854 //Harmattan device screen width
#define N950SH 480 //Harmattan device screen height

//w a s d
#define FORWARD_X (VBW + VBS * 2)
#define FORWARD_Y (N950SH - VBW * 3 - VBS * 3 + VBS / 2)
#define BACKWARD_X (VBW + VBS * 2)
#define BACKWARD_Y (N950SH - VBS - VBW + VBS / 2)
#define MOVELEFT_X (VBS)
#define MOVELEFT_Y (N950SH - VBW * 2 - VBS * 2 + VBS / 2)
#define MOVERIGHT_X (VBW * 2 + VBS * 3)
#define MOVERIGHT_Y (N950SH - VBW * 2 - VBS * 2 + VBS / 2)
//home end pageup pagedown center
#define TURNUP_X (N950SW - VBW * 2 - VBS * 2)
#define TURNUP_Y (N950SH - VBW * 3 - VBS * 3 + VBS / 2)
#define TURNDOWN_X (N950SW - VBW * 2 - VBS * 2)
#define TURNDOWN_Y (N950SH - VBS - VBW + VBS / 2)
#define TURNLEFT_X (N950SW - VBW * 3 - VBS * 3)
#define TURNLEFT_Y (N950SH - VBW * 2 - VBS * 2 + VBS / 2)
#define TURNRIGHT_X (N950SW - VBW - VBS)
#define TURNRIGHT_Y (N950SH - VBW * 2 - VBS * 2 + VBS / 2)
#define TURNCENTER_X (N950SW - VBW * 2 - VBS * 2)
#define TURNCENTER_Y (N950SH - VBW * 2 - VBS * 2 + VBS / 2)
//1 - 9
#define WEAPON1_X (N950SW / 2 - VBS * 2 - VBW / 2 * 9)
#define WEAPON2_X (N950SW / 2 - VBS / 2 * 3 - VBW / 2 * 7)
#define WEAPON3_X (N950SW / 2 - VBS - VBW / 2 * 5)
#define WEAPON4_X (N950SW / 2 - VBS / 2 - VBW / 2 * 3)
#define WEAPON5_X (N950SW / 2 - VBW / 2)
#define WEAPON6_X (N950SW / 2 + VBS / 2 + VBW / 2)
#define WEAPON7_X (N950SW / 2 + VBS + VBW / 2 * 3)
#define WEAPON8_X (N950SW / 2 + VBS / 2 * 3 + VBW / 2 * 5)
#define WEAPON9_X (N950SW / 2 + VBS * 2 + VBW / 2 * 7)
#define WEAPON1_Y 0
#define WEAPON2_Y 0
#define WEAPON3_Y 0
#define WEAPON4_Y 0
#define WEAPON5_Y 0
#define WEAPON6_Y 0
#define WEAPON7_Y 0
#define WEAPON8_Y 0
#define WEAPON9_Y 0
//prev next
#define PREVWEAPON_X (N950SW / 2 - VBW / 2 * 3 - VBS * 2)
#define PREVWEAPON_Y (N950SH - VBS * 2 - VBW * 2)
#define NEXTWEAPON_X (N950SW / 2 + VBS * 2)
#define NEXTWEAPON_Y (N950SH - VBS * 2 - VBW * 2)
//jump down
#define DOWN_X (N950SW / 2 - 2 * VBW - VBS)
#define DOWN_Y (N950SH - VBS - VBW)
#define JUMP_X (N950SW / 2 + VBS)
#define JUMP_Y (N950SH - VBS - VBW)
//functional
#define FIRE_X (N950SW - VBW / 2 * 3)
#define FIRE_Y (N950SH - VBW * 4 - VBS * 3)
#define USEITEM_X 0
#define USEITEM_Y (N950SH - VBW * 5 - VBS * 4)
#define ESCAPE_X (N950SW - VBW)
#define ESCAPE_Y 0
#define SCORE_X 0
#define SCORE_Y 0
#define ZOOM_X 0
#define ZOOM_Y (N950SH - VBW * 4 - VBS * 3)
#define WALK_X (N950SW - VBW / 2 * 3)
#define WALK_Y (N950SH - VBW * 5 - VBS * 4)

//for menu 640x480
//up down left right arrow
#define MENUSW 640
#define MENUSH 480

#define TOUP_X (VBW + VBS * 2)
#define TOUP_Y (MENUSH - VBW * 3 - VBS * 3)
#define TODOWN_X (VBW + VBS * 2)
#define TODOWN_Y (MENUSH - VBS - VBW)
#define TOLEFT_X (VBS)
#define TOLEFT_Y (MENUSH - VBW * 2 - VBS * 2)
#define TORIGHT_X (VBW * 2 + VBS * 3)
#define TORIGHT_Y (MENUSH - VBW * 2 - VBS * 2)
//base functional
#define ENTER_X (MENUSW - VBW * 2 - VBS)
#define ENTER_Y (MENUSH - VBW / 2 * 5 - VBS / 2 * 5)
#define CANCEL_X (MENUSW - VBW * 2 - VBS)
#define CANCEL_Y (MENUSH - VBW / 2 * 3 - VBS / 2 * 3)

#endif
