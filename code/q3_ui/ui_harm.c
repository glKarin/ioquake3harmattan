#ifdef _HARMATTAN_3_EXT

#include "ui_local.h"

#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"	
#define ART_FIGHT0			"menu/art/load_0"
#define ART_FIGHT1			"menu/art/load_1"
#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"

#define ID_BACK				10

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menucommon_s		content;

	menubitmap_s	back;
} harm_t;

static harm_t	s_harm;

// return skip distance on y.
static float karinRenderString(const char *str, int x, int y, unsigned style, vec4_t color, float spacing, unsigned int max_per_line, int *ret)
{
	if(!str || !color)
	{
		if(ret)
			*ret = -1;
		return 0.0;
	}

	if(max_per_line == 0)
	{
		UI_DrawProportionalString(x, y, str, style, color);
		if(ret)
			*ret = 1;
		return 0.0;
	}

	char *str_tmp = strdup(str);
	char *ptr = str_tmp;
	char ch = '\0';
	unsigned int lines = 0;
	float y_move = y;
	int i;
	float s = spacing < 0.0 ? -spacing : spacing;

	while(*ptr)
	{
		i = 0;
		while(*(ptr + i) && i < 40)
		{
			if(ptr[i] != '\0' && isspace(ptr[i]))
				ch = (char)i;
			i++;
		}
		if(ptr[i] != '\0' && !isspace(ptr[i]))
			i = ch + 1;
		ch = ptr[i];
		ptr[i] = '\0';
		lines++;
		UI_DrawProportionalString(x, y_move, ptr, style, color);
		if(ch == '\0')
			break;
		y_move += s;
		ptr[i] = ch;
		ptr = ptr + i;
	}
	free(str_tmp);
	if(ret)
		*ret = lines;
	return y_move;
}

static void UI_Harm_Draw( void *self ) {
	if(!self)
		return;

	const menucommon_s *widget = (menucommon_s *)self;

	int		y;
	const int _X = widget->x + 120;
	const float _Spacing = 1.5;
	const float _Small_Spacing = 1.0;
	float *_Color = color_white;
	char str[512];
	const unsigned _Style = UI_LEFT | UI_SMALLFONT;
	const char _Fmt_Str[] = "[%s]: %s";

	y = widget->y;
	UI_DrawProportionalString( 320, y, _HARMATTAN_APPNAME, (_Style & (~UI_LEFT)) | UI_CENTER, color_red );

	sprintf( str, _Fmt_Str, "Package", _HARMATTAN_PKG);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( _X, y, str, _Style, _Color );

	sprintf( str, "[%s]: %s-%s", "Version", _HARMATTAN_VER, _HARMATTAN_PATCH);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( _X, y, str, _Style, _Color );

	sprintf( str, _Fmt_Str, "Code", _HARMATTAN_DEVCODE);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( _X, y, str, _Style, _Color );

	sprintf( str, _Fmt_Str, "Dev", _HARMATTAN_DEV);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( _X, y, str, _Style, _Color );

	sprintf( str, _Fmt_Str, "Release", _HARMATTAN_RELEASE);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	UI_DrawProportionalString( _X, y, str, _Style, _Color );

	sprintf( str, _Fmt_Str, "Desc", _HARMATTAN_DESC);
	y += _Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE;
	y += karinRenderString(str, _X, y, _Style, _Color, _Small_Spacing * PROP_HEIGHT * PROP_SMALL_SIZE_SCALE, 40, NULL);

}

/*
===============
UI_Harm_MenuEvent
===============
*/
static void UI_Harm_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch ( ((menucommon_s*)ptr)->id ) {
	case ID_BACK:
		UI_PopMenu();
		break;
	}
}

/*
===============
UI_Harm_MenuInit
===============
*/
static void UI_Harm_MenuInit( void ) {
	UI_HarmMenu_Cache();

	memset( &s_harm, 0 ,sizeof(harm_t) );
	s_harm.menu.wrapAround = qtrue;
	s_harm.menu.fullscreen = qtrue;

	s_harm.banner.generic.type		= MTYPE_BTEXT;
	s_harm.banner.generic.x			= 320;
	s_harm.banner.generic.y			= 16;
	s_harm.banner.string			= "HARMATTAN VER";
	s_harm.banner.color				= color_white;
	s_harm.banner.style				= UI_CENTER;

	s_harm.framel.generic.type		= MTYPE_BITMAP;
	s_harm.framel.generic.name		= ART_FRAMEL;
	s_harm.framel.generic.flags		= QMF_INACTIVE;
	s_harm.framel.generic.x			= 0;  
	s_harm.framel.generic.y			= 78;
	s_harm.framel.width				= 256;
	s_harm.framel.height			= 329;

	s_harm.framer.generic.type		= MTYPE_BITMAP;
	s_harm.framer.generic.name		= ART_FRAMER;
	s_harm.framer.generic.flags		= QMF_INACTIVE;
	s_harm.framer.generic.x			= 376;
	s_harm.framer.generic.y			= 76;
	s_harm.framer.width				= 256;
	s_harm.framer.height			= 334;

	s_harm.back.generic.type		= MTYPE_BITMAP;
	s_harm.back.generic.name		= ART_BACK0;
	s_harm.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_harm.back.generic.id			= ID_BACK;
	s_harm.back.generic.callback	= UI_Harm_MenuEvent;
	s_harm.back.generic.x			= 0;
	s_harm.back.generic.y			= 480-64;
	s_harm.back.width				= 128;
	s_harm.back.height				= 64;
	s_harm.back.focuspic			= ART_BACK1;

	// scan for harm
	s_harm.content.type		= MTYPE_ACTION;
	s_harm.content.flags		= QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s_harm.content.x			= 0;
	s_harm.content.y			= 130;
	s_harm.content.ownerdraw = UI_Harm_Draw;


	Menu_AddItem( &s_harm.menu, &s_harm.banner );
	Menu_AddItem( &s_harm.menu, &s_harm.framel );
	Menu_AddItem( &s_harm.menu, &s_harm.framer );
	Menu_AddItem( &s_harm.menu, &s_harm.content);
	Menu_AddItem( &s_harm.menu, &s_harm.back );
}

/*
=================
UI_Harm_Cache
=================
*/
void UI_HarmMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_FIGHT0 );
	trap_R_RegisterShaderNoMip( ART_FIGHT1 );
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
}


/*
===============
UI_HarmMenu
===============
*/
void UI_HarmMenu( void ) {
	UI_Harm_MenuInit();
	UI_PushMenu( &s_harm.menu );
}

#endif
