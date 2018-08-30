#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GLES/egl.h>
#include <GLES/gl.h>

#include "egl_glimp.h"
#include "../client/client.h"
#include "../renderer/tr_local.h"
#include "../qcommon/q_shared.h"

#ifdef _HARMATTAN_PLUS
#include "../karin/m_xi2.h"
#include "../karin/gl_vkb.h"
#endif

#ifdef _HARMATTAN_UNUSED
#define ACCEL_PATH	"/sys/class/i2c-adapter/i2c-3/3-001d/"
#define PROXY_PATH	"/sys/devices/platform/gpio-switch/proximity/"
#endif

#ifdef _HARMATTAN_PLUS
#define FIXED_X_COORD(x) (x + 107)
#endif

#ifdef _HARMATTAN_UNUSED
static int mwx, mwy;
#endif
static int mx = 0, my = 0;
static qboolean mouse_active = qfalse;
static qboolean mouse_avail = qfalse;

cvar_t *in_nograb;
static cvar_t *in_mouse;

#ifdef _HARMATTAN_UNUSED
static cvar_t *accel_angle;
static cvar_t *accel_debug;
static cvar_t *accel_epsilon_x;
/* Cache angle and its sin/cos */
static int accel_angle_value;
static float accel_sin, accel_cos;

static cvar_t *accel_epsilon_y;
static cvar_t *accel_fudge;
static cvar_t *accel_jump;
#endif

#ifdef _HARMATTAN_UNUSED
static int mouse_accel_denominator;
static int mouse_accel_numerator;
static int mouse_threshold;
#endif

static int win_x, win_y;

#ifdef _HARMATTAN_3
//#define _FLOAT_TO_INT round
//#define _FLOAT_TO_INT floor
#define _FLOAT_TO_INT
//#define HARM_SWIPE_SENS(x) ((x) / 2)

static qboolean motionPressed = qtrue;

static int karinSwipeSens(int x)
{
	float sens = harm_swipeSens->value;
	if(sens <= 0.0)
		sens = 0.5;
	return (int)_FLOAT_TO_INT((float)x * sens);
}

static unsigned karinHandleVKBAction(int action, unsigned pressed, int dx, int dy)
{
#define		MAXCMDLINE	256
#define		MAXCMDLENGTH 1024
	static int _keys[MAXCMDLINE];
	static char _cmd[MAXCMDLENGTH];
	unsigned int key_count = 0;
	unsigned int t = Sys_XTimeToSysTime(myxitime);

	int r = karinGetActionData(action, _keys, MAXCMDLINE, &key_count, _cmd, MAXCMDLENGTH);
	if(r == Cmd_Data)
	{
		char cmd[MAXCMDLENGTH];
		if(pressed)
		{
			if (_cmd[0] == '+')
			{	// button commands add keynum and time as a parm
				Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", _cmd, _keys[0], t);
				Cbuf_AddText(cmd);
			}
			else if(_cmd[0] == '*')
			{
				if(Key_GetCatcher() == 0 && strncmp(_cmd + 1, "attack", 6) == 0)
				{
					_cmd[0] = '+';
					Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", _cmd, _keys[0], t);
					motionPressed = qfalse;
					CL_AutoAim();
					Cbuf_AddText(cmd);
				}
			}
			else
			{
				Cbuf_AddText(_cmd);
				Cbuf_AddText("\n");
			}
		}
		else
		{
			if (_cmd[0] == '+')
			{
				Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", _cmd+1, _keys[0], t);
				Cbuf_AddText(cmd);
				if(Key_GetCatcher() == 0 && strncmp(_cmd, "+attack", 7) == 0)
				{
					motionPressed = qtrue;
				}
			}
			else if(_cmd[0] == '*')
			{
				if(Key_GetCatcher() == 0 && strncmp(_cmd + 1, "attack", 6) == 0)
				{
					_cmd[0] = '-';
					motionPressed = qtrue;
					Com_sprintf (cmd, sizeof(cmd), "%s %i %i\n", _cmd, _keys[0], t);
					Cbuf_AddText(cmd);
				}
			}
		}
		return 1;
	}
	else if(r == Key_Data)
	{
		int i = 0;
		for(i = 0; i < key_count; i++)
		{
			Com_QueueEvent(t, SE_KEY, _keys[i], pressed, 0, NULL);
		}
		return 1;
	}
	else if(r == Char_Data)
	{
		int i = 0;
		for(i = 0; i < key_count; i++)
		{
			if(_keys[i] == K_ENTER || _keys[i] == K_KP_ENTER)
				Com_QueueEvent(t, SE_KEY, _keys[i], pressed, 0, NULL);
			else
			{
				if(_keys[i] == K_SPACE && Key_GetCatcher() & KEYCATCH_UI)
					Com_QueueEvent(t, SE_KEY, _keys[i], pressed, 0, NULL);
				if(pressed/* && isprint((char)_keys[i])*/)
					Com_QueueEvent(t, SE_CHAR, _keys[i], 0, 0, NULL);
			}
		}
		return 1;
	}
	else if(r == Button_Data)
	{
		if(pressed)
		{
			int fdx = karinSwipeSens(dx);
			int fdy = karinSwipeSens(-dy);
			if(fdx != 0 || fdy != 0)
				Com_QueueEvent(t, SE_MOUSE, fdx, fdy, 0, NULL);
			/*
				 mx += dx;
				 my -= dy;
				 mouse_buttonstate = 0;
				 */
		}
	}

	return 0;
#undef MAXCMDLINE
#undef MAXCMDLENGTH
}

