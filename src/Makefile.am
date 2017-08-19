## Run automake on this file to produce Makefile.in
# This file is part of FIG - Facility for Interactive Generation of figures
#
# Copyright (c) 2016 Thomas Loimer <thomas.loimer@tuwien.ac.at>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved. This file is offered as-is,
# without any warranty.

AM_CPPFLAGS = $(PATH_CPPFLAGS) -DXFIGLIBDIR=\"$(compkeydbdir)\" \
 -DOBJLIBDIR=\"$(pkgdatadir)/Libraries\" -DXFIGDOCDIR=\"$(docdir)\"

AM_LDFLAGS = $(PATH_LDFLAGS)
#AM_CFLAGS = -Wno-parentheses
# LIBOBJS must be placed in LDADD or xfig_LDADD - otherwise, the
# lib-dir is not distributed
# LIBOBJS may contain the replacement functions from AC_REPLACE_FUNCS.
xfig_LDADD = $(LIBOBJS) $(X_LIBS)
# xmkmf compiliert mit, abgesehen von obigem: -DNEWARROWPROTO

bin_PROGRAMS = xfig

xfig_SOURCES = d_arcbox.c d_arcbox.h d_arc.c d_arc.h d_box.c d_box.h \
	d_ellipse.c d_ellipse.h d_line.c d_line.h d_picobj.c d_picobj.h \
	d_regpoly.c d_regpoly.h d_spline.c d_spline.h d_subspline.c \
	d_subspline.h d_text.c d_text.h dirstruct.h e_addpt.c e_addpt.h \
	e_align.c e_align.h e_arrow.c e_arrow.h e_break.c e_break.h e_chop.c \
	e_chop.h e_compound.c e_compound.h e_convert.c e_convert.h e_copy.c \
	e_copy.h e_delete.c e_delete.h e_deletept.c e_deletept.h e_edit.c \
	e_edit.h e_flip.c e_flip.h e_glue.c e_glue.h e_joinsplit.c \
	e_joinsplit.h e_measure.c e_measure.h e_move.c e_move.h e_movept.c \
	e_movept.h e_placelib.c e_placelib.h e_rotate.c e_rotate.h e_scale.c \
	e_scale.h e_tangent.c e_tangent.h e_update.c e_update.h fig.h figx.h \
	f_load.c f_load.h f_neuclrtab.c f_neuclrtab.h f_picobj.c f_picobj.h \
	f_read.c f_readeps.c f_readeps.h f_readgif.c f_readgif.h f_read.h \
	f_readold.c f_readold.h f_readpcx.c f_readpcx.h f_readpng.c \
	f_readpng.h f_readppm.c f_readppm.h f_readtif.c f_readtif.h \
	f_readxbm.c f_readxbm.h f_readxpm.h f_save.c f_save.h f_util.c \
	f_util.h f_wrpcx.c f_wrpng.c f_wrpng.h main.c main.h mode.c mode.h \
	object.c object.h paintop.h pcx.h resources.c resources.h \
	u_bound.c u_bound.h u_create.c u_create.h u_drag.c u_drag.h u_draw.c \
	u_draw.h u_elastic.c u_elastic.h u_error.c u_error.h u_fonts.c \
	u_fonts.h u_free.c u_free.h u_geom.c u_geom.h u_list.c u_list.h \
	u_markers.c u_markers.h u_pan.c u_pan.h u_print.c u_print.h \
	u_quartic.c u_quartic.h u_redraw.c u_redraw.h u_scale.c u_scale.h \
	u_search.c u_search.h u_smartsearch.c u_smartsearch.h u_translate.c \
	u_translate.h u_undo.c u_undo.h w_browse.c w_browse.h w_canvas.c \
	w_canvas.h w_capture.c w_capture.h w_cmdpanel.c w_cmdpanel.h w_color.c \
	w_color.h w_cursor.c w_cursor.h w_digitize.c w_digitize.h w_dir.c \
	w_dir.h w_drawprim.c w_drawprim.h w_export.c w_export.h w_file.c \
	w_file.h w_fontbits.c w_fontbits.h w_fontpanel.c w_fontpanel.h \
	w_grid.c w_grid.h w_help.c w_help.h w_icons.c w_icons.h w_indpanel.c \
	w_indpanel.h w_intersect.c w_intersect.h w_keyboard.c w_keyboard.h \
	w_layers.c w_layers.h w_library.c w_library.h w_listwidget.c \
	w_listwidget.h w_listwidgetP.h w_modepanel.c w_modepanel.h \
	w_mousefun.c w_mousefun.h w_msgpanel.c w_msgpanel.h w_print.c \
	w_print.h w_rottext.c w_rottext.h w_rulers.c w_rulers.h w_setup.c \
	w_setup.h w_snap.c w_snap.h w_srchrepl.c w_srchrepl.h w_style.c \
	w_style.h w_util.c w_util.h w_zoom.c w_zoom.h Xfuncs.h Xosdefs.h \
	splash.xbm version.xbm

if USE_XPM
xfig_SOURCES += f_readxpm.c f_readxpm.h
endif

if USE_SPLASH
xfig_SOURCES += splash.xpm
endif

if USE_JPEG
xfig_SOURCES += f_readjpg.c f_readjpg.h
endif

if I18N
xfig_SOURCES += w_i18n.c w_i18n.h
endif

if !XAW3D1_5E
xfig_SOURCES += w_menuentry.c w_menuentry.h w_menuentryP.h \
	SmeBSB.h SmeCascade.c SmeCascade.h SmeCascadeP.h \
	SimpleMenu.c SmeBSB.c SmeBSBP.h
endif

EXTRA_DIST = u_draw_spline.c

# These files depend on PACKAGE_VERSION, which is
# defined in config.h ( = $(CONFIG_HEADER))
f_save.$(OBJEXT) main.$(OBJEXT) u_error.$(OBJEXT): $(CONFIG_HEADER)

if USE_SPLASH
w_icons.$(OBJEXT): version.xbm splash.xbm splash.xpm
else
w_icons.$(OBJEXT): version.xbm splash.xbm
endif

MAINTAINERCLEANFILES = Makefile.in

version.xbm: version.fig.in $(top_srcdir)/version.m4
	fig2dev -L ppm version.fig | ppmtopgm | pgmtopbm | pnmcrop \
	| pbmtoxbm | sed '1,3 s/noname_/letters_/; 3 s/char/unsigned &/' > $@

# $(srcdir) is most probably not necessary
splash.xpm: $(top_srcdir)/splash-image/spl_bckgnd.fig
	fig2dev -L xpm $(top_srcdir)/splash-image/spl_bckgnd.fig \
	| sed '1d; 2 {s/static //; s/noname/spl_bckgnd_xpm/}' > $@

splash.xbm: $(top_srcdir)/splash-image/spl_bckgnd.fig
	fig2dev -L xbm -S 4 $(top_srcdir)/splash-image/spl_bckgnd.fig \
	| sed '1,3 s/noname/spl_bckgnd/; 3 s/char/unsigned &/' > $@
