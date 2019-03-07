/*
 *
 *  This file is part of the XForms library package.
 *
 * XForms is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1, or
 * (at your option) any later version.
 *
 * XForms is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with XForms; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */


/*
 * $Id: forms.c,v 1.0 2002/05/08 05:16:30 zhao Release $
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  Main event dispatcher.
 *
 */

#if defined(F_ID) || defined(DEBUG)
char *fl_id_fm = "$Id: forms.c,v 1.0 2002/05/08 05:16:30 zhao Release $";
#endif

#include "forms.h"
#include "global.h"

static int select_form_event(Display *, XEvent *, char *);
static int get_next_event(int, FL_FORM **, XEvent *);
static void force_visible(FL_FORM * form, int itx, int ity);

static FL_FORM *fl_mainform;
static int nomainform;
static int reopened_group;
static int do_x_only;

static int fl_XLookupString(XKeyEvent *, char *, int, KeySym *);


#define SHORT_PAUSE          10	/* check_form wait             */


void
fl_set_no_connection(int yes)
{
    if((fl_no_connection = yes))
      fl_internal_init();
}


/* Starts a form definition */
FL_FORM *
fl_bgn_form(int type, FL_Coord w, FL_Coord h)
{
    if (!fl_no_connection && !flx->display)
    {
	M_err(0, "Missing or failed fl_initialize()");
	exit(0);
    }

    if (fl_current_form)
    {
	/* error actually is more serious and can't be fixed easily as it
	   might indicate a bad recursion */
	M_err("fl_bgn_form", "You forgot to call fl_end_form");
	exit(1);
    }

    fl_current_form = fl_make_form(w, h);
    fl_add_box(type, 0, 0, w, h, "");

    return fl_current_form;
}

/* Ends a form definition */
void
fl_end_form(void)
{
    if (fl_current_form == NULL)
	fl_error("fl_end_form", "Ending form definition of NULL form.");
    fl_current_form = NULL;
}

/* Reopens a form for input */
void
fl_addto_form(FL_FORM * form)
{
    if (fl_current_form != NULL)
	fl_error("fl_addto_form", "You forgot to call fl_end_form");

    if (form == NULL)
    {
	fl_error("fl_addto_form", "Adding to NULL form.");
	return;
    }
    fl_current_form = form;
}

/* Starts a group definition */
FL_OBJECT *
fl_bgn_group(void)
{
    static int id = 1;

    if (!fl_current_form)
    {
	fl_error("fl_bgn_group", "Starting group in NULL form.");
	return 0;
    }

    if (fl_current_group)
    {
	fl_error("fl_bgn_group", "You forgot to call fl_end_group.");
	fl_end_group();
    }

    fl_current_group = fl_make_object(FL_BEGIN_GROUP, 0, 0, 10, 10, 0, "", 0);
    fl_current_group->group_id = id++;
    fl_add_object(fl_current_form, fl_current_group);

    return fl_current_group;
}

/* Ends a group definition */
FL_OBJECT *
fl_end_group(void)
{
    FL_OBJECT *ob = fl_current_group;
    int id;

    if (fl_current_form == NULL)
    {
	fl_error("fl_end_group", "Ending group in NULL form.");
	return NULL;
    }

    if (fl_current_group == NULL)
    {
	fl_error("fl_end_group", "Ending NULL group.");
	return NULL;
    }

    id = fl_current_group->group_id;
    fl_current_group = NULL;

    if (!reopened_group)
    {
	ob = fl_make_object(FL_END_GROUP, 0, 0, 0, 0, 0, "", NULL);
	ob->group_id = id;
	fl_add_object(fl_current_form, ob);
    }

    if (reopened_group == 3)
	fl_end_form();
    reopened_group = 0;

    return ob;
}

/************************ Doing the Interaction *************************/

#define MAX_FORM	64

static FL_FORM *forms[MAX_FORM];	/* The forms being shown. */
static int formnumb;		/* Their number. */
static FL_FORM *mouseform;	/* The current form under mouse */
static FL_FORM *keyform;	/* keyboard focus form          */
FL_OBJECT *fl_pushobj;		/* latest pushed object */
FL_OBJECT *fl_mouseobj;		/* object under the mouse */

void
fl_freeze_all_forms(void)
{
    int i;
    for (i = 0; i < formnumb; i++)
	fl_freeze_form(forms[i]);
}

void
fl_unfreeze_all_forms(void)
{
    int i;
    for (i = 0; i < formnumb; i++)
	fl_unfreeze_form(forms[i]);
}


/* Corrects the shape of the form based on the shape of its window */
static void
reshape_form(FL_FORM * form)
{
    FL_Coord w, h;
    fl_get_wingeometry(form->window, &(form->x), &(form->y), &w, &h);
    fl_set_form_size(form, w, h);
}

static void
move_form(FL_FORM * form)
{
    if (form->visible > 0)
	XMoveWindow(flx->display, form->window, form->x, form->y);
}

static void
resize_form_win(FL_FORM * form, int neww, int newh)
{
    if (form->visible > 0)
	fl_winresize(form->window, neww, newh);
}

/* Scales a form with the given scaling factors. */
#if FL_CoordIsFloat
#define DELTA  (0.00001f)	/* must be less than 1/1000 */
#else
#define DELTA  (0.2f)		/* arbitary.  */
#endif

/* scale a form and take care of object gravity. This one differs from
   fl_scale_form in the fact that we don't reshape the window in any
   way. Most useful as a follow up to ConfigureNotify event
 */
static void
scale_form(FL_FORM * form, double xsc, double ysc)
{
    FL_OBJECT *obj;
    FL_Coord fromleft2, frombott1, frombott2;
    FL_Coord fromright1, fromright2, fromtop2;
    FL_Coord neww, newh;

    neww = (FL_Coord)(form->w * xsc + DELTA);
    newh = (FL_Coord)(form->h * ysc + DELTA);

    if (FL_abs(neww - form->w) < 1 && FL_abs(newh - form->h) < 1)
	return;

    if (form->hotx >= 0 || form->hoty >= 0)
    {
	form->hotx = (int)(form->hotx * xsc);
	form->hoty = (int)(form->hoty * ysc);
    }

    /* need to handle different resizing request */
    for (obj = form->first; obj; obj = obj->next)
    {
	/* calculate various positions before scaling */
	fromleft2 = obj->x + obj->w;

	fromright1 = form->w - obj->x;
	fromright2 = form->w - obj->x - obj->w;

	fromtop2 = obj->y + obj->h;

	frombott1 = form->h - obj->y;
	frombott2 = form->h - obj->y - obj->h;

	/* special case to keep the center of gravity */
	if (obj->resize == FL_RESIZE_NONE &&
	    obj->segravity == ForgetGravity &&
	    obj->nwgravity == ForgetGravity)
	{
	    float xx = (float) form->w / (obj->x + obj->w / 2);
	    float yy = (float) form->h / (obj->y + obj->h / 2);
	    obj->x =  (int)(neww / xx) - obj->w / 2;
	    obj->y =  (int)(newh / yy) - obj->h / 2;
	    continue;
	}

	if ((obj->resize & FL_RESIZE_X))
	{
	    FL_Coord xf = (FL_Coord)((obj->w + obj->x) * xsc + DELTA);
	    FL_Coord xi = (FL_Coord)(obj->x * xsc + DELTA);
	    obj->w = (FL_Coord)((xf - xi) + DELTA);
	}

	if ((obj->resize & FL_RESIZE_Y))
	{
	    FL_Coord yf = (FL_Coord)((obj->h + obj->y) * ysc + DELTA);
	    FL_Coord yi = (FL_Coord)(obj->y * ysc + DELTA);
	    obj->h = (FL_Coord)((yf - yi) + DELTA);
	}

	/* top-left corner of the object */
	switch (obj->nwgravity)
	{
	case NorthWestGravity:
	    ;
	    break;
	case NorthGravity:
	    obj->x = (FL_Coord)(obj->x * xsc + DELTA);
	    break;
	case NorthEastGravity:
	    obj->x = neww - fromright1;
	    break;
	case WestGravity:
	    obj->y = (FL_Coord)(obj->y * ysc + DELTA);
	    break;
	case EastGravity:
	    obj->x = neww - fromright1;
	    obj->y = (FL_Coord)(obj->y * ysc + DELTA);
	    break;
	case SouthWestGravity:
	    obj->y = newh - frombott1;
	    break;
	case SouthGravity:
	    obj->x = (FL_Coord)(obj->x * xsc + DELTA);
	    obj->y = newh - frombott1;
	    break;
	case SouthEastGravity:
	    obj->x = neww - fromright1;
	    obj->y = newh - frombott1;
	    break;
	default:
	    obj->x = (FL_Coord)(obj->x * xsc + DELTA);
	    obj->y = (FL_Coord)(obj->y * ysc + DELTA);
	    break;
	}

	/* re-scaling might result */
	switch (obj->segravity)
	{
	case NorthWestGravity:
	    obj->w = fromleft2 - obj->x;
	    obj->h = fromtop2 - obj->y;
	    break;
	case NorthGravity:
	    obj->h = fromtop2 - obj->y;
	    break;
	case NorthEastGravity:
	    obj->w = neww - fromright2 - obj->x;
	    obj->h = fromtop2 - obj->y;
	    break;
	case WestGravity:
	    obj->w = fromleft2 - obj->x;
	    break;
	case EastGravity:
	    obj->w = neww - fromright2 - obj->x;
	    break;
	case SouthWestGravity:
	    obj->w = fromleft2 - obj->x;
	    obj->h = newh - frombott2 - obj->y;
	    break;
	case SouthGravity:
	    obj->h = newh - frombott2 - obj->y;
	    break;
	case SouthEastGravity:
	    obj->w = neww - fromright2 - obj->x;
	    obj->h = newh - frombott2 - obj->y;
	    break;
	}
    }

    form->w = neww;
    form->h = newh;
}

