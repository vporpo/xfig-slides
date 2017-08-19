# ===========================================================================
#   tl_search_libs_path.m4
# ===========================================================================
#
# SYNOPSIS
#
#   TL_SEARCH_LIBS_PATH(FUNCTION, LIBRARIES, SEARCH-PATHS,
#			[OTHER-LIBRARIES], [SOURCE])
#
#   Check, whether FUNCTION exists in LIBRARIES. Call
#   AC_SEARCH_LIBS(FUNCTION, LIBRARIES). Set ac_cv_search_FUNCTION, as
#   well as tl_cv_libs_path_FUNCTION, to `-lLIB', or "none required", if
#   the library is found in the standard search path. If not found,
#   search for LIBARARIES in each path in SEARCH-PATHS in turn. Set
#   tl_cv_libs_path_FUNCTION to the path, `-LPATH, where a LIBRARY
#   providing FUNCTION was found, or to `no', if none was found. If
#   found, prepend `-lLIBRARY' to LIBS and prepend `-LPATH' to LDFLAGS
#   and PATH_LDFLAGS. Run AC_LINK_IFELSE with SOURCE, if given.
#
#   The test is based on AC_SEARCH_LIBS in autoconf version 2.69.
#
#   TODO: Omits to set $LIBS or $PATH_LDFLAGS, if results are already
#   cached. This is different from the standard behaviour of autoconf
#   macros. For instance, running AC_SEARCH_LIBS(pow, m) twice results
#   in LIBS="-lm -lm ...".
#
#   Example:
#
#   To find -lpng on Mac Darwin, if libpng was installed via Macports
#   or Fink,
#
#   TL_SEARCH_LIBS_PATH([png_read_info], [png], [/opt/local/lib /sw/lib])
#
#   In the configure.ac file, use
#   AC_SUBST([PATH_LDFLAGS])
#   and in the Makefile
#   AM_LDFLAGS = $(PATH_LDFLAGS) ...

#   Since TL_SEARCH_LIBS_PATH permanently sets LIBS and LDFLAGS, save
#   LDFLAGS for your users like
#
#   my_save_LDFLAGS=$LDFLAGS
#   TL_SEARCH_LIBS_PATH...
#   TL_SEARCH_LIBS_PATH...
#   .
#   .
#   AC_SEARCH_LIBS
#   .
#   .
#   LDFLAGS=$my_save_LDFLAGS
#
#
# LICENSE
#
#   Copyright (c) 2016 Thomas Loimer <thomas.loimer@tuwien.ac.at>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
#
#   This macro has taken ideas and was stimulated by the AX_EXT_HAVE_LIB
#   macro from Duncan Simpson, see
#   www.gnu.org/software/autoconf-archive/ax_ext_have_lib.html


#   TL_SEARCH_LIBS_PATH(FUNCTION, LIBRARIES, SEARCH-PATHS,
#			[OTHER-LIBRARIES], [SOURCE])

AC_DEFUN([TL_SEARCH_LIBS_PATH],
[tl_libs_path_save_LDFLAGS=$LDFLAGS
AC_SEARCH_LIBS([$1], [$2],
    [AS_VAR_COPY(AS_TR_SH([tl_cv_libs_path_$1]), AS_TR_SH([ac_cv_search_$1]))],
    [AS_VAR_PUSHDEF([TL_libs_path], [tl_cv_libs_path_$1])dnl
     AC_CACHE_CHECK([for $2 in $3], [TL_libs_path],
	[tl_libs_path_save_LIBS=$LIBS
	 AS_VAR_SET([TL_libs_path], [no])
	 AS_UNSET([tl_result])
	 m4_ifval([$5],
	    [AC_LANG_CONFTEST([AC_LANG_SOURCE([$5])])],
	    [AC_LANG_CONFTEST([AC_LANG_CALL([], [$1])])])
	 AS_FOR([TL_dir], [tl_dir], $3,
	    [AS_IF([test -d TL_dir],
		[LDFLAGS="-L[]TL_dir $tl_libs_path_save_LDFLAGS"
		 AS_FOR(TL_lib, tl_lib, $2,
		    [LIBS="-l[]TL_lib $4 $tl_libs_path_save_LIBS"
		     AC_LINK_IFELSE([],
			[AS_VAR_SET([TL_libs_path], ["-L[]TL_dir"])
			 AS_IF([test -n "$PATH_LDFLAGS"],
			    [PATH_LDFLAGS="-L[]TL_dir $PATH_LDFLAGS"],
			    [PATH_LDFLAGS="-L[]TL_dir"])
			 tl_result="-l[]TL_lib"
			 break])])
		AS_VAR_SET_IF([tl_result], [break])])])
	 AS_VAR_SET_IF([tl_result],
	    [LIBS="$tl_result $tl_libs_path_save_LIBS"],
	    [LIBS=$tl_libs_path_save_LIBS
	     LDFLAGS=$tl_libs_path_save_LDFLAGS])
	 rm conftest.$ac_ext])dnl
     AS_VAR_POPDEF([TL_libs_path])],
    [$4])dnl
])
