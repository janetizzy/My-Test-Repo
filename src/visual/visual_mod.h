#ifndef _TEK_MODS_VISUAL_MOD_H
#define _TEK_MODS_VISUAL_MOD_H

/*
**	teklib/src/visual/visual_mod.h - Visual module
**	Written by Timm S. Mueller <tmueller at neoscientists.org>
**	See copyright notice in teklib/COPYRIGHT
*/

#include <tek/debug.h>
#include <tek/exec.h>
#include <tek/teklib.h>
#include <tek/mod/visual.h>
#include <tek/inline/exec.h>
#include <tek/proto/display.h>
#if !defined(VISUAL_USE_INTERNAL_HASH)
#include <tek/proto/util.h>
#endif

/*****************************************************************************/

#define VISUAL_VERSION		4
#define VISUAL_REVISION		0
#define VISUAL_NUMVECTORS	41

#ifndef LOCAL
#define LOCAL
#endif

#ifndef EXPORT
#define EXPORT TMODAPI
#endif

#define VISUAL_MAXREQPERINSTANCE	64

/*****************************************************************************/

struct vis_Hash;

struct vis_HashNode
{
	struct TNode node;
	TSTRPTR key;
	TTAG value;
	TUINT hash;
};

struct vis_FontQueryHandle
{
	struct THandle vfq_Handle;
};

struct TVisualBase
{
	/* Module header: */
	struct TModule vis_Module;
	
	#if !defined(VISUAL_USE_INTERNAL_HASH)
	/* Util module base ptr: */
	TAPTR vis_UtilBase;
	#endif
	
	/* Module global memory manager (thread safe): */
	TAPTR vis_MemMgr;
	/* Locking for module base structure: */
	TAPTR vis_Lock;
	/* Number of module opens: */
	TUINT vis_RefCount;
	/* Flags: */
	TUINT vis_Flags;
	/* Hash of displays: */
	struct vis_Hash *vis_Displays;
	/* Instance-specific: */
	struct TVRequest *vis_InitRequest;
	/* Display: */
	TAPTR vis_Display;
	/* Window on the display: */
	TAPTR vis_Window;
	/* Current set of input types to listen for: */
	TUINT vis_InputMask;
	/* Request pool: */
	struct TList vis_ReqPool;
	/* Replyport for requests: */
	TAPTR vis_CmdRPort;
	/* Port for input messages: */
	TAPTR vis_IMsgPort;
	/* List of waiting asynchronous requests: */
	struct TList vis_WaitList;
	/* Number of requests allocated so far: */
	TINT vis_NumRequests;
};

#define TVISFL_CMDRPORT_OWNER	0x0001
#define TVISFL_IMSGPORT_OWNER	0x0002

/*****************************************************************************/

LOCAL struct vis_Hash *vis_createhash(struct TVisualBase *mod, TAPTR udata);
LOCAL void vis_destroyhash(struct TVisualBase *mod, struct vis_Hash *hash);
LOCAL int vis_puthash(struct TVisualBase *mod, struct vis_Hash *hash,
	const TSTRPTR key, TTAG value);
LOCAL int vis_gethash(struct TVisualBase *mod, struct vis_Hash *hash,
	const TSTRPTR key, TTAG *valp);
LOCAL int vis_remhash(struct TVisualBase *mod, struct vis_Hash *hash,
	const TSTRPTR key);
LOCAL TUINT vis_hashtolist(struct TVisualBase *mod, struct vis_Hash *hash,
	struct TList *list);
LOCAL void vis_hashunlist(struct TVisualBase *mod, struct vis_Hash *hash);

/*****************************************************************************/

EXPORT struct TVisualBase *vis_openvisual(struct TVisualBase *mod,
	TTAGITEM *tags);
EXPORT void vis_closevisual(struct TVisualBase *mod, struct TVisualBase *inst);
EXPORT struct TVisualBase *vis_attach(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT struct TVRequest *vis_openfont(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT void vis_closefont(struct TVisualBase *mod, struct TVRequest *font);
EXPORT TUINT vis_getfattrs(struct TVisualBase *mod, struct TVRequest *font,
	TTAGITEM *tags);
EXPORT TINT vis_textsize(struct TVisualBase *mod, struct TVRequest *font, TSTRPTR t);
EXPORT TAPTR vis_queryfonts(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT TTAGITEM *vis_getnextfont(struct TVisualBase *mod, TAPTR fqhandle);

EXPORT TAPTR vis_getport(struct TVisualBase *mod);
EXPORT TUINT vis_setinput(struct TVisualBase *mod, TUINT cmask, TUINT smask);
EXPORT TUINT vis_getattrs(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT TUINT vis_setattrs(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT TVPEN vis_allocpen(struct TVisualBase *mod, TUINT rgb);
EXPORT void vis_freepen(struct TVisualBase *mod, TVPEN pen);
EXPORT void vis_setfont(struct TVisualBase *mod, struct TVRequest *font);
EXPORT void vis_clear(struct TVisualBase *mod, TVPEN pen);
EXPORT void vis_rect(struct TVisualBase *mod, TINT x, TINT y, TINT w, TINT h,
	TVPEN pen);
EXPORT void vis_frect(struct TVisualBase *mod, TINT x, TINT y, TINT w, TINT h,
	TVPEN pen);
EXPORT void vis_line(struct TVisualBase *mod, TINT x1, TINT y1, TINT x2,
	TINT y2, TVPEN pen);
EXPORT void vis_plot(struct TVisualBase *mod, TINT x, TINT y, TVPEN pen);
EXPORT void vis_text(struct TVisualBase *mod, TINT x, TINT y, TSTRPTR t,
	TUINT l, TVPEN fg);

EXPORT void vis_drawstrip(struct TVisualBase *mod, TINT *array, TINT num,
	TTAGITEM *tags);
EXPORT void vis_drawtags(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT void vis_drawfan(struct TVisualBase *mod, TINT *array, TINT num,
	TTAGITEM *tags);

EXPORT void vis_copyarea(struct TVisualBase *mod, TINT x, TINT y, TINT w,
	TINT h, TINT dx, TINT dy, TTAGITEM *tags);
EXPORT void vis_setcliprect(struct TVisualBase *mod, TINT x, TINT y, TINT w,
	TINT h, TTAGITEM *tags);

EXPORT TAPTR vis_opendisplay(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT void vis_closedisplay(struct TVisualBase *mod, TAPTR display);
EXPORT TAPTR vis_querydisplays(struct TVisualBase *mod, TTAGITEM *tags);
EXPORT TTAGITEM *vis_getnextdisplay(struct TVisualBase *mod, TAPTR dqhandle);

EXPORT void vis_unsetcliprect(struct TVisualBase *mod);

EXPORT void vis_drawbuffer(struct TVisualBase *inst,
	TINT x, TINT y, TAPTR buf, TINT w, TINT h, TINT totw, TTAGITEM *tags);

#endif
