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
static qboolean motionPressed = qtrue;

qboolean IN_MotionPressed(void)
{
	return motionPressed;
}
#endif

#ifdef _HARMATTAN_UNUSED
#define ACCEL_PATH	"/sys/class/i2c-adapter/i2c-3/3-001d/"
#define PROXY_PATH	"/sys/devices/platform/gpio-switch/proximity/"
#endif

#ifdef _HARMATTAN_PLUS
#define FIXED_X_COORD(x) (x + 107)
#endif

#ifdef _HARMATTAN
#include "egl_vkb.h"

typedef struct
{
	const char *binding;
	int key;
}key_binding_map;

static key_binding_map vb_functions[VB_TotalFunction] = 
{
	{ "+attack", K_ENTER },
	{ NULL, K_ESCAPE },
	{ "+lookup", K_UPARROW},
	{ "+lookdown", K_DOWNARROW},
	{ "+left", K_LEFTARROW},
	{ "+right", K_RIGHTARROW},
	{ "centerview", 0},
	{ "+forward", 0},
	{ "+back", 0},
	{ "+moveleft", 0},
	{ "+moveright", 0},
	{ "+moveup", 0},
	{ "+movedown", 0},
	{ "weapon 1", 0},
	{ "weapon 2", 0},
	{ "weapon 3", 0},
	{ "weapon 4", 0},
	{ "weapon 5", 0},
	{ "weapon 6", 0},
	{ "weapon 7", 0},
	{ "weapon 8", 0},
	{ "weapon 9", 0},
	{ "weapprev", 0},
	{ "weapnext", 0},
	{ "+button2", 0},
	{ "+scores", 0},
	{ "+speed", 0},
	{ "+zoom", 0}
};
typedef struct
{
	int TLx; //top-left point X
	int TLy; //top-left point Y
	int BRx; //bottom-right point X
	int BRy; //bottom-right point Y
	const char *binding; //binding key
	int key; //key
} VB_Rect;

static VB_Rect vb_rects[VB_TotalFunction];
static VB_Rect vb_baserects[turncenter_vkb];

static void InitRectPosition(int index, int x, int y, int count)
{
	if(index < 0 || index >= VB_TotalFunction)
		return;
	VB_Rect *rect = &(vb_rects[index]);
	rect -> TLx = x;
	rect -> TLy = y;
	rect -> BRx = x + VBW / 2 * count;
	rect -> BRy = y + VBW;
	rect -> binding = vb_functions[index].binding;
	rect -> key = vb_functions[index].key;
}

static void InitMenuRectPosition(int index, int x, int y, int count)
{
	if(index < 0 || index >= turncenter_vkb)
		return;
	VB_Rect *rect = &(vb_baserects[index]);
	rect -> TLx = FIXED_X_COORD(x);
	rect -> TLy = y;
	rect -> BRx = FIXED_X_COORD(x) + VBW / 2 * count;
	rect -> BRy = y + VBW;
	rect -> binding = NULL;
	rect -> key = vb_functions[index].key;
}

static void InitRectPositionByCoord(int index, int x1, int y1, int x2, int y2)
{
	if(index < 0 || index >= VB_TotalFunction)
		return;
	VB_Rect *rect = &(vb_rects[index]);
	rect -> TLx = x1;
	rect -> TLy = y1;
	rect -> BRx = x2 + VBW;
	rect -> BRy = y2 + VBW;
	rect -> binding = vb_functions[index].binding;
	rect -> key = vb_functions[index].key;
}

