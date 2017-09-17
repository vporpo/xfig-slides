/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 2013-2017 by Vasileios Porpodas (Slides support)
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and documentation
 * files (the "Software"), including without limitation the rights to use,
 * copy, modify, merge, publish distribute, sublicense and/or sell copies of
 * the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

#include "config.h"

#ifdef SLIDES_SUPPORT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "w_util.h"
#include "w_setup.h"
#include "w_snap.h"
#include "w_drawprim.h"
#include "w_file.h"
#include "f_read.h"
#include "u_redraw.h"
#include <ctype.h>
#include "w_slides.h"
#include <limits.h>
#include "w_canvas.h"
#include "u_search.h"
#include "w_msgpanel.h"
#include "f_util.h"
#include "w_dir.h"
#include "w_cmdpanel.h"
#include "u_undo.h"
#include "w_export.h"
#include "w_mousefun.h"
#include "w_cursor.h"
#include "w_color.h"
#include "f_save.h"

DeclareStaticArgs(20);

extern int last_action;         /* u_undo.c */

/* Deep copy of slides */
void copy_slides_from_to(slides_t from, slides_t to) {
  memcpy(to, from, sizeof(*from));
}

/* **************** */
/* Object Utilities */
/* **************** */

/* Convert ojbect.h types to strings */
const char *
get_type_name(int type)
{
  switch(type) {
  case O_COLOR_DEF:
    return "O_COLOR_DEF";
  case O_ELLIPSE:
    return "O_ELLIPSE";
  case O_POLYLINE:
    return "O_POLYLINE";
  case O_SPLINE:
    return "O_SPLINE";
  case O_TXT:
    return "O_TXT";
  case O_ARC:
    return "O_ARC";
  case O_COMPOUND:
    return "O_COMPOUND";
  case O_END_COMPOUND:
    return "O_END_COMPOUND";
  case O_FIGURE:
    return "O_FIGURE";
  case O_ALL_OBJECT:
    return "O_ALL_OBJECT";
  default:
    assert(0);
  }
}


/* Call FUNC (OBJ, TYPE) for each object in the compound C.
   If RECURSIVE then recurse to other coumpounds in C

   WARNING: must reset for_all_cnt before calling this function!!!
*/
static int for_all_cnt = 0;
void
for_all_objects_in_compound_do(F_compound *c,
                               void (*func)(void *obj, int type, int cnt, void *extra),
                               void *extra, Boolean recursive)
{
  F_line *l;
  F_spline *s;
  F_ellipse *e;
  F_text *t;
  F_arc *a;
  F_compound *c1;
  for (a = c->arcs; a != NULL; a = a->next)
    func((void *)a, O_ARC, for_all_cnt++, extra);
  for (e = c->ellipses; e != NULL; e = e->next)
    func((void *)e, O_ELLIPSE, for_all_cnt++, extra);
  for (l = c->lines; l != NULL; l = l->next)
    func((void *)l, O_POLYLINE, for_all_cnt++, extra);
  for (s = c->splines; s != NULL; s = s->next)
    func((void *)s, O_SPLINE, for_all_cnt++, extra);
  for (t = c->texts; t != NULL; t = t->next)
    func((void *)t, O_TXT, for_all_cnt++, extra);
  for (c1 = c->compounds; c1 != NULL; c1 = c1->next) {
    func((void *)c1, O_COMPOUND, for_all_cnt++, extra);
    if (recursive) {
      for_all_objects_in_compound_do(c1, func, extra, recursive);
    }
  }
}


/* Call FUNC(OBJ, TYPE) for each object.
   TYPEs defined in object.h (O_ELLIPSE etc.).
   If RECURSIVE, then iterate over objects in compounds. */
static int
for_all_objects_do(void (*func) (void *obj, int type, int cnt, void *extra),
                   void *extra, Boolean recursive)
{
  for_all_cnt = 0;
  for_all_objects_in_compound_do(&objects, func, extra, recursive);
  return for_all_cnt;
}


/* Execute FUNC () for OBJ of OBJ_TYPE.
   If RECURSIVE, then execute FUNC() for all the objects it contains. */
static void
for_all_objects_in_object_do(void *obj, int obj_type,
                             void (*func)(void *obj, int type, int cnt, void *extra),
                             void *extra, Boolean recursive)
{
  for_all_cnt = 0;
  if (obj_type == O_COMPOUND) {
    func(obj, obj_type, ++for_all_cnt, extra);
    for_all_objects_in_compound_do((F_compound *) obj, func, extra, recursive);
  } else {
    func(obj, obj_type, ++for_all_cnt, extra);
  }
}


/* ******************** */
/* Interfaces to SLIDES */
/* ******************** */

/* Safely set the I'th slide of SLIDES to VALUE.
   Return False on failure. */
Boolean
slide_set(slides_t slides, int i, Boolean value)
{
  /* Check the slide I */
  if (i < FIRST_SLIDE || i > LAST_SLIDE) {
    put_msg("ERROR: Only slide numbers >= 1 and < MAX_SLIDE are allowed!");
    beep();
    return False;
  }

  int bitmap_idx = get_bitmap_idx(i);
  if (value == True) {
    if (! slides->bitmap[bitmap_idx]) {
      if (bitmap_idx <= LAST_SLIDE) {
        slides->bitmap[bitmap_idx] = True;
        slides->cnt++;
      } else {
        put_msg("ERROR: Will exceed MAX_SLIDES limit!");
        beep();
        return False;
      }
      assert(slides->cnt <= MAX_SLIDES && "Exceeded MAX_SLIDES");
    }
  } else {
    if (slides->bitmap[bitmap_idx]) {
      if (slides_get_cnt(slides) > 1) {
        slides->bitmap[bitmap_idx] = False;
        slides->cnt--;
        /* We should not allow any object to disappear completely */
      } else {
        put_msg("ERROR: There is an object only active in this slide! "
                "Cannot delete it. Use \"Delete\" tool to delete it.");
        beep();
        return False;
      }
      assert(slides->cnt >= 1);
    }
  }
  return True;
}

/* Clear the slides bitmap */
void
slide_reset(slides_t slides)
{
  int i;
  FOR_EACH_SLIDE_I(i) {
    slides->bitmap[get_bitmap_idx(i)] = 0;
  }

  slides->cnt = 0;
  slides->fail = 0;
}

/* Return the last slide set in SLIDES.
   Returns NULL_SLIDE if no slide is set. */
int
get_last_slide(slides_t slides) {
  int si, last_slide = NULL_SLIDE;
  FOR_EACH_SLIDE_IN_SLIDES(si, slides) {
    last_slide = si;
  }
  return last_slide;
}

/* Swap SLIDE1 and SLIDE2 */
void
slide_swap(slides_t slides, int slide1, int slide2)
{
  assert(slide1 <= get_last_used_slide()
         && slide1 >= get_first_used_slide());
  assert(slide2 <= get_last_used_slide()
         && slide2 >= get_first_used_slide());
  int slide1_idx = get_bitmap_idx(slide1);
  int slide2_idx = get_bitmap_idx(slide2);
  int sv_slide1 = slides->bitmap[slide1_idx];
  slides->bitmap[slide1_idx] = slides->bitmap[slide2_idx];
  slides->bitmap[slide2_idx] = sv_slide1;
}

/* Get if SLIDES has the I'th slide enabled */
Boolean
is_slide_set(slides_t slides, int i)
{
  assert(i >= FIRST_SLIDE && "Only Slide numbers >= 1 are allowed");
  int bitmap_idx = get_bitmap_idx(i);
  return slides->bitmap[bitmap_idx];
}

/* Get the number of "ON" slides in SLIDES */
int
slides_get_cnt(slides_t slides)
{
  return slides->cnt;
}


Boolean
slides_include_slide(slides_t slides, int slide)
{
  return is_slide_set(slides, slide) > 0;
}

/* Allocate new slides. Return the newly created slides. */
slides_t
get_new_slides(void)
{
  slides_t slides = (slides_t) calloc(1, sizeof(slides[0]));
  slides->fail = False;
  slides->cnt = 0;
  return slides;
}

void
free_slides(slides_t slides)
{
  /* 1. Free the actual slides object */
  if (slides != NULL) {
    free(slides);
  }
}

slides_t
copy_slides(slides_t slides)
{
  if (slides == NULL) {
    return NULL;
  }
  slides_t new_slides = get_new_slides();
  memcpy((void *)new_slides, (const void *)slides, sizeof(slides[0]));
  return new_slides;
}

/* Return TRUE if BUF looks like a line describing slides */
char *
get_slides_line(char *buf)
{
  int cnt_matches = 0;
  for (char *c = buf; *c != '\n'; ++c) {
    if (*c == SLIDES_BEGIN_CHAR) {
      return c + 1;
    }
  }
  return NULL;
}

/* Print-append range FROM_SLIDE-TO_SLIDE to SLIDES_EDIT_STR. */
static void
append_range_to_str(char *slides_edit_str, int from_slide, int to_slide)
{
  char tmp[MAX_SLIDES_STR];
  /* We don't need a range since FROM-TO too close */
  if (to_slide <= from_slide + 1) {
    snprintf(tmp, MAX_SLIDES_STR, "%d,", to_slide);
  }
  /* We generate the FROM-TO range */
  else {
    snprintf(tmp, MAX_SLIDES_STR, "%d-%d,", from_slide, to_slide);
  }
  strcat(slides_edit_str, tmp);
}

/* Returns a string representation of SLIDES.
   It is used in the edit dialogue box. */
char *
slides_to_str(slides_t slides, const char *prefix)
{
  int slide;
  char tmp[MAX_SLIDES_STR];
  static char slides_edit_str[MAX_SLIDES_STR];
  if (slides == NULL)
    return "";

  /* Initliaze with SLIDES_BEGIN_CHAR */
  sprintf(slides_edit_str, "%s", prefix);
  /* Append the slides, e.g.:  1,2,3,4,5 */
  int from_slide = NULL_SLIDE, to_slide = NULL_SLIDE, last_slide = NULL_SLIDE;
  int last_slide_in_slides = get_last_slide(slides);
  FOR_EACH_SLIDE_IN_SLIDES(slide, slides) {
    /* 1. Collect range */
    if (slide == last_slide + 1
        || last_slide == NULL_SLIDE) {
      /* Collect range as long as slide == last_slide + 1 */
      to_slide = slide;
      if (from_slide == NULL_SLIDE) {
        from_slide = slide;
      }
    }

    /* 2. We reached the end of the range.
          Current slide is not in range. Print range collected so far. */
    if (slide != last_slide + 1 && last_slide != NULL_SLIDE) {
      append_range_to_str(slides_edit_str, from_slide, to_slide);
      from_slide = slide;
      to_slide = slide;
    }
    last_slide = slide;
  }
  /* We need to print the last range we collected. */
  append_range_to_str(slides_edit_str, from_slide, to_slide);
  /* Finalize with end-of-string */
  strcat(slides_edit_str, "\0");

  return strdup(slides_edit_str);
}