void karinAdjustFrom640(int *ox, int *oy, int x, int y)
{
	// for 640x480 virtualized screen
	static const float _WP = 1.0 / 640.0;
	static const float _HP = 1.0 / 480.0;
	static const float _P = 640.0 / 480.0;

	float xscale = glConfig.vidWidth * _WP;
	float yscale = glConfig.vidHeight * _HP;
	float bias;
	if (glConfig.vidWidth * 480 > glConfig.vidHeight * 640) {
		// wide screen
		bias = 0.5 * (glConfig.vidWidth - (glConfig.vidHeight * _P));
		xscale = yscale;
	}
	else {
		// no wide screen
		bias = 0;
	}
	//printf("%f %f %f\n", xscale, yscale, bias);
	//printf("%f %f %f\n", 640.0*xscale, 480.0*yscale, bias);

	if(ox)
	{
		int w = (int)((float)SCREEN_WIDTH * xscale);
		int sx = (int)bias;
		int fx;
		if(x < sx)
			fx = 0;
		else if(x >= sx + w)
			fx = w;
		else
			fx = x - sx;
		*ox = fx;
	}
	if(oy)
	{
		int h = (int)((float)SCREEN_HEIGHT * yscale);
		int fy;
		if(y < 0)
			fy = 0;
		else if(y >= h)
			fy = h;
		else
			fy = y;
		*oy = fy;
	}
}

Bool karinXI2MouseEvent(int button, Bool pressed, int x, int y)
{
	if(harm_usingVKB->integer && karinVKBMouseEvent(button, pressed, x, glConfig.vidHeight - y, karinHandleVKBAction))
		return True;
	if (Key_GetCatcher() & (KEYCATCH_UI)) {
		unsigned int t = Sys_XTimeToSysTime(myxitime);
		if(pressed)
		{
			int fx;
			int fy;
			karinAdjustFrom640(&fx, &fy, x, y);
			Com_QueueEvent(t, SE_MOUSE, fx - mx, fy - my, 0, NULL); // for ui.qvm in game data pak1.pak when client not load ui.so of ioquake3-touch package.
			Com_QueueEvent(t, SE_TOUCH, fx, fy, 0, NULL); // for ui.so in ioquake3-touch package.
			mx = fx;
			my = fy;
		}
		Com_QueueEvent(t, SE_KEY, K_MOUSE1, pressed, 0, NULL);
		return True;
	}
	return False;
}