static void InitVirtualButton(void)
{
	Com_DPrintf("\n------- On-Screen Buttons Position Initialization...... -------\n");
	//for game
	// w s a d
	InitRectPositionByCoord(forward_vkb, MOVELEFT_X, FORWARD_Y, MOVERIGHT_X,  FORWARD_Y);
	InitRectPositionByCoord(backward_vkb, MOVELEFT_X, BACKWARD_Y, MOVERIGHT_X, BACKWARD_Y);
	InitRectPositionByCoord(moveleft_vkb, MOVELEFT_X, FORWARD_Y, MOVELEFT_X, BACKWARD_Y);
	InitRectPositionByCoord(moveright_vkb, MOVERIGHT_X, FORWARD_Y, MOVERIGHT_X, BACKWARD_Y);
	//home end pageup pagedown center
	InitRectPositionByCoord(turnup_vkb, TURNLEFT_X, TURNUP_Y, TURNRIGHT_X, TURNUP_Y);
	InitRectPositionByCoord(turndown_vkb, TURNLEFT_X, TURNDOWN_Y, TURNRIGHT_X, TURNDOWN_Y);
	InitRectPositionByCoord(turnleft_vkb, TURNLEFT_X, TURNUP_Y, TURNLEFT_X, TURNDOWN_Y);
	InitRectPositionByCoord(turnright_vkb, TURNRIGHT_X, TURNUP_Y, TURNRIGHT_X, TURNDOWN_Y);
	InitRectPosition(turncenter_vkb, TURNCENTER_X, TURNCENTER_Y, 2);
	//jump down
	InitRectPosition(jump_vkb, JUMP_X, JUMP_Y, 4);
	InitRectPosition(down_vkb, DOWN_X, DOWN_Y, 4);
	//1 - 9
	InitRectPosition(weapon1_vkb, WEAPON1_X, WEAPON1_Y, 2);
	InitRectPosition(weapon2_vkb, WEAPON2_X, WEAPON2_Y, 2);
	InitRectPosition(weapon3_vkb, WEAPON3_X, WEAPON3_Y, 2);
	InitRectPosition(weapon4_vkb, WEAPON4_X, WEAPON4_Y, 2);
	InitRectPosition(weapon5_vkb, WEAPON5_X, WEAPON5_Y, 2);
	InitRectPosition(weapon6_vkb, WEAPON6_X, WEAPON6_Y, 2);
	InitRectPosition(weapon7_vkb, WEAPON7_X, WEAPON7_Y, 2);
	InitRectPosition(weapon8_vkb, WEAPON8_X, WEAPON8_Y, 2);
	InitRectPosition(weapon9_vkb, WEAPON9_X, WEAPON9_Y, 2);
	//prev next
	InitRectPosition(prevweapon_vkb, PREVWEAPON_X, PREVWEAPON_Y, 3);
	InitRectPosition(nextweapon_vkb, NEXTWEAPON_X, NEXTWEAPON_Y, 3);
	//functional
	InitRectPosition(fire_vkb, FIRE_X, FIRE_Y, 3);
	InitRectPosition(useitem_vkb, USEITEM_X, USEITEM_Y, 3);
	InitRectPosition(escape_vkb, ESCAPE_X, ESCAPE_Y, 2);
	InitRectPosition(score_vkb, SCORE_X, SCORE_Y, 2);
	InitRectPosition(walk_vkb, WALK_X, WALK_Y, 3);
	InitRectPosition(zoom_vkb, ZOOM_X, ZOOM_Y, 3);

	//for menu
	//enter cancel
	InitMenuRectPosition(fire_vkb, ENTER_X, ENTER_Y, 4);
	InitMenuRectPosition(escape_vkb, CANCEL_X, CANCEL_Y, 4);
	//left right up down arrow
	InitMenuRectPosition(turnup_vkb, TOUP_X, TOUP_Y, 2);
	InitMenuRectPosition(turndown_vkb, TODOWN_X, TODOWN_Y, 2);
	InitMenuRectPosition(turnleft_vkb, TOLEFT_X, TOLEFT_Y, 2);
	InitMenuRectPosition(turnright_vkb, TORIGHT_X, TORIGHT_Y, 2);
	Com_DPrintf("------- done -------\n");
}
#endif

#ifdef _HARMATTAN_PLUS
XIDeviceInfo *xi_master = NULL;
int xi_opcode = 0;

#define ATOM_COUNT 3
#define XI2_X11_ATOM_NAME(id,name) name,