/* ************************************** */
/* Interfaces to ALL_SLIDES data structue */
/* ************************************** */

/* Clear ALL_SLIDES.ACCUM[] */
static void
all_slides_clear_accum(void)
{
  int i;
  FOR_EACH_SLIDE_I(i) {
    all_slides.accum[get_bitmap_idx(i)] = 0;
  }
}

/* Initialize ALL_SLIDES data structure */
static void
all_slides_init(void)
{
  all_slides.max = NULL_SLIDE;
  all_slides.min = NULL_SLIDE_MAX;
  all_slides.num_active = 0;
  all_slides_clear_accum();

  all_slides.active_min = NULL_SLIDE_MAX;
  all_slides.active_max = NULL_SLIDE;
  all_slides.active_cnt = 0;
}


/* append OBJ of TYPE into ALL_SLIDES.ACCUM[]  */
void
all_slides_append_accum(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  /* sl=obj->slides */
  SET_TO_OBJ_ATTR(sl, obj, type, slides)

    int i;
  slides_t slides = sl;
  if (slides == NULL) {
    return;
  }
  FOR_EACH_SLIDE_I(i) {
    all_slides.accum[get_bitmap_idx(i)] += (long) is_slide_set(slides, i);
  }
  FOR_EACH_SLIDE_I(i) {
    if (all_slides.accum[get_bitmap_idx(i)] > 0) {
      if (i > get_last_used_slide()) {
        all_slides.max = i;
      }
      if (i < get_first_used_slide()) {
        all_slides.min = i;
      }
    }
  }
}

/* Collect all the slide info and write it to ALL_SLIDES
   This is used regularly to keep the slides state up to date */
void
collect_all_slides_info(void)
{
  F_arc *a;
  F_compound *c;
  F_ellipse *e;
  F_line *l;
  F_spline *s;
  F_text *t;

  int si;
  int cnt_active = 0;

  all_slides_init();
  /* Accumulate data for all objects into all_slides */
  for_all_objects_do(all_slides_append_accum, NULL, True);

  /* Update all_slides.{active_min, active_max} */
  FOR_EACH_USED_SLIDE(si) {
    cnt_active ++;
    if (all_slides.active_max < si) {
      all_slides.active_max = si;
    }
    if (all_slides.active_min > si) {
      all_slides.active_min = si;
    }
  }

  /* Update all_slides.active_cnt  */
  FOR_EACH_SELECTED_SLIDE(si) {
    all_slides.active_cnt ++;
  }

  /* Update all_slides.num_active*/
  all_slides.num_active = cnt_active;
}

/* Return the last selected slide.
   If not found return NULL_SLIDE.  */
static int
get_current_slide(void)
{
  int si, last_si = NULL_SLIDE;
  FOR_EACH_SELECTED_SLIDE(si) {
    last_si = si;
  }
  return last_si;
}

/* Check whether the slide is ON/OFF */
static Boolean
active_slides_chk_get(int i)
{
  int bitmap_idx = get_bitmap_idx(i);
  return (Boolean) all_slides.active_slides_chk[bitmap_idx];
}

/* Return the first activated slide or NULL_SLIDE if not found. */
static int
active_slides_chk_get_first(void)
{
  int si;
  FOR_EACH_SELECTED_SLIDE(si)
    return si;
  return NULL_SLIDE;
}

/* Activate/Deactivate a specific slide */
static void
active_slides_chk_set(int i, char value)
{
  assert(i >= FIRST_SLIDE);
  int bitmap_idx = get_bitmap_idx(i);
  /* Saturate value i to max */
  if (value == 1 && bitmap_idx > all_slides.active_max) {
    bitmap_idx = all_slides.active_max;
  }

  if (value == 1 && all_slides.active_slides_chk[bitmap_idx] == 0) {
    all_slides.active_cnt ++;
  }

  if (value == 0 && all_slides.active_slides_chk[bitmap_idx] == 1) {
    all_slides.active_cnt --;
  }

  all_slides.active_slides_chk[bitmap_idx] = value;
  return;
}

static int
active_slides_min(void)
{
  return all_slides.active_min;
}

static int
active_slides_max(void)
{
  return all_slides.active_max;
}

/* Return the number of slides checked in the side panel */
static int
active_slides_chk_cnt(void)
{
  return all_slides.active_cnt;
}

/* Set only the SI'th with VALUE and all the reset with !VALUE */
static void
active_slides_chk_setonly(int si, Boolean value)
{
  assert(si >= FIRST_SLIDE);
  Boolean not_value = ! value;
  int i;
  FOR_EACH_SLIDE_I(i) {
    active_slides_chk_set(i, not_value);
  }
  active_slides_chk_set(si, value);
}

/* Toggle slide */
static void
active_slides_chk_flip(int i)
{
  assert(i >= FIRST_SLIDE);
  active_slides_chk_set(i, ! active_slides_chk_get(i));
}

/* Return TRUE if the slides are activated are NUM. */
static Boolean
num_of_active(int num)
{
  int i;
  int cnt = 0;
  FOR_EACH_SELECTED_SLIDE(i) {
    cnt ++;
  }
  if (cnt == num) {
    return True;
  }
  return False;
}

/* Return the total number of slides in this session. */
int
num_of_used_slides(void)
{
  int i;
  int cnt = 0;
  FOR_EACH_USED_SLIDE(i) {
    cnt ++;
  }
  return cnt;
}

/* Return the only selected slide.
   If more are selected or not found return NULL_SLIDE.  */
static int
get_single_current_slide(void)
{
  int si, last_si = NULL_SLIDE;
  if (! num_of_active(1)) {
    put_msg("ERROR: Please select 1 slide only!");
    beep();
    return NULL_SLIDE;
  }
  FOR_EACH_SELECTED_SLIDE(si) {
    last_si = si;
  }
  return last_si;
}

int
get_last_used_slide(void)
{
  return all_slides.max;
}

int
get_first_used_slide(void)
{
  return all_slides.min;
}

static int
get_first_used_slide_safe(void) {
  int first_slide = get_first_used_slide();
  if (first_slide == NULL_SLIDE_MAX) {
    first_slide = FIRST_SLIDE;
  }
  return first_slide;
}

void
reset_side_slides(void)
{
  int i;
  FOR_EACH_SLIDE_I(i) {
    active_slides_chk_set(i, 0);
  }
}


/* Return a static string (csv) of the slides that are currently selected. */
char *
selected_slides_str(void)
{
  static char buf[MAX_SLIDES_STR];
  int si;
  int limit = MAX_SLIDES_STR;

  buf[0] = '\0';
  FOR_EACH_SELECTED_SLIDE(si) {
    char sld_str[80];
    sld_str[0] = '\0';
    snprintf(sld_str, 80, "%d,", si);
    strcat(buf, sld_str);
    limit -= strlen(sld_str);
    assert(limit > 0);		/* string too long */
  }
  assert(strlen(buf) > 0);
  return buf;
}


/* Return TRUE if OBJ should be displayed. 
   TYPE is one of the O_* types in object.h */
Boolean
active_object_slides(void *obj, int type)
{
  slides_t obj_sl;
  SET_TO_OBJ_ATTR(obj_sl, obj, type, slides);
  return active_slides(obj_sl);
}

/* Return TRUE if slides1 and slides2 differ */
Boolean slides_differ(slides_t slides1, slides_t slides2) {
  return memcmp(slides1->bitmap, slides2->bitmap,
                MAX_SLIDES * sizeof(slides1->bitmap[0])) != 0;
}

/* Return TRUE if SLIDES is active. Does NOT check parent !!! */
Boolean
active_slides(slides_t slides)
{
  /* If in Preview, then return TRUE */
  if (canvas_win != main_canvas) {
    return True;
  }

  if (slides == NULL) {
    return True;
  }

  int i;
  FOR_EACH_SLIDE_IN_SLIDES(i, slides) {
    if (active_slides_chk_get(i)) {
      return True;
    }
  }
  return False;
}


/* Return TRUE if we should display CMPND */
Boolean
any_active_slides_in_compound(F_compound *cmpnd)
{
  F_ellipse *e;
  F_arc *a;
  F_line *l;
  F_spline *s;
  F_text *t;
  F_compound *c;

  /* If the compound itself is active then return TRUE */
  if (active_object_slides(cmpnd, O_COMPOUND))
    return True;
  for (a = cmpnd->arcs; a != NULL; a = a->next)
    if (active_object_slides(a, O_ARC))
      return True;
  for (e = cmpnd->ellipses; e != NULL; e = e->next)
    if (active_object_slides(e, O_ELLIPSE))
      return True;
  for (l = cmpnd->lines; l != NULL; l = l->next)
    if (active_object_slides(l, O_POLYLINE))
      return True;
  for (s = cmpnd->splines; s != NULL; s = s->next)
    if (active_object_slides(s, O_SPLINE))
      return True;
  for (t = cmpnd->texts; t != NULL; t = t->next)
    if (active_object_slides(t, O_TXT))
      return True;
  for (c = cmpnd->compounds; c != NULL; c = c->next)
    if (any_active_slides_in_compound(c))
      return True;
  return False;
}


static Boolean
is_first_used_slide(int slide)
{
  int si;
  FOR_EACH_USED_SLIDE(si) {
    if (si == slide) {
      return True;
    }
    break;
  }
  return False;
}

/* Return TRUE if SLIDE is the biggest of the used slides. */
static Boolean
is_last_used_slide(int slide)
{
  int si;
  FOR_EACH_USED_SLIDE_REVERSE(si) {
    if (si == slide) {
      return True;
    }
    break;
  }
  return False;
}

enum direction_
  {
    DIR_NONE,
    DIR_NORMAL,
    DIR_REVERSE,
    DIR_MAX
  };
typedef enum direction_ dir_t;

/* Return the opposite direction  */
dir_t
get_opposite_direction(dir_t dir) {
  switch (dir) {
  case DIR_NORMAL:
    return DIR_REVERSE;
  case DIR_REVERSE:
    return DIR_NORMAL;
  default:
    assert(0 && "bad DIR");
  }
}

