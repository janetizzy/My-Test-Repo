
#include "visual_lua.h"

static TAPTR checkinstptr(lua_State *L, int n, const char *classname)
{
	TAPTR p = luaL_checkudata(L, n, classname);
	if (p == TNULL) luaL_argerror(L, n, "Closed handle");
	return p;
}

#define getvisptr(L, n) luaL_checkudata(L, n, TEK_LIB_VISUAL_CLASSNAME)
#define getpenptr(L, n) luaL_checkudata(L, n, TEK_LIB_VISUALPEN_CLASSNAME)
#define checkvisptr(L, n) checkinstptr(L, n, TEK_LIB_VISUAL_CLASSNAME)
#define checkpenptr(L, n) checkinstptr(L, n, TEK_LIB_VISUALPEN_CLASSNAME)
#define checkfontptr(L, n) checkinstptr(L, n, TEK_LIB_VISUALFONT_CLASSNAME)

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_wait(lua_State *L)
{
	TEKVisual *vis;
	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	TWait(TGetPortSignal(vis->vis_IMsgPort));
	lua_pop(L, 1);
	return 0;
}

/*****************************************************************************/
/*
**	Sleep specified number of microseconds
*/

LOCAL LUACFUNC TINT
tek_lib_visual_sleep(lua_State *L)
{
	TEKVisual *vis;
	TTIME dt;
	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	dt.tdt_Int64 = luaL_checknumber(L, 1) * 1000;
	TWaitTime(&dt, 0);
	lua_pop(L, 1);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_gettime(lua_State *L)
{
	TEKVisual *vis;
	TTIME dt;
	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	TGetSystemTime(&dt);
	lua_remove(L, -3);
	lua_pushinteger(L, dt.tdt_Int64 / 1000000);
	lua_pushinteger(L, dt.tdt_Int64 % 1000000);
	return 2;
}

/*****************************************************************************/
/*
**	openfont(name, pxsize)
*/

LOCAL LUACFUNC TINT
tek_lib_visual_openfont(lua_State *L)
{
	TTAGITEM ftags[5], *tp = ftags;
	TEKVisual *vis;
	TEKFont *font;
	TSTRPTR name = (TSTRPTR) luaL_optstring(L, 1, "");
	TINT size = luaL_optinteger(L, 2, -1);

	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	vis = lua_touserdata(L, -1);
	lua_pop(L, 1);

	if (name && name[0] != 0)
	{
		tp->tti_Tag = TVisual_FontName;
		tp++->tti_Value = (TTAG) name;
	}

	if (size > 0)
	{
		tp->tti_Tag = TVisual_FontPxSize;
		tp++->tti_Value = (TTAG) size;
	}

	tp->tti_Tag = TVisual_Display;
	tp++->tti_Value = (TTAG) vis->vis_Display;

	tp->tti_Tag = TTAG_DONE;

	/* reserve userdata for a collectable object: */
	font = lua_newuserdata(L, sizeof(TEKFont));
	/* s: fontdata */
	font->font_Font = TVisualOpenFont(vis->vis_Base, ftags);
	if (font->font_Font)
	{
		font->font_VisBase = vis->vis_Base;

		ftags[0].tti_Tag = TVisual_FontHeight;
		ftags[0].tti_Value = (TTAG) &font->font_Height;
		ftags[1].tti_Tag = TVisual_FontUlPosition;
		ftags[1].tti_Value = (TTAG) &font->font_UlPosition;
		ftags[2].tti_Tag = TVisual_FontUlThickness;
		ftags[2].tti_Value = (TTAG) &font->font_UlThickness;
		ftags[3].tti_Tag = TTAG_DONE;

		if (TVisualGetFontAttrs(vis->vis_Base, font->font_Font, ftags) == 3)
		{
			TDBPRINTF(TDB_INFO,("Height: %d - Pos: %d - Thick: %d\n",
				font->font_Height, font->font_UlPosition,
				font->font_UlThickness));

			/* attach class metatable to userdata object: */
			luaL_newmetatable(L, TEK_LIB_VISUALFONT_CLASSNAME);
			/* s: fontdata, meta */
			lua_setmetatable(L, -2);
			/* s: fontdata */
			lua_pushinteger(L, font->font_Height);
			/* s: fontdata, height */
			return 2;
		}

		TDestroy(font->font_Font);
	}

	lua_pop(L, 1);
	lua_pushnil(L);
	return 1;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_closefont(lua_State *L)
{
	TEKFont *font = luaL_checkudata(L, 1, TEK_LIB_VISUALFONT_CLASSNAME);
	if (font->font_Font)
	{
		TVisualCloseFont(font->font_VisBase, font->font_Font);
		font->font_Font = TNULL;
	}
	return 0;
}

/*****************************************************************************/
/*
**	return width, height of the specified font and text
*/

LOCAL LUACFUNC TINT
tek_lib_visual_textsize_font(lua_State *L)
{
	TEKFont *font = checkfontptr(L, 1);
	TSTRPTR s = (TSTRPTR) luaL_checkstring(L, 2);
	lua_pushinteger(L,
		TVisualTextSize(font->font_VisBase, font->font_Font, s));
	lua_pushinteger(L, font->font_Height);
	return 2;
}

/*****************************************************************************/
/*
**	set font attributes in passed (or newly created) table
*/

LOCAL LUACFUNC TINT
tek_lib_visual_getfontattrs(lua_State *L)
{
	TEKFont *font = checkfontptr(L, 1);
	if (lua_type(L, 2) == LUA_TTABLE)
		lua_pushvalue(L, 2);
	else
		lua_newtable(L);
	lua_pushinteger(L, font->font_Height);
	lua_setfield(L, -2, "Height");
	lua_pushinteger(L, font->font_Height - font->font_UlPosition);
	lua_setfield(L, -2, "UlPosition");
	lua_pushinteger(L, font->font_UlThickness);
	lua_setfield(L, -2, "UlThickness");
	return 1;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setinput(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TUINT mask = (TUINT) lua_tointeger(L, 2);
	TVisualSetInput(vis->vis_Visual, 0, mask);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_clearinput(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TUINT mask = (TUINT) lua_tointeger(L, 2);
	TVisualSetInput(vis->vis_Visual, mask, 0);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_getmsg(lua_State *L)
{
	TEKVisual *vis;
	TIMSG *imsg;
	lua_getfield(L, LUA_REGISTRYINDEX, TEK_LIB_VISUAL_BASECLASSNAME);
	/* s: visbase */
	vis = lua_touserdata(L, -1);
	imsg = (TIMSG *) TGetMsg(vis->vis_IMsgPort);
	if (imsg)
	{
		if (lua_istable(L, 1))
			lua_pushvalue(L, 1);
		else
		{
			TINT size = 8;
			if (imsg->timsg_Type == TITYPE_REFRESH)
				size += 4;
			else if (imsg->timsg_Type == TITYPE_KEYUP ||
				imsg->timsg_Type == TITYPE_KEYDOWN)
				size++;
			lua_createtable(L, size, 0);
		}

		lua_getmetatable(L, -2);
		/* s: msgtab, reftab */
		if (imsg->timsg_UserData > 0)
		{
			/* If we have a userdata, we regard it as an index into the
			visual metatable, referencing a visual: */
			TEKVisual *refvis;
			lua_rawgeti(L, -1, (int) imsg->timsg_UserData);
			/* s: msgtable, reftab, visual */
			refvis = lua_touserdata(L, -1);
			/* from there, we retrieve the visual's userdata, which
			stored as a reference in the same table: */
			lua_rawgeti(L, -2, refvis->vis_refUserData);
			/* s: msgtable, reftab, visual, userdata */
			lua_remove(L, -2);
			/* s: msgtable, reftab, userdata */
		}
		else
		{
			/* otherwise, we retrieve a "raw" user data package: */
			lua_pushlstring(L, (void *) (imsg + 1), imsg->timsg_ExtraSize);
		}

		/* store the userdata in the message at index -1: */

		lua_rawseti(L, -3, -1);
		/* s: msgtable, reftab */

		lua_pop(L, 1);
		/* s: msgtable */

		lua_pushinteger(L, imsg->timsg_TimeStamp.tdt_Int64 % 1000000);
		lua_rawseti(L, -2, 0);
		lua_pushinteger(L, imsg->timsg_TimeStamp.tdt_Int64 / 1000000);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, imsg->timsg_Type);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, imsg->timsg_Code);
		lua_rawseti(L, -2, 3);
		lua_pushinteger(L, imsg->timsg_MouseX);
		lua_rawseti(L, -2, 4);
		lua_pushinteger(L, imsg->timsg_MouseY);
		lua_rawseti(L, -2, 5);
		lua_pushinteger(L, imsg->timsg_Qualifier);
		lua_rawseti(L, -2, 6);

		/* extra information depending on event type: */
		switch (imsg->timsg_Type)
		{
			case TITYPE_REFRESH:
				lua_pushinteger(L, imsg->timsg_X);
				lua_rawseti(L, -2, 7);
				lua_pushinteger(L, imsg->timsg_Y);
				lua_rawseti(L, -2, 8);
				lua_pushinteger(L, imsg->timsg_X + imsg->timsg_Width - 1);
				lua_rawseti(L, -2, 9);
				lua_pushinteger(L, imsg->timsg_Y + imsg->timsg_Height - 1);
				lua_rawseti(L, -2, 10);
				break;
			case TITYPE_KEYUP:
			case TITYPE_KEYDOWN:
				/* UTF-8 representation of keycode: */
				lua_pushstring(L, (const char *) imsg->timsg_KeyCode);
				lua_rawseti(L, -2, 7);
				break;
		}

		/* s: visbase, desttable */
		TAckMsg(imsg);
		lua_remove(L, -2);
		return 1;
	}
	lua_pop(L, 1);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_allocpen(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT r = luaL_checkinteger(L, 2);
	TINT g = luaL_checkinteger(L, 3);
	TINT b = luaL_checkinteger(L, 4);
	TEKPen *pen = lua_newuserdata(L, sizeof(TEKPen));
	/* s: pendata */
	pen->pen_Pen = TVPEN_UNDEFINED;
	/* attach class metatable to userdata object: */
	luaL_newmetatable(L, TEK_LIB_VISUALPEN_CLASSNAME);
	/* s: pendata, meta */
	lua_setmetatable(L, -2);
	/* s: pendata */
	r = TCLAMP(0, r, 255);
	g = TCLAMP(0, g, 255);
	b = TCLAMP(0, b, 255);
	pen->pen_Pen = TVisualAllocPen(vis->vis_Visual, (r << 16) | (g << 8) | b);
	pen->pen_Visual = vis;
	return 1;
}

LOCAL LUACFUNC TINT
tek_lib_visual_freepen(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TEKPen *pen = getpenptr(L, 2);
	if (pen->pen_Pen != TVPEN_UNDEFINED)
	{
		if (vis != pen->pen_Visual)
			luaL_argerror(L, 2, "Pen not from visual");
		TVisualFreePen(vis->vis_Visual, pen->pen_Pen);
		pen->pen_Pen = TVPEN_UNDEFINED;
	}
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_rect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen = checkpenptr(L, 6);
	TVisualRect(vis->vis_Visual, x0, y0, x1 - x0 + 1, y1 - y0 + 1,
		pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_frect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen;

	if (x0 > x1 || y0 > y1)
		return 0;
	
	if (lua_type(L, 6) == LUA_TSTRING)
	{
		size_t srclen;
		const char *srcbuf = luaL_checklstring(L, 6, &srclen);
		int tw, h, maxv;
	
		if (sscanf(srcbuf, "P6\n%d %d\n%d\n", &tw, &h, &maxv) == 3 ||
			sscanf(srcbuf, "P6\n#%*80[^\n]\n%d %d\n%d\n", &tw, &h, &maxv) == 3)
		{
			if (maxv > 0 && maxv < 256)
			{
				TINT w = x1 - x0 + 1;
				TUINT *buf;
				srcbuf += srclen - 3 * tw * h;
				buf = TExecAlloc(vis->vis_ExecBase, TNULL, w * h * sizeof(TUINT));
				if (buf)
				{
					TUINT8 r, g, b;
					int x, y;
					for (y = 0; y < h; ++y)
					{
						TUINT *bp = buf + y * w;
						for (x = 0; x < w; ++x)
						{
							r = *srcbuf++;
							g = *srcbuf++;
							b = *srcbuf++;
							*bp++ = (r << 16) | (g << 8) | b;
						}
						srcbuf += (tw - w) * 3;
					}
					TVisualDrawBuffer(vis->vis_Visual, x0, y0, buf, w, h, w, TNULL);
					TExecFree(vis->vis_ExecBase, buf);
				}
			}
		}
	}
	else
	{
		pen = checkpenptr(L, 6);
		TVisualFRect(vis->vis_Visual, x0, y0, x1 - x0 + 1, y1 - y0 + 1,
			pen->pen_Pen);
	}

	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_line(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT x1 = luaL_checkinteger(L, 4) + sx;
	TINT y1 = luaL_checkinteger(L, 5) + sy;
	TEKPen *pen = checkpenptr(L, 6);
	TVisualLine(vis->vis_Visual, x0, y0, x1, y1, pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_plot(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TEKPen *pen = checkpenptr(L, 4);
	TVisualPlot(vis->vis_Visual, x0, y0, pen->pen_Pen);
	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_text(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	size_t tlen;
	TSTRPTR text = (TSTRPTR) luaL_checklstring(L, 4, &tlen);
	TEKPen *fpen = checkpenptr(L, 5);
	TVPEN bpen = TVPEN_UNDEFINED;
	if (lua_isuserdata(L, 6))
		bpen = ((TEKPen *) checkpenptr(L, 6))->pen_Pen;
	TVisualText(vis->vis_Visual, x0, y0, text, tlen, fpen->pen_Pen, bpen);
	return 0;
}

/*****************************************************************************/
/*
**	drawimage(visual, image, r1, r2, r3, r4, pentab, override_pen)
**
**	Layout of image data structure:
**
**	{
**		[1] = { x0, y0, x1, y1, ... }, -- coordinates (x/y)
**		[2] = {  -- primitives
**			{ [1]=fmtcode, [2]=numpts, [3]={ indices }, [4]=pen_or_pentable },
**			...
**		}
**		[3] = boolean -- is_transparent
**	}
**
**	format codes:
**		0x1000 - strip
**		0x2000 - fan
*/

LOCAL LUACFUNC TINT
tek_lib_visual_drawimage(lua_State *L)
{
	TEKPen *pen_override = TNULL;
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	lua_Integer rect[4], scalex, scaley;
	size_t primcount, i, j;
	TTAGITEM tags[2];
	tags[1].tti_Tag = TTAG_DONE;
	
	if (lua_type(L, 7) != LUA_TTABLE)
		pen_override = luaL_checkudata(L, 7, TEK_LIB_VISUALPEN_CLASSNAME);
	
	rect[0] = luaL_checkinteger(L, 3);
	rect[1] = luaL_checkinteger(L, 4);
	rect[2] = luaL_checkinteger(L, 5);
	rect[3] = luaL_checkinteger(L, 6);
	scalex = rect[2] - rect[0];
	scaley = rect[1] - rect[3];
	
	lua_rawgeti(L, 2, 1);
	/* s: coords */
	lua_rawgeti(L, 2, 2);
	/* s: coords, primitives */
	primcount = lua_objlen(L, -1);

	for (i = 0; i < primcount; i++)
	{
		lua_Integer nump, fmt;
		size_t bufsize;
		void *buf;
		TINT *coord;
		TINT *pentab;
		
		lua_rawgeti(L, -1, i + 1);
		/* s: coords, primitives, prim[i] */
		lua_rawgeti(L, -1, 1);
		/* s: coords, primitives, prim[i], fmtcode */
		fmt = luaL_checkinteger(L, -1);
		lua_rawgeti(L, -2, 2);
		/* s: coords, primitives, prim[i], fmtcode, nump */
		nump = luaL_checkinteger(L, -1);
		
		bufsize = sizeof(TINT) * 3 * nump;
		buf = vis->vis_VisBase->vis_DrawBuffer;
		if (buf && TGetSize(buf) < bufsize)
		{
			TFree(buf);
			buf = TNULL;
		}
		if (buf == TNULL)
			buf = TAlloc(TNULL, bufsize);
		vis->vis_VisBase->vis_DrawBuffer = buf;
		if (buf == TNULL)
		{
			lua_pushstring(L, "out of memory");
			lua_error(L);
		}
		coord = buf;
		
		lua_rawgeti(L, -3, 3);
		/* s: coords, primitives, prim[i], fmtcode, nump, indices */
		lua_rawgeti(L, -4, 4);
		/* s: coords, primitives, prim[i], fmtcode, nump, indices, pt */
		
		pentab = lua_type(L, -1) == LUA_TTABLE ? coord + 2 * nump : TNULL;
		if (pentab)
			tags[0].tti_Tag = TVisual_PenArray;
		else
		{
			tags[0].tti_Tag = TVisual_Pen;
			if (pen_override)
				tags[0].tti_Value = pen_override->pen_Pen;
			else
			{
				lua_Integer pnr = lua_tonumber(L, -1);
				lua_rawgeti(L, 7, pnr);
				tags[0].tti_Value = ((TEKPen *) checkpenptr(L, -1))->pen_Pen;
				lua_pop(L, 1);
			}
		}
		
		for (j = 0; j < (size_t) nump; ++j)
		{
			lua_Integer idx;
			lua_rawgeti(L, -2, j + 1);
			idx = lua_tointeger(L, -1);
			lua_rawgeti(L, -8, idx * 2 - 1);
			lua_rawgeti(L, -9, idx * 2);
			/* index, x, y */
			coord[j * 2] = rect[0] + sx + 
				(lua_tointeger(L, -2) * scalex) / 0x10000;
			coord[j * 2 + 1] = rect[3] + sy +
				(lua_tointeger(L, -1) * scaley) / 0x10000;
			if (pentab)
			{
				lua_rawgeti(L, -7, idx + 1);
				pentab[j] = ((TEKPen *) checkpenptr(L, -1))->pen_Pen;
				lua_pop(L, 4);
			}
			else
				lua_pop(L, 3);
		}
		
		switch (fmt & 0xf000)
		{
			case 0x1000:
			case 0x4000:
				TVisualDrawStrip(vis->vis_Visual, coord, nump, tags);
				break;
			case 0x2000:
				TVisualDrawFan(vis->vis_Visual, coord, nump, tags);
				break;
		}
		
		lua_pop(L, 5);
	}
	
	lua_pop(L, 2);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_getattrs(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TTAGITEM tags[5];
	TINT pw, ph, x, y;

	tags[0].tti_Tag = TVisual_Width;
	tags[0].tti_Value = (TTAG) &pw;
	tags[1].tti_Tag = TVisual_Height;
	tags[1].tti_Value = (TTAG) &ph;
	tags[2].tti_Tag = TVisual_WinLeft;
	tags[2].tti_Value = (TTAG) &x;
	tags[3].tti_Tag = TVisual_WinTop;
	tags[3].tti_Value = (TTAG) &y;
	tags[4].tti_Tag = TTAG_DONE;

	TVisualGetAttrs(vis->vis_Visual, tags);

	lua_pushinteger(L, pw);
	lua_pushinteger(L, ph);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	return 4;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_getuserdata(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	if (vis->vis_refUserData >= 0)
	{
		lua_getmetatable(L, 1);
		/* s: metatable */
		lua_rawgeti(L, -1, vis->vis_refUserData);
		/* s: metatable, userdata */
		lua_remove(L, -2);
	}
	else
		lua_pushnil(L);
	return 1;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setattrs(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TTAGITEM tags[7], *tp = tags;

	lua_getfield(L, 2, "MinWidth");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MinWidth;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MinHeight");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MinHeight;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MaxWidth");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MaxWidth;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	lua_getfield(L, 2, "MaxHeight");
	if (lua_isnumber(L, -1) || lua_isnil(L, -1))
	{
		tp->tti_Tag = (TTAG) TVisual_MaxHeight;
		tp->tti_Value = (TTAG) lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
		tp++;
		lua_pop(L, 1);
	}

	tp->tti_Tag = TTAG_DONE;
	lua_pushnumber(L, TVisualSetAttrs(vis->vis_Visual, tags));

	return 1;
}

/*****************************************************************************/
/*
**	textsize_visual: return text width, height using the current font
*/

LOCAL LUACFUNC TINT
tek_lib_visual_textsize_visual(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TSTRPTR s = (TSTRPTR) luaL_checkstring(L, 2);
	lua_pushinteger(L,
		TVisualTextSize(vis->vis_Base, vis->vis_Font, s));
	lua_pushinteger(L, vis->vis_FontHeight);
	return 2;
}

/*****************************************************************************/
/*
**	setfont(font): attach a font to a visual
*/

LOCAL LUACFUNC TINT
tek_lib_visual_setfont(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TEKFont *font = checkfontptr(L, 2);
	if (font->font_Font && vis->vis_Font != font->font_Font)
	{
		lua_getmetatable(L, 1);
		/* s: vismeta */

		if (vis->vis_refFont != -1)
		{
			/* unreference old current font: */
			luaL_unref(L, -1, vis->vis_refFont);
			vis->vis_refFont = -1;
		}

		TVisualSetFont(vis->vis_Visual, font->font_Font);
		vis->vis_Font = font->font_Font;
		vis->vis_FontHeight = font->font_Height;

		/* reference new font: */
		lua_pushvalue(L, 2);
		/* s: vismeta, font */
		vis->vis_refFont = luaL_ref(L, -2);
		/* s: vismeta */
		lua_pop(L, 1);
	}
	return 0;
}

/*****************************************************************************/

static TTAG hookfunc(struct THook *hook, TAPTR obj, TTAG msg)
{
	TEKVisual *vis = hook->thk_Data;
	TINT *rect = (TINT *) msg;
	TINT *newbuf = vis->vis_RectBuffer ?
		TExecRealloc(vis->vis_ExecBase, vis->vis_RectBuffer,
			(vis->vis_RectBufferNum + 4) * sizeof(TINT)) :
		TExecAlloc(vis->vis_ExecBase, TNULL, sizeof(TINT) * 4);

	if (newbuf)
	{
		vis->vis_RectBuffer = newbuf;
		newbuf += vis->vis_RectBufferNum;
		vis->vis_RectBufferNum += 4;
		newbuf[0] = rect[0];
		newbuf[1] = rect[1];
		newbuf[2] = rect[2];
		newbuf[3] = rect[3];
	}

	return 0;
}

LOCAL LUACFUNC TINT
tek_lib_visual_copyarea(lua_State *L)
{
	TTAGITEM tags[2], *tp = TNULL;
	struct THook hook;

	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x = luaL_checkinteger(L, 2) + sx;
	TINT y = luaL_checkinteger(L, 3) + sy;
	TINT w = luaL_checkinteger(L, 4);
	TINT h = luaL_checkinteger(L, 5);
	TINT dx = luaL_checkinteger(L, 6) + sx;
	TINT dy = luaL_checkinteger(L, 7) + sy;

	if (lua_istable(L, 8))
	{
		vis->vis_RectBuffer = TNULL;
		vis->vis_RectBufferNum = 0;
		TInitHook(&hook, hookfunc, vis);
		tags[0].tti_Tag = TVisual_ExposeHook;
		tags[0].tti_Value = (TTAG) &hook;
		tags[1].tti_Tag = TTAG_DONE;
		tp = tags;
	}

	TVisualCopyArea(vis->vis_Visual, x, y, w, h, dx, dy, tp);

	if (tp)
	{
		TINT i;
		for (i = 0; i < vis->vis_RectBufferNum; ++i)
		{
			lua_pushinteger(L, vis->vis_RectBuffer[i]);
			lua_rawseti(L, 8, i + 1);
		}
		TExecFree(vis->vis_ExecBase, vis->vis_RectBuffer);
		vis->vis_RectBuffer = TNULL;
	}

	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setcliprect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT x = luaL_checkinteger(L, 2);
	TINT y = luaL_checkinteger(L, 3);
	TINT w = luaL_checkinteger(L, 4);
	TINT h = luaL_checkinteger(L, 5);
	TVisualSetClipRect(vis->vis_Visual, x, y, w, h, TNULL);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_unsetcliprect(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TVisualUnsetClipRect(vis->vis_Visual);
	return 0;
}

/*****************************************************************************/

LOCAL LUACFUNC TINT
tek_lib_visual_setshift(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = luaL_checkinteger(L, 2);
	TINT sy = luaL_checkinteger(L, 3);
	vis->vis_ShiftX += sx;
	vis->vis_ShiftY += sy;
	return 0;
}

/*****************************************************************************/
/*
**	drawrgb(visual, x0, y0, table, width, height, pixwidth, pixheight)
*/

LOCAL LUACFUNC TINT
tek_lib_visual_drawrgb(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 2) + sx;
	TINT y0 = luaL_checkinteger(L, 3) + sy;
	TINT w = luaL_checkinteger(L, 5);
	TINT h = luaL_checkinteger(L, 6);
	TINT pw = luaL_checkinteger(L, 7);
	TINT ph = luaL_checkinteger(L, 8);
	TUINT *buf;
	TINT bw = w * pw;
	TINT bh = h * ph;

	luaL_checktype(L, 4, LUA_TTABLE);

	buf = TExecAlloc(vis->vis_ExecBase, TNULL, bw * bh * sizeof(TUINT));
	if (buf)
	{
		TUINT rgb;
		TUINT *p = buf;
		TINT i = 0;
		TINT xx, yy, x, y;

		for (y = 0; y < h; ++y)
		{
			TUINT *lp = p;
			for (x = 0; x < w; ++x)
			{
				lua_rawgeti(L, 4, i++);
				rgb = lua_tointeger(L, -1);
				lua_pop(L, 1);
				for (xx = 0; xx < pw; ++xx)
					*p++ = rgb;
			}

			for (yy = 0; yy < ph - 1; ++yy)
			{
				TExecCopyMem(vis->vis_ExecBase, lp, p, bw * sizeof(TUINT));
				p += bw;
			}
		}

		TVisualDrawBuffer(vis->vis_Visual, x0, y0, buf, bw, bh, bw, TNULL);

		TExecFree(vis->vis_ExecBase, buf);
	}

	return 0;
}

/*****************************************************************************/
/*
**	drawppm(visual, ppm, x0, y0)
*/

LOCAL LUACFUNC TINT
tek_lib_visual_drawppm(lua_State *L)
{
	TEKVisual *vis = checkvisptr(L, 1);
	size_t srclen;
	const char *srcbuf = luaL_checklstring(L, 2, &srclen);
	TINT sx = vis->vis_ShiftX, sy = vis->vis_ShiftY;
	TINT x0 = luaL_checkinteger(L, 3) + sx;
	TINT y0 = luaL_checkinteger(L, 4) + sy;
	int w, h, maxv;

	if (sscanf(srcbuf, "P6\n%d %d\n%d\n", &w, &h, &maxv) == 3 ||
		sscanf(srcbuf, "P6\n#%*80[^\n]\n%d %d\n%d\n", &w, &h, &maxv) == 3)
	{
		if (maxv > 0 && maxv < 256)
		{
			TUINT *buf;
			srcbuf += srclen - 3 * w * h;
			buf = TExecAlloc(vis->vis_ExecBase, TNULL, w * h * sizeof(TUINT));
			if (buf)
			{
				TUINT8 r, g, b;
				int i;
				for (i = 0; i < w * h; ++i)
				{
					r = *srcbuf++;
					g = *srcbuf++;
					b = *srcbuf++;
					buf[i] = (r << 16) | (g << 8) | b;
				}
				TVisualDrawBuffer(vis->vis_Visual, x0, y0, buf, w, h, w, TNULL);
				TExecFree(vis->vis_ExecBase, buf);
			}
		}
	}
	return 0;
}