const char * atom_names[ATOM_COUNT] = {
	XI2_X11_ATOM_NAME(AbsMTTrackingID, "Abs MT Tracking ID")
		XI2_X11_ATOM_NAME(AbsMTPositionX, "Abs MT Position X")
		XI2_X11_ATOM_NAME(AbsMTPositionY, "Abs MT Position Y")
};

Atom atoms[ATOM_COUNT];

static inline void MultiClickHandler(int index, int event_x, int event_y, Bool state, VB_Rect *rect)
{
	if(((event_x >= rect -> TLx) && (event_x <= rect -> BRx)) 
			&& ((event_y >= rect -> TLy) && (event_y <= rect -> BRy)))
	{
		qboolean pressed = state ? qtrue : qfalse;
		if(Key_GetCatcher() == 0 && index == fire_vkb)
		{
			motionPressed = pressed ? qfalse : qtrue;
			if(pressed)
				CL_AutoAim();
		}
		if(Key_GetCatcher() == 0)
			CL_UpdateVirtualButtonState(index, (qboolean)state);
		else
			CL_UpdateMenuVirtualButtonState(index, (qboolean)state);
		//int	time = Sys_Milliseconds();
		int key;
		if(rect -> binding)
			key = Key_GetKey(rect -> binding);
		else
			key = rect -> key;
		if(key > -1)
		{
			Com_QueueEvent(Sys_Milliseconds(), SE_KEY, key, pressed, 0, NULL);
		}
	}
}

static inline void MultiCancelHandler(int index, int nx, int ny, int ox, int oy, VB_Rect *rect)
{
	qboolean ob = (((ox >= rect -> TLx) && (ox <= rect -> BRx)) 
			&& ((oy >= rect -> TLy) && (oy <= rect -> BRy)));
	qboolean nb = (((nx >= rect -> TLx) && (nx <= rect -> BRx)) 
			&& ((ny >= rect -> TLy) && (ny <= rect -> BRy)));
	if(ob == qtrue && nb == qfalse) //release
	{
		if(Key_GetCatcher() == 0 && index == fire_vkb)
		{
			motionPressed = qtrue;
		}
		if(Key_GetCatcher() == 0)
			CL_UpdateVirtualButtonState(index, qfalse);
		else
			CL_UpdateMenuVirtualButtonState(index, qfalse);
		//int	time = Sys_Milliseconds();
		int key;
		if(rect -> binding)
			key = Key_GetKey(rect -> binding);
		else
			key = rect -> key;
		if(key > -1)
		{
			Com_QueueEvent(Sys_Milliseconds(), SE_KEY, key, qfalse, 0, NULL);
		}
	}
	else if(ob == qfalse && nb == qtrue) //press
	{
		if(Key_GetCatcher() == 0 && index == fire_vkb)
		{
			motionPressed = qfalse;
			CL_AutoAim();
		}
		if(Key_GetCatcher() == 0)
			CL_UpdateVirtualButtonState(index, qtrue);
		else
			CL_UpdateMenuVirtualButtonState(index, qtrue);
		//int	time = Sys_Milliseconds();
		int key;
		if(rect -> binding)
			key = Key_GetKey(rect -> binding);
		else
			key = rect -> key;
		if(key > -1)
		{
			Com_QueueEvent(Sys_Milliseconds(), SE_KEY, key, qtrue, 0, NULL);
		}
	}
	//else not to do
}

static void CheckMultiButton(int x, int y, Bool state)
{
	int i;
	if(Key_GetCatcher() == 0)
	{
		for(i = 0; i < VB_TotalFunction; i++)
			MultiClickHandler(i, x, y, state, &(vb_rects[i]));
	}
	else
	{
		for(i = 0; i < turncenter_vkb; i++)
			MultiClickHandler(i, x, y, state, &(vb_baserects[i]));
	}
}