/* Return the nearest active slide at the direction DIR. */
static int
get_nearest_used(int si, dir_t dir)
{
  int i;
  if (dir == DIR_NORMAL) {
    for (i = si + 1; i<=all_slides.max; i++)
      if (all_slides.accum[get_bitmap_idx(i)] > 0)
        return i;
    return NULL_SLIDE;
  } else if (dir == DIR_REVERSE) {
    for (i = si - 1; i>=all_slides.min; i--)
      if (all_slides.accum[get_bitmap_idx(i)] > 0)
        return i;
    return NULL_SLIDE;
  } else {
    assert(0 && "Bad dir");
  }
}

/* Return the nearest slide at the direction DIR. */
static int
get_nearest(int si, dir_t dir)
{
  int i;
  if (dir == DIR_NORMAL) {
    for (i = si + 1; i <= all_slides.active_max; i++)
      return i;
    return NULL_SLIDE;
  }
  else if (dir == DIR_REVERSE) {
    for (i = si - 1; i >= all_slides.min; i--)
      return i;
    return NULL_SLIDE;
  }
  else
    assert(0 && "Bad dir");
}


/* ************************************* */
/* Slides ... dialog window in File menu */
/* ************************************* */

/* Usage: gen_slide_fname ("1", LANG_FIG) */
char *
gen_slide_fname(int slide, int lang_enum) {
  static char name_nosuffix[PATH_MAX];
  static char slide_expname[PATH_MAX];
  snprintf(name_nosuffix, PATH_MAX, "%s", strdup(cur_filename));

  /* FIXME: Dirty hack to remove suffix: */
  name_nosuffix[strlen(name_nosuffix) - strlen(".fig")] = '\0';

  snprintf(slide_expname, PATH_MAX, "%s-%02d.%s", name_nosuffix, slide,
           lang_items[lang_enum]);

  return slide_expname;
}

/* Helper for sorting slides */
static int
slides_cmp(const void *v_slide1, const void *v_slide2) {
  const char *slide1 = *(const char **) v_slide1;
  const char *slide2 = *(const char **) v_slide2;
  return strcmp(slide1, slide2);
}

/* The text box that describes all the .fig files that will be generated. */
static char *
get_slides_txtbox(void) {
  int slide, i;
  static char all_slides_fnames[MAX_SLIDES * PATH_MAX];
  char buf[80];
  all_slides_fnames[0] = '\0';

  /* FIXME: Make all sprintf and strcat buffer safe  */

  sprintf(all_slides_fnames, "Slides: ");
  FOR_EACH_USED_SLIDE(slide) {
    snprintf(buf, 80, "%d,", slide);
    strcat(all_slides_fnames, buf);
  }
  strcat(all_slides_fnames, "\n----------------------------\nSlide files:\n");

  FOR_EACH_USED_SLIDE(slide) {
    static char formatted_fname[PATH_MAX];
    char *slide_fname = gen_slide_fname(slide, LANG_FIG);
    snprintf(formatted_fname, PATH_MAX, "%2d: %s\n", slide, slide_fname);
    strcat(all_slides_fnames, formatted_fname);
  }
  return all_slides_fnames;
}


Boolean slides_up = False;
Widget slides_popup, slides_panel, slides_msg, slides_panel_cancel_button;
Widget slides_label;
Widget slides_panel_save_button, slides_panel_export_button;

/* Callback to cancel (Slides dialog) */
static void
slides_panel_cancel_cb(Widget w, XButtonEvent *ev)
{
  XtPopdown(slides_popup);
  slides_up = False;
}

/* Callback to save all slides (Slides dialog) */
static void
slides_panel_save_cb(Widget w, XButtonEvent *ev)
{
  popup_file_slides_panel();
  /* Close window after the file panel */
  slides_panel_cancel_cb(w, ev);
}

/* Callback to export all slides (Slides dialog) */
static void
slides_panel_export_cb(Widget w, XButtonEvent *ev)
{
  extern Boolean slides_export_flag;
  slides_export_flag = True;
  popup_export_panel(w);
  /* Close window after the file panel */
  slides_panel_cancel_cb(w, ev);
  slides_export_flag = False;
}



#ifndef XAW3D1_5E
/* popup message over command button when mouse enters it */
static void slides_balloon_trigger(Widget widget, XtPointer closure,
                                   XEvent *event,
                                   Boolean *continue_to_dispatch);
static void slides_unballoon(Widget widget, XtPointer closure, XEvent *event,
                             Boolean *continue_to_dispatch);
#endif /* XAW3D1_5E */

static String slides_translations =
  "<Message>WM_PROTOCOLS: CloseSlides()\n\
   <Key>Return: SaveSlides()\n\
   <Key>Escape: CloseSlides()";

static XtActionsRec slides_actions[] = {
  {"CloseSlides", (XtActionProc) slides_panel_cancel_cb},
  {"SaveSlides", (XtActionProc) slides_panel_save_cb},
};

/* Create the slides dialog. */
static void
create_slides_panel(void)
{
  Widget file, beside, below, butbelow;
  XFontStruct *temp_font;
  static Boolean actions_added=False;
  Widget preview_form;
  Position xposn, yposn;
  int WIDTH = 400, HEIGHT = 300;
  int NUM_OF_BUTTONS = 3;
  int BUTTON_HEIGHT = 25, BUTTON_WIDTH = 115;
  int BUTTON_SEPARATOR=10;
  static Boolean slides_actions_added = False;

  /* Read slides so we can use this info in the windows */
  collect_all_slides_info();

  XtTranslateCoords(tool, (Position) 0, (Position) 0, &xposn, &yposn);

  FirstArg(XtNx, xposn+50);
  NextArg(XtNy, yposn+50);
  NextArg(XtNtitle, "Xfig: Slides menu");
  NextArg(XtNcolormap, tool_cm);
  NextArg(XtNwidth, WIDTH);
  NextArg(XtNheight, HEIGHT);
  slides_popup = XtCreatePopupShell("slides_popup",
                                    transientShellWidgetClass,
                                    tool, Args, ArgCount);

  /* Keyboard shortcuts */
  if (! slides_actions_added) {
    slides_actions_added = True;
    XtAppAddActions(tool_app, slides_actions,
                    XtNumber(slides_actions));
  }

  slides_panel = XtCreateManagedWidget("slides_panel",
                                       formWidgetClass,
                                       slides_popup,
                                       NULL, ZERO);
  /* Keyboard shortcuts */
  XtOverrideTranslations(slides_panel,
                         XtParseTranslationTable(slides_translations));

  /* Slides: */
  char label_txt[80];
  snprintf(label_txt, 80,
           "Generating %d slides (csv):", all_slides.max);
  FirstArg(XtNlabel, label_txt);
  NextArg(XtNjustify, XtJustifyLeft);
  NextArg(XtNborderWidth, 0);
  SetValues(slides_panel);
  NextArg(XtNresize, False);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainLeft);
  slides_label = XtCreateManagedWidget("slides_label", labelWidgetClass,
                                       slides_panel, Args, ArgCount);
  /* Keyboard shortcuts */
  XtOverrideTranslations(slides_label,
                         XtParseTranslationTable(slides_translations));

  /* MSG size */
  FirstArg(XtNwidth, WIDTH - 8);
  NextArg(XtNheight, HEIGHT - BUTTON_HEIGHT - 4*INTERNAL_BW - 3 * 2 - 20);

  /* Vertical Alignment */
  NextArg(XtNfromVert, slides_label);
  NextArg(XtNvertDistance, 2);

  NextArg(XtNfromHoriz, 0);

  /* NextArg(XtNhorizDistance, 0); */
  NextArg(XtNborderWidth, INTERNAL_BW);

  /* EXTRA INFO */
  NextArg(XtNeditType, XawtextRead);
  NextArg(XtNdisplayCaret, False);

  NextArg(XtNscrollHorizontal, XawtextScrollWhenNeeded);
  NextArg(XtNscrollVertical, XawtextScrollAlways);

  /* Chaining */
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainBottom);
  slides_msg = XtCreateManagedWidget("slides_msg",
                                     asciiTextWidgetClass,
                                     slides_panel,
                                     Args, ArgCount);
  /* Keyboard shortcuts */
  XtOverrideTranslations(slides_msg,
                         XtParseTranslationTable(slides_translations));

  FirstArg(XtNlabel, "Cancel");
  /* Horizontal Alignment */
  NextArg(XtNfromHoriz, 0);
  NextArg(XtNhorizDistance, 0.5 * (WIDTH - (NUM_OF_BUTTONS * BUTTON_WIDTH
                                            + (NUM_OF_BUTTONS - 1)
                                            * BUTTON_SEPARATOR)));
  /* Vertical Alignment */
  NextArg(XtNfromVert, slides_msg);
  NextArg(XtNvertDistance, 2);
  /* Button size */
  NextArg(XtNheight, BUTTON_HEIGHT);
  NextArg(XtNwidth, BUTTON_WIDTH);

  NextArg(XtNborderWidth, INTERNAL_BW);
  NextArg(XtNtop, XtChainBottom);
  NextArg(XtNbottom, XtChainBottom);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainLeft);
  slides_panel_cancel_button = XtCreateManagedWidget("cancel",
                                                     commandWidgetClass,
                                                     slides_panel,
                                                     Args, ArgCount);

  XtAddEventHandler(slides_panel_cancel_button,
                    ButtonReleaseMask, False,
                    (XtEventHandler)slides_panel_cancel_cb,
                    (XtPointer) NULL);

  /* Slides button */
  FirstArg(XtNlabel, "Generate Slides");
  /* Horizontal Alignment */
  NextArg(XtNfromHoriz, slides_panel_cancel_button);
  NextArg(XtNhorizDistance, BUTTON_SEPARATOR);
  /* Vertical Alignment */
  NextArg(XtNfromVert, slides_msg);
  NextArg(XtNvertDistance, 2);
  /* Button size */
  NextArg(XtNheight, BUTTON_HEIGHT);
  NextArg(XtNwidth, BUTTON_WIDTH);

  NextArg(XtNborderWidth, INTERNAL_BW);
  NextArg(XtNtop, XtChainBottom);
  NextArg(XtNbottom, XtChainBottom);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainLeft);
  slides_panel_save_button = XtCreateManagedWidget("save_slides",
                                                   commandWidgetClass,
                                                   slides_panel,
                                                   Args, ArgCount);
  XtAddEventHandler(slides_panel_save_button,
                    ButtonReleaseMask, False,
                    (XtEventHandler)slides_panel_save_cb,
                    (XtPointer) NULL);

  /* Export Slides button */
  FirstArg(XtNlabel, "Export Slides");
  /* Horizontal Alignment */
  NextArg(XtNfromHoriz, slides_panel_save_button);
  NextArg(XtNhorizDistance, BUTTON_SEPARATOR);
  /* Vertical Alignment */
  NextArg(XtNfromVert, slides_msg);
  NextArg(XtNvertDistance, 2);
  /* Button size */
  NextArg(XtNheight, BUTTON_HEIGHT);
  NextArg(XtNwidth, BUTTON_WIDTH);

  NextArg(XtNborderWidth, INTERNAL_BW);
  NextArg(XtNtop, XtChainBottom);
  NextArg(XtNbottom, XtChainBottom);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainLeft);
  slides_panel_export_button = XtCreateManagedWidget("export_slides",
                                                     commandWidgetClass,
                                                     slides_panel,
                                                     Args, ArgCount);
  XtAddEventHandler(slides_panel_export_button,
                    ButtonReleaseMask, False,
                    (XtEventHandler)slides_panel_export_cb,
                    (XtPointer) NULL);

  /* We are done with setting up the window.*/
  XawTextBlock block;
  char *tmpstr = get_slides_txtbox();
  static int slides_msg_length = 0;
  block.firstPos = 0;
  block.ptr = tmpstr;
  block.length = strlen(tmpstr);
  block.format = FMT8BIT;
  /* make editable to add new message */
  FirstArg(XtNeditType, XawtextEdit);
  SetValues(slides_msg);
  /* insert the new message after the end */
  (void) XawTextReplace(slides_msg,slides_msg_length,
                        slides_msg_length, &block);
  (void) XawTextSetInsertionPoint(slides_msg, slides_msg_length);

  /* make read-only again */
  FirstArg(XtNeditType, XawtextRead);
  SetValues(slides_msg);
  slides_msg_length += block.length;
}