/* Externally visible routine to scale a form. Need to reshape the window */
void
fl_scale_form(FL_FORM * form, double xsc, double ysc)
{
    int neww, newh;

    if (form == NULL)
    {
	fl_error("fl_scale_form", "Scaling NULL form.");
	return;
    }

    if (FL_abs(xsc - 1) < 0.001 && FL_abs(ysc - 1) < 0.001)
	return;

    /* if form is visible, after reshaping of the window, a configurenotify
       will be generated, which will then call scale_form. So, don't have to
       do it here. */
    neww = (int)(form->w * xsc + DELTA);
    newh = (int)(form->h * ysc + DELTA);

    if (!form->visible)
	scale_form(form, xsc, ysc);

    /* resize the window */
    resize_form_win(form, neww, newh);
}

void
fl_set_form_minsize(FL_FORM * form, FL_Coord w, FL_Coord h)
{
    if (!form)
    {
	Bark("FormMinSize", "Null Form");
	return;
    }
    fl_winminsize(form->window, w, h);
}

void
fl_set_form_maxsize(FL_FORM * form, FL_Coord w, FL_Coord h)
{
    if (!form)
    {
	Bark("FormMaxSize", "Null Form");
	return;
    }
    fl_winmaxsize(form->window, w, h);
}

void
fl_set_form_dblbuffer(FL_FORM * form, int y)
{
    form->use_pixmap = y;
}

/* Sets the size of the form on the screen. */
void
fl_set_form_size(FL_FORM * form, FL_Coord w, FL_Coord h)
{
    if (form == NULL)
    {
	fl_error("fl_set_form_size", "Changing size of NULL form.");
	return;
    }

    if (w != form->w || h != form->h)
	fl_scale_form(form, ((double) w) / form->w, ((double) h) / form->h);
}

/*
 * Sets the position of the form on the screen. Note that the location
 * is specified relative to lower-left corner
 */
void
fl_set_form_position(FL_FORM * form, FL_Coord x, FL_Coord y)
{
    int oldx, oldy;

    if (form == NULL)
    {
	fl_error("fl_set_form_position", "Changing position NULL form.");
	return;
    }

    oldx = form->x;
    oldy = form->y;
    form->x = (x >= 0) ? x : (fl_scrw + x);
    form->y = (y >= 0) ? y : (fl_scrh + y);

/*    force_visible(form, 2,2); */

    if (form->visible && (oldx != form->x || oldy != form->y))
	move_form(form);
}

/* Sets the position of the form on the screen. */
void
fl_set_form_hotspot(FL_FORM * form, FL_Coord x, FL_Coord y)
{
    if (form == NULL)
    {
	fl_error("fl_set_form_hotspot", "setting hotspot for NULL form.");
	return;
    }

    form->hotx = x;
    form->hoty = y;
}

void
fl_set_form_hotobject(FL_FORM * form, FL_OBJECT * ob)
{
    if (ob)
	fl_set_form_hotspot(form, ob->x + ob->w / 2, ob->y + ob->h / 2);
}

/* make sure a form is completely visible */
static void
force_visible(FL_FORM * form, int itx, int ity)
{
    if (form->x < itx)
	form->x = itx;

    if (form->x > fl_scrw - form->w - 2 * itx)
	form->x = fl_scrw - form->w - 2 * itx;

    if (form->y < ity)
	form->y = ity;

    if (form->y > fl_scrh - form->h - itx)
	form->y = fl_scrh - form->h - 2 * itx;

    /* be a paranoid */
    if (form->x < 0 || form->x > fl_scrw - form->w)
    {
	if (form->w < fl_scrw - 20)
	    M_err("ForceVisible", "Can't happen x=%d", form->x);
	form->x = 10;
    }

    if (form->y < 0 || form->y > fl_scrh - form->h)
    {
	M_warn("ForceVisible", "Can't happen y=%d", form->y);
	form->y = 20;
    }

}


void
fl_set_form_title(FL_FORM * form, const char *name)
{
    if (form->label != name)
    {
	if (form->label)
	    fl_free(form->label);
	form->label = fl_strdup(name ? name : "");
    }

    if (form->window)
	fl_wintitle(form->window, form->label);
}

/* Displays a particular form. Returns window handle. */

static int has_initial;
static int unmanaged_count;
static int auto_count;

long
fl_prepare_form_window(FL_FORM * form, int place, int border, const char *name)
{
    long screenw, screenh;
    int itx = 0, ity = 0, dont_fix_size = 0;
    FL_Coord mx, my, nmx, nmy;

    if (border == 0)
	border = FL_FULLBORDER;

    if (formnumb == MAX_FORM)
    {
	fl_error("fl_show_form", "Can show only 32 forms.");
	return -1;
    }

    if (fl_current_form)
    {
	M_err(0, "You forget to call fl_end_form %s", name ? name : "");
	fl_current_form = 0;
    }

    if (form == NULL)
    {
	fl_error("fl_show_form", "Trying to display NULL form.");
	return -1;
    }

    if (form->visible)
	return form->window;

    if (name && form->label != name)
    {
	if (form->label)
	    fl_free(form->label);
	form->label = fl_strdup(name);
    }

    /* if we are using private colormap or non-default visual, unmanaged
       window will not get correct colormap installed by the WM
       automatically. Make life easier by forcing a managed window. fl_vroot
       stuff is a workaround for tvtwm */
#if 0
    if (border != FL_FULLBORDER &&
	(fl_state[fl_vmode].pcm ||
	 fl_visual(fl_vmode) != DefaultVisual(flx->display, fl_screen)))
/*       || fl_root != fl_vroot))  */
    {
	border = FL_TRANSIENT;
    }
#endif

    if (border != FL_NOBORDER)
    {
	FL_WM_STUFF *fb = &fl_wmstuff;
	itx = fb->bw + (border == FL_TRANSIENT ? fb->trpx : fb->rpx);
	ity = fb->bw + (border == FL_TRANSIENT ? fb->trpy : fb->rpy);
    }
    else
	unmanaged_count++;

    /* see if automatic */
    auto_count += form->has_auto > 0;

    form->wm_border = border;
    forms[formnumb++] = form;
    form->deactivated = 0;
    screenw = fl_scrw;
    screenh = fl_scrh;

    fl_get_mouse(&mx, &my, &fl_keymask);

    if ((dont_fix_size = (place & FL_FREE_SIZE)))
	place &= ~FL_FREE_SIZE;

    if (place == FL_PLACE_SIZE)
	fl_pref_winsize((int) form->w, (int) form->h);
    else if (place == FL_PLACE_ASPECT)
	fl_winaspect(0, (int) form->w, (int) form->h);
    else if (place == FL_PLACE_POSITION)
    {
	if (fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER)
	{
	    form->x -= itx;
	    form->y -= ity;
	}
	fl_pref_winposition((int) form->x, (int) form->y);
	fl_initial_winsize(form->w, form->h);
    }
    else if (place != FL_PLACE_FREE)
    {
	switch (place)
	{
	case FL_PLACE_CENTER:
	case FL_PLACE_FREE_CENTER:
	    form->x = (int) ((screenw - form->w) / 2);
	    form->y = (int) ((screenh - form->h) / 2);
	    break;
	case FL_PLACE_MOUSE:
	    form->x = (mx - form->w / 2);
	    form->y = (my - form->h / 2);
	    break;
	case FL_PLACE_FULLSCREEN:
	    form->x = 0;
	    form->y = 0;
	    fl_set_form_size(form, screenw, screenh);
	    break;
	case FL_PLACE_HOTSPOT:
	    if (form->hotx < 0)
	    {			/* never set */
		form->hotx = form->w / 2;
		form->hoty = form->h / 2;
	    }

	    nmx = mx;
	    nmy = my;
	    form->x = (int) (mx - form->hotx);
	    form->y = (int) (my - form->hoty);
	    force_visible(form, itx, ity);
	    nmx = form->x + form->hotx;
	    nmy = form->y + form->hoty;
	    if (nmx != mx || nmy != my)
		fl_set_mouse(nmx, nmy);
	    break;
	}

	if (place == FL_PLACE_GEOMETRY)
	{
	    /* Correct form position. X < 0 means measure from right */
	    if (form->x < 0)
		form->x = screenw + form->x - 2 - itx;
	    /* y < 0 means from right */
	    if (form->y < 0)
		form->y = screenh + form->y - 2 - ity;
	}

	/* final check. Make sure form is visible */
	force_visible(form, itx, ity);

	/* take care of reparenting stuff */
	if (fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER)
	{
	    form->x -= itx;
	    form->y -= ity;
	}

	((dont_fix_size && place != FL_PLACE_GEOMETRY) ?
	 fl_initial_wingeometry : fl_pref_wingeometry)
	    (form->x, form->y, form->w, form->h);
    }
    else if (place == FL_PLACE_FREE)
    {
	fl_initial_winsize(form->w, form->h);
	if (has_initial)
	{
	    if (fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER)
	    {
		form->x -= itx;
		form->y -= ity;
	    }
	    fl_initial_wingeometry(form->x, form->y, form->w, form->h);
	}
    }
    else
    {
	M_err("ShowForm", "Unknown requests: %d", place);
	fl_initial_wingeometry(form->x, form->y, form->w, form->h);
    }

    /* WM typically does not allow dragging transient windows */
    if (border != FL_FULLBORDER)
    {
	if (place == FL_PLACE_ASPECT || place == FL_PLACE_FREE)
	{
	    form->x = (mx - form->w/2);
	    form->y = (my - form->h/2);
	    force_visible(form, itx, ity);
	    fl_initial_winposition(form->x, form->y);
	}
	(border == FL_NOBORDER ? fl_noborder : fl_transient) ();
    }

    form->vmode = fl_vmode;

    if (place == FL_PLACE_ICONIC)
	fl_initial_winstate(IconicState);
    if (form->icon_pixmap)
	fl_winicon(0, form->icon_pixmap, form->icon_mask);

    has_initial = 0;
    fl_init_colormap(fl_vmode);

    form->window = fl_create_window(fl_root, fl_colormap(fl_vmode), name);
    fl_winicontitle(form->window, name);

    if (border == FL_FULLBORDER || (form->prop & FL_COMMAND_PROP))
	fl_set_form_property(form, FL_COMMAND_PROP);
    return form->window;
}