static void CheckMultiMove(int nx, int ny, int ox, int oy, Bool state)
{
	if(!state)
		return;
	int i;
	if(Key_GetCatcher() == 0)
	{
		for(i = 0; i < VB_TotalFunction; i++)
			MultiCancelHandler(i, nx, ny, ox, oy, &(vb_rects[i]));
	}
	else
	{
		for(i = 0; i < turncenter_vkb; i++)
			MultiCancelHandler(i, nx, ny, ox, oy, &(vb_baserects[i]));
	}
}

/* These are static for our mouse handling code */
typedef struct
{
	int MouseX;
	int MouseY;
	int DeltaX;
	int DeltaY;
	Bool ButtonState;
}MultiMouse;
#define MAXMOUSE 10
static MultiMouse multi_mouses[MAXMOUSE];
#define MOUSE_MAX_X (N950SW - 1)
#define MOUSE_MAX_Y (N950SH - 1)

void MouseInit(void)
{
	/* The mouse is at (0,0) */
	int i;
	for (i = 0; i < MAXMOUSE; i++) {
		multi_mouses[i].MouseX = 0;
		multi_mouses[i].MouseY = 0;
		multi_mouses[i].DeltaX = 0;
		multi_mouses[i].DeltaY = 0;
		multi_mouses[i].ButtonState = False;
	}
}

static Bool GetMultiMouseState (int which, int *x, int *y)
{
	const MultiMouse * const mm = &(multi_mouses[which]);
	if ( x ) {
		*x = mm -> MouseX;
	}
	if ( y ) {
		*y = mm -> MouseY;
	}
	return(mm -> ButtonState);
}

static Bool GetRelativeMultiMouseState (int which, int *x, int *y)
{
	MultiMouse * const mm = &(multi_mouses[which]);
	if ( x )
		*x = mm -> DeltaX;
	if ( y )
		*y = mm -> DeltaY;
	mm -> DeltaX = 0;
	mm -> DeltaY = 0;
	return(mm -> ButtonState);
}

static Bool PrivateMultiMouseMotion(int which, Bool buttonstate, int relative, int x, int y)
{
	int X, Y;
	int Xrel;
	int Yrel;

	MultiMouse * const mm = &(multi_mouses[which]);
	/* Default buttonstate is the current one */
	if ( ! buttonstate ) {
		buttonstate = mm -> ButtonState;
	}

	Xrel = x;
	Yrel = y;
	if ( relative ) {
		/* Push the cursor around */
		x = (mm -> MouseX + x);
		y = (mm -> MouseY + y);
	} else {
		/* Do we need to clip {x,y} ? */
		//ClipOffset(&x, &y);
	}

	/* Mouse coordinates range from 0 - width-1 and 0 - height-1 */
	if ( x < 0 )
		X = 0;
	else
	if ( x >= MOUSE_MAX_X )
		X = MOUSE_MAX_X-1;
	else
		X = x;

	if ( y < 0 )
		Y = 0;
	else
	if ( y >= MOUSE_MAX_Y )
		Y = MOUSE_MAX_Y-1;
	else
		Y = y;

	/* If not relative mode, generate relative motion from clamped X/Y.
	   This prevents lots of extraneous large delta relative motion when
	   the screen is windowed mode and the mouse is outside the window.
	*/
	if ( ! relative ) {
		Xrel = X - mm -> MouseX;
		Yrel = Y - mm -> MouseY;
	}

	/* Drop events that don't change state */
	if ( ! Xrel && ! Yrel ) {
#if 0
printf("Mouse event didn't change state - dropped!\n");
#endif
		return(False);
	}

	/* Update internal mouse state */
	mm -> ButtonState = buttonstate;
	mm -> MouseX = X;
	mm -> MouseY = Y;
	mm -> DeltaX += Xrel;
	mm -> DeltaY += Yrel;
	/*
	if (which == 0) {
		// Redraw main pointer
		//SDL_MoveCursor(X, Y);
		//WILLTODO
	}
*/

	CheckMultiMove(X, Y, X - Xrel, Y - Yrel, buttonstate);

	/* Post the event, if desired */
	/*
	if ( SDL_ProcessEvents[SDL_MOUSEMOTION] == SDL_ENABLE ) {
		SDL_Event event;
		SDL_memset(&event, 0, sizeof(event));
		event.type = SDL_MOUSEMOTION;
		event.motion.which = which;
		event.motion.state = buttonstate;
		event.motion.x = X;
		event.motion.y = Y;
		event.motion.xrel = Xrel;
		event.motion.yrel = Yrel;
		if ( (SDL_EventOK == NULL) || (*SDL_EventOK)(&event) ) {
			posted = 1;
			SDL_PushEvent(&event);
		}
	}
	return(posted);
	*/
	return(True);
}

