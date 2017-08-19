#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# This file is part of FIG - Facility for Interactive Generation of figures
#
# Copyright (c) 2016 Thomas Loimer <thomas.loimer@tuwien.ac.at>
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved. This file is offered as-is,
# without any warranty.

dnl The minimum autoconf version to process this file. In version 2.60,
dnl the directory variables changed, see info autoconf, section 4.8.3.
AC_PREREQ([2.60])

dnl Define VERSION in version.m4
m4_include([version.m4])

# Initialize
AC_INIT([xfig], [XFIG_VERSION], [thomas.loimer@tuwien.ac.at],
	[], [https://sourceforge.net/projects/mcj/])
dnl AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS],
dnl	[TARNAME], [URL])
dnl TARNAME defaults to FULL-PACKAGE-NAME, converted to lower case
dnl and all characters other than alphanumerics and _ changed to -.
dnl defines PACKAGE_NAME, PACKAGE_TARNAME, PACKAGE_VERSION, PACKAGE_STRING,
dnl PACKAGE_BUGREPORT and PACKAGE_URL

# Define variables that are written into config.h.
dnl Need to double-quote (shell-)vars identical to m4-variables.
AC_DEFINE([PROTOCOL_VERSION], ["3.2"],
	[Define the fig file format. Identical to major.minor \
		substring of xfig version.])
