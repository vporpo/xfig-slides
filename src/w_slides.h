/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 2013-2018 by Vasileios Porpodas (Slides support)
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

#ifdef SLIDES_SUPPORT



#ifndef W_SLIDES_H
#define W_SLIDES_H

#include "fig.h"
#include "figx.h"
#include <assert.h>
#include <w_setup.h>

/* cyclic dependency F_comound, slides_t */
typedef struct slides_ *slides_t;
#include "object.h"
typedef struct f_compound F_compound;

/* Characters used to specify slides. Example: #$1,2,3,4 */
#define SLIDES_BEGIN_CHAR '$'
#define SLIDES_PREFIX "#$"

/* We limit the maximum number of slides, because the current
   implementation uses a fixed-size array of MAX_SLIDES size
   as the main data structure.
   TODO: use a dynamically allocated data structure instead. */
#define MAX_SLIDES 32
#define MAX_SLIDES_MSG 16284
#define MAX_SLIDES_STR MAX_SLIDES*5
/* The first and last slide allowed */
#define FIRST_SLIDE 1
#define LAST_SLIDE (FIRST_SLIDE + MAX_SLIDES - 1)

#define NULL_SLIDE -1
#define NULL_SLIDE_MAX INT_MAX

#define SLIDES_HT_CANVAS_PERCENT 50 /* (%) */
#define SLIDES_DIST_FROM_LAYERS 5

static int SLIDES_WD = DEF_LAYER_WD;
static int SLIDES_HT;

/* These control write_objects() in f_save.c */
/* TODO: these should become static within w_slides.c */
/* This is set if the "SAVE SLIDES" button has been pressed. */
extern Boolean sv_slides;
/* This is the slide currently being saved */
extern int current_sv_slide;
/* Emit all slides strings for all objects being emitted  */
extern Boolean emit_all_slides;

extern void do_save_slides(Widget w, XButtonEvent *ev);
extern void init_slides_side_panel(Widget parent);

struct slides_;
/* This is a bitmap of the slides. */
struct slides_
{
  /* slides bitmap */
  char bitmap[MAX_SLIDES];

  /* The number of ON slides */
  int cnt;
  /* FAIL is TRUE if there is something wrong */
  Boolean fail;
  /* True if this slide should be enabled whenever we add a new slide. */
  Boolean is_unbounded;
};
typedef struct slides_ *slides_t;

/* Deep copy of slides */
extern void copy_slides_from_to(slides_t from, slides_t to);

/* This is the main data structure for representing the slides state */
struct all_slides_
{
  /* This is where all slides bitmaps are accumulated */
  long accum[MAX_SLIDES];
  /* current max */
  int max;
  /* current min */
  int min;
  /* Number of active slides */
  int num_active;
  /* The slides that are checked on the side bar */
  char active_slides_chk[MAX_SLIDES];
  /* the first active slide */
  int active_min;
  /* the max active slide */
  int active_max;
  /* count the checked slides */
  int active_cnt;
};
extern struct all_slides_ all_slides;

extern int get_bitmap_idx(int slide_i);
extern Boolean is_slide_active(int slide_i); /* accum */
extern void slide_active_clear(int slide_i); /* accum */
extern Boolean has_next_slide(int slide_i);

/* which layer buttons are active */

#define FOR_EACH_SLIDE_I(I)                     \
  for (I = FIRST_SLIDE; I <= LAST_SLIDE; I++)

#define FOR_EACH_SLIDE_I_REV(I)                   \
  for (I = LAST_SLIDE; I >= FIRST_SLIDE; I--)

#define FOR_EACH_SLIDE_I_UNTIL(I, UNTIL)        \
  for (I = FIRST_SLIDE; I < UNTIL; I++)

#define FOR_EACH_SLIDE_IN_SLIDES(SI,SLIDES)     \
  FOR_EACH_SLIDE_I(SI)                          \
  if (SLIDES && SLIDES->bitmap[get_bitmap_idx(SI)] > 0)

#define FOR_EACH_USED_SLIDE(SI)                   \
  FOR_EACH_SLIDE_I(SI)                            \
    if (is_slide_active(SI))