/* Helper for get_cum_slides_in_compound() */
static void
accum_slides_helper(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  slides_t sl_accum = (slides_t) extra;
  SET_TO_OBJ_ATTR(sl, obj, type, slides);
  int i;
  FOR_EACH_SLIDE_I(i) {
    if (is_slide_set(sl, i)) {
      slide_set(sl_accum, i, True);
    }
  }
}

/* Return the cummulative slides of the compound C. */
void
get_cum_slides_in_compound(F_compound *c, slides_t sl_accum)
{
  slide_reset(sl_accum);
  for_all_cnt = 0;
  for_all_objects_in_compound_do(c, accum_slides_helper, sl_accum, True);
}

/* This is the function called when the Slides... option is selected.
   This is set in w_cmdpanel.c */
void
popup_slides_panel(void)
{
  DeclareArgs(5);

  /* turn off Compose key LED */
  setCompLED(0);


  /* already up? */
  if (slides_up) {
    /* yes, just raise to top */
    XRaiseWindow(tool_d, XtWindow(slides_popup));
    return;
  }

  /* create it if it isn't already created */
  if (!slides_popup) {
    create_slides_panel();
  } else {
    /* FirstArg(XtNstring, cur_file_dir); */
    /* SetValues(file_dir); */
    Rescan(0, 0, 0, 0);
  }

  FirstArg(XtNtitle, "Xfig: Slides");
  SetValues(slides_popup);

  XtPopup(slides_popup, XtGrabNone);

  set_cmap(XtWindow(slides_popup));
  XSetWMProtocols(tool_d, XtWindow(slides_popup), &wm_delete_window, 1);

  slides_up = True;

  reset_cursor();

  Rescan(0, 0, 0, 0);
  ;
}

/* Save for the file panel */
void
do_save_slides(Widget w, XButtonEvent *ev)
{
  int current;
  extern Boolean sv_slides;
  sv_slides = True;
  do_save(w, ev);
  /* sv_slides is set to False in do_save() after save_slide_files() */
}

/* Returns 0 on failure */
static int
append_slides(slides_t slides, int new_slide_num)
{
  if (new_slide_num > LAST_SLIDE) {
    return 0;
  }
  slide_set(slides, new_slide_num, True);
  return 1;
}

/* Return TRUE if STR is an integer, FALSE otherwise. */
static Boolean
is_int(char *str)
{
  const char *c;
  for (c = str; *c != '\0'; c++) {
    if (! isdigit(*c)) {
      return False;
    }
  }
  return True;
}

/* Acceptable tokens are:
   i) integers (e.g. '7'). Returns 1.
   ii) ranges of integers (e.g. '3-7'). Returns the number of slides in range.
   The range of slides are placed into slides_array[].
   Returns the number of slides in the token.
   Returns 0 on failure.
 */
static int
parse_slides_in_token(char *token_str, int slides_array[MAX_SLIDES])
{
  /* i) integer */
  if (is_int(token_str)) {
    slides_array[0] = atoi(token_str);
    return 1;
  }
  /* ii) range of integers */
  int slide_range[2];
  int cnt = 0;
  char *saveptr;
  char *token = strtok_r(token_str, "-", &saveptr);
  while (token) {
    if (!is_int(token)) {
      /* Slide not an integer */
      file_msg("Slide '%s' not an integer. Should be in [%d,%d] !",
               token, FIRST_SLIDE, LAST_SLIDE);
      beep();
      return 0;
    }
    int slide = atoi(token);
    if (cnt > 1) {
      /* Not in NUM-NUM format! */
      file_msg("Bad range: '%d-%d-%d'", slide_range[0],
               slide_range[1], slide_range[2]);
      beep();
      return 0;
    }
    if (slide < FIRST_SLIDE || slide > LAST_SLIDE) {
      /* Slide out of bounds */
      file_msg("Slide '%d' out of bounds. Should be in [%d,%d] !",
               slide, FIRST_SLIDE, LAST_SLIDE);
      beep();
      return 0;
    }
    slide_range[cnt++] = slide;
    token = strtok_r(NULL, "-", &saveptr);
  }

  int slide_from = slide_range[0];
  int slide_to = slide_range[1];
  if (slide_from > slide_to
      || slide_from < FIRST_SLIDE
      || slide_to > LAST_SLIDE) {
    /* Invalid range */
    file_msg("Range out of bounds: '%d-%d'. Should be in [%d,%d] !",
             slide_from, slide_to, FIRST_SLIDE, LAST_SLIDE);
    beep();
    return 0;
  }

  /* The range looks valid. Enumerate the slides in the range. */
  for (int slide = slide_from, i = 0; slide <= slide_to; ++slide, ++i) {
    if (i > LAST_SLIDE) {
      /* Too many slides! Increase the size of slides_array[]. */
      file_msg("Attempting to set slide %d. Too many slides! Should be <= %d.",
               i, LAST_SLIDE);
      beep();
      return 0;
    }
    slides_array[i] = slide;
  }
  return slide_to - slide_from + 1;
}

/* Parse the CSV inside BUF and return the corresponding slides.
   BUF is a '\0' terminated string.
   This is used in e_edit.c new_generic_values(). */
slides_t
parse_slides_str(char *buf)
{
  slides_t slides = get_new_slides();
  char *saveptr;
  char *token = strtok_r(buf, ",", &saveptr);
  while (token) {
    /* Found end of slides, return */
    if (token[0] == '\n') {
      return slides;
    }
    int slides_in_token_array[MAX_SLIDES];
    int num_slides_in_token = parse_slides_in_token(token, slides_in_token_array);
    /* Error if parsing the token failed.
       We don't fail. Instead we skip it and just keep parsing. */
    if (num_slides_in_token > 0) {
      /* The token contains 1 or more slides in slides_in_token_array[]. */
      for (int i = 0; i != num_slides_in_token; ++i) {
        int slide = slides_in_token_array[i];
        assert(slide >= FIRST_SLIDE && slide <= LAST_SLIDE);
        int ret = append_slides(slides, slide);
        assert(ret);
      }
    }
    token = strtok_r(NULL, ",", &saveptr);
  }
  return slides;
}


void
check_missing_slide_file(void)
{
  char buf[80];
  snprintf(buf, 80,
           "ERROR: Missing file: %s. Please Generate Slides first!",
           export_slide_file_not_found);
  if (export_slide_file_not_found) {
    file_msg(buf);
    beep();
  }
  free(export_slide_file_not_found);
  export_slide_file_not_found = NULL;
}


/* *************************** */
/*          SIDE PANEL         */
/* *************************** */

int
get_bitmap_idx(int slide_i) {
  return slide_i - FIRST_SLIDE;
}

/* Return TRUE if SLIDE_I is active. */
Boolean
is_slide_active(int slide_i)
{
  int bitmap_idx = get_bitmap_idx(slide_i);
  return all_slides.accum[bitmap_idx] > 0;
}

void
slide_active_clear(int slide_i)
{
  int bitmap_idx = get_bitmap_idx(slide_i);
  all_slides.accum[bitmap_idx] = 0;
}

Widget slides_side_form;
static Boolean gray_slides = True;
static int saved_min_slide, saved_max_slide; /* saved min/max depth */

/* LOCALS */
static void toggle_show_depths(void);
static void save_active_layers(void);
static void save_depths(void);
static void save_counts(void);
static void save_counts_and_clear(void);
static void restore_active_layers(void);
static void restore_depths(void);
static void restore_counts(void);


static Widget all_active_but, all_inactive_but, toggle_all_but;
static Widget play_but, playrev_but, new_but, del_but;
static Widget graytoggle, blanktoggle, graylabel, blanklabel;
static Widget slides_viewp, slides_canvas;
static void toggle_slide(Widget w, XButtonEvent *event, String *params, Cardinal *nparams);
static void active_slides_chk_all_on(void);
static void active_slides_chk_all_on_cb(Widget w, XtPointer closure, XtPointer call_data);
static void active_slides_chk_play_cb(Widget w, XtPointer closure, XtPointer call_data);
static void active_slides_chk_rplay_cb(Widget w, XtPointer closure, XtPointer call_data);
static void move_slide_up_down(Widget w, XtPointer closure, XtPointer call_data, dir_t direction);
static void new_slide_cb(Widget w, XtPointer closure, XtPointer call_data);
static void del_slide_cb(Widget w, XtPointer closure, XtPointer call_data);
static void move_slide_up_cb(Widget w, XtPointer closure, XtPointer call_data);
static void move_slide_down_cb(Widget w, XtPointer closure, XtPointer call_data);
static void active_slides_chk_all_off(void);
static void active_slides_chk_all_off_cb(Widget w, XtPointer closure, XtPointer call_data);
static void toggle_all(Widget w, XtPointer closure, XtPointer call_data);
static void slides_exposed(Widget w, XExposeEvent *event, String *params, Cardinal *nparams);

#ifndef XAW3D1_5E
/* popup message over command button when mouse enters it */
static void layer_balloon_trigger(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);
static void layer_unballoon(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);
#endif /* XAW3D1_5E */

static void draw_slides_buttons(void);