Bool karinXI2MotionEvent(int button, Bool pressed, int x, int y, int dx, int dy)
{
	if(harm_usingVKB->integer && karinVKBMouseMotionEvent(button, pressed, x, glConfig.vidHeight - y, dx, -dy, karinHandleVKBAction))
		return True;
	if (Key_GetCatcher() & (KEYCATCH_UI)) {
		if(pressed)
		{
			unsigned int t = Sys_XTimeToSysTime(myxitime);
			int fx;
			int fy;
			karinAdjustFrom640(&fx, &fy, x, y);
			Com_QueueEvent(t, SE_MOUSE, fx - mx, fy - my, 0, NULL); // for ui.qvm in game data pak1.pak when client not load ui.so of ioquake3-touch package.
			Com_QueueEvent(t, SE_TOUCH, fx, fy, 0, NULL); // for ui.so in ioquake3-touch package.
			mx = fx;
			my = fy;
		}
		return True;
	}
	return False;
}
#endif

/*****************************************************************************
 ** KEYBOARD
 ** NOTE TTimo the keyboard handling is done with KeySyms
 **   that means relying on the keyboard mapping provided by X
 **   in-game it would probably be better to use KeyCode (i.e. hardware key codes)
 **   you would still need the KeySyms in some cases, such as for the console and all entry textboxes
 **     (cause there's nothing worse than a qwerty mapping on a french keyboard)
 **
 ** you can turn on some debugging and verbose of the keyboard code with #define KBD_DBG
 ******************************************************************************/

#define KBD_DBG