#define FOR_EACH_USED_SLIDE_REVERSE(SI)         \
  for (SI = LAST_SLIDE; SI >= FIRST_SLIDE; SI--) \
    if (is_slide_active(SI))

#define FOR_EACH_CHK_I(I)                           \
  for ((I) = FIRST_SLIDE; (I) <= all_slides.active_max; (I)++)


#define FOR_EACH_SELECTED_SLIDE(SI)             \
  FOR_EACH_SLIDE_I((SI))                        \
  if (all_slides.active_slides_chk[get_bitmap_idx(SI)] > 0)

#define FOR_EACH_SELECTED_SLIDE_REV(SI)                   \
  FOR_EACH_SLIDE_I_REV((SI))                              \
  if (all_slides.active_slides_chk[get_bitmap_idx(SI)] > 0)


#define FOR_EACH_SELECTED_VISIBLE_SLIDE(SI)     \
  FOR_EACH_CHK_I((SI))                          \
  if (all_slides.active_slides_chk[get_bitmap_idx(SI)] > 0)


/* Use this to automatically set:
   DEST = OBJ->ATTR
   for OBJ TYPE.
   Types are defined in object.h (like O_ELLIPSE etc.) */
#define SET_TO_OBJ_ATTR(DEST, OBJ, TYPE, ATTR)  \
  switch (TYPE) {                               \
  case O_POLYLINE:                              \
    DEST = ((F_line *) (OBJ)) -> ATTR;          \
    break;                                      \
  case O_TXT:                                   \
    DEST = ((F_text *) (OBJ)) -> ATTR;          \
    break;                                      \
  case O_ELLIPSE:                               \
    DEST = ((F_ellipse *) (OBJ)) -> ATTR;       \
    break;                                      \
  case O_ARC:                                   \
    DEST = ((F_arc *) (OBJ)) -> ATTR;           \
    break;                                      \
  case O_SPLINE:                                \
    DEST = ((F_spline *) (OBJ)) -> ATTR;        \
    break;                                      \
  case O_COMPOUND:                              \
    DEST = ((F_compound *) (OBJ)) -> ATTR;      \
    break;                                      \
  case O_FIGURE:                                \
    DEST = ((F_compound *) (OBJ)) -> ATTR;      \
    break;                                      \
  default:                                      \
    assert (0);                                 \
  }

#define SET_OBJ_ATTR_TO(OBJ, TYPE, ATTR, SRC) \
  switch (TYPE) {                             \
    case O_POLYLINE:                          \
      ((F_line *) (OBJ))->ATTR = (SRC);       \
      break;                                  \
    case O_TXT:                               \
      ((F_text *) (OBJ))->ATTR = (SRC);       \
      break;                                  \
    case O_ELLIPSE:                           \
      ((F_ellipse *) (OBJ))->ATTR = (SRC);    \
      break;                                  \
    case O_ARC:                               \
      ((F_arc *) (OBJ))->ATTR = (SRC);        \
      break;                                  \
    case O_SPLINE:                            \
      ((F_spline *) (OBJ))->ATTR = (SRC);     \
      break;                                  \
    case O_COMPOUND:                          \
      ((F_compound *) (OBJ))->ATTR = (SRC);   \
      break;                                  \
    case O_FIGURE:                            \
      ((F_compound *) (OBJ))->ATTR = (SRC);   \
      break;                                  \
    default:                                  \
      assert (0);                             \
    }


extern slides_t copy_slides(slides_t slides);

extern char *gen_slide_fname(int slide, int lang_enum);
extern Boolean slides_include_slide(slides_t slides, int slide);
extern slides_t parse_slides_str(char *buf);
extern char * slides_to_str(slides_t slides, const char *prefix);
extern char *get_slides_line(char *buf);

extern slides_t cur_slides; /* w_indpanel.c */