XtActionsRec slides_side_actions[] = {
  {"ExposeLayerButtons", (XtActionProc) slides_exposed},
  {"ToggleSlideButton", (XtActionProc) toggle_slide},
};

static String slides_side_translations =
  "<Expose>:ExposeLayerButtons()\n\
   <Btn1Down>:ToggleSlideButton()\n";


/* "All On" callback */
static void
active_slides_chk_all_on_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  active_slides_chk_all_on();
}

/* "All Off" callback */
static void
active_slides_chk_all_off_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  active_slides_chk_all_off();
}

void
init_slides_side_panel(Widget parent)
{
  Widget label, below;
  Widget slides_viewform;
  extern Widget layer_form; /* w_layers.c */

  /* MOUSEFUN_HT and ind_panel height aren't known yet */
  SLIDES_HT = CANVAS_HT * SLIDES_HT_CANVAS_PERCENT / 100;

  /* main form to hold all the slides stuff */
  FirstArg(XtNfromHoriz, sideruler_sw);
  NextArg(XtNfromVert, layer_form);
  NextArg(XtNvertDistance, SLIDES_DIST_FROM_LAYERS);
  NextArg(XtNdefaultDistance, 1);
  NextArg(XtNwidth, SLIDES_WD);
  NextArg(XtNheight, SLIDES_HT);
  NextArg(XtNleft, XtChainRight);
  NextArg(XtNright, XtChainRight);
  slides_side_form = XtCreateWidget("slides_side_form", formWidgetClass,
                                    parent, Args, ArgCount);
  if (appres.showslidesmanager)
    XtManageChild(slides_side_form);

  /* a label */
  FirstArg(XtNlabel, "Slides ");
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  NextArg(XtNborderWidth, 0);
  label = below = XtCreateManagedWidget("layer_label", labelWidgetClass,
                                        slides_side_form,	Args, ArgCount);

  /* buttons to make all active, all inactive or toggle all */
  FirstArg(XtNlabel, "All On");
  NextArg(XtNfromVert, label);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = all_active_but = XtCreateManagedWidget("all_active",
                                                 commandWidgetClass,
                                                 slides_side_form, Args,
                                                 ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)active_slides_chk_all_on_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Display all slides simultaneously");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */



  FirstArg(XtNlabel, "All Off");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = all_inactive_but = XtCreateManagedWidget("all_inactive",
                                                   commandWidgetClass,
                                                   slides_side_form, Args,
                                                   ArgCount);
  XtAddCallback(below, XtNcallback,
                (XtCallbackProc)active_slides_chk_all_off_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Hide all slides");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon,
                    (XtPointer) NULL);
#endif /* XAW3D1_5E */


  FirstArg(XtNlabel, "Toggle ");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = toggle_all_but = XtCreateManagedWidget("toggle_all", commandWidgetClass,
                                                 slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)toggle_all, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Toggle displayed/hidden slides");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */



  FirstArg(XtNlabel, "Play >");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = play_but = XtCreateManagedWidget("play_active", commandWidgetClass,
                                           slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)active_slides_chk_play_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger, (XtPointer) "Go to next slide (>)");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */



  FirstArg(XtNlabel, "<");
  NextArg(XtNfromHoriz, below);
  NextArg(XtNfromVert, toggle_all_but);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainLeft);
  below = playrev_but = XtCreateManagedWidget("rev_play_active", commandWidgetClass,
                                              slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)active_slides_chk_rplay_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger, (XtPointer) "Go to previous slide (<)");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */



  FirstArg(XtNlabel, "Move Up");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = new_but = XtCreateManagedWidget("move_slide_up", commandWidgetClass,
                                          slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)move_slide_up_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Move Slide Up.");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */


  FirstArg(XtNlabel, "Move Down");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = new_but = XtCreateManagedWidget("move_slide_down", commandWidgetClass,
                                          slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)move_slide_down_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Move Slide Down.");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */


  FirstArg(XtNlabel, "New Slide");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainTop);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = new_but = XtCreateManagedWidget("new_slide", commandWidgetClass,
                                          slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)new_slide_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "Create new slide. Copy current to next and move to next.");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */


  /* viewport to be able to scroll the layer buttons */
  FirstArg(XtNborderWidth, 1);
  NextArg(XtNdefaultDistance, 1);
  NextArg(XtNwidth, SLIDES_WD);
  NextArg(XtNallowVert, True);
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainTop);
  NextArg(XtNbottom, XtChainBottom);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = slides_viewp = XtCreateManagedWidget("slides_viewp", viewportWidgetClass,
                                               slides_side_form, Args, ArgCount);



  XtAddEventHandler(slides_viewp, StructureNotifyMask, False,
                    (XtEventHandler)update_slides, (XtPointer) NULL);
  /* canvas (label, actually) in which to create the buttons */
  /* we're not using real commandButtons because of the time to create
     potentially hundreds of them depending on the number of layers in the figure */
  FirstArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  /* NextArg(XtNtop, XtChainTop); */
  /* NextArg(XtNbottom, XtChainBottom); */
  NextArg(XtNlabel, "");
  slides_canvas = XtCreateManagedWidget("slides_canvas", labelWidgetClass,
                                        slides_viewp, Args, ArgCount);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(slides_canvas, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger, (XtPointer) "Tick to display/hide slide.");
  XtAddEventHandler(slides_canvas, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */



  FirstArg(XtNlabel, "DEL Slide");
  NextArg(XtNfromVert, below);
  NextArg(XtNtop, XtChainBottom);
  NextArg(XtNbottom, XtChainBottom);
  NextArg(XtNleft, XtChainLeft);
  NextArg(XtNright, XtChainRight);
  below = del_but = XtCreateManagedWidget("del_slide", commandWidgetClass,
                                          slides_side_form, Args, ArgCount);
  XtAddCallback(below, XtNcallback, (XtCallbackProc)del_slide_cb, (XtPointer) NULL);
#ifndef XAW3D1_5E
  /* popup when mouse passes over button */
  XtAddEventHandler(below, EnterWindowMask, False,
                    (XtEventHandler)slides_balloon_trigger,
                    (XtPointer) "DELETE current slide!");
  XtAddEventHandler(below, LeaveWindowMask, False,
                    (XtEventHandler)slides_unballoon, (XtPointer) NULL);
#endif /* XAW3D1_5E */


  /* make actions/translations for user to click on a layer "button" and
     to redraw buttons on expose */
  XtAppAddActions(tool_app, slides_side_actions, XtNumber(slides_side_actions));
  XtOverrideTranslations(slides_canvas,
                         XtParseTranslationTable(slides_side_translations));
}



#ifndef XAW3D1_5E
static Widget slides_balloon_popup = (Widget) 0;
static XtIntervalId balloon_id = (XtIntervalId) 0;
static Widget balloon_w;
static char *popmsg;

static void slides_balloon(Widget w, XtPointer closure, XtPointer call_data);

/* come here when the mouse passes over a button in the depths panel */
static void
slides_balloon_trigger(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
  if (!appres.showballoons)
    return;
  balloon_w = widget;
  /* save the message to popup */
  popmsg = (char *) closure;
  /* if an old balloon is still up, destroy it */
  if ((balloon_id != 0) || (slides_balloon_popup != (Widget) 0)) {
    slides_unballoon((Widget) 0, (XtPointer) 0, (XEvent*) 0, (Boolean*) 0);
  }
  balloon_id = XtAppAddTimeOut(tool_app, appres.balloon_delay,
                               (XtTimerCallbackProc) slides_balloon,
                               (XtPointer) widget);
}

/* come here when the timer times out (and the mouse is still over the button) */
static void
slides_balloon(Widget w, XtPointer closure, XtPointer call_data)
{
  Position  x, y;
  Dimension wid, ht;
  Widget box, balloon_label;
  XtWidgetGeometry xtgeom,comp;

  XtTranslateCoords(w, 0, 0, &x, &y);
  FirstArg(XtNx, x);
  NextArg(XtNy, y);
  slides_balloon_popup = XtCreatePopupShell("slides_balloon_popup",
                                            overrideShellWidgetClass, tool,
                                            Args, ArgCount);
  FirstArg(XtNborderWidth, 0);
  NextArg(XtNhSpace, 0);
  NextArg(XtNvSpace, 0);
  NextArg(XtNorientation, XtorientVertical);
  box = XtCreateManagedWidget("box", boxWidgetClass,
                              slides_balloon_popup, Args, ArgCount);

  /* make label for mouse message */
  /* if the message was passed via the callback */
  if (popmsg) {
    FirstArg(XtNborderWidth, 0);
    NextArg(XtNlabel, popmsg);
    balloon_label = XtCreateManagedWidget("l_label", labelWidgetClass,
                                          box, Args, ArgCount);
  } else {
    /* otherwise it is the two-line message with mouse indicators */
    FirstArg(XtNborderWidth, 0);
    NextArg(XtNleftBitmap, mouse_l);	/* bitmap of mouse with left button pushed */
    NextArg(XtNlabel, "Display or hide depth    ");
    balloon_label = XtCreateManagedWidget("l_label", labelWidgetClass,
                                          box, Args, ArgCount);
    FirstArg(XtNborderWidth, 0);
    NextArg(XtNleftBitmap, mouse_r);	/* bitmap of mouse with right button pushed */
    NextArg(XtNlabel, "Set current depth to this");
    (void) XtCreateManagedWidget("r_label", labelWidgetClass,
                                 box, Args, ArgCount);
  }

  /* realize the popup so we can get its size */
  XtRealizeWidget(slides_balloon_popup);

  /* get width/height */
  FirstArg(XtNwidth, &wid);
  NextArg(XtNheight, &ht);
  GetValues(balloon_label);
  /* only change X position of widget */
  xtgeom.request_mode = CWX;
  /* shift popup left */
  xtgeom.x = x-wid-5;
  /* if the mouse is in the depth button area, position the y there */
  if (w == slides_viewp) {
    int wx, wy;	/* XTranslateCoordinates wants int, not Position */
    get_pointer_win_xy(&wx, &wy);
    xtgeom.request_mode |= CWY;
    xtgeom.y = y+wy - (int) ht - 60;
  }
  (void) XtMakeGeometryRequest(slides_balloon_popup, &xtgeom, &comp);

  XtPopup(slides_balloon_popup,XtGrabNone);
  XtRemoveTimeOut(balloon_id);
  balloon_id = (XtIntervalId) 0;
}

/* come here when the mouse leaves a button in the slides panel */
static void
slides_unballoon(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
  if (balloon_id) {
    XtRemoveTimeOut(balloon_id);
  }
  balloon_id = (XtIntervalId) 0;
  if (slides_balloon_popup != (Widget) 0) {
    XtDestroyWidget(slides_balloon_popup);
    slides_balloon_popup = (Widget) 0;
  }
}
#endif /* XAW3D1_5E */