long
fl_show_form_window(FL_FORM * form)
{
    if (form->window == 0 || form->visible)
	return form->window;

    fl_winshow(form->window);
    form->visible = 1;
    reshape_form(form);
    fl_redraw_form(form);

    return form->window;
}

long
fl_show_form(FL_FORM * form, int place, int border, const char *name)
{
    fl_prepare_form_window(form, place, border, name);
    return fl_show_form_window(form);
}


/* Hides a particular form */
static void
close_form_win(Window win)
{
    XEvent xev;

    XUnmapWindow(flx->display, win);
    XDestroyWindow(flx->display, win);

    XSync(flx->display, 0);
    while (XCheckWindowEvent(flx->display, win, FL_ALL_MASKS, &xev))
	fl_xevent_name("Eaten", &xev);

    /* this gives subwindow a chance to handle destroy event promptly */

#if 1
    /* TODO: consolidate event handling into fl_dispatch_XEvent(xev) and
       absorb this piece of code in that. Also the function should be of
       general use */
    while (XCheckTypedEvent(flx->display, DestroyNotify, &xev))
    {
	FL_FORM *form;
	if (select_form_event(flx->display, &xev, (char *) &form))
	{
	    int i;
	    form->visible = 0;
	    form->window = 0;
	    for (i = 0; i < formnumb; i++)
		if (form == forms[i])
		    forms[i] = forms[--formnumb];
	}

	else
	    fl_XPutBackEvent(&xev);
    }
#else
    /* suspend timeout and IO stuff. need to complete hide_form without
       re-entring the main loop */

    do_x_only = 1;

    if (fl_check_forms())
	fl_XNextEvent(&xev);
    if (fl_check_forms())
	fl_XNextEvent(&xev);

    do_x_only = 0;
#endif
}

FL_FORM *
fl_property_set(unsigned prop)
{
    int i;
    for (i = 0; i < formnumb; i++)
	if ((forms[i]->prop & prop) && (forms[i]->prop & FL_PROP_SET))
	    return forms[i];
    return 0;
}

void
fl_set_form_property(FL_FORM * form, unsigned prop)
{
    if (!form || fl_property_set(prop))
	return;

    if (prop & FL_COMMAND_PROP)
    {
	if (form->window)
	{
	    fl_set_property(form->window, FL_COMMAND_PROP);
	    form->prop |= FL_PROP_SET;
	}
	form->prop |= FL_COMMAND_PROP;
	fl_mainform = form;
    }
    else
    {
	M_err("FormProperty", "Unknown form property request %u", prop);
    }
}

void
fl_hide_form(FL_FORM * form)
{
    int i;
    FL_OBJECT *obj = 0;
    Window owin;

    if (form == NULL)
    {
	fl_error("fl_hide_form", "Hiding NULL form");
	return;
    }

    if (!fl_is_good_form(form))
    {
	M_err("fl_hide_form", "Hiding invisible/freeed form");
	return;
    }

    if (form->visible == FL_BEING_HIDDEN)
    {
	M_err("fl_hide_form", "recursive call ?");
	/* return; */
    }

    form->visible = FL_BEING_HIDDEN;
    fl_set_form_window(form);

    /* checking mouseobj->form is necessary as it might be deleted from a
       form */
    if (fl_mouseobj != NULL && fl_mouseobj->form == form)
    {
#if (FL_DEBUG >= ML_WARN)
	if (!fl_mouseobj->visible)
	    M_err("fl_hide_form", "Out dated mouseobj %s",
		  fl_mouseobj->label ? fl_mouseobj->label : "");
#endif
	obj = fl_mouseobj;
	fl_mouseobj = NULL;
	fl_handle_object(obj, FL_LEAVE, 0, 0, 0, 0);
    }

    if (fl_pushobj != NULL && fl_pushobj->form == form)
    {
	obj = fl_pushobj;
	fl_pushobj = NULL;
	fl_handle_object(obj, FL_RELEASE, 0, 0, 0, 0);
    }

    if (form->focusobj != NULL)
    {
	obj = form->focusobj;
	fl_handle_object(form->focusobj, FL_UNFOCUS, 0, 0, 0, 0);
	fl_handle_object(obj, FL_FOCUS, 0, 0, 0, 0);
    }

#ifdef DELAYED_ACTION
    fl_object_qflush(form);
#endif
    /* free backing store pixmap but keep the pointer */
    fl_free_flpixmap(form->flpixmap);

    if (mouseform && mouseform->window == form->window)
	mouseform = 0;

    form->deactivated = 1;
    form->visible = FL_INVISIBLE;
    owin = form->window;
    form->window = 0;

    fl_hide_tooltip();

    close_form_win(owin);

    for (i = 0; i < formnumb; i++)
	if (form == forms[i])
	    forms[i] = forms[--formnumb];

    if (form->wm_border == FL_NOBORDER)
    {
	unmanaged_count--;
	if (unmanaged_count < 0)
	{
	    M_err("fl_hide_form", "Bad unmanaged count");
	    unmanaged_count = 0;
	}
    }

    if (form->has_auto)
    {
	auto_count--;
	if (auto_count < 0)
	{
	    M_err("fl_hide_form", "Bad auto count");
	    auto_count = 0;
	}
    }

    /* need to re-establish command property */
    if (formnumb && (form->prop & FL_COMMAND_PROP))
	fl_set_form_property(forms[0], FL_COMMAND_PROP);

    if (form == keyform)
	keyform = 0;
}

/* Frees the memory used by a form, together with all its objects. */
void
fl_free_form(FL_FORM * form)
{
    FL_OBJECT *current, *next;

    /* check whether ok to free */
    if (form == NULL)
    {
	fl_error("fl_free_form", "Trying to free NULL form.");
	return;
    }

    if (form->visible == FL_VISIBLE)
    {
	M_err("fl_free_form", "Freeing visible form.");
	fl_hide_form(form);
    }

    /* free the objects */
    for (next = form->first; next != NULL;)
    {
	current = next;
	next = current->next;
	fl_free_object(current);
    }

    form->first = 0;

    if (form->flpixmap)
    {
	fl_free_flpixmap(form->flpixmap);
	fl_free(form->flpixmap);
	form->flpixmap = 0;
    }

    if (form->label)
    {
	fl_free(form->label);
	form->label = 0;
    }

    if (form == fl_mainform)
	fl_mainform = 0;

    /* free the form structure */
    fl_addto_freelist(form);
}

/* activates a form */
void
fl_activate_form(FL_FORM * form)
{
    if (form == NULL)
    {
	fl_error("fl_activate_form", "Activating NULL form.");
	return;
    }

    if (form->deactivated)
    {
	form->deactivated--;

	if (!form->deactivated && form->activate_callback)
	    form->activate_callback(form, form->activate_data);
    }

    if (form->child)
	fl_activate_form(form->child);
}

/* deactivates a form */
void
fl_deactivate_form(FL_FORM * form)
{
    if (form == NULL)
    {
	fl_error("fl_deactivate_form", "Deactivating NULL form.");
	return;
    }

    if (!form->deactivated && fl_mouseobj != NULL && fl_mouseobj->form == form)
	fl_handle_object(fl_mouseobj, FL_LEAVE, 0, 0, 0, 0);

    if (!form->deactivated && form->deactivate_callback)
	form->deactivate_callback(form, form->deactivate_data);

    form->deactivated++;

    if (form->child)
	fl_deactivate_form(form->child);
}


FL_FORM_ATACTIVATE
fl_set_form_atactivate(FL_FORM * form, FL_FORM_ATACTIVATE cb, void *data)
{
    FL_FORM_ATACTIVATE old = 0;

    if (form)
    {
	old = form->activate_callback;
	form->activate_callback = cb;
	form->activate_data = data;
    }
    return old;
}

FL_FORM_ATDEACTIVATE
fl_set_form_atdeactivate(FL_FORM * form, FL_FORM_ATDEACTIVATE cb, void *data)
{
    FL_FORM_ATDEACTIVATE old = 0;

    if (form)
    {
	old = form->deactivate_callback;
	form->deactivate_callback = cb;
	form->deactivate_data = data;
    }
    return old;
}

/* activates all forms */
void
fl_activate_all_forms(void)
{
    int i;
    for (i = 0; i < formnumb; i++)
	fl_activate_form(forms[i]);
}

/* deactivates all forms */
void
fl_deactivate_all_forms(void)
{
    int i;
    for (i = 0; i < formnumb; i++)
	fl_deactivate_form(forms[i]);
}

static FL_Coord fl_mousex, fl_mousey;	/* last known mouse pos */

#include <ctype.h>

static int
do_radio(FL_OBJECT * ob, void *d)
{
    FL_OBJECT *p = d;

    if (ob->pushed && ob->radio && ob != d && ob->group_id == p->group_id)
    {
	fl_handle_object_direct(ob, FL_PUSH, ob->x, ob->y, 0, 0);
	fl_handle_object_direct(ob, FL_RELEASE, ob->x, ob->y, 0, 0);
	ob->pushed = 0;
    }

    return 0;
}


/* a radio object is pushed */
void
fl_do_radio_push(FL_OBJECT * obj, FL_Coord xx, FL_Coord yy, int key, void *xev)
{
    FL_OBJECT *obj1 = obj;

    /* if this radio button does not belong to any group, have to search the
       entire form */
    if (obj->group_id == 0)
    {
	fl_for_all_objects(obj->form, do_radio, obj);
    }
    else
    {
	/* find the begining of the current obj belongs */
	while (obj1->prev != NULL && obj1->objclass != FL_BEGIN_GROUP)
	    obj1 = obj1->prev;

	while (obj1 != NULL && obj1->objclass != FL_END_GROUP)
	{
	    if (obj1->radio && obj1->pushed)
	    {
		fl_handle_object_direct(obj1, FL_PUSH, xx, yy, key, xev);
		fl_handle_object_direct(obj1, FL_RELEASE, xx, yy, key, xev);
		obj1->pushed = 0;
	    }
	    obj1 = obj1->next;
	}
    }
}