static Bool PrivateMultiMouseButton(int which, Bool state, int button/*left button*/, int x, int y)
{
	int move_mouse;
	Bool buttonstate;
	MultiMouse * const mm = &(multi_mouses[which]);

	/* Check parameters */
	if ( x || y ) {
		//ClipOffset(&x, &y);
		move_mouse = 1;
		/* Mouse coordinates range from 0 - width-1 and 0 - height-1 */
		if ( x < 0 )
			x = 0;
		else
		if ( x >= MOUSE_MAX_X )
			x = MOUSE_MAX_X-1;

		if ( y < 0 )
			y = 0;
		else
		if ( y >= MOUSE_MAX_Y )
			y = MOUSE_MAX_Y-1;
	} else {
		move_mouse = 0;
	}
	if ( ! x )
		x = mm -> MouseX;
	if ( ! y )
		y = mm -> MouseY;

	/* Figure out which event to perform */
	buttonstate = mm -> ButtonState;
	buttonstate = state;

	/* Update internal mouse state */
	mm -> ButtonState = buttonstate;
	if ( move_mouse ) {
		mm -> MouseX = x;
		mm -> MouseY = y;
		/*
		if (which == 0) {
			//SDL_MoveCursor(x, y);
			//WILLTODO
		}
		*/
	}

	CheckMultiButton(x, y, buttonstate);
	/* Post the event, if desired */
	/*
	if ( SDL_ProcessEvents[event.type] == SDL_ENABLE ) {
		event.button.which = which;
		event.button.state = state;
		event.button.button = button;
		event.button.x = x;
		event.button.y = y;
		if ( (SDL_EventOK == NULL) || (*SDL_EventOK)(&event) ) {
			posted = 1;
			SDL_PushEvent(&event);
		}
	}
	*/
	return(True);
}

static void ResetMultiMouse(int which)
{
	if( !multi_mouses[which].ButtonState )
		return;

	PrivateMultiMouseButton(which, False, 1/*left button*/, 0, 0);
}

static void ResetMouse(void)
{
	int i;
	for (i = 0; i < MAXMOUSE; i++) {
		ResetMultiMouse(i);
	}
}

//extern
int X11_XInput2_SetMasterPointer(int deviceid)
{
	int device_count = 0, i;
	if (xi_master) {
		XIFreeDeviceInfo(xi_master);
		xi_master = NULL;
	}

	xi_master = XIQueryDevice(dpy, deviceid, &device_count);
	if (!xi_master) {
		/* Master deviceid no longer exists? */
		return -1;
	}

	for (i = 0; i < xi_master->num_classes; i++) {
		if (xi_master->classes[i]->type == XIValuatorClass) {
			XIValuatorClassInfo *valuator = (XIValuatorClassInfo*)(xi_master->classes[i]);
			if (valuator->label == atom(AbsMTTrackingID)) {
				break;
			}
		}
	}

	return 0;
}
/* Ack!  XPending() actually performs a blocking read if no events available */
static int X11_Pending(Display *display)
{
	/* Flush the display connection and look to see if events are queued */
	XFlush(display);
	if ( XEventsQueued(display, QueuedAlready) ) {
		return(1);
	}

	/* More drastic measures are required -- see if X is ready to talk */
	{
		static struct timeval zero_time;	/* static == 0 */
		int x11_fd;
		fd_set fdset;

		x11_fd = ConnectionNumber(display);
		FD_ZERO(&fdset);
		FD_SET(x11_fd, &fdset);
		if ( select(x11_fd+1, &fdset, NULL, NULL, &zero_time) == 1 ) {
			return(XPending(display));
		}
	}

	/* Oh well, nothing is ready .. */
	return(0);
}