/* now that the mouse function and indicator panels have been sized
   correctly, resize our form */
void
setup_slides_side_panel(void)
{
  Dimension	 ind_ht, snap_ht=0;
  /* get height of indicator and snap panels */
  FirstArg(XtNheight, &ind_ht);
  GetValues(ind_panel);
  FirstArg(XtNheight, &snap_ht);
  GetValues(snap_indicator_panel);
  /* Get height of layers panel */
  Dimension layers_ht = 0;
  FirstArg(XtNheight, &layers_ht);
  extern Widget layer_form;     /* w_layers.c */
  GetValues(layer_form);
  int total = layers_ht * 100 / (100 - SLIDES_HT_CANVAS_PERCENT);
  SLIDES_HT = total * SLIDES_HT_CANVAS_PERCENT / 100 - 2 * SLIDES_DIST_FROM_LAYERS;

  /* now that the bitmaps have been created, put the checkmark in the proper toggle */
  XtUnmanageChild(slides_side_form);
  FirstArg(XtNheight, SLIDES_HT);
  SetValues(slides_side_form);
  XtManageChild(slides_side_form);

  redisplay_canvas();
}

/* This is enables slide 0 */
void
setup_slides_2(void)
{
  update_slides();
  redisplay_canvas();
}

#define B_WIDTH  38
#define B_BORDER 2
#define C_SIZE   10
#define TOT_B_HEIGHT (C_SIZE+2*B_BORDER)

#define draw_l(w, x1, y1, x2, y2)                   \
  XDrawLine(tool_d, w, button_gc, x1, y1, x2, y2);

/* Draw slide SLIDE_I on WIN */
static void
draw_slides_button(Window win, int slide_i)
{
  /* return; */
  char str[20];
  int x, y, w, h, i;
  int si;

  x = B_BORDER;
  w = B_WIDTH;

  y = B_BORDER;
  y += (slide_i - FIRST_SLIDE) * TOT_B_HEIGHT;
  h = TOT_B_HEIGHT;

  /* the whole border */
  draw_l(win,  x,  y,x+w,  y);
  draw_l(win,x+w,  y,x+w,y+h);
  draw_l(win,x+w,y+h,  x,y+h);
  draw_l(win,x,  y+h,  x,  y);

  /* now the checkbox border */
  x=x+B_BORDER; y=y+B_BORDER;
  w=C_SIZE;

  draw_l(win,  x,  y,x+w,  y);
  draw_l(win,x+w,  y,x+w,y+w);
  draw_l(win,x+w,y+w,  x,y+w);
  draw_l(win,  x,y+w,  x,  y);

  /* draw in the checkmark or a blank */
  if (active_slides_chk_get(slide_i))
    XCopyArea(tool_d, sm_check_pm, win, button_gc, 0, 0,
              sm_check_width, sm_check_height, x+1, y+1);
  else
    XCopyArea(tool_d, sm_null_check_pm, win, button_gc, 0, 0,
              sm_check_width, sm_check_height, x+1, y+1);

  /* now draw in the layer number for the button */
  sprintf(str, "%3d", slide_i);
  XDrawString(tool_d, win, button_gc, x+w+3*B_BORDER, y+w, str, strlen(str));
}

/* Draw slide buttons on the slides side bar */
static void
draw_slides_buttons(void)
{
  Window w = XtWindow(slides_canvas);
  XClearWindow(tool_d, w);
  int bi;
  FOR_EACH_CHK_I(bi) {
    draw_slides_button(w, bi);
  }
}

/* Activate all slides */
static void
active_slides_chk_all_on(void)
{
  int i;
  Boolean changed = False;
  collect_all_slides_info();
  FOR_EACH_USED_SLIDE(i) {
    if (!active_slides_chk_get(i)) {
      active_slides_chk_set(i, True);
      changed = True;
    }
  }

  /* only redisplay if any of the buttons changed */
  if (changed) {
    draw_slides_buttons();
    redisplay_canvas();
  }
}

static void
active_slides_chk_all_off(void)
{
  int i;
  Boolean changed = False;

  collect_all_slides_info();
  FOR_EACH_USED_SLIDE(i) {
    if (active_slides_chk_get(i)) {
      active_slides_chk_set(i, False);
      changed = True;
    }
  }
  /* only redisplay if any of the buttons changed */
  if (changed) {
    draw_slides_buttons();
    redisplay_canvas();
  }
}

/* Return the number of currently active slides */
static int
get_num_active_slides(void)
{
  int cnt = 0, i;
  FOR_EACH_SELECTED_SLIDE(i) {
    cnt++;
  }
  return cnt;
}

/* Return the nearest slide to SLIDE that is under use by objects. */
static int
find_nearest_used_slide(int slide) {
  int i;
  /* Try larger slides first */
  FOR_EACH_USED_SLIDE(i) {
    if (i > slide) {
      return i;
    }
  }
  FOR_EACH_USED_SLIDE_REVERSE(i) {
    if (i < slide) {
      return i;
    }
  }
}


/* Update the state of slides and the screen */
void
update_slides(void)
{
  Window w = XtWindow(slides_canvas);
  int i, height;

  /* Don't update depth panel when previewing or reading in a Fig file */
  if (preview_in_progress || defer_update_layers) {
    return;
  }

  collect_all_slides_info();

  height = B_BORDER * 3 + num_of_used_slides() * TOT_B_HEIGHT;
  FirstArg(XtNheight, (Dimension)height);
  SetValues(slides_canvas);

  XClearWindow(tool_d, w);

  /* Make sure that the active slides reflect the ones that are actually used */
  int last_deleted_active_slide = NULL_SLIDE;
  FOR_EACH_SELECTED_SLIDE(i) {
    if (! is_slide_active(i)) {
      last_deleted_active_slide = i;
      active_slides_chk_set(i, False);
    }
  }

  /* If the selected slide no longer exists (off limits),
     then there will be no checked slide in the slides bar.
     Check the nearest selected slide. */
  if (get_num_active_slides() == 0) {
    Boolean must_update = False;
    int selected_slide = NULL_SLIDE;
    if (active_slides_max() == NULL_SLIDE) {
      /* Corner case: no objects left. Default to FIRST_SLIDE. */
      selected_slide = get_first_used_slide_safe();
      must_update = True;
    }
    else {
      /* Usual case: Find nearest used slide. */
      selected_slide = find_nearest_used_slide(last_deleted_active_slide);
      must_update = True;
    }
    if (must_update) {
      active_slides_chk_setonly(selected_slide, 1);
      redisplay_canvas();
    }
  }

  /* Force drawing */
  draw_slides_buttons();
}


/* static int pressed_but = NULL_SLIDE; */

/* Returns the number of the slide clicked in the side bar */
static int
calculate_pressed_slide(int y)
{
  int i, y1;

  y1 = TOT_B_HEIGHT;
  FOR_EACH_SLIDE_I(i) {
    if (y < y1) return i;
    y1 += TOT_B_HEIGHT;
  }
  return LAST_SLIDE;
}

/* Callback to refresh the slides buttons that have been exposed */
static void
slides_exposed(Widget w, XExposeEvent *event, String *params, Cardinal *nparams)
{
  draw_slides_buttons();
}

/* Toggle 1 slide */
static void
toggle_slide(Widget w, XButtonEvent *event, String *params, Cardinal *nparams)
{
  Window win = XtWindow(slides_canvas);
  Boolean obscure;

  if (min_depth < 0) return;  /* if no objects, return */

  int but_slide = calculate_pressed_slide(event->y);
  int but_idx = get_bitmap_idx(but_slide);

  if (all_slides.accum[but_idx] == 0)
    return;  /* no such button */

  /* yes, toggle visibility and redraw */
  active_slides_chk_flip(but_slide);
  draw_slides_button(win, but_slide);
  /* if user just turned on a slide, draw only it if there are no active slides on top */
  if (is_slide_active(but_slide)) {
    obscure = False;
    int i;
    FOR_EACH_SLIDE_I_UNTIL(i, but_slide) {
      if (is_slide_active(i)) {
        obscure = True;
        break;
      }
    }
    if (!obscure) {
      clearcounts();
      redisplay_compoundobject(&objects, but_slide);
    } else
      redisplay_canvas();
  } else {
    /* otherwise redraw whole canvas to get rid of that layer */
    redisplay_canvas();
  }
}


/* Main PLAY function. The direction (Norma/Reverse) is set in  DIR. */
void
play_direction(dir_t dir)
{
  int from, to;
  Boolean changed = False;

  /* If no slides are available, ERROR */
  if (all_slides.num_active == 0)
    assert(0);

  from = active_slides_chk_get_first();
  if (dir == DIR_NORMAL) {
    if ((from == NULL_SLIDE)
        || (from == active_slides_max()))
      to = FIRST_SLIDE;
    else
      to = from + 1;
  }
  else if (dir == DIR_REVERSE) {
    if ((from == NULL_SLIDE) || (from == FIRST_SLIDE))
      to = active_slides_max();
    else
      to = from - 1;
  }
  else
    assert(0 && "Bad dir");

  to = (to < FIRST_SLIDE) ? FIRST_SLIDE : to;

  active_slides_chk_setonly(to, 1);
  changed = True;

  /* only redisplay if any of the buttons changed */
  if (changed) {
    draw_slides_buttons();
    redisplay_canvas();
  }

}

/* Right shift the bitmap of SLIDES starting at POINT.
   Return False on failure. */
static Boolean
shift_slides(slides_t slides, int point, dir_t direction)
{
  int i;
  if (direction == DIR_NORMAL) {
    /* If the LAST_SLIDE is used */
    if (is_slide_set(slides, LAST_SLIDE)
        || point > LAST_SLIDE) {
      put_msg("ERROR: can't add new slide. Reached MAX_SLIDES limit.");
      beep();
      return False;
    }
    for (i = LAST_SLIDE-1; i > point; i --) {
      if (!(slide_set(slides, i+1, is_slide_set(slides, i))))
        return False;
    }
    return True;
  } else if (direction == DIR_REVERSE) {
    for (i = point; i < LAST_SLIDE; i ++)
      if (! (slide_set(slides, i, is_slide_set(slides, i+1))))
        return False;
    return True;
  }
  else
    assert(0 && "Bad direction");
  return False;
}

/* Helper for NEW_SLIDE_CB().
   We clear the marks so that insertion does not run twice for a single object. */
static void
clear_marks(void *obj, int type, int cnt, void *extra)
{
  SET_OBJ_ATTR_TO(obj, type, extra, 0);
}