static int
do_shortcut(FL_FORM * form, int key,
	    FL_Coord x, FL_Coord y, XEvent * xev)
{
    int i, key1, key2;
    FL_OBJECT *obj1 = form->first;

    /* Check whether the <Alt> key is pressed */
    key1 = key2 = key;
    if ((fl_keypressed(XK_Alt_L) || fl_keypressed(XK_Alt_R)))
    {
	if (key < 256)
	{
	    /* always a good idea to make Alt_k case insensitive */
	    key1 = FL_ALT_VAL + (islower(key) ? toupper(key) : tolower(key));
	    key2 = key + FL_ALT_VAL;
	}
	else
	{
	    key1 = key2 = key + FL_ALT_VAL;
	}
    }

    M_info("Shortcut", "win=%lu key=%d %d %d", form->window, key, key1, key2);

    /* Check whether an object has this as shortcut. */
    for (i = -1; obj1; obj1 = obj1->next, i = -1)
    {
	if (!obj1->visible || obj1->active <= 0)
	    continue;

	while (obj1->shortcut[++i] != '\0')
	{
	    if (obj1->shortcut[i] == key1 || obj1->shortcut[i] == key2)
	    {
		if (obj1->objclass == FL_INPUT)
		{
		    if (obj1 != form->focusobj)
		    {
			fl_handle_object(form->focusobj, FL_UNFOCUS, x, y, 0, xev);
			fl_handle_object(obj1, FL_FOCUS, x, y, 0, xev);
		    }
		}
		else
		{
		    if (obj1->radio)
			fl_do_radio_push(obj1, x, y, key1, xev);
		    XAutoRepeatOff(flx->display);
		    fl_handle_object(obj1, FL_SHORTCUT, x, y, key1, xev);
		    fl_context->mouse_button = FL_SHORTCUT + key1;
		    /* this is not exactly correct as shortcut might quit,
		       fl_finish will restore the keyboard state */
		    if (fl_keybdcontrol.auto_repeat_mode == AutoRepeatModeOn)
			XAutoRepeatOn(flx->display);
		}
		return 1;
	    }
	}
    }
    return 0;
}

int
fl_do_shortcut(FL_FORM * form, int key,
	       FL_Coord x, FL_Coord y, XEvent * xev)
{
    int ret;

    if (!(ret = do_shortcut(form, key, x, y, xev)))
    {
	if (form->child)
	    ret = do_shortcut(form->child, key, x, y, xev);
	if (!ret && form->parent)
	    ret = do_shortcut(form->parent, key, x, y, xev);
    }

    return ret;
}

static void
fl_keyboard(FL_FORM * form, int key, FL_Coord x, FL_Coord y, void *xev)
{
    FL_OBJECT *obj, *obj1, *special;

    /* always check  shortcut first */
    if (fl_do_shortcut(form, key, x, y, xev))
	return;

    /* focus policy is done as follows: Input object has the highiest
       priority. Next is the object that wants special keys which is followed
       by mouseobj that has the lowest. Focusobj == FL_INPUT OBJ */

    special = fl_find_first(form, FL_FIND_KEYSPECIAL, 0, 0);
    obj1 = special ? fl_find_object(special->next, FL_FIND_KEYSPECIAL, 0, 0) : 0;

    /* if two or more objects that want keyboard input, none will get it and
       keyboard input will go to mouseobj instead */

    if (obj1 && obj1 != special)
	special = fl_mouseobj;

    if (form->focusobj)
    {
	FL_OBJECT *focusobj = form->focusobj;

	/* handle special keys first */
	if (key > 255)
	{
	    if (IsLeft(key) || IsRight(key) || IsHome(key) || IsEnd(key))
		fl_handle_object(focusobj, FL_KEYBOARD, x, y, key, xev);
	    else if ((IsUp(key) || IsDown(key) ||
		      IsPageUp(key) || IsPageDown(key)) &&
		     (focusobj->wantkey & FL_KEY_TAB))
		fl_handle_object(focusobj, FL_KEYBOARD, x, y, key, xev);
	    else if (special && (special->wantkey & FL_KEY_SPECIAL))
	    {
		/* moving the cursor in input field that does not have focus
		   looks weird */
		if (special->objclass != FL_INPUT)
		    fl_handle_object(special, FL_KEYBOARD, x, y, key, xev);
	    }
	    else if (key == XK_BackSpace || key == XK_Delete)
		fl_handle_object(focusobj, FL_KEYBOARD, x, y, key, xev);
	    return;
	}

	/* dispatch tab & return switches focus if not FL_KEY_TAB */
	if ((key == 9 || key == 13) && !(focusobj->wantkey & FL_KEY_TAB))
	{
	    if ((((XKeyEvent *) xev)->state & fl_context->navigate_mask))
	    {
		if (focusobj == fl_find_first(form, FL_FIND_INPUT, 0, 0))
		    obj = fl_find_last(form, FL_FIND_INPUT, 0, 0);
		else
		    obj = fl_find_object(focusobj->prev, FL_FIND_INPUT, 0, 0);
	    }
	    else
		obj = fl_find_object(focusobj->next, FL_FIND_INPUT, 0, 0);

	    if (obj == NULL)
		obj = fl_find_first(form, FL_FIND_INPUT, 0, 0);

	    fl_handle_object(focusobj, FL_UNFOCUS, x, y, 0, xev);
	    fl_handle_object(obj, FL_FOCUS, x, y, 0, xev);
	}
	else if (focusobj->wantkey != FL_KEY_SPECIAL)
	    fl_handle_object(focusobj, FL_KEYBOARD, x, y, key, xev);
	return;
    }

    /* keyboard input is not wanted */
    if (!special || special->wantkey == 0)
	return;

    /* space is an exception for browser */
    if ((key > 255 || key == ' ') && (special->wantkey & FL_KEY_SPECIAL))
	fl_handle_object(special, FL_KEYBOARD, x, y, key, xev);
    else if (key < 255 && (special->wantkey & FL_KEY_NORMAL))
	fl_handle_object(special, FL_KEYBOARD, x, y, key, xev);
    else if (special->wantkey == FL_KEY_ALL)
	fl_handle_object(special, FL_KEYBOARD, x, y, key, xev);

#if FL_DEBUG >= ML_INFO
    M_info("KeyBoard", "(%d %d)pushing %d to %s\n", x, y, key, special->label);
#endif
}

/* updates a form according to an event */
static void
fl_handle_form(FL_FORM * form, int event, int key, XEvent * xev)
{
    FL_OBJECT *obj = 0, *obj1;
    FL_Coord xx, yy;

    if (!form || !form->visible)
	return;

    if (form->deactivated && event != FL_DRAW)
	return;

    if (form->parent_obj && form->parent_obj->active == DEACTIVATED && 
          event != FL_DRAW)
	return;

    if (event != FL_STEP)
	fl_set_form_window(form);

    xx = fl_mousex;
    yy = fl_mousey;

    if (event != FL_STEP && event != FL_DRAW)
    {
	if (event == FL_PUSH || event == FL_RELEASE || event == FL_ENTER ||
	    event == FL_LEAVE || event == FL_KEYBOARD || event == FL_MOUSE)
	{
	    xx = fl_mousex;
	    yy = fl_mousey;
	}
	else
	{
	    fl_get_form_mouse(form, &xx, &yy, &fl_keymask);
	}

	/* obj under the mouse */
	obj = fl_find_last(form, FL_FIND_MOUSE, xx, yy);
    }

#if 1
    /* this is an ugly hack. This is necessary due to popup pointer grab
       where a button release is eaten. Really should do a send event from
       the pop-up routines */

    if (fl_pushobj && !button_down(fl_keymask) /* && event == FL_ENTER */ )
    {
	obj = fl_pushobj;
	fl_pushobj = NULL;
	fl_handle_object(obj, FL_RELEASE, xx, yy, key, xev);
    }
#endif

    switch (event)
    {

    case FL_DRAW:		/* form must be redrawn */
	fl_redraw_form(form);
	break;
    case FL_ENTER:		/* Mouse did enter the form */
	fl_mouseobj = obj;
	fl_handle_object(fl_mouseobj, FL_ENTER, xx, yy, 0, xev);
	break;
    case FL_LEAVE:		/* Mouse did leave the form */
	fl_handle_object(fl_mouseobj, FL_LEAVE, xx, yy, 0, xev);
	fl_mouseobj = NULL;
	break;
    case FL_MOUSE:		/* Mouse position changed in the form */
	if (fl_pushobj != NULL)
	    fl_handle_object(fl_pushobj, FL_MOUSE, xx, yy, key, xev);
	else if (obj != fl_mouseobj)
	{
	    fl_handle_object(fl_mouseobj, FL_LEAVE, xx, yy, 0, xev);
	    fl_handle_object(fl_mouseobj = obj, FL_ENTER, xx, yy, 0, xev);
	}
#if 1
	/* can't remember why this is here, probably does something required */
	else if (fl_mouseobj != NULL)
	    fl_handle_object(fl_mouseobj, FL_MOTION, xx, yy, 0, xev);
#endif
	break;
    case FL_PUSH:		/* Mouse was pushed inside the form */
	/* change input focus */
#if 1
	if (obj != NULL && obj->input && form->focusobj != obj)
	{
	    fl_handle_object(form->focusobj, FL_UNFOCUS, xx, yy, key, xev);
	    fl_handle_object(obj, FL_FOCUS, xx, yy, key, xev);
	}
#else
	/* 05/29/99 Always force unfocus. Could be risky just before release */
	if (obj != NULL && form->focusobj != obj)
	{
	    FL_OBJECT *tmpobj = form->focusobj;
	    fl_handle_object(form->focusobj, FL_UNFOCUS, xx, yy, key, xev);
	    if (obj->input)
		fl_handle_object(obj, FL_FOCUS, xx, yy, key, xev);
	    else
		fl_handle_object(tmpobj, FL_FOCUS, xx, yy, key, xev);
	}
#endif

	if (form->focusobj)
	    keyform = form;

	/* handle a radio button */
	if (obj != NULL && obj->radio)
	    fl_do_radio_push(obj, xx, yy, key, xev);

	/* push the object except when focus is overriden  */
	if (obj && (!obj->input || (obj->input && obj->focus)))
	    fl_handle_object(obj, FL_PUSH, xx, yy, key, xev);
	fl_pushobj = obj;
	break;
    case FL_RELEASE:		/* Mouse was released inside the form */
	obj = fl_pushobj;
	fl_pushobj = NULL;
	fl_handle_object(obj, FL_RELEASE, xx, yy, key, xev);
	break;
    case FL_KEYBOARD:		/* A key was pressed */
	fl_keyboard(form, key, xx, yy, xev);
	break;
    case FL_STEP:		/* A simple step */
	obj1 = fl_find_first(form, FL_FIND_AUTOMATIC, 0, 0);
	if (obj1 != NULL)
	    fl_set_form_window(form);	/* set only if required */
	while (obj1 != NULL)
	{
	    fl_handle_object(obj1, FL_STEP, xx, yy, 0, xev);
	    obj1 = fl_find_object(obj1->next, FL_FIND_AUTOMATIC, 0, 0);
	}
	break;
    case FL_OTHER:
	/* need to dispatch it thru all objects and monitor the status of
	   forms as it may get closed */
	for (obj1 = form->first; obj1 && form->visible; obj1 = obj1->next)
	    if (obj1->visible)
		fl_handle_object(obj1, FL_OTHER, xx, yy, key, xev);
	break;
    }
}