static inline void X11_XInput2_ClipTouch(int* val, int min, int size)
{
	if (*val < min || *val > (min + size)) {
		*val = -1;
	} else {
		*val -= min;
	}
}

static int X11_XInput2_DispatchTouchDeviceEvent(XIDeviceEvent *e)
{
	double v;
	int x, y;
	int active;
	int i;

	/* Sadly, we need to scale and clip the coordinates on our own. Prepare for this. */
	//const int screen_w = DisplayWidth(SDL_Display, SDL_Screen);
	//const int screen_h = DisplayHeight(SDL_Display, SDL_Screen);
	const int screen_w = N950SW;
	const int screen_h = N950SH;

	active = 0;
	for (i = 0; i < xi_master->num_classes; i++) {
		XIAnyClassInfo* any = xi_master->classes[i];
		//if (xi_master->classes[i]->type == XIValuatorClass) {
		if (any->type == XIValuatorClass) {
			XIValuatorClassInfo *valuator = (XIValuatorClassInfo*)(any);
			int n = valuator->number;

			if (!XIMaskIsSet(e->valuators.mask, n)) {
				/* This event does not contain this evaluator's value. */
				continue;
			}

			if (valuator->label == atom(AbsMTPositionX)) {
				v = e->valuators.values[n];
				v = (v - valuator->min) / (valuator->max - valuator->min);
				x = round(screen_w * v);
				//X11_XInput2_ClipTouch(&x, win_x, SCREEN_WIDTH); // No Use in full screen mode.
			} else if (valuator->label == atom(AbsMTPositionY)) {
				v = e->valuators.values[n];
				v = (v - valuator->min) / (valuator->max - valuator->min);
				y = round(screen_h * v);
				//X11_XInput2_ClipTouch(&y, win_y, SCREEN_HEIGHT); // No Use in full screen mode.
			} else if (valuator->label == atom(AbsMTTrackingID)) {
				/* Tracking ID is always the last valuator for a contact point,
				 * and indicates which finger we have been talking about previously. */
				int id = e->valuators.values[n];
				if (id >= MAXMOUSE) {
					/* Too many contact points! Discard! */
					continue;
				}
				if (x == -1 || y == -1) {
					/* Outside of the window, discard. */
					continue;
				}
				active |= 1 << id;
				if (GetMultiMouseState(id, NULL, NULL)) {
					/* We already knew about this finger; therefore, this is motion. */
					PrivateMultiMouseMotion(id, True, 0, x, y);
				} else {
					/* We did not know about this finger; therefore, this is a button press. */
					PrivateMultiMouseMotion(id, False, 0, x, y);
					PrivateMultiMouseButton(id, True, 1/*left button*/, 0, 0);
				}
			}
		}
	}

	/* Now enumerate all mouses and kill those that are not active. */
	for (i = 0; i < MAXMOUSE; i++) {
		if (!(active & (1 << i))) {
			ResetMultiMouse(i); /* Will send released events for pressed buttons. */
		}
	}

	return 1;
}

static int X11_XInput2_DispatchDeviceChangedEvent(XIDeviceChangedEvent *e)
{
	if (xi_master && e->deviceid == xi_master->deviceid) {
		/* Only care about slave change events of the master pointer, for now. */
		ResetMouse();
		X11_XInput2_SetMasterPointer(e->deviceid);
		return 1;
	}
	return 0;
}

static void LookupMultiButtonState(void)
{
	ri.Printf(PRINT_ALL, "------------------------------\n");
	int i;
	for(i = 0; i < MAXMOUSE; i++)
	{
		if(multi_mouses[i].ButtonState)
			ri.Printf(PRINT_ALL, "Finger -> %d ,Mouse X -> %d ,Mouse Y -> %d , Delta X -> %d ,Delta Y -> %d ,Button State -> %d\n", i, multi_mouses[i].MouseX, multi_mouses[i].MouseY, multi_mouses[i].DeltaX, multi_mouses[i].DeltaY, multi_mouses[i].ButtonState);
	}
}