/* Helper for NEW_SLIDE_CB (). 
   Inject new slide after CURRENT_SLIDE. */
static int current_slide;
/* Return status. Should be initialized to True. */
static Boolean insert_new_slide_success;
static void
insert_new_slide(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  char marked;
  /* sl=obj->slides; */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);
  SET_TO_OBJ_ATTR(marked, obj, type, extra);

  /* If already shifted, quit. */
  if (marked)
    return;
  SET_OBJ_ATTR_TO(obj, type, extra, 1);

  Boolean now_bit = active_slides(sl);
  /* Inject NOW_BIT in CURRENT_SLIDE by right shifting the rest. */
  Boolean success = shift_slides(sl, current_slide, DIR_NORMAL);
  if (! success) {
      insert_new_slide_success = False;
      return;
    }
  /* Set bit */
  slide_set(sl, current_slide + 1, now_bit);
}

/* Helper struct for swap_slide_direction() */
struct swap_slide_data {
  int slide1;
  int slide2;
};

/* Swap slides extra->slide1 with extra->slide2. */
static void
swap_slides(void *obj, int type, int cnt, void *extra)
{
  struct swap_slide_data *data = (struct swap_slide_data *)extra;
  slides_t sl;
  char marked;
  /* sl=obj->slides; */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);
  SET_TO_OBJ_ATTR(marked, obj, type, extra);

  /* If already shifted, quit. */
  if (marked)
    return;
  SET_OBJ_ATTR_TO(obj, type, extra, 1);

  /* Inject NOW_BIT in CURRENT_SLIDE by right shifting the rest. */
  slide_swap(sl, data->slide1, data->slide2);
}


struct slides_ last_kick_slides_; /* For undo */
slides_t last_kick_slides = &last_kick_slides_;
void *last_kick_obj;
int last_kick_type;

/* Return the slides of OBJ with TYPE */
static slides_t
get_object_slide(void *obj, int type)
{
  slides_t sl;
  /* sl = obj->slides */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);
  return sl;
}

/* Save state of slides */
static void set_last_slides(void *obj, int type, int slide_num)
{
    copy_slides_from_to(get_object_slide(obj, type), last_kick_slides);
    last_kick_obj = obj;
    last_kick_type = type;
}

slides_t slides_saved;          /* For undo */
char active_slides_chk_saved[MAX_SLIDES];   /* For undo */

/* Helper for undo: save single object slide */

static void
save_obj_slide(void *obj, int type, int cnt, void *extra)
{
  slides_t *data = (slides_t *) extra;
  slides_t sl;
  /* sl=obj->slides; */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);

  *data = (slides_t) realloc(*data, sizeof(**data) * (cnt+1));
  copy_slides_from_to(sl, &(*data)[cnt]);
}


/* Helper for undo: Save slides for all objects */
static void
save_all_obj_slides(slides_t *save_to)
{
  free(*save_to);
  *save_to = NULL;
  for_all_objects_do(save_obj_slide, (void *)save_to, True);
}

/* Helper for undo: Restore a single object slides */
static void
restore_obj_slide(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  /* sl=obj->slides; */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);

  copy_slides_from_to(&slides_saved[cnt], sl);
}

static void
save_active_slides_chk(char save_to[MAX_SLIDES])
{
  void *dest = memcpy(save_to, all_slides.active_slides_chk,
                      MAX_SLIDES * sizeof(save_to[0]));
  assert(dest);
}

static void
restore_active_slides_chk(char restore_from[MAX_SLIDES])
{
  void *dest = memcpy(all_slides.active_slides_chk, restore_from,
                      MAX_SLIDES * sizeof(restore_from[0]));
  assert(dest);
}

static void
copy_active_slides_chk(char from[MAX_SLIDES], char to[MAX_SLIDES])
{
  void *dest = memcpy(to, from, MAX_SLIDES * (sizeof(from[0])));
  assert(dest);
}

/* Generate New slide by appending a new slide to the current live slide. */
static void
new_slide_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  int sel_slide = num_of_active(1);
  if (! num_of_active(1)) {
    put_msg("NEW: Please select ONE slide ONLY.");
    beep();
    return;
  }
  current_slide = get_current_slide();
  if (current_slide == NULL_SLIDE) {
    put_msg("NEW: No slide selected. Please select a slide.");
    beep();
    return;
  }

  /* For undo: save slides status for all objects */
  save_all_obj_slides(&slides_saved);
  save_active_slides_chk(active_slides_chk_saved);

  insert_new_slide_success = True;
  for_all_objects_do(clear_marks, NULL, True);
  for_all_objects_do(insert_new_slide, NULL, True);
  collect_all_slides_info();
  if (insert_new_slide_success) {
    play_direction(DIR_NORMAL);
  }

  /* For undo */
  set_action(F_NEW_SLIDE);

  /* Might need to add scroll-bar at the slides bar */
  redisplay_canvas();
}


/* Callback function for moving slide up */
static void
move_slide_up_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  move_slide_up_down(w, closure, call_data, DIR_REVERSE);
}

/* Callback function for moving slide down */
static void
move_slide_down_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  move_slide_up_down(w, closure, call_data, DIR_NORMAL);
}


/* Moves current slide up/down depending on direction.
   DIR_NORMAL:  down
   DIR_REVERSE: up
*/
static void
move_slide_up_down(Widget w, XtPointer closure, XtPointer call_data,
                   dir_t direction)
{
  int sel_slide = num_of_active(1);
  if (! num_of_active(1)) {
    put_msg("NEW: Please select ONE slide ONLY.");
    beep();
    return;
  }
  current_slide = get_current_slide();
  if (current_slide == NULL_SLIDE) {
    put_msg("NEW: No slide selected. Please select a slide.");
    beep();
    return;
  }
  /* Check if we are at an acceptalbe slide, depending on direction. */
  switch(direction) {
  case DIR_NORMAL:              /* Down */
    if (current_slide >= get_last_used_slide()) {
      put_msg("Already at last slide, cannot move down!");
      beep();
      return;
    }
    break;
  case DIR_REVERSE:             /* Up */
    if (current_slide <= get_first_used_slide()) {
      put_msg("Already at first slide, cannot move up!");
      beep();
      return;
    }
    break;
  default:
    assert(0 && "Bad DIRECTION");
  }

  /* Calculate which slide to move up and which to move down
     depending on direction. */
  int slide1;
  int slide2;
    switch(direction) {
  case DIR_NORMAL:              /* Down */
    slide1 = current_slide;
    slide2 = current_slide + 1;
    break;
  case DIR_REVERSE:             /* Up */
    slide1 = current_slide - 1;
    slide2 = current_slide;
    break;
  default:
    assert(0 && "Bad DIRECTION");
  }

  /* For undo: save slides status for all objects */
  save_all_obj_slides(&slides_saved);
  for_all_objects_do(clear_marks, NULL, True);
  struct swap_slide_data data;
  data.slide1 = slide1;
  data.slide2 = slide2;

  for_all_objects_do(swap_slides, (void *)&data, True);
  collect_all_slides_info();
  save_active_slides_chk(active_slides_chk_saved);
  play_direction(direction);

  /* For undo */
  set_action(F_SWAP_SLIDE);

  /* Message */
  put_msg("Succesfully moved slide");

  /* Might need to add scroll-bar at the slides bar */
  redisplay_canvas();
}



/* Helper for DEL_SLIDE_CB ().
   Return status is writtent to DEL_SLIDE_SCUCESS. */
/* Return status. Should be initialized to True. */
static Boolean del_slide_success;
static void
del_slide(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  /* sl=obj->slides; */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);

  Boolean now_bit = active_slides(sl);

  /* Inject NOW_BIT in CURRENT_SLIDE by right shifting the rest. */
  Boolean success = shift_slides(sl, current_slide, DIR_REVERSE);
  if (success) {
    slide_set(sl, LAST_SLIDE, 0);
  } else {
    del_slide_success = False;
  }
}


/* Helper for undo: Restore slides for all objects */
static void
restore_all_obj_slides(void)
{
  for_all_objects_do(restore_obj_slide, NULL, True);
}

/* Helper dummy function for count_all_objects */
static void
dummy(void *obj, int type, int cnt, void *extra)
{
  ;
}

/* Return the total number of objects */
static int
count_all_objects() {
  return for_all_objects_do(dummy, NULL, True);
}

void
copy_all_obj_slides(slides_t from, slides_t to)
{
  int obj_cnt = count_all_objects();
  for (int i = 0; i != obj_cnt; ++i) {
    copy_slides_from_to(&from[i], &to[i]);
  }
}

/* Callback for deleting current slide */
static void
del_slide_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  int sel_slide = num_of_active(1);
  if (! num_of_active(1)) {
    put_msg("DEL: Please select ONE slide ONLY.");
    return;
  }
  current_slide = get_current_slide();

  /* If this is the last slide, then leave. */
  if (is_first_used_slide(current_slide)
      && (num_of_used_slides() == 1)) {
    put_msg("DEL: Can't delete last slide!");
    return;
  }

  /* For undo: save slides status for all objects */
  save_all_obj_slides(&slides_saved);
  save_active_slides_chk(active_slides_chk_saved);

  /* Delete slide */
  del_slide_success = True;
  for_all_objects_do(del_slide, NULL, True);

  /* Play */
  if (del_slide_success
      && (! is_first_used_slide(current_slide)))
    play_direction(DIR_REVERSE);

  collect_all_slides_info();

  draw_slides_buttons();
  redisplay_canvas();

  /* For undo */
  set_action(F_DEL_SLIDE);
}

/* Forward Play callback */
static void
active_slides_chk_play_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  play_direction(DIR_NORMAL);
}

/* Reverse Play callback */
static void
active_slides_chk_rplay_cb(Widget w, XtPointer closure, XtPointer call_data)
{
  play_direction(DIR_REVERSE);
}

/* Forward Play callback (keyboard shortcut) ">" */
void
stub_slides_play(void)
{
  play_direction(DIR_NORMAL);
}

/* Reverse Play callback (keyboard shortcut) "<" */
void
stub_slides_rplay(void)
{
  play_direction(DIR_REVERSE);
}

/* Callback for Toggle button */
static void
toggle_all(Widget w, XtPointer closure, XtPointer call_data)
{
  int i;

  FOR_EACH_CHK_I(i) {
    active_slides_chk_flip(i);
  }
  draw_slides_buttons();
  redisplay_canvas();
}

/* Fix the currently played slide. 
   If we break a compound it might be the case that
   nothing is displayed. */
void
fix_played_slide(void)
{
  int si;
  redisplay_canvas();
}