static char *XLateKey(XKeyEvent * ev, int *key)
{
	static char buf[64];
	KeySym keysym;
	int XLookupRet;

	*key = 0;

	XLookupRet = XLookupString(ev, buf, sizeof buf, &keysym, 0);
#ifdef _HARMATTAN_UNUSED
#ifdef KBD_DBG
	ri.Printf(PRINT_ALL, "XLookupString ret: %d buf: %s keysym: %x\n",
			XLookupRet, buf, keysym);
#endif
#endif

	switch (keysym) {
		case XK_KP_Page_Up:
		case XK_KP_9:
			*key = K_KP_PGUP;
			break;
		case XK_Page_Up:
			*key = K_PGUP;
			break;

		case XK_KP_Page_Down:
		case XK_KP_3:
			*key = K_KP_PGDN;
			break;
		case XK_Page_Down:
			*key = K_PGDN;
			break;

		case XK_KP_Home:
			*key = K_KP_HOME;
			break;
		case XK_KP_7:
			*key = K_KP_HOME;
			break;
		case XK_Home:
			*key = K_HOME;
			break;

		case XK_KP_End:
		case XK_KP_1:
			*key = K_KP_END;
			break;
		case XK_End:
			*key = K_END;
			break;

		case XK_KP_Left:
			*key = K_KP_LEFTARROW;
			break;
		case XK_KP_4:
			*key = K_KP_LEFTARROW;
			break;
		case XK_Left:
			*key = K_LEFTARROW;
			break;

		case XK_KP_Right:
			*key = K_KP_RIGHTARROW;
			break;
		case XK_KP_6:
			*key = K_KP_RIGHTARROW;
			break;
		case XK_Right:
			*key = K_RIGHTARROW;
			break;

		case XK_KP_Down:
		case XK_KP_2:
			*key = K_KP_DOWNARROW;
			break;
		case XK_Down:
			*key = K_DOWNARROW;
			break;

		case XK_KP_Up:
		case XK_KP_8:
			*key = K_KP_UPARROW;
			break;
		case XK_Up:
			*key = K_UPARROW;
			break;

		case XK_Escape:
#ifdef _HARMATTAN
		case 0xfe03: //N950 Fn
#else
		case 0x3d:			/* N900: (Fn +) = */
#endif
			*key = K_ESCAPE;
			break;

		case XK_KP_Enter:
			*key = K_KP_ENTER;
			break;
		case XK_Return:
			*key = K_ENTER;
			break;

		case XK_Tab:
			*key = K_TAB;
			break;

		case XK_F1:
			*key = K_F1;
			break;

		case XK_F2:
			*key = K_F2;
			break;

		case XK_F3:
			*key = K_F3;
			break;

		case XK_F4:
			*key = K_F4;
			break;

		case XK_F5:
			*key = K_F5;
			break;

		case XK_F6:
			*key = K_F6;
			break;

		case XK_F7:
			*key = K_F7;
			break;

		case XK_F8:
			*key = K_F8;
			break;

		case XK_F9:
			*key = K_F9;
			break;

		case XK_F10:
			*key = K_F10;
			break;

		case XK_F11:
			*key = K_F11;
			break;

		case XK_F12:
			*key = K_F12;
			break;

			// bk001206 - from Ryan's Fakk2 
			//case XK_BackSpace: *key = 8; break; // ctrl-h
		case XK_BackSpace:
			*key = K_BACKSPACE;
			break;		// ctrl-h

		case XK_KP_Delete:
		case XK_KP_Decimal:
			*key = K_KP_DEL;
			break;
		case XK_Delete:
			*key = K_DEL;
			break;

		case XK_Pause:
			*key = K_PAUSE;
			break;

		case XK_Shift_L:
		case XK_Shift_R:
			*key = K_SHIFT;
			break;

		case XK_Execute:
		case XK_Control_L:
		case XK_Control_R:
			*key = K_CTRL;
			break;

		case XK_Alt_L:
		case XK_Meta_L:
		case XK_Alt_R:
		case XK_Meta_R:
			*key = K_ALT;
			break;

		case XK_KP_Begin:
			*key = K_KP_5;
			break;

		case XK_Insert:
			*key = K_INS;
			break;
		case XK_KP_Insert:
		case XK_KP_0:
			*key = K_KP_INS;
			break;

		case XK_KP_Multiply:
			*key = '*';
			break;
		case XK_KP_Add:
			*key = K_KP_PLUS;
			break;
		case XK_KP_Subtract:
			*key = K_KP_MINUS;
			break;
		case XK_KP_Divide:
			*key = K_KP_SLASH;
			break;

			// bk001130 - from cvs1.17 (mkv)
		case XK_exclam:
			*key = '1';
			break;
		case XK_at:
			*key = '2';
			break;
		case XK_numbersign:
			*key = '3';
			break;
		case XK_dollar:
			*key = '4';
			break;
		case XK_percent:
			*key = '5';
			break;
		case XK_asciicircum:
			*key = '6';
			break;
		case XK_ampersand:
			*key = '7';
			break;
		case XK_asterisk:
			*key = '8';
			break;
		case XK_parenleft:
			*key = '9';
			break;
		case XK_parenright:
			*key = '0';
			break;

		case XK_twosuperior:
#ifdef _HARMATTAN_3
		case XK_Multi_key:  /* 0xff20     Multi-key character compose */
			// N950 Sym key, also is 0xff20, same as N900 Sym key.
#else
		case 0xff20:			/* N900: Fn + Sym/Ctrl */
#endif
			*key = K_CONSOLE;
			*buf = '\0';
			break;

			// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=472
		case XK_space:
		case XK_KP_Space:
			*key = K_SPACE;
			break;

		default:
			if (XLookupRet == 0) {
				if (com_developer->value) {
					ri.Printf(PRINT_ALL,
							"Warning: XLookupString failed on KeySym %d\n",
							keysym);
				}
				return NULL;
			} else {
				// XK_* tests failed, but XLookupString got a buffer, so let's try it
				*key = *(unsigned char *)buf;
				if (*key >= 'A' && *key <= 'Z')
					*key = *key - 'A' + 'a';
				// if ctrl is pressed, the keys are not between 'A' and 'Z', for instance ctrl-z == 26 ^Z ^C etc.
				// see https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=19
				else if (*key >= 1 && *key <= 26)
					*key = *key + 'a' - 1;
			}
			break;
	}

	return buf;
}

#ifdef _HARMATTAN_UNUSED
static void Proximity_HandleEvents(void)
{
	char buf[16];
	FILE *fp;
	int t;
	static qboolean oldState = qfalse, state = qfalse;

	fp = fopen(PROXY_PATH "/state", "r");
	if (!fp)
		return;
	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (Q_stricmp(buf, "closed\n") == 0) {
		state = qtrue;
	} else {
		state = qfalse;
	}

	if (state != oldState) {
		t = Sys_Milliseconds();
		Com_QueueEvent(t, SE_KEY, K_MOUSE1, state, 0, NULL);
		oldState = state;
	}
}