#endif

#ifdef _HARMATTAN_UNUSED
static int mwx, mwy;
static int mx = 0, my = 0;
#endif
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
		case 0xff20:			/* N900: Fn + Sym/Ctrl */
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

qboolean IN_MotionPressed(void)
{
	return motionPressed;
}
#endif

static void HandleEvents(void)
{
	int key;
	XEvent event;
#ifdef _HARMATTAN_PLUS
	XIDeviceEvent *xi_event = NULL;
	XIDeviceChangedEvent *xi_changeevent = NULL;
#endif
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

#ifdef _HARMATTAN_PLUS
				// Only XInput2 event
			case GenericEvent:
				if (event.xcookie.extension == xi_opcode) {
					if (XGetEventData(dpy, &event.xcookie)) {
						switch (((XIEvent *)event.xcookie.data) ->evtype) {
							case XI_ButtonPress:
							case XI_ButtonRelease:
							case XI_Motion:
#ifdef _HARMATTAN_UNUSED
								motionPressed = (qboolean) (multi_mouses[0].ButtonState);
#endif
								xi_event = (XIDeviceEvent *)event.xcookie.data;
								X11_XInput2_DispatchTouchDeviceEvent(xi_event);
#ifdef _HARMATTAN_UNUSED
								if(xi_event -> evtype == XI_Motion)
								{
									if(multi_mouses[0].MouseX < 108)
										dx = 108;
									else if(multi_mouses[0].MouseX > 747)// 854 - 640 = 214 / 2 = 107  854 - 107 = 747
										dx = 747;
									else
										dx = multi_mouses[0].MouseX;
										dy = multi_mouses[0].MouseY;

										if (!(Key_GetCatcher() & KEYCATCH_CGAME)) {
											dx -= (glConfig.vidWidth - SCREEN_WIDTH) / 2;
											dy -= (glConfig.vidHeight - SCREEN_HEIGHT) / 2;
										}

										dx -= x;
										dy -= y;

									if(multi_mouses[0].MouseX < 108)
										x = 108;
									else if(multi_mouses[0].MouseX > 747)// 854 - 640 = 214 / 2 = 107  854 - 107 = 747
										x = 747;
									else
										x = multi_mouses[0].MouseX;
										y = multi_mouses[0].MouseY;
										if (!(Key_GetCatcher() & KEYCATCH_CGAME)) {
											x -= (glConfig.vidWidth - SCREEN_WIDTH) / 2;
											y -= (glConfig.vidHeight - SCREEN_HEIGHT) / 2;
										}
								}
								else if(xi_event -> evtype == XI_ButtonPress || xi_event -> evtype == XI_ButtonRelease)
								{
									//if(motionPressed != (qboolean) (multi_mouses[0].ButtonState))
									{
										int time = Sys_Milliseconds();
										motionPressed = (qboolean)(multi_mouses[0].ButtonState);
										if (Key_GetCatcher() & (KEYCATCH_CGAME | KEYCATCH_UI)) {
											Com_QueueEvent(time, SE_KEY, K_MOUSE1, motionPressed, 0, NULL);
										}
									}
								}
								//LookupMultiButtonState();
#endif
								break;
							case XI_DeviceChanged:
								xi_changeevent = (XIDeviceChangedEvent *)event.xcookie.data;
								X11_XInput2_DispatchDeviceChangedEvent(xi_changeevent);
								break;
							default:
#ifdef DEBUG_XEVENTS
								printf("Unhandled XInput2 event %d\n", xevent->evtype);
#endif
								break;
						}
						XFreeEventData(dpy, &event.xcookie);
					}
				}
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
}

void IN_Frame(void)
{
	qboolean loading;

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

#ifdef _HARMATTAN
	InitVirtualButton();
#endif
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