/***************
  Routine to check for events
***************/

#define NTIMER  FL_NTIMER
static long lastsec[NTIMER];
static long lastusec[NTIMER];

typedef struct
{
    long sec;
    long usec;
}
TimeVal;

static TimeVal tp;

/* Resets the timer */
void
fl_reset_time(int n)
{
    n %= NTIMER;
    fl_gettime(&lastsec[n], &lastusec[n]);
}

/* Returns the time passed since the last call */
float
fl_time_passed(int n)
{
    n %= NTIMER;
    fl_gettime(&tp.sec, &tp.usec);
    return (float) (tp.sec - lastsec[n]) + (tp.usec - lastusec[n])*0.000001f;
}

#if (FL_DEBUG >= ML_DEBUG )	/* { */
static void
hack_test(void)
{
    int ff, jj = 0;
    static int k;
    FL_FORM *f[20] =
    {0};

    fl_set_graphics_mode((++k) % 6, 1);

    /* force the action */
    for (ff = 0; ff < MAX_FORM; ff++)
    {
	if (forms[ff] && forms[ff]->visible)
	    fl_hide_form(f[jj++] = forms[ff]);
    }

    for (ff = 0; ff < jj; ff++)
	if (f[ff])
	    fl_show_form(f[ff], FL_PLACE_GEOMETRY, 1, f[ff]->label);
}

#endif /* } */


/* formevent is either FL_KEYPRESS or FL_KEYRELEASE */
static void
do_keyboard(XEvent * xev, int formevent)
{

    Window win;
    int kbuflen;

    /* before doing anything, save the current modifier key for the handlers */
    win = xev->xkey.window;
    fl_keymask = xev->xkey.state;

    if (win && (!keyform || !fl_is_good_form(keyform)))
	keyform = fl_win_to_form(win);

    /* switch keyboard input only if different top-level form */
    if (keyform && keyform->window != win)
    {
	M_warn("KeyEvent", "pointer/keybd focus differ");
	if ((keyform->child && keyform->child->window == win) ||
	    (keyform->parent && keyform->parent->window == win))
	    ;
	else
	    keyform = fl_win_to_form(win);
    }

    if ( keyform ) {

	KeySym keysym = 0;
	unsigned char keybuf[227];

	kbuflen = fl_XLookupString((XKeyEvent *) xev, (char *) keybuf,
				   sizeof(keybuf), &keysym);
	
	if (kbuflen < 0) {
	    
	    if ( kbuflen != INT_MIN ) {
		
		/* buffer overflow, should not happen */
		M_err("DoKeyBoard", "keyboad buffer overflow ?");
		
	    } else {

		M_err("DoKeyBoard", "fl_XLookupString failed ?");
	    
	    }

	} else {
	    
	    /* ignore modifier keys as they don't cause action and are
	       taken care of by the lookupstring routine */
	    
	    if (IsModifierKey(keysym))
		;
	    else if (IsTab(keysym)) {
		
		/* fake a tab key. */
		/* some system shift+tab does not generate tab */
		
		fl_handle_form(keyform, formevent, 9, xev);

	    }
#if (FL_DEBUG >= ML_DEBUG )
	    else if (keysym == XK_F10)	/* && controlkey_down(fl_keymask)) */
		hack_test();
#endif
	    
	    /* pass all keys to the handler */
	    else if (IsCursorKey(keysym) || kbuflen == 0)
		fl_handle_form(keyform, formevent, keysym, xev);
	    else {

		unsigned char *ch;

		/* all regular keys, including mapped strings */

		for (ch = keybuf; ch < (keybuf + kbuflen) && keyform; ch++)
		    fl_handle_form(keyform, formevent, *ch, xev);

	    }
	    
	}

    }

}

FL_FORM_ATCLOSE
fl_set_form_atclose(FL_FORM * form, FL_FORM_ATCLOSE fmclose, void *data)
{
    FL_FORM_ATCLOSE old = form->close_callback;
    form->close_callback = fmclose;
    form->close_data = data;
    return old;
}

FL_FORM_ATCLOSE
fl_set_atclose(FL_FORM_ATCLOSE fmclose, void *data)
{
    FL_FORM_ATCLOSE old = fl_context->atclose;
    fl_context->atclose = fmclose;
    fl_context->close_data = data;
    return old;
}

/*
 * ClientMessage is intercepted if it is delete window
 */
static void
handle_client_message(FL_FORM * form, void *xev)
{
    XClientMessageEvent *xcm = xev;
    static Atom atom_protocol;
    static Atom atom_del_win;

    if (!atom_del_win)
    {
	atom_protocol = XInternAtom(xcm->display, "WM_PROTOCOLS", 0);
	atom_del_win = XInternAtom(xcm->display, "WM_DELETE_WINDOW", 0);
    }

    /* if delete top-level window, quit unless handlers are installed */
    if (xcm->message_type == atom_protocol && xcm->data.l[0] == atom_del_win)
    {
	if (form->close_callback)
	{
	    if (form->close_callback(form, form->close_data) != FL_IGNORE)
	    {
		if (form->visible == FL_VISIBLE)
		    fl_hide_form(form);
	    }

	    if (form->sort_of_modal)
		fl_activate_all_forms();

	}
	else if (fl_context->atclose)
	{
	    if (fl_context->atclose(form, fl_context->close_data) != FL_IGNORE)
		exit(1);
	}
	else
	    exit(1);
    }
    else
    {
	/* pump it thru current form */
	fl_handle_form(form, FL_OTHER, 0, xev);
    }
}

static int pre_emptive_consumed(FL_FORM *, int, XEvent *);

FL_FORM *
fl_win_to_form(register Window win)
{
    register FL_FORM **fws, **fw;
    for (fw = forms, fws = fw + formnumb; fw < fws; fw++)
    {
	if ((*fw)->window == win)
	    return *fw;
    }
    return 0;
}

void (*fl_handle_signal) (void);
int (*fl_handle_clipboard) (void *);

void
fl_handle_automatic(XEvent * xev, int idle_cb)
{
    FL_FORM **f = forms, **fe;
    FL_IDLE_REC *idle_rec;
    static int nc;

    if (fl_handle_signal)
	fl_handle_signal();

    for (fe = f + formnumb; auto_count && f < fe; f++)
    {
	if ((*f)->has_auto)
	    fl_handle_form(*f, FL_STEP, 0, xev);
    }

    if (idle_cb)
    {
	if (((++nc) & 0x40))
	{
	    fl_free_freelist();
	    nc = 0;
	}

	if ((idle_rec = fl_context->idle_rec) && idle_rec->callback)
	    idle_rec->callback(xev, idle_rec->data);

	fl_handle_timeouts(200);	/* force a re-sync */
    }
}


/* how frequent to generate FL_STEP EVENT, in milli-seconds. These
 * are modified if idle callback exists
 */

static int delta_msec = TIMER_RES;

static int form_event_queued(XEvent *, int);


static XEvent st_xev;
const XEvent *
fl_last_event(void)
{
    return &st_xev;
}

static unsigned long uev_cmask = PointerMotionMask | ButtonMotionMask;
static int ignored_fake_configure;

static int
button_is_really_down(void)
{
    FL_Coord x, y;
    unsigned km;
    fl_get_mouse(&x, &y, &km);
    return button_down(km);
}

/* should pass the mask instead of button numbers into the
 * event handler. basically throwing away info ..
 */
static int
xmask2key(unsigned mask)
{
    int ret = 0;

    /* once the FL_XXX_MOUSE is changed to mask, just loose the else */
    if (mask & Button1Mask)
	ret |= FL_LEFT_MOUSE;
    else if (mask & Button2Mask)
	ret |= FL_MIDDLE_MOUSE;
    else if (mask & Button3Mask)
	ret |= FL_RIGHT_MOUSE;
    return ret;
}