static void Accelerometer_WriteFudge(void)
{
	FILE *fp;

	if (!accel_fudge)
		accel_fudge = Cvar_Get("accel_fudge", "3", CVAR_ARCHIVE);
	if (access(ACCEL_PATH "/fudge", W_OK) < 0)
		return;
	if (!(fp = fopen(ACCEL_PATH "/fudge", "w")))
		return;

	fprintf(fp, "%d\n", accel_fudge->integer);

	fclose(fp);
}

static void Accelerometer_ReadCoords(int *x, int *y, int *z)
{
	char buf[16];
	FILE *fp;
	static qboolean fudge = qfalse;

	if (!fudge) {
		Accelerometer_WriteFudge();
		fudge = qtrue;
	}

	fp = fopen(ACCEL_PATH "/coord", "r");
	if (!fp)
		return;

	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d %d %d\n", x, y, z);

	if (accel_debug->integer) {
		Com_DPrintf("Accelerometer_ReadCoords: %d %d %d\n", *x, *y, *z);
	}

	fclose(fp);
}

static void Accelerometer_HandleEvents(void)
{
	int dx = 0, dy = 0;
	int t, tmp;
	int x, y, z;
	static int jumpTime = 0;

	t = Sys_Milliseconds();

	if (!accel_angle) {
		accel_angle = Cvar_Get("accel_angle", "45", CVAR_ARCHIVE);
	}
	if (!accel_debug) {
		accel_debug = Cvar_Get("accel_debug", "0", CVAR_ARCHIVE);
	}
	if (!accel_jump) {
		accel_jump = Cvar_Get("accel_jump", "-1300", CVAR_ARCHIVE);
	}
	if (!accel_epsilon_x) {
		accel_epsilon_x =
			Cvar_Get("accel_epsilon_x", "100", CVAR_ARCHIVE);
	}
	if (!accel_epsilon_y) {
		accel_epsilon_y =
			Cvar_Get("accel_epsilon_y", "100", CVAR_ARCHIVE);
	}

	Accelerometer_ReadCoords(&x, &y, &z);

	/* only update sin and cos if the cvar's changed */
	tmp = accel_angle->value;
	if(tmp != accel_angle_value) {
		/* what happened to sincosf()? */
		accel_sin = sin(DEG2RAD(tmp));
		accel_cos = cos(DEG2RAD(tmp));
		accel_angle_value = tmp;
	}
	tmp = y * accel_cos - z * accel_sin;
	z = z * accel_cos + y * accel_sin;
	y = tmp;

	if (accel_debug->integer) {
		Com_DPrintf("Accelerometer_HandleEvents: y = %d\n", y);
	}

	if (accel_jump->value) {
		float mag = sqrtf(y*y+z*z);
		// HACK - change the sign of jump to be +ve not -ve
		int looks_like_a_jump = (mag > -accel_jump->value) && (z < accel_jump->value / 2.0f);
		if (!jumpTime && looks_like_a_jump) {
			Com_QueueEvent(t, SE_KEY, K_SPACE, qtrue, 0, NULL);
			jumpTime = t;
		} else if (jumpTime && !looks_like_a_jump) {
			Com_QueueEvent(t, SE_KEY, K_SPACE, qfalse, 0, NULL);
			jumpTime = 0;
		}
	}

	if (x > accel_epsilon_x->integer)
		dx = -(x - accel_epsilon_x->integer);
	else if (x < -accel_epsilon_x->integer)
		dx = -(x + accel_epsilon_x->integer);

	if (y > accel_epsilon_y->integer)
		dy = -(y - accel_epsilon_y->integer);
	else if (y < -accel_epsilon_y->integer)
		dy = -(y + accel_epsilon_y->integer);

	Com_QueueEvent(t, SE_ACCEL, dx, dy, 0, NULL);
}
#endif

#ifdef _HARMATTAN_UNUSED
static qboolean motionPressed = qfalse;
#endif

qboolean IN_MotionPressed(void)
{
	return motionPressed;
}

