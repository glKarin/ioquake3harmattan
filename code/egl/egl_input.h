/*
 * Copyright (C) 2009  Nokia Corporation.  All rights reserved.
 */

#ifndef __EGL_INPUT_H__
#define __EGL_INPUT_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GLES/egl.h>
#include <GLES/gl.h>

#ifdef _HARMATTAN_PLUS
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#endif

#include "../qcommon/q_shared.h"

#ifdef _HARMATTAN_PLUS
extern XIDeviceInfo *xi_master;
extern int xi_opcode;
#define XI2_X11_ATOM_ID(id,name) id,
enum
{
	XI2_X11_ATOM_ID(AbsMTTrackingID, "Abs MT Tracking ID")
		XI2_X11_ATOM_ID(AbsMTPositionX, "Abs MT Position X")
		XI2_X11_ATOM_ID(AbsMTPositionY, "Abs MT Position Y")
		ATOM_COUNT
};

extern const char * atom_names[ATOM_COUNT];
extern Atom atoms[ATOM_COUNT];
#define atom(x) atoms[x]
int X11_XInput2_SetMasterPointer(int deviceid);
#endif

extern cvar_t *in_nograb;
qboolean IN_MotionPressed(void);
void IN_ActivateMouse(void);
void IN_DeactivateMouse(void);
void IN_Frame(void);
void IN_Init(void);
void IN_Shutdown(void);
void IN_Restart(void);

#endif