/* Ensure that the tabfolder forms' x,y coords are updated correctly */
static void
fl_get_tabfolder_origin(FL_FORM * form)
{
    FL_OBJECT *ob = 0;

    for (ob = form->first; ob; ob = ob->next) {
	if (ob->objclass == FL_TABFOLDER) {
	    FL_FORM * const folder = fl_get_active_folder(ob);
	    if (folder && folder->window) {
		fl_get_winorigin(folder->window, &(folder->x), &(folder->y));
		/* Don't forget nested folders */
		fl_get_tabfolder_origin(folder);
	    }
	}
    }
}

static void
do_interaction_step(int wait_io)
{
    Window win;
    FL_FORM *evform = 0;
    int has_event;
    static unsigned auto_cnt, query_cnt;
    static int lasttimer;

    has_event = get_next_event(wait_io, &evform, &st_xev);

    if (!has_event)
    {
	/* we are idling */
	st_xev.type = TIMER3;

	/* certain events like Selection/GraphicsExpose do not have a window
	   member itself, but xany.window happens to be the one we want */

	if ((query_cnt++ % 100) == 0)
	{
	    fl_get_form_mouse(mouseform, &fl_mousex, &fl_mousey, &fl_keymask);
	    st_xev.xany.window = mouseform ? mouseform->window : 0;
	    st_xev.xany.send_event = 1;		/* indicating synthetic event 
						 */
	    st_xev.xmotion.state = fl_keymask;
	    st_xev.xmotion.x = fl_mousex;
	    st_xev.xmotion.y = fl_mousey;
	    st_xev.xmotion.is_hint = 0;
	}
	else
	    st_xev.xmotion.time += (wait_io ? delta_msec : SHORT_PAUSE);
    }
    else
    {
	/* got an event for forms */
#if (FL_DEBUG >= ML_WARN)
	if (st_xev.type != MotionNotify || fl_cntl.debug > 2)
	    fl_xevent_name("MainLoop", &st_xev);
#endif

	if (!evform)
	    M_err("FormEvent", "Something is wrong");

	fl_compress_event(&st_xev, evform->compress_mask);

	lasttimer = 0;
	query_cnt = 0;

	if (pre_emptive_consumed(evform, st_xev.type, &st_xev))
	    return;
    }

    win = st_xev.xany.window;

    switch (st_xev.type)
    {

    case TIMER3:

#if (FL_DEBUG >= ML_TRACE)
	M_info("Main", "Timer Event");
#endif

	/* need to do FL_MOUSE for touch buttons */
	st_xev.type = MotionNotify;

	/* unless ButtonRelease does both Release and Leave, have to generate
	   at least one lasttimer due to the possibility of a
	   Release/Leave/Motion eaten by popups. True fix would be to have
	   xpopup send an motion event */

	if (button_down(fl_keymask) || fl_pushobj || !lasttimer)
	{
	    fl_handle_form(mouseform, FL_MOUSE, xmask2key(fl_keymask), &st_xev);
	    lasttimer = 1;
	}

	/* handle both automatic and idle callback */
	fl_handle_automatic(&st_xev, 1);
	return;

    case MappingNotify:

	XRefreshKeyboardMapping((XMappingEvent *) & st_xev);
	return;

    case FocusIn:
	if (fl_context->xic)
	{
	    M_info("Focus", "Setting focus window for IC");
	    XSetICValues(fl_context->xic,
			 XNFocusWindow, st_xev.xfocus.window,
			 XNClientWindow, st_xev.xfocus.window,
			 0);
	}
	break;

    case KeyPress:

	fl_mousex = st_xev.xkey.x;
	fl_mousey = st_xev.xkey.y;
	fl_keymask = st_xev.xkey.state;
	do_keyboard(&st_xev, FL_KEYPRESS);
	return;

    case KeyRelease:

	fl_mousex = st_xev.xkey.x;
	fl_mousey = st_xev.xkey.y;
	fl_keymask = st_xev.xkey.state;
	do_keyboard(&st_xev, FL_KEYRELEASE);
	return;

    case EnterNotify:

	fl_keymask = st_xev.xcrossing.state;

	if (button_down(fl_keymask) && st_xev.xcrossing.mode != NotifyUngrab)
	    break;

	fl_mousex = st_xev.xcrossing.x;
	fl_mousey = st_xev.xcrossing.y;

	if (mouseform)
	    fl_handle_form(mouseform, FL_LEAVE, xmask2key(fl_keymask), &st_xev);
	mouseform = 0;

	if (evform)
	{
	    mouseform = evform;

	    /* this is necessary because win might be un-managed. To be
	       friendly to other applications, grab focus only if abslutely
	       necessary */
	    if (mouseform->deactivated == 0 &&
		!st_xev.xcrossing.focus && unmanaged_count > 0)
	    {
		fl_check_key_focus("EnterNotify", win);
		fl_winfocus(win);
	    }
	    fl_handle_form(mouseform, FL_ENTER, xmask2key(fl_keymask), &st_xev);
	}
#if (FL_DEBUG >= ML_DEBUG)
	else
	{
	    M_err("EnterNotify", "Null form!");
	}
#endif

	break;

    case LeaveNotify:

	fl_keymask = st_xev.xcrossing.state;

	if (button_down(fl_keymask) && st_xev.xcrossing.mode == NotifyNormal)
	    break;

	/* olvwm sends leavenotify with NotifyGrab whenever button is
	   clicked. Ignore it. Due to Xpoup grab, (maybe Wm bug ?), end grab
	   can also generate this event. we can tell these two situations by
	   doing a real button_down test (as opposed to relying on the
	   keymask in event) */

	if (st_xev.xcrossing.mode == NotifyGrab && button_is_really_down())
	    break;

	fl_mousex = st_xev.xcrossing.x;
	fl_mousey = st_xev.xcrossing.y;

	if (mouseform)
	{
	    /* due to grab in pop-up , FL_RELEASE is necessary */
	    fl_handle_form(mouseform, FL_RELEASE, 0, &st_xev);
	    fl_handle_form(mouseform, FL_LEAVE, xmask2key(fl_keymask), &st_xev);
	    mouseform = NULL;
	}
	break;

    case MotionNotify:

	fl_keymask = st_xev.xmotion.state;
	fl_mousex = st_xev.xmotion.x;
	fl_mousey = st_xev.xmotion.y;

	if (!mouseform)
	{
	    M_warn("Main-NoMotionForm", "evwin=0x%lx", win);
	    break;
	}

	if (mouseform->window != win)
	{
	    M_warn("*Motion", "mousewin=0x%ld evwin=0x%ld",
		   mouseform->window, win);
	    fl_mousex += evform->x - mouseform->x;
	    fl_mousey += evform->y - mouseform->y;
	}

	fl_handle_form(mouseform, FL_MOUSE, xmask2key(fl_keymask), &st_xev);

	/* Handle FL_STEP. Every 10 events. This will reduce cpu usage */
	if ((++auto_cnt % 10) == 0)
	{
#if (FL_DEBUG >= ML_TRACE)
	    M_info("Motion", "Auto FL_STEP");
#endif
	    fl_handle_automatic(&st_xev, 0);
	    auto_cnt = 0;
	}
	break;

    case ButtonPress:

	fl_keymask = st_xev.xbutton.state;
	fl_mousex = st_xev.xbutton.x;
	fl_mousey = st_xev.xbutton.y;
	fl_context->mouse_button = st_xev.xbutton.button;
	if (metakey_down(fl_keymask) && st_xev.xbutton.button == 2)
	    fl_print_version(1);
	else
	    fl_handle_form(mouseform, FL_PUSH, st_xev.xbutton.button, &st_xev);
	return;

    case ButtonRelease:

	fl_keymask = st_xev.xbutton.state;
	fl_mousex = st_xev.xbutton.x;
	fl_mousey = st_xev.xbutton.y;
	fl_context->mouse_button = st_xev.xbutton.button;

#if (FL_DEBUG >= ML_DEBUG )
	if (!mouseform)
	    M_warn("ButtonRelease", "mouse form == 0!");
#endif
	if (mouseform)
	    fl_handle_form(mouseform, FL_RELEASE, st_xev.xbutton.button, &st_xev);

	mouseform = evform;
	return;

    case Expose:

	if (evform)
	{
	    fl_set_perm_clipping(st_xev.xexpose.x, st_xev.xexpose.y,
			       st_xev.xexpose.width, st_xev.xexpose.height);
	    fl_set_clipping(st_xev.xexpose.x, st_xev.xexpose.y,
			    st_xev.xexpose.width, st_xev.xexpose.height);

	    /* run into trouble by ignoring configure notify */
	    if (ignored_fake_configure)
	    {
		FL_Coord neww, newh;

		M_warn("Expose", "Run into trouble - correcting it");
		fl_get_winsize(evform->w, &neww, &newh);
		scale_form(evform, (double) neww / evform->w,
			   (double) newh / evform->h);
	    }

	    fl_handle_form(evform, FL_DRAW, 0, &st_xev);

	    fl_unset_perm_clipping();
	    fl_unset_clipping();
	    fl_unset_text_clipping();
	}
	break;

    case ConfigureNotify:

	if (!evform)
	    break;

	if (!st_xev.xconfigure.send_event)
	    fl_get_winorigin(win, &(evform->x), &(evform->y));
	else
	{
	    evform->x = st_xev.xconfigure.x;
	    evform->y = st_xev.xconfigure.y;
	    M_warn("Configure", "WMConfigure:x=%d y=%d", evform->x, evform->y);
	}

	/* mwm sends bogus configurenotify randomly without following up with
	   a redraw event, but it does set send_event. The check is somewhat
	   dangerous, use ignored_fake_configure to make sure when we got
	   expose we can respond correctly. The correct fix is always to get
	   window geometry in Expose handler, but that has a two-way traffic
	   overhead */

	ignored_fake_configure = (st_xev.xconfigure.send_event &&
				  (st_xev.xconfigure.width != evform->w ||
				   st_xev.xconfigure.height != evform->h));


	/* Ensure that the tabfolder forms' x,y coords are updated correctly */
	fl_get_tabfolder_origin(evform);

	if (!st_xev.xconfigure.send_event)
	{
	    /* can't just set form->{w,h}. Need to take care of obj gravity */
	    scale_form(evform,
		       (double) st_xev.xconfigure.width / evform->w,
		       (double) st_xev.xconfigure.height / evform->h);
	}
	break;

    case ClientMessage:
	handle_client_message(evform, &st_xev);
	break;

    case DestroyNotify:	/* only sub-form gets this due to parent destroy */
	{
	    int i;
	    evform->visible = 0;
	    evform->window = 0;
	    for (i = 0; i < formnumb; i++)
		if (evform == forms[i])
		    forms[i] = forms[--formnumb];
	}

	break;

    case SelectionClear:
    case SelectionRequest:
    case SelectionNotify:
	if (!fl_handle_clipboard || fl_handle_clipboard(&st_xev) < 0)
	    fl_handle_form(evform, FL_OTHER, 0, &st_xev);
	break;

    default:
	fl_handle_form(evform, FL_OTHER, 0, &st_xev);
	break;
    }

}