static void HandleEvents(void)
{
	int key;
	XEvent event;
	char *p;
#ifdef _HARMATTAN_UNUSED
	static int x = 0, y = 0;	// absolute
	int dx = 0, dy = 0;	// delta (new-old)
#endif
	int t = 0;		// default to 0 in case we don't set

	if (!dpy)
		return;

#ifdef _HARMATTAN_PLUS
	while (X11_Pending(dpy)) 
#else
		while (XPending(dpy)) 
#endif
		{
			XNextEvent(dpy, &event);
			switch (event.type) {
				case KeyPress:
					t = Sys_XTimeToSysTime(event.xkey.time);
					p = XLateKey(&event.xkey, &key);
					if (key) {
#ifdef _HARMATTAN_PLUS
						if(Key_GetCatcher() == 0 && key == Key_GetKey("+attack"))
						{
							motionPressed = qfalse;
							CL_AutoAim();
						}
#endif
						Com_QueueEvent(t, SE_KEY, key, qtrue, 0, NULL);
					}
					if (p) {
						while (*p) {
							//printf("%d '%c'\n", *p, *p);
							Com_QueueEvent(t, SE_CHAR, *p++, 0, 0,
									NULL);
						}
					}
					break;

				case KeyRelease:
					t = Sys_XTimeToSysTime(event.xkey.time);
#if 0
					// bk001206 - handle key repeat w/o XAutRepatOn/Off
					//            also: not done if console/menu is active.
					// From Ryan's Fakk2.
					// see game/q_shared.h, KEYCATCH_* . 0 == in 3d game.  
					if (cls.keyCatchers == 0) {	// FIXME: KEYCATCH_NONE
						if (repeated_press(&event) == qtrue)
							continue;
					}	// if
#endif
					XLateKey(&event.xkey, &key);
#ifdef _HARMATTAN_PLUS
					if(Key_GetCatcher() == 0 && key == Key_GetKey("+attack"))
					{
						motionPressed = qtrue;
					}
#endif

					Com_QueueEvent(t, SE_KEY, key, qfalse, 0, NULL);
					break;

#ifdef _HARMATTAN_UNUSED
					//Now X11 ungrab mouse, all mouse events is GenericEvent of XInput2.
				case MotionNotify:
					t = Sys_XTimeToSysTime(event.xkey.time);

					dx = event.xmotion.x;
					dy = event.xmotion.y;

					if (!(Key_GetCatcher() & KEYCATCH_CGAME)) {
						dx -= (glConfig.vidWidth - SCREEN_WIDTH) / 2;
						dy -= (glConfig.vidHeight - SCREEN_HEIGHT) / 2;
					}

					dx -= x;
					dy -= y;

#if 0
					Com_Printf ("KeyCatcher: %d\n", Key_GetCatcher());

					Com_Printf ("MotionNotify: event.xmotion = (%d, %d) delta = (%d, %d)\n",
							event.xmotion.x, event.xmotion.y,
							dx, dy);
#endif

					x = event.xmotion.x;
					y = event.xmotion.y;
					if (!(Key_GetCatcher() & KEYCATCH_CGAME)) {
						x -= (glConfig.vidWidth - SCREEN_WIDTH) / 2;
						y -= (glConfig.vidHeight - SCREEN_HEIGHT) / 2;
					}
					break;

				case ButtonPress:
				case ButtonRelease:
					t = Sys_XTimeToSysTime(event.xkey.time);
					motionPressed = (qboolean) (event.type == ButtonPress);
					if (Key_GetCatcher() & (KEYCATCH_CGAME | KEYCATCH_UI)) {
						Com_QueueEvent(t, SE_KEY, K_MOUSE1,
								motionPressed, 0, NULL);
					}
					break;
#endif

				case CreateNotify:
					win_x = event.xcreatewindow.x;
					win_y = event.xcreatewindow.y;
					break;

				case ConfigureNotify:
					win_x = event.xconfigure.x;
					win_y = event.xconfigure.y;
					break;

					// Only XInput2 event
#ifdef _HARMATTAN_3
				case GenericEvent:
					karinXI2Event(&event);
					/*
						 else
						 printf("Unhandle XGenericEvent\n");
						 */
					break;
#endif
			}
		}

#ifdef _HARMATTAN_UNUSED
	if (motionPressed) {
		Com_QueueEvent(t, SE_MOUSE, dx, dy, 0, NULL);
	}
#endif

#ifdef _HARMATTAN_UNUSED
	Proximity_HandleEvents();

	Accelerometer_HandleEvents();
#endif
}