/* ********** */
/* KICK tool */
/* ********** */

/* Helper function for setting:
   OBJ->slide[HELPER_CURRENT_SLIDE] = HELPER_VALUE
   This is a safe function that will not make objects disappear,
   nor will it exceed the slides limit.
*/
static int helper_current_slide;
static int helper_value;
static Boolean set_object_slide_status;
static void
set_object_slide_safe(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  /* sl = obj->slides */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);

  /* this object should be active (otherwise how did we select it?) */
  /* Well, it could be part of a compound, which no longer exists. */
  /* assert (active_object_slides (obj, type)); */

  if (helper_value == False) { /* Deactivate slide */
    set_object_slide_status
      = slide_set(sl, helper_current_slide, False);
  } else { /* Activate slide */
    set_object_slide_status
      = slide_set(sl, helper_current_slide, True);
  }
}

/* Set OBJ->slides->bitmap[SLIDE] = VALUE
   If RECURSIVE, then set it recursively to its sub-objects. */
static void
object_set_slide(void *obj, int type, int slide, Boolean value, Boolean recursive)
{
  helper_current_slide = slide;
  helper_value = value;
  for_all_objects_in_object_do(obj, type, set_object_slide_safe, NULL, True);
}

/* Callback */
static void
init_kick_to_prev_slide(F_line *p, int type, int x, int y, int px, int py)
{
  int current_slide = get_single_current_slide();
  if (current_slide == NULL_SLIDE) {
    return;
  }
  /* For Undo: Remember last slides for this object */
  set_last_slides((void *) p, type, current_slide - 1);
  set_action(F_KICK_SLIDES);

  object_set_slide((void *) p, type, current_slide - 1, True, True);
  if (set_object_slide_status) {
    put_msg("KICK: object copied to prev slide.");
  }
  update_slides();
}

/* Callback */
static void
init_kick_to_next_slide(F_line *p, int type, int x, int y, int px, int py)
{
  slides_t sl;
  int current_slide = get_single_current_slide();
  if (current_slide == NULL_SLIDE) {
    return;
  }

  /* For undo */
  set_last_slides((void *) p, type, current_slide + 1);
  set_action(F_KICK_SLIDES);

  object_set_slide((void *) p, type, current_slide + 1, True, True);

  if (set_object_slide_status) {
    put_msg("KICK: object copied to next slide.");
  }
  update_slides();
}

/* Callback */
static void
init_kick_current_slide(F_line *p, int type, int x, int y, int px, int py)
{
  slides_t sl;
  int current_slide = get_single_current_slide();
  if (current_slide == NULL_SLIDE) {
    return;
  }

  /* For undo */
  set_last_slides((void *) p, type, current_slide);
  set_action(F_KICK_SLIDES);

  object_set_slide((void *) p, type, current_slide, False, True);

  update_slides();
  redisplay_canvas();
}


void
kick_slides_selected(void)
{
  set_mousefun("copy to prev", "delete current", "copy to next", LOC_OBJ, "", LOC_OBJ);
  canvas_kbd_proc = null_proc;
  canvas_locmove_proc = null_proc;
  canvas_ref_proc = null_proc;

  init_searchproc_left(init_kick_to_prev_slide);
  init_searchproc_middle(init_kick_current_slide);
  init_searchproc_right(init_kick_to_next_slide);

  canvas_leftbut_proc = object_search_left;
  canvas_middlebut_proc = object_search_middle;
  canvas_rightbut_proc = object_search_right;

  set_cursor(pick9_cursor);
  /* set_cursor(buster_cursor); */
  /*    force_nopositioning(); */
  reset_action_on();
}


/* ***** */
/* Misc. */
/* ***** */

/* Return the slides of a newly created object. */
slides_t
get_new_object_slides(void)
{
  slides_t new_slide = NULL;
  new_slide = get_new_slides();

  /* If this is the first slide, then enable slide 0 */
  if (all_slides.max == NULL_SLIDE) {
    slide_set(new_slide, FIRST_SLIDE, True);
    return new_slide;
  }

  /* copy the current state (checked slides) into the new slide */
  int i;
  FOR_EACH_SELECTED_SLIDE(i) {
    slide_set(new_slide, i, True);
  }
  return new_slide;
}

/* If NULL, allocate and initialize with the FIRST_SLIDE */
slides_t
get_slides_parsed_safe(slides_t slides_parsed)
{
  if (! slides_parsed) {
    slides_t new_slides = get_new_slides();
    slide_set(new_slides, FIRST_SLIDE, True);
    return new_slides;
  } else {
    return slides_parsed;
  }
}


/* UNDO */

void
undo_kick_slides(void)
{
    slides_t curr_obj_slides = get_object_slide(last_kick_obj, last_kick_type);

    struct slides_ curr_slides_saved_;
    slides_t curr_slides_saved = &curr_slides_saved_;

    if (slides_differ(curr_obj_slides, last_kick_slides)) {
      /* Save current slides state for undoing this undo */
      copy_slides_from_to(curr_obj_slides, curr_slides_saved);

      /* Undo current slides using last_kick_slides */
      copy_slides_from_to(last_kick_slides, curr_obj_slides);
      update_slides();
      redisplay_canvas();

      /* Update last_kick_slides for undoing this undo */
      copy_slides_from_to(curr_slides_saved, last_kick_slides);
    }
}


void
undo_del_slide(void)
{
    /* For undoing undo */
    slides_t tmp_all_slides_saved = NULL;
    save_all_obj_slides(&tmp_all_slides_saved);

    /* Undo */
    restore_all_obj_slides();
    update_slides();
    redisplay_canvas();

    /* For undoing the undo */
    last_action = F_NEW_SLIDE;
    copy_all_obj_slides(tmp_all_slides_saved, slides_saved);


    /* Undo the active slides */
    char tmp_active_slides_chk[MAX_SLIDES];
    /* For undoing the undo */
    save_active_slides_chk(tmp_active_slides_chk);
    /* Undo */
    restore_active_slides_chk(active_slides_chk_saved);
    copy_active_slides_chk(active_slides_chk_saved, tmp_active_slides_chk);
    draw_slides_buttons();
    redisplay_canvas();
}

void
undo_new_slide(void)
{
    /* For undoing undo */
    slides_t tmp_all_slides_saved = NULL;
    save_all_obj_slides(&tmp_all_slides_saved);

    /* Undo */
    restore_all_obj_slides();
    update_slides();
    redisplay_canvas();

    last_action = F_DEL_SLIDE;
    copy_all_obj_slides(tmp_all_slides_saved, slides_saved);

    /* Undo the active slides */
    char tmp_active_slides_chk[MAX_SLIDES];
    /* For undoing the undo */
    save_active_slides_chk(tmp_active_slides_chk);
    /* Undo */
    restore_active_slides_chk(active_slides_chk_saved);
    copy_active_slides_chk(active_slides_chk_saved, tmp_active_slides_chk);
    draw_slides_buttons();
    redisplay_canvas();
}

/* Undo "Move Up" and "Move Down" slides movement */
void
undo_swap_slide(void)
{
    /* For undoing undo */
    slides_t tmp_all_slides_saved = NULL;
    save_all_obj_slides(&tmp_all_slides_saved);

    /* Undo */
    restore_all_obj_slides();
    update_slides();
    redisplay_canvas();

    last_action = F_SWAP_SLIDE;
    copy_all_obj_slides(tmp_all_slides_saved, slides_saved);


    /* Undo the active slides */
    char tmp_active_slides_chk[MAX_SLIDES];
    /* For undoing the undo */
    save_active_slides_chk(tmp_active_slides_chk);
    /* Undo */
    restore_active_slides_chk(active_slides_chk_saved);
    copy_active_slides_chk(active_slides_chk_saved, tmp_active_slides_chk);
    draw_slides_buttons();
    redisplay_canvas();
}



/* ******************** */
/* Debug/Dump functions */
/* ******************** */
static void
dump_bitmap(char *buf, char *bitmap)
{
  int i;
  char tmp[10];
  sprintf(buf, "Bitmap: ");
  if (! bitmap) {
    strcat(buf, "NULL");
    return;
  } else {
    FOR_EACH_SLIDE_I(i) {
      snprintf(tmp, 10, "%d,", bitmap[get_bitmap_idx(i)]);
      strcat(buf, tmp);
    }
  }
}

void
debug_bitmap(char *bitmap)
{
  char *buf = (char *) malloc(MAX_SLIDES_STR * sizeof(buf[0]));
  buf[0] = '\0';
  dump_bitmap(buf, bitmap);
  fprintf(stderr, "Bitmap: %s\n", buf);
  free(buf);
}

void
debug_slides(slides_t slides)
{
  debug_bitmap(slides->bitmap);
  if (slides) {
    fprintf(stderr, "cnt: %d\n", slides->cnt);
    fprintf(stderr, "fail: %d\n", slides->fail);
  } else {
    fprintf(stderr, "cnt: -\n");
    fprintf(stderr, "fail: -\n");
  }
}

/* Helper function for DEBUG_ALL_OBJECTS() */
void
debug_slides_of_object(void *obj, int type, int cnt, void *extra)
{
  slides_t sl;
  /* sl=obj->slides */
  SET_TO_OBJ_ATTR(sl, obj, type, slides);

  fprintf(stderr, "%d. Type: %s.\n", cnt, get_type_name(type));
  debug_slides(sl);
  fprintf(stderr, "\n");
}

/* Print all objects to stderr */
void
debug_all_objects(void)
{
  for_all_objects_do(debug_slides_of_object, NULL, True);
}



/* These control write_objects() in f_save.c */
/* This is set if the "SAVE SLIDES" button has been pressed. */
Boolean sv_slides;
/* This is the slide currently being saved */
int current_sv_slide;
/* Emit all slides strings for all objects being emitted  */
Boolean emit_all_slides;

/* Helper for writing to file: w_file.c */
void
save_slide_files(void)
{
  collect_all_slides_info();
  /* Save EMIT_ALL_SLIDES */
  Boolean sv_emit_all_slides = emit_all_slides;
  emit_all_slides = False;

  int slide;
  FOR_EACH_USED_SLIDE(slide) {
    char * slide_fname;
    current_sv_slide = slide;
    /* Generate the new file name */
    slide_fname = gen_slide_fname(slide, LANG_FIG);
    write_file(slide_fname, True);
  }
  current_sv_slide = NULL_SLIDE;

  /* Restore EMIT_ALL_SLIDES */
  emit_all_slides = sv_emit_all_slides;
}



/* Global variables */
slides_t cur_slides; /* w_indpanel.c */
char *export_slide_file_not_found; /* u_print.c */
#endif  /* SLIDES_SUPPORT */