/* Handle all events in the queue and flush output buffer */
void
fl_treat_interaction_events(int wait)
{
    XEvent xev;

    do
	do_interaction_step(wait);
    /* if no event, output buffer will be flushed. If event exists,
       XNextEvent in do_interaction will flush the output buffer */
    while (form_event_queued(&xev, QueuedAfterFlush));
}

/* Checks all forms. Does not wait. */
FL_OBJECT *
fl_check_forms(void)
{
    FL_OBJECT *obj;

    if ((obj = fl_object_qread()) == NULL)
    {
	fl_treat_interaction_events(0);
	fl_treat_user_events();
	obj = fl_object_qread();
    }
    return obj;
}

/* Checks all forms. Waits if nothing happens. */
FL_OBJECT *
fl_do_forms(void)
{
    FL_OBJECT *obj;

    while (1)
    {
	if ((obj = fl_object_qread()) != NULL)
	    return obj;
	fl_treat_interaction_events(1);
	fl_treat_user_events();
    }
}

/* Same as fl_check_forms but never returns FL_EVENT. */
FL_OBJECT *
fl_check_only_forms(void)
{
    FL_OBJECT *obj;

    if ((obj = fl_object_qread()) == NULL)
    {
	fl_treat_interaction_events(0);
	obj = fl_object_qread();
    }
    return obj;
}

/* Same as fl_do_forms but never returns FL_EVENT. */
FL_OBJECT *
fl_do_only_forms(void)
{
    FL_OBJECT *obj;

    while (1)
    {
	if ((obj = fl_object_qread()) != NULL)
	{
	    if (obj == FL_EVENT)
		M_warn("DoOnlyForms", "Shouldn't happen");
	    return obj;
	}
	fl_treat_interaction_events(1);
    }
}


/*
 * Check if an event exists that is meant for FORMS
 */
static int
select_form_event(Display * d, XEvent * xev, char *arg)
{
    register int i;
    register Window win = ((XAnyEvent *) xev)->window;

    for (i = 0; i < formnumb; i++)
	if (win == forms[i]->window)
	{
	    *((FL_FORM **) arg) = forms[i];
	    return 1;
	}
    return 0;
}


static int
form_event_queued(XEvent * xev, int mode)
{
    if (XEventsQueued(flx->display, mode))
    {
	FL_FORM *f;
	XPeekEvent(flx->display, xev);
	return (select_form_event(flx->display, xev, (void *) &f));
    }
    return 0;
}


/* remove RCS keywords */
const char *
fl_rm_rcs_kw(register const char *s)
{
    static unsigned char buf[5][255];
    static int nbuf;
    register unsigned char *q = buf[(nbuf = (nbuf + 1) % 5)];
    int left = 0, lastc = -1;

    while (*s && (q - buf[nbuf]) < sizeof(buf[nbuf]) - 2)
    {
	switch (*s)
	{
	case '$':
	    if ((left = !left))
		while (*s && *s != ':')
		    s++;
	    break;
	default:
	    /* copy the char and remove extra space */
	    if (!(lastc == ' ' && *s == ' '))
		*q++ = lastc = *s;
	    break;
	}
	s++;
    }
    *q = '\0';
    return (const char *) buf[nbuf];
}


void
fl_set_initial_placement(FL_FORM * form, FL_Coord x, FL_Coord y,
			 FL_Coord w, FL_Coord h)
{
    fl_set_form_position(form, x, y);
    fl_set_form_size(form, w, h);

    /* this alters the windowing defaults */
    fl_initial_wingeometry(form->x, form->y, form->w, form->h);
    has_initial = 1;
}

/* register pre-emptive event handlers */
FL_RAW_CALLBACK
fl_register_raw_callback(FL_FORM * form, unsigned long mask, FL_RAW_CALLBACK rcb)
{
    FL_RAW_CALLBACK old_rcb = 0;
    int valid = 0;

    if (!form)
    {
	Bark("RawCallBack", "Null form");
	return 0;
    }

    if ((mask & FL_ALL_EVENT) == FL_ALL_EVENT)
    {
	old_rcb = form->all_callback;
	form->evmask = mask;
	form->all_callback = rcb;
	return old_rcb;
    }

    if ((mask & (KeyPressMask | KeyReleaseMask)))
    {
	form->evmask |= (mask & (KeyPressMask | KeyReleaseMask));
	old_rcb = form->key_callback;
	form->key_callback = rcb;
	valid = 1;
    }

    if ((mask & ButtonPressMask) || (mask & ButtonReleaseMask))
    {
	form->evmask |= (mask & (ButtonPressMask | ButtonReleaseMask));
	old_rcb = form->push_callback;
	form->push_callback = rcb;
	valid = 1;
    }

    if ((mask & EnterWindowMask) || (mask & LeaveWindowMask))
    {
	form->evmask |= (mask & EnterWindowMask) | (mask & LeaveWindowMask);
	old_rcb = form->crossing_callback;
	form->crossing_callback = rcb;
	valid = 1;
    }

    if ((mask & ButtonMotionMask) || (mask & PointerMotionMask))
    {
	form->evmask |= (mask & ButtonMotionMask) | (mask & PointerMotionMask);
	old_rcb = form->motion_callback;
	form->motion_callback = rcb;
	valid = 1;
    }

    if (!valid)			/* not supported mask */
	Bark("RawCallBack", "Unsupported mask 0x%x", mask);

    return old_rcb;
}

static int
pre_emptive_consumed(FL_FORM * form, int type, XEvent * xev)
{
    if (!form || !form->evmask || form->deactivated)
	return 0;

    if (((form->evmask & FL_ALL_EVENT) == FL_ALL_EVENT) && form->all_callback)
	return form->all_callback(form, xev);

    switch (type)
    {
    case ButtonPress:
	if ((form->evmask & ButtonPressMask) && form->push_callback)
	    return form->push_callback(form, xev);
	break;
    case ButtonRelease:
	if ((form->evmask & ButtonReleaseMask) && form->push_callback)
	    return form->push_callback(form, xev);
	break;
    case KeyPress:
    case KeyRelease:
	if ((form->evmask & (KeyPressMask | KeyRelease)) && form->key_callback)
	    return form->key_callback(form, xev);
	break;
    case EnterNotify:
	if ((form->evmask & EnterWindowMask) && form->crossing_callback)
	    return form->crossing_callback(form, xev);
	break;
    case LeaveNotify:
	if ((form->evmask & LeaveWindowMask) && form->crossing_callback)
	    return form->crossing_callback(form, xev);
	break;
    case MotionNotify:
	if ((form->evmask & (ButtonMotionMask | PointerMotionMask)) &&
	    form->motion_callback)
	    return form->motion_callback(form, xev);

    }
    return 0;
}

void
fl_set_form_event_cmask(FL_FORM * form, unsigned long cmask)
{
    if (form)
	form->compress_mask = cmask;
}

unsigned long
fl_get_form_event_cmask(FL_FORM * form)
{
    return form ? form->compress_mask : 0;
}

/*
 * cleanup everything. At the moment, only need to restore the keyboard
 * settings. This routine is registered as an atexit in fl_initialize
 * in flresource.c
 */
void
fl_finish(void)
{
    /* make sure the connection is alive */
    if (flx->display)
    {
	XChangeKeyboardControl(flx->display, fl_keybdmask, &fl_keybdcontrol);
	XCloseDisplay(flx->display);
	flx->display = fl_display = 0;
    }
}

/* Sets the call_back routine for the form */
void
fl_set_form_callback(FL_FORM * form, FL_FORMCALLBACKPTR callback, void *d)
{
    if (form == NULL)
    {
	fl_error("fl_set_form_callback", "Setting callback of NULL form.");
	return;
    }

    form->form_callback = callback;
    form->form_cb_data = d;
}


/* currently only a single idle callback is support */
static void
add_idle_callback(FL_APPEVENT_CB cb, void *data)
{
    if (!fl_context->idle_rec)
    {
	fl_context->idle_rec = fl_malloc(sizeof(*(fl_context->io_rec)));
	fl_context->idle_rec->next = 0;
    }

    fl_context->idle_rec->callback = cb;
    fl_context->idle_rec->data = data;
}

/* idle callback */
FL_APPEVENT_CB
fl_set_idle_callback(FL_APPEVENT_CB callback, void *user_data)
{
    FL_APPEVENT_CB old = fl_context->idle_rec ? fl_context->idle_rec->callback : 0;

    add_idle_callback(callback, user_data);

    /* if we have idle callbacks, decrease the wait time */
    delta_msec = (int)(TIMER_RES * (callback ? 0.8f : 1.0f));

    fl_context->idle_delta = delta_msec;

    return old;
}