#define SET_ALL_OBJ(ATTR, VAL)                          \
  {                                                     \
    F_arc *a;                                           \
    F_compound *c;                                      \
    F_ellipse *e;                                       \
    F_line *l;                                          \
    F_spline *s;                                        \
    F_text *t;                                          \
    for (a = objects.arcs; a != NULL; a = a->next)      \
      a->ATTR = VAL;                                    \
    for (c = objects.compounds; c != NULL; c = c->next) \
      c->ATTR = VAL;                                    \
    for (e = objects.ellipses; e != NULL; e = e->next)  \
      e->ATTR = VAL;                                    \
    for (l = objects.lines; l != NULL; l = l->next)     \
      l->ATTR = VAL;                                    \
    for (s = objects.splines; s != NULL; s = s->next)   \
      s->ATTR = VAL;                                    \
    for (t = objects.texts; t != NULL; t = t->next)     \
      t->ATTR = VAL;                                    \
  }                                                     \


#define DO_FOR_ALL_OBJ(ATTR, FUNC)                      \
  {                                                     \
    F_arc *a;                                           \
    F_compound *c;                                      \
    F_ellipse *e;                                       \
    F_line *l;                                          \
    F_spline *s;                                        \
    F_text *t;                                          \
    for (a = objects.arcs; a != NULL; a = a->next)      \
      FUNC(a->ATTR);                                    \
    for (c = objects.compounds; c != NULL; c = c->next) \
      FUNC(c->ATTR);                                    \
    for (e = objects.ellipses; e != NULL; e = e->next)  \
      FUNC(e->ATTR);                                    \
    for (l = objects.lines; l != NULL; l = l->next)     \
      FUNC(l->ATTR);                                    \
    for (s = objects.splines; s != NULL; s = s->next)   \
      FUNC(s->ATTR);                                    \
    for (t = objects.texts; t != NULL; t = t->next)     \
      FUNC(t->ATTR);                                    \
  }                                                     \


/* SIDE PANEL */
extern void init_slides_side_panel(Widget parent); /* called by main.c */
extern void setup_slides_side_panel(void); /* called by main.c */
extern void setup_slides_2(void);

extern void update_slides(void);

extern Boolean any_active_slides_in_compound(F_compound *cmpnd);

extern void toggle_show_slides(void);
/* used in exporting (print_to_printer()) to check if slide fig file exists */
extern char *export_slide_file_not_found;
extern slides_t get_new_object_slides(void);

extern void kick_slides_selected(void);
extern void kut_slides_selected(void);
extern void stub_slides_play(void);
extern void stub_slides_rplay(void);
extern Boolean slide_set(slides_t slides, int i, Boolean value);
extern void slide_reset(slides_t slides);
/* The last used slide in SLIDES */
extern int get_last_slide(slides_t slides);
/* The first used slide in SLIDES */
extern int get_first_slide(slides_t slides);
extern int get_beginning_of_last_range(slides_t slides);
extern int get_last_used_slide(void);
extern int get_first_used_slide(void);
extern Boolean is_slide_set(slides_t slides, int i);
extern int slides_get_cnt(slides_t slides);
extern char *selected_slides_str(void);
extern void for_all_objects_in_compound_do(F_compound *c, void (*func)(void *obj, int type, int cnt, void *extra), void *extra, Boolean recursive);
int for_all_objects_do(void (*func) (void *obj, int type, int cnt, void *extra), void *extra, Boolean recursive);
extern Boolean active_slides(slides_t slides);
extern Boolean active_object_slides(void *obj, int type);
extern Boolean slides_differ(slides_t slides1, slides_t slides2);

extern slides_t get_slides_parsed_safe(slides_t slides_parsed);
extern void fix_played_slide(void);

extern void get_cum_slides_in_compound(F_compound *c, slides_t sl_accum);
extern void collect_all_slides_info(void);
extern int num_of_used_slides(void);

extern void undo_swap_slide(void);
extern void undo_new_slide(void);
extern void undo_del_slide(void);
extern void undo_kut_slides(void);
extern void undo_kut_merge(void);
extern void undo_kick_slides(void);

extern void write_slides(FILE *fp, slides_t slides);


#endif  /* SLIDES_SUPPORT */
#endif