static Cursor CreateNullCursor(Display * display, Window root)
{
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	XColor dummycolour;
	Cursor cursor;

	cursormask = XCreatePixmap(display, root, 1, 1, 1 /*depth */ );
	xgc.function = GXclear;
	gc = XCreateGC(display, cursormask, GCFunction, &xgc);
	XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
	dummycolour.pixel = 0;
	dummycolour.red = 0;
	dummycolour.flags = 0x4;
	cursor =
		XCreatePixmapCursor(display, cursormask, cursormask, &dummycolour,
				&dummycolour, 0, 0);
	XFreePixmap(display, cursormask);
	XFreeGC(display, gc);
	return cursor;
}

static void hildon_set_non_compositing(void)
{
	Atom atom;
	int one = 1;

	atom = XInternAtom(dpy, "_HILDON_NON_COMPOSITED_WINDOW", False);
	XChangeProperty(dpy, win, atom, XA_INTEGER, 32, PropModeReplace,
			(unsigned char *)&one, 1);
}

static void install_grabs(void)
{
	XSync(dpy, False);

	hildon_set_non_compositing();

	XDefineCursor(dpy, win, CreateNullCursor(dpy, win));

#ifdef _HARMATTAN_UNUSED
	XGrabPointer(dpy, win, False, MOUSE_MASK, GrabModeAsync, GrabModeAsync,
			win, None, CurrentTime);

	XGetPointerControl(dpy, &mouse_accel_numerator,
			&mouse_accel_denominator, &mouse_threshold);

	XChangePointerControl(dpy, True, True, 1, 1, 0);

	XSync(dpy, False);

	mwx = glConfig.vidWidth / 2;
	mwy = glConfig.vidHeight / 2;
	mx = my = 0;
#endif

	XGrabKeyboard(dpy, win, False, GrabModeAsync, GrabModeAsync,
			CurrentTime);

	XSync(dpy, False);
}

static void uninstall_grabs(void)
{
#ifdef _HARMATTAN_UNUSED
	XChangePointerControl(dpy, qtrue, qtrue, mouse_accel_numerator,
			mouse_accel_denominator, mouse_threshold);

	XUngrabPointer(dpy, CurrentTime);
#endif
	XUngrabKeyboard(dpy, CurrentTime);

	XUndefineCursor(dpy, win);
}

void IN_ActivateMouse(void)
{
	if (!mouse_avail || !dpy || !win)
		return;

	if (!mouse_active) {
		if (!in_nograb->value)
			install_grabs();
		mouse_active = qtrue;
	}
}

void IN_DeactivateMouse(void)
{
	if (!mouse_avail || !dpy || !win)
		return;

	if (mouse_active) {
		if (!in_nograb->value)
			uninstall_grabs();
		mouse_active = qfalse;
	}
#ifdef _HARMATTAN_3
	mx = my = 0;
#endif
}

void IN_Frame(void)
{
	qboolean loading;

#ifdef _HARMATTAN_3
	karinGetClientState();
#endif
	HandleEvents();

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	loading = !!(cls.state != CA_DISCONNECTED && cls.state != CA_ACTIVE);

	if (!r_fullscreen->integer && (Key_GetCatcher() & KEYCATCH_CONSOLE)) {
		// Console is down in windowed mode
		IN_DeactivateMouse();
	} else if (!r_fullscreen->integer && loading) {
		// Loading in windowed mode
		IN_DeactivateMouse();
	} else
		IN_ActivateMouse();
}

void IN_Init(void)
{
	Com_DPrintf("\n------- Input Initialization -------\n");

	// mouse variables
	in_mouse = Cvar_Get("in_mouse", "1", CVAR_ARCHIVE);
	in_nograb = Cvar_Get("in_nograb", "0", CVAR_ARCHIVE);

	if (in_mouse->value) {
		mouse_avail = qtrue;
		IN_ActivateMouse();
	} else {
		IN_DeactivateMouse();
		mouse_avail = qfalse;
	}

	Com_DPrintf("------------------------------------\n");
}

void IN_Shutdown(void)
{
	IN_DeactivateMouse();
	mouse_avail = qfalse;
}

void IN_Restart(void)
{
	IN_Init();
}

