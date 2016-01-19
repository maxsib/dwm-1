/* See LICENSE file for copyright and license details. */
#include "gaplessgrid.c"
#include "selfrestart.c"
#include "zoomswap.c"

/* mbp-mappings */
#define XF86AudioMute           0x1008ff12
#define XF86AudioLowerVolume    0x1008ff11
#define XF86AudioRaiseVolume    0x1008ff13
#define XF86TouchpadToggle		0x1008ffa9

/* appearance */
static const char *fonts[] = {
	"Input Mono Condensed:size=10",
	"i3fonticon:size=10"
};

static const unsigned int borderpx  		= 3;   /* border pixel of windows */
static const unsigned int snap      		= 32;  /* snap pixel */
static const unsigned int gappx 			= 12;  /* gap pixel between windows */
static const unsigned int systraypinning 	= 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing 	= 2;   /* systray spacing */
static const int systraypinningfailfirst 	= 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        		= 1;   /* 0 means no systray */
static const int showbar            		= 1;   /* 0 means no bar */
static const int topbar             		= 1;   /* 0 means bottom bar */

#define NUMCOLORS 9
static const char colors[NUMCOLORS][MAXCOLORS][9] = {
	// border foreground background
	{ "#212121", "#696969", "#121212" }, // 0 = normal
	{ "#696969", "#E0E0E0", "#121212" }, // 1 = selected
	{ "#212121", "#CF4F88", "#121212" }, // 2 = red
	{ "#212121", "#53A6A6", "#121212" }, // 3 = green
	{ "#212121", "#914E89", "#121212" }, // 4 = yellow
	{ "#212121", "#4779B3", "#121212" }, // 5 = blue
	{ "#212121", "#47959E", "#121212" }, // 6 = cyan
	{ "#212121", "#7E62B3", "#121212" }, // 7 = magenta
	{ "#212121", "#899CA1", "#121212" }, // 8 = grey
};

/* tagging */
static const char *tags[] = { "web", "term", "code", "media", "game", "misc" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     	instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     	NULL,       NULL,       0,            0,           -1 },
	{ "Firefox",  	NULL,       NULL,       1 << 0,       0,           -1 },
	{ "Steam",  	NULL,       NULL,       1 << 4,       1,           -1 },
};

/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[T]",      tile },    /* first entry is default */
	{ "[F]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "[B]",      htile },
	{ "[G]",      gaplessgrid },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2]				= "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[]		= { "spawn_rofi", NULL };
static const char *termcmd[]		= { "st", NULL };
static const char *volup[]			= { "pulseaudio-ctl", "up", NULL };
static const char *voldown[]		= { "pulseaudio-ctl", "down", NULL };
static const char *voltoggle[]		= { "pulseaudio-ctl", "mute", NULL };
static const char *togtouchpad[]	= { "touchpad", NULL };
static const char *screenshot[]		= { "screenshot", NULL};


static Key keys[] = {
	/* modifier                     key        		        function        argument */
	{ MODKEY,                       XK_r,              		spawn,          {.v = dmenucmd } },
	{ MODKEY,     					XK_Return,    			runorraise,     {.v = termcmd } },
	{ MODKEY|ShiftMask,     		XK_b,              		togglebar,      {0} },
	{ MODKEY,                       XK_Right,               focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_Left,              	focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,               	incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,              		incnmaster,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_Left,              	setmfact,       {.f = -0.05} },
	{ MODKEY|ShiftMask,             XK_Right,              	setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Up,      			setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_Down,      			setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      				setcfact,       {.f =  0.00} },
	{ MODKEY,           			XK_Down,      			pushdown,       {0} },
    { MODKEY,           			XK_Up,      			pushup,         {0} },
	{ MODKEY|ShiftMask,     		XK_Return,    			zoom,           {0} },
	{ MODKEY,                       XK_Tab,         		view,           {0} },
	{ MODKEY|ShiftMask,     		XK_w,             		killclient,     {0} },
	{ MODKEY,                       XK_t,              		setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,              		setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,            		setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_b,            		setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_g,             		setlayout,      {.v = &layouts[4] } },
	{ MODKEY,                       XK_space,     			setlayout,      {0} },
	{ MODKEY|ShiftMask,     		XK_space,     			togglefloating, {0} },
	{ MODKEY,                       XK_0,              		view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,     		XK_0,              		tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,   			focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,     			focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,     		XK_comma,  				tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,     		XK_period,    			tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,					0)
	TAGKEYS(                        XK_2,					1)
	TAGKEYS(                        XK_3,					2)
	TAGKEYS(                        XK_4,					3)
	TAGKEYS(                        XK_5,					4)
	TAGKEYS(                        XK_6,					5)
	TAGKEYS(                        XK_7,					6)
	TAGKEYS(                        XK_8,					7)
	TAGKEYS(                        XK_9,					8)
	{ MODKEY|ShiftMask,    			XK_q,					quit,           {0} },
	{ MODKEY|ShiftMask,    			XK_r,					self_restart,   {0} },
    { 0,                            XK_Print,				spawn,          {.v = screenshot } },
	{ 0,                            XF86AudioRaiseVolume,	spawn,          {.v = volup } },
    { 0,                            XF86AudioLowerVolume,	spawn,          {.v = voldown } },
    { 0,                            XF86AudioMute,			spawn,          {.v = voltoggle } },
    { 0,                            XF86TouchpadToggle,		spawn,          {.v = togtouchpad } },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