void
fl_set_idle_delta(long delta)
{
    if (delta < 0)
	delta = TIMER_RES;
    else if (delta == 0)
	delta = TIMER_RES / 10;

    delta_msec = delta;
    fl_context->idle_delta = delta;
}

/*
 * xevent has about 10:1 priority as compared to user  sockets
 */

static void handle_global_event(XEvent *);
#define IsGlobalEvent(xev)  (xev->type == MappingNotify)

static int
get_next_event(int wait_io, FL_FORM ** form, XEvent * xev)
{
    int msec, has_event = 0;
    static int dox;

    if ((++dox % 11) && XEventsQueued(flx->display, QueuedAfterFlush))
    {
	XNextEvent(flx->display, xev);

	if (IsGlobalEvent(xev))
	{
	    handle_global_event(xev);
	    return has_event;
	}

	if (!(has_event = select_form_event(flx->display, xev, (char *) form)))
	{
	    fl_compress_event(xev, ExposureMask | uev_cmask);
	    /* process user events */
#if (FL_DEBUG >= ML_WARN)
	    if (xev->type != MotionNotify || fl_cntl.debug > 2)
		fl_xevent_name("MainLoopUser", xev);
#endif
	    fl_XPutBackEvent(xev);
	}
    }

    if (has_event || do_x_only)
	return has_event;

    /* if incoming XEvent has already being pumped from the socket, 
       watch_io() will time out, causing a bad delay in handling xevent. 
       Make sure there is no event in the X event queue before we go into
       watch_io() */
    if ((dox % 11) && XEventsQueued(flx->display, QueuedAfterFlush))
	return has_event;

    /* compute how much time to wait */
    if (!wait_io)
	msec = SHORT_PAUSE;
    else
	msec = (auto_count || fl_pushobj ||
		fl_context->idle_rec || fl_context->timeout_rec) ?
	    delta_msec : FL_min(delta_msec * 3, 300);

    fl_watch_io(fl_context->io_rec, msec);

    if (fl_context->timeout_rec)
	fl_handle_timeouts(msec);

    return has_event;
}


void
fl_set_form_icon(FL_FORM * form, Pixmap p, Pixmap m)
{
    if (form)
    {
	form->icon_pixmap = p;
	form->icon_mask = m;
	if (form->window)
	    fl_winicon(form->window, p, m);
    }
}


void
fl_set_app_mainform(FL_FORM * form)
{
    fl_mainform = form;
    fl_set_form_property(form, FL_COMMAND_PROP);
}

FL_FORM *
fl_get_app_mainform(void)
{
    return nomainform ? 0 : fl_mainform;
}

void
fl_set_app_nomainform(int flag)
{
    nomainform = flag;
}

static void
handle_global_event(XEvent * xev)
{
    if (xev->type == MappingNotify)
	XRefreshKeyboardMapping((XMappingEvent *) & st_xev);
}

/* never shrinks a form. margin is the minimum margin to leave */
void
fl_fit_object_label(FL_OBJECT * obj, FL_Coord xmargin, FL_Coord ymargin)
{
    FL_OBJECT *ob;
    int sw, sh, osize;
    double factor, xfactor, yfactor;
    FL_Coord i;

    if (fl_no_connection)
	return;

    fl_get_string_dimension(obj->lstyle, obj->lsize, obj->label,
			    strlen(obj->label), &sw, &sh);

    if (sw <= (obj->w - 2 * (FL_abs(obj->bw) + xmargin)) &&
	sh <= (obj->h - 2 * (FL_abs(obj->bw) + ymargin)))
	return;

    if ((osize = obj->w - 2 * (FL_abs(obj->bw) + xmargin)) <= 0)
	osize = 1;
    xfactor = (double) sw / osize;

    if ((osize = obj->h - 2 * (FL_abs(obj->bw) + ymargin)) <= 0)
	osize = 1;
    yfactor = (double) sh / osize;


    factor = FL_max(xfactor, yfactor);

    if (factor > 1.5)
	factor = 1.5;

    i = 0;
    fl_scale_length(&i, &obj->form->w, factor);
    i = 0;
    fl_scale_length(&i, &obj->form->h, factor);

    /* must suspend gravity and rescale */
    for (ob = obj->form->first; ob; ob = ob->next)
    {
	if (ob->objclass != FL_BEGIN_GROUP &&
	    ob->objclass != FL_END_GROUP)
	    fl_scale_object(ob, factor, factor);
    }
    fl_redraw_form(obj->form);
}

int
fl_is_good_form(FL_FORM * form)
{
    FL_FORM **f = forms, **fe;

    for (fe = f + formnumb; form && f < fe; f++)
	if (*f == form)
	    return 1;

    if (form)
	M_warn(0, "skipped invisible form");

    return 0;
}


void
fl_recount_auto_object(void)
{
    int i;
    for (auto_count = i = 0; i < formnumb; i++)
	if (forms[i]->has_auto)
	    auto_count++;
}

void
fl_addto_group(FL_OBJECT * group)
{
    if (group == 0)
    {
	fl_error("addto_group", "trying add to null group");
	return;
    }

    if (group->objclass != FL_BEGIN_GROUP)
    {
	fl_error("addto_group", "parameter is not a group object");
	return;
    }

    if (fl_current_form && fl_current_form != group->form)
    {
	fl_error("addto_group", "can't switch to a group on different from");
	return;
    }

    if (fl_current_group && fl_current_group != group)
    {
	fl_error("addto_group", "you forgot to call fl_end_group");
	fl_end_group();
    }

    reopened_group = 1;
    reopened_group += (fl_current_form == 0) ? 2 : 0;
    fl_current_form = group->form;
    fl_current_group = group;
}

int
fl_form_is_visible(FL_FORM * form)
{
    return (form && form->window) ? form->visible : FL_INVISIBLE;
}


/* similar to fit_object_label, but will do it for all objects and has a smaller
 * threshold. Mainly intended for compensation for font size variations
 */
double
fl_adjust_form_size(FL_FORM * form)
{
    FL_OBJECT *ob;
    float xfactor, yfactor, max_factor, factor;
    int sw, sh, osize;
    float xm = 0.5f, ym = 0.5f;
    int bw;

    if (fl_no_connection)
	return 1.0f;

    max_factor = factor = 1.0f;
    for (ob = form->first; ob; ob = ob->next)
    {
	if ((ob->align == FL_ALIGN_CENTER || (ob->align & FL_ALIGN_INSIDE) ||
	     ob->objclass == FL_INPUT) &&
	    !ob->is_child && *(ob->label) && ob->label[0] != '@' &&
	    ob->boxtype != FL_NO_BOX &&
	    (ob->boxtype != FL_FLAT_BOX || ob->objclass == FL_MENU))
	{
	    fl_get_string_dimension(ob->lstyle, ob->lsize, ob->label, 
                       strlen(ob->label), &sw, &sh);

	    bw = (ob->boxtype == FL_UP_BOX || ob->boxtype == FL_DOWN_BOX) ?
		FL_abs(ob->bw) : 1;

	    if (ob->objclass == FL_BUTTON &&
		(ob->type == FL_RETURN_BUTTON || ob->type == FL_MENU_BUTTON))
		sw += FL_min(0.6f * ob->h, 0.6f * ob->w) - 1;

	    if (ob->objclass == FL_BUTTON && ob->type == FL_LIGHTBUTTON)
		sw += FL_LIGHTBUTTON_MINSIZE + 1;

	    if (sw <= (ob->w - 2 * (bw + xm)) && sh <= (ob->h - 2 * (bw + ym)))
		continue;

	    if ((osize = ob->w - 2 * (bw + xm)) <= 0)
		osize = 1;
	    xfactor = (float) sw / (float)osize;

	    if ((osize = ob->h - 2 * (bw + ym)) <= 0)
		osize = 1;
	    yfactor = (float) sh / osize;

	    if (ob->objclass == FL_INPUT)
	    {
		xfactor = 1.0f;
		yfactor = (sh + 1.6f) / osize;
	    }

	    if ((factor = FL_max(xfactor, yfactor)) > max_factor)
	    {
		max_factor = factor;
	    }
	}
    }

    if (max_factor <= 1.0f)
	return 1.0f;

    max_factor = 0.01f * (int) (max_factor * 100.0f);

    if (max_factor > 1.25f)
	max_factor = 1.25f;

    /* scale all objects. No gravity */
    for (ob = form->first; ob; ob = ob->next)
    {
	if (ob->objclass != FL_BEGIN_GROUP && ob->objclass != FL_END_GROUP)
	    fl_scale_object(ob, max_factor, max_factor);
    }

    sw = 0;
    fl_scale_length(&sw, &form->w, max_factor);
    sw = 0;
    fl_scale_length(&sw, &form->h, max_factor);

    fl_redraw_form(form);

    return max_factor;
}

long
fl_mouse_button(void)
{
    return fl_context->mouse_button;
}

FL_TARGET *
fl_internal_init(void)
{
    static FL_TARGET *default_flx;

    if (!default_flx)
	default_flx = fl_calloc(1, sizeof(*default_flx));

    return flx = default_flx;
}

/* fl_display is exposed to the outside world. Bad */
void
fl_switch_target(FL_TARGET * newtarget)
{
    flx = newtarget;
    fl_display = flx->display;
}

void
fl_restore_target(void)
{
    fl_internal_init();
    fl_display = flx->display;
}


static int
fl_XLookupString(XKeyEvent * xkey, char *buf, int buflen, KeySym * ks)
{
    int len = INT_MIN;

    if (!fl_context->xic)
    {
	len = XLookupString(xkey, buf, buflen, ks, 0);
    }
    else
    {
	Status status;

	if (XFilterEvent((XEvent *) xkey, None))
	{
	    *ks = NoSymbol;
	    return 0;
	}

	len =
	    XmbLookupString(fl_context->xic, xkey, buf, buflen, ks, &status);

	switch (status)
	{
	case XBufferOverflow:
	    len = -len;
	    break;
	default:
	    break;
	}
    }
    return len;
}