#ifndef _KARIN_GL_VKB_H
#define _KARIN_GL_VKB_H

#include "vkb.h"

void karinNewVKB(float x, float y, float z, float w, float h);
void karinDeleteVKB(void);
void karinRenderVKB(void);
unsigned karinVKBMouseMotionEvent(int b, int p, int x, int y, int dx, int dy, VKB_Key_Action_Function f);
unsigned karinVKBMouseEvent(int b, int p, int x, int y, VKB_Key_Action_Function f);

#endif