AC_SUBST([RELEASE_DATE], ["RELEASEDATE"])dnl
dnl For the tests. (How to quote output-variable from interpretation
dnl as m4-variable.
dnl AC_SUBST([FIG@&t@_VERSION], ["FIG_VERSION"])dnl
dnl AC_SUBST([PATCH@&t@LEVEL], ["PATCHLEVEL"])dnl

AC_CONFIG_MACRO_DIR([m4])

# Add unusual search paths here.
# Macports by default installs under /opt/local, fink under /sw.
m4_define([LIBPATHS], [/opt/local/lib /sw/lib])
m4_define([INCLUDEPATHS], [/opt/local/include /sw/include])
m4_define([X_LIBPATHS], [LIBPATHS /opt/X11/lib ])
m4_define([X_INC_PATHS], [INCLUDEPATHS /opt/X11/include])
dnl To search in additional paths, use
dnl TL_SEARCH_LIBS_PATH instead of AC_SEARCH_LIBS, and
dnl TL_CHECK_HEADER_PATH instead of AC_CHECK_HEADER.
dnl The TL_-macros set PATH_CPPFLAGS, PATH_LDFLAGS and LIBS to
dnl report their findings.
AC_SUBST(PATH_CPPFLAGS)
AC_SUBST(PATH_LDFLAGS)

# Set automake options.
dnl The options are written to AUTOMAKE_OPTIONS. A minimum version
dnl of automake, e.g., 1.6.1, can also be put into AM_INIT_AUTOMAKE.
# AM_INIT_AUTOMAKE([-Wall foreign dist-xz no-dist-gzip subdir-objects])
AM_INIT_AUTOMAKE([-Wall foreign dist-xz no-dist-gzip subdir-objects])
dnl  -Wall.....enable all (automake) warnings
dnl  -Werror...and report them as error
dnl  foreign...do no require the files ChangeLog COPYING AUTHORS NEWS
dnl		README INSTALL
dnl  subdir-objects...place objects into the dir where the corresponding
dnl		source file resides, not into the build tree;
dnl		necessary to use LIBOBJS from /lib in /src

# Check whether ./configure resides in the correct directory.
AC_CONFIG_SRCDIR([src/main.c])

# Write the variables determined by ./configure into config.h.
# Therefore, make sure to #include "config.h" in each source file.
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
# Use ISO Standard C, currently C99.
AC_PROG_CC_STDC
AS_IF([test "x$ac_cv_prog_cc_stdc" = xno],dnl
      [AC_C_CONST
       AC_C_INLINE])
# Sets MKDIR_P - which is, however, set anyhow
AC_PROG_MKDIR_P
AC_PROG_LN_S

dnl  To the comfort of the maintainer
dnl AC_PROG_SED
dnl AC_CHECK_PROG([GIT], [git], [git], [no])
dnl AM_CONDITIONAL([HAS_GIT_CONTROL],
dnl    [test `git -C $srcdir describe 2>/dev/null`])
dnl then, bracket the version.m4 target in ./Makefile.am with
dnl if HAS_GIT_CONTROL ... endif

# Search for pkg-config and set PKG_CONFIG to the path found, else empty.
# PKG_PROG_PKG_CONFIG
# On debian, pkg-config does not find xaw3d. On archlinux, pkg-config
# --libs xaw3d yields -lXaw3d -lXmu -lXt -lX11 -- but -lXmu is not
# strictly necessary. Therefore, do not use pkg-config, we know better.

dnl Not useful for packagers.
dnl AC_CHECK_PROG(XDG_ICON, [xdg-icon-resource], [xdg-icon-resource])
dnl AC_CHECK_PROG(XDG_MENU, [xdg-desktop-menu], [xdg-desktop-menu])

# Checks for libraries.
xfig_save_LDFLAGS="$LDFLAGS"
TL_SEARCH_LIBS_PATH([pow], [m], [LIBPATHS])dnl
dnl AC_SEARCH_LIBS([deflate], [z])  # libz is not needed.
TL_SEARCH_LIBS_PATH([png_read_info], [png], [LIBPATHS])
AS_IF([test "x$tl_cv_libs_path_pow" = xno \
	|| test "x$tl_cv_libs_path_png_read_info" = xno],
    [AC_MSG_ERROR([Necessary libraries not found.])])
# Now, LIBS may contain a -lLIB, which is not found in the standard
# search path. But LIBS is added to any compilation attempt further
# below. Therefore, temporarily add PATH_LDFLAGS to LDFLAGS, even
# when only testing header files. It might be wise, to collect all
# TL_SEARCH_LIBS_PATH together. No need to save PATH_CPPFLAGS.
# LDFLAGS="$PATH_LDFLAGS $LDFLAGS"
# CPPFLAGS="$CPPFLAGS $PATH_CPPFLAGS"

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS_ONCE([features.h sys/time.h])

# Provide for unusual include paths, as above for libraries.
TL_CHECK_HEADER_PATH([png.h], [INCLUDEPATHS])dnl

# Get X header and library location.
# Simply add libraries to LIBS, x_includes to PATH_CPPFLAGS
# and x_libraries to PATH_LDFLAGS.
# The search for png.h might have added a path to CPPFLAGS. Provide
# that to AC_PATH_X.
xfig_save_CPPFLAGS="$CPPFLAGS"
# do not add spaces, as with the simple CPPFLAGS="$CPPFLAGS $PATH_CPPFLAGS"
AS_IF([test -n "$PATH_CPPFLAGS"], [AS_IF([test -n "$CPPFLAGS"],
	[CPPFLAGS="$CPPFLAGS $PATH_CPPFLAGS"], [CPPFLAGS="$PATH_CPPFLAGS"])])
AC_PATH_X
AS_IF([test "x$no_x" != xyes],
    [AS_IF([test -n "$x_includes"],
	[PATH_CPPFLAGS="$PATH_CPPFLAGS -I$x_includes"])
     AS_IF([test -n "$x_libraries"],
	[PATH_LDFLAGS="-L$x_libraries $PATH_LDFLAGS"])])
# (1) Add "-lXt -lX11" to LIBS.
# The check below did not work on Darwin (quote in AC_PATH_X: Do not try a
# hair to link a X-program), therefore, use hard-coded X_LIBS
# Further below, though, linking with -lXpm or -lXaw works.
X_LIBS="-lXt -lX11"
AC_SUBST([X_LIBS])
# (2) Do not give up, check in unusual places.
#TL_SEARCH_LIBS_PATH([XBell], [X11], [X_LIBPATHS])
#TL_SEARCH_LIBS_PATH([XtName], [Xt], [X_LIBPATHS])
#AS_IF([test -n "$PATH_LDFLAGS"], [AS_IF([test -n "$LDFLAGS"],
#	[LDFLAGS="$PATH_LDFLAGS $LDFLAGS"], [LDFLAGS="$PATH_LDFLAGS"])])
TL_CHECK_HEADER_PATH([X11/X.h], [X_INC_PATHS])
CPPFLAGS="$xfig_save_CPPFLAGS"

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_DECLS([S_IFDIR, S_IWRITE],[],[],[[#include <sys/stat.h>]])dnl
AC_CHECK_DECL([REG_NOERROR],[],
	[AC_DEFINE([REG_NOERROR], 0,dnl
		[Define to 0 if not provided by regex.h.])],dnl
	[[#include <regex.h>]])dnl

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
dnl AC_FUNC_STRTOD
# The setlocale seems to be broken, grep HAVE_SETLOCALE, setlocale
AC_CHECK_FUNCS_ONCE([getcwd setlocale strerror])
AC_REPLACE_FUNCS([isascii strstr strchr strrchr strcasecmp strncasecmp \
	strdup strndup])

AC_CONFIG_LIBOBJ_DIR([lib])

# Query user options
AC_ARG_ENABLE(versioning, [AS_HELP_STRING([--enable-versioning],
	[enable changing the version number, only useful for \
	 hacking (default: disable)])],
    [],[enableval=no])dnl
AM_CONDITIONAL([ENABLE_VERSIONING], [test "x$enableval" = xyes])dnl

AC_ARG_ENABLE(arrows4to14, [AS_HELP_STRING([--disable-arrows4to14],
	[disable arrow types 4 to 14 (default: enable)])],
    [],[enableval=yes])dnl
AS_IF([test "x$enableval" = xyes],
    [AC_DEFINE([ARROWS4TO14], 1,
	 [Define to use arrow types 4 to 14 (22 arrows).])])dnl

AC_ARG_ENABLE(i18n, [AS_HELP_STRING([--disable-i18n],
	[disable internationalization features (default: enable)])],
    [],[enableval=yes])dnl
AS_IF([test "x$enableval" = xyes],
    [AC_DEFINE([I18N], 1,
	[Define to enable internationalization features.])dnl
     AC_DEFINE([I18N_USE_PREEDIT], 1,
	[Define to use preediting with i18n.])])
AM_CONDITIONAL([I18N], [test "x$enableval" = xyes])dnl

AC_ARG_ENABLE(jpeg, [AS_HELP_STRING([--disable-jpeg],
	[disable support to import jpeg files (default: enable)])],
    [],[enableval=yes])
AS_IF([test "x$enableval" = xyes],
    [TL_SEARCH_LIBS_PATH([jpeg_read_header], [jpeg], [LIBPATHS])
     AS_IF([test "x$tl_cv_libs_path_jpeg_read_header" != xno],
	[AC_DEFINE([USE_JPEG], 1,
	    [Define to enable support for importing jpeg files.])])])
AM_CONDITIONAL([USE_JPEG], [test "x$enableval" = xyes && \
	test "x$tl_cv_libs_path_jpeg_read_header" != xno])dnl

AC_ARG_WITH(gs, [AS_HELP_STRING([--without-gs],
	[do not use ghostscript to render embedded eps or pdf images)])],
    [],[withval=yes])dnl
AS_IF([test "x$withval" = xyes],
    [AC_DEFINE([GSBIT], 1,
	[Define to use ghostscript for rendering of embedded \
		eps or pdf-images.])])dnl

dnl With # as comment, configure fails on the lines below!
dnl Currently only w_rottext.c includes CACHE_[XIMAGES|BITMAPS], logic there
dnl sets CACHE_BITMAPS if CACHE_XIMAGES is not set.
AC_ARG_ENABLE(ximages-cache, [AS_HELP_STRING([--enable-ximages-cache],
	[enable caching of x-images, instead of bitmaps, \
		in the X-server (default: disable, cache bitmaps)])],
    [],[enableval=no])dnl
AS_IF([test "x$enableval" != xno],
    [AC_DEFINE([CACHE_XIMAGES], 1,
	[Define to cache X-images, undefine to cache \
		bitmaps in the X-server.])])dnl

AC_ARG_ENABLE(cache-size, [AS_HELP_STRING([--enable-cache-size=<kB>],
	[set the maximum size of bitmaps (kB) \
		cached in the X-server (default: 20000)])],
    [], [enableval=20000])dnl
AS_IF([test "$enableval" -gt 0], [], [enableval=20000])dnl
AC_DEFINE_UNQUOTED([CACHE_SIZE_LIMIT], [$enableval],
    [Define to the maximum size of bitmaps (kB) cached in the X-server.])dnl

AC_ARG_ENABLE(xpm, [AS_HELP_STRING([--disable-xpm],
	[disable support for xpm color-bitmaps, internally \
		 and for import/export (default: enable)])],
    [], [enableval=yes])dnl
AS_IF([test "x$enableval" = xyes],
    [TL_SEARCH_LIBS_PATH([XpmFree], [Xpm], [X_LIBPATHS])
     AS_IF([test "x$tl_cv_libs_path_XpmFree" = xno],
	[enableval=no],
	[AC_DEFINE([USE_XPM], 1, [Define to use xpm color-bitmaps and enable \
		    import/export to xpm files.])])])
AM_CONDITIONAL([USE_XPM], [test "x$enableval" = xyes])dnl

AC_ARG_ENABLE(splash, [AS_HELP_STRING([--enable-splash],
	[enable splash screen (default: disable)])],
    [], [enableval=no])dnl
AS_IF([test "x$enableval" = xyes],
    [AC_DEFINE([USE_SPLASH], 1, [Define to show a splash screen.])])
AM_CONDITIONAL([USE_SPLASH], [test "x$enableval" = xyes])dnl

AC_ARG_WITH(xaw3d1_5e, [AS_HELP_STRING([--without-xaw3d1_5e],
	[do not use version Xaw3d >= 1.5e (default: use)])],
    [],[with_xaw3d1_5e=yes])dnl

AC_ARG_WITH(xaw3d, [AS_HELP_STRING([--without-xaw3d],
	[do not use X Athena Widget toolkit with 3D-effects (default: use)])],
    [],[withval=yes])dnl

dnl AS_IF([test -n "$PKG_CONFIG" && `$PKG_CONFIG --exists xaw3d`],dnl
dnl    [AS_IF([test "x$withval" = xyes],dnl
dnl	[PKG_CHECK_MODULES(XAW, xaw3d)],
dnl	[PKG_CHECK_MODULES(XAW, xaw7, [],
dnl	    [AC_MSG_WARN("$XAW_PKG_ERRORS")
dnl	     PKG_CHECK_MODULES(XAW, xaw6)])dnl

# Re-use PATH_CPPFLAGS to really set config-CPPFLAGS.
AS_IF([test "x$withval" = xyes],
    [AS_IF([test -z "$PATH_CPPFLAGS"],
	[PATH_CPPFLAGS="-DXAW_INTERNATIONALIZATION"],
	[PATH_CPPFLAGS="$PATH_CPPFLAGS -DXAW_INTERNATIONALIZATION"])
     AC_DEFINE([XAW3D], 1,
		[Define to use X Athena Widgets with 3D-effects.])
     AS_IF([test "x$with_xaw3d1_5e" = xyes],
	[X_LIBS="-lXaw3d $X_LIBS"
	 AC_DEFINE([XAW3D1_5E], 1,
		[Define to use version Xaw3d >= 1.5e.])],
	[X_LIBS="-lXaw3d -lXmu $X_LIBS"])],
    [X_LIBS="-lXmu $X_LIBS"
     TL_SEARCH_LIBS_PATH([XawInitializeWidgetSet], [Xaw8 Xaw7 Xaw6],
	[X_LIBPATHS], [$X_LIBS])])
AM_CONDITIONAL([XAW3D1_5E],
    [test "x$withval" = xyes && test "x$with_xaw3d1_5e" = xyes])dnl

AC_ARG_ENABLE(tablet, [AS_HELP_STRING([--enable-tablet],
	[enable support for tablet device (default: disable)])],
    [], [enableval=no])dnl
AS_IF([test "x$enableval" = xyes],
    [TL_SEARCH_LIBS_PATH([XFreeDeviceList], [Xi], [X_LIBPATHS])
     AS_IF([test "x$tl_cv_libs_path_XFreeDeviceList" != xno],
	[AC_DEFINE([USE_TAB], 1,
	    [Define for using an input tablet.])])])

AC_ARG_ENABLE(comp-led, [AS_HELP_STRING([--enable-comp-led=#],
    [turn on the compose indicator LED when entering \
		multi-key characters (default: 3)])],
    [],[enableval=3])dnl
AS_IF([test "$enableval" -ge 1 && test "$enableval" -le 4],
    [AC_DEFINE_UNQUOTED([COMP_LED], [$enableval],dnl
	[Define and set to a number (1-4) to turn on the compose indicator LED \
	 when entering multi-key characters.])])dnl

printcapfile="/etc/printcap"
AC_ARG_WITH(printcap, [AS_HELP_STRING([--with-printcap=<pathname>],
	[specify path to printcap file (default: /etc/printcap)])],
    [AS_IF([test "x$withval" = xno || test "x$withval" = xyes],
	[AC_MSG_NOTICE([using $printcapfile as printcap file])],
	[printcapfile=$withval])])
AC_DEFINE_UNQUOTED([PRINTCAP], ["$printcapfile"],
	[Define to the path of the printcap-file.])

dnl Nice idea, to set appdefaultdir using pkg-config.
dnl Alas, this results in a hard-coded path, which breaks the
dnl make distckeck target, because make distcheck _requires_
dnl user-writeable locations. Here is the attempt:
dnl    [PKG_CHECK_MODULES(APPDEFAULTDIR, xt)
dnl	AS_IF([test -n "$PKG_CONFIG"],dnl
dnl	[appdefaultdir="$($PKG_CONFIG --variable=appdefaultdir xt)"],dnl

default_appdefaultdir=
AC_ARG_WITH(appdefaultdir, [AS_HELP_STRING([--with-appdefaultdir=<pathname>],
	[specifiy directory for X resources file (default: \
		${datarootdir}/X11/app-defaults)])],
    [],[withval='${datarootdir}/X11/app-defaults'])dnl
AC_SUBST(appdefaultdir, ["$withval"])
# In addition, expand now, to substitute in xfig.man.
# First expand datarootdir, secondly prefix.
# Below, all variables for xfig.man are expanded together.
AS_IF([test "x$prefix" = xNONE],
    [prefix="$ac_default_prefix";
     APPDEFAULTDIR=`eval echo "$withval"`;
     APPDEFAULTDIR=`eval echo "$APPDEFAULTDIR"`; prefix=NONE],
    [APPDEFAULTDIR=`eval echo "$withval"`;
     APPDEFAULTDIR=`eval echo "$APPDEFAULTDIR"`;])dnl
AC_SUBST([APPDEFAULTDIR])dnl

AC_ARG_WITH(icondir, [AS_HELP_STRING([--with-icondir=<pathname>],
	[specify icon directory, to put xfig.png \
		(default: $datarootdir/pixmaps)])],
    [], [withval='${datarootdir}/pixmaps'])dnl
AC_SUBST(icondir, ["$withval"])dnl

AC_ARG_WITH(applicationsdir, AS_HELP_STRING([--with-applicationsdir=<pathname>],
	[specify directory for xfig.desktop file \
		(default: $datarootdir/applications)]),
    [], [withval='${datarootdir}/applications'])dnl
AC_SUBST(applicationsdir, ["$withval"])dnl

AC_ARG_WITH(compkeydb, [AS_HELP_STRING([--without-compkeydb],
	[do not use Compose Key database file \
		shipped with xfig (default: use)])],
    [], [withval=yes])dnl
AS_IF([test "x$withval" != xyes],dnl
    [AC_DEFINE([NO_COMPKEYDB], 1,dnl
	[Define to not use the Compose Key Database file shipped with xfig.])])

AC_ARG_WITH(compkeydbdir, [AS_HELP_STRING([--with-compkeydbdir=<pathname>],
	[specify directory for the compose key database file, CompKeyDB \
		(default: $datadir/xfig)])],
    [], [withval='${datadir}/xfig'])dnl
AC_SUBST(compkeydbdir, ["$withval"])dnl

LDFLAGS="$xfig_save_LDFLAGS"

# Set the variables for substitution in xfig.man
# This builds on the side-effect of the AC_SUBST above, to write, e.g.,
# compkeydbdir="$withval" into the ./configure script.
AS_IF([test "x$prefix" = xNONE],
	[prefix="$ac_default_prefix"
	 ICONDIR=`eval echo "$icondir"`
	 ICONDIR=`eval echo "$ICONDIR"`
	 APPLICATIONSDIR=`eval echo "$applicationsdir"`
	 APPLICATIONSDIR=`eval echo "$APPLICATIONSDIR"`
	 COMPKEYDBDIR=`eval echo "$compkeydbdir"`
	 COMPKEYDBDIR=`eval echo "$COMPKEYDBDIR"`
	 COMPKEYDBDIR=`eval echo "$COMPKEYDBDIR"`
	 HTMLDIR=`eval echo "$htmldir"`	# $docdir
	 HTMLDIR=`eval echo "$HTMLDIR"`	# $datarootdir/doc/$PACKAGE_TARNAME
	 HTMLDIR=`eval echo "$HTMLDIR"`	# $prefix/....
	 HTMLDIR=`eval echo "$HTMLDIR"`
	 DOCDIR=`eval echo "$docdir"`
	 DOCDIR=`eval echo "$DOCDIR"`
	 DOCDIR=`eval echo "$DOCDIR"`
	 # a kludge, set PKGDATADIR to what automake most probably would set it
	 PKGDATADIR=`eval echo ${datarootdir}/xfig`
	 PKGDATADIR=`eval echo $PKGDATADIR`
	 prefix=NONE],
	[ICONDIR=`eval echo "$icondir"`
	 ICONDIR=`eval echo "$ICONDIR"`
	 APPLICATIONSDIR=`eval echo "$applicationsdir"`
	 APPLICATIONSDIR=`eval echo "$APPLICATIONSDIR"`
	 COMPKEYDBDIR=`eval echo "$compkeydbdir"`
	 COMPKEYDBDIR=`eval echo "$COMPKEYDBDIR"`
	 COMPKEYDBDIR=`eval echo "$COMPKEYDBDIR"`
	 HTMLDIR=`eval echo "$htmldir"`	# $docdir
	 HTMLDIR=`eval echo "$HTMLDIR"`	# $datarootdir/doc/$PACKAGE_TARNAME
	 HTMLDIR=`eval echo "$HTMLDIR"`	# $prefix/....
	 HTMLDIR=`eval echo "$HTMLDIR"`
	 DOCDIR=`eval echo "$docdir"`
	 DOCDIR=`eval echo "$DOCDIR"`
	 DOCDIR=`eval echo "$DOCDIR"`
	 PKGDATADIR=`eval echo ${datarootdir}/xfig`
	 PKGDATADIR=`eval echo $PKGDATADIR`])dnl
AC_SUBST([ICONDIR])dnl
AC_SUBST([APPLICATIONSDIR])dnl
AC_SUBST([COMPKEYDBDIR])dnl
AC_SUBST([HTMLDIR])dnl
AC_SUBST([DOCDIR])dnl
AC_SUBST([PKGDATADIR])dnl

AC_ARG_WITH(xfig-libraries, AS_HELP_STRING([--without-xfig-libraries],
	[do not install xfig-Libraries (default: install)]),
    [], [withval=yes])dnl
AM_CONDITIONAL([XFIG_LIBRARIES],
    [test "x$withval" = xyes && test -d "$srcdir/Libraries"])dnl

# Autotest infrastructure.
AC_CONFIG_TESTDIR([tests])dnl
AM_MISSING_PROG([AUTOM4TE], [autom4te])dnl

dnl If doc/xfig_man.html.in is not present, simply touch it, e.g.
dnl touch -r doc/FORMAT1.3 doc/xfig_man.html.in

AC_CONFIG_FILES([Makefile
		app-defaults/Fig
		src/Makefile
		src/version.fig
		doc/Makefile
		doc/xfig.man
		doc/xfig_man.html
		tests/Makefile
		tests/atlocal])dnl

AC_OUTPUT

AC_MSG_NOTICE([
  Compile, \$CC=$CC \$PATH_CPPFLAGS=${PATH_CPPFLAGS:-<empty>} \\
	\$CPPFLAGS=${CPPFLAGS:-<empty>} \$CFLAGS=${CFLAGS:-<empty>}
  Link, $CC \$CFLAGS=${CFLAGS:-<empty>} \$PATH_LDFLAGS=${PATH_LDFLAGS:-<empty>} \\
	\$LDFLAGS=${LDFLAGS:-<empty>} <object> \\
	\$X_LIBS=${X_LIBS:-<empty>} \$LIBS=${LIBS:-<empty>}

Installation locations:
  $APPDEFAULTDIR/ Fig	(--with-appdefaultdir)
  $APPLICATIONSDIR/ xfig.desktop	(--with-applicationsdir)
  $ICONDIR/ xfig.png	(--with-icondir)
  $PKGDATADIR/ Libraries (--datarootdir)
  $HTMLDIR/ html	(--htmldir)
  $DOCDIR/ doc-files	(--docdir)])
