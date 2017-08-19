# ===========================================================================
#    tl_check_header_path.m4
# ===========================================================================
#
# SYNOPSIS
#
#   TL_CHECK_HEADER_PATH(HEADER-FILE, SEARCH-PATHS)
#
# DESCRIPTION
#
#   Check whether the system header file HEADER-FILE is compilable,
#   searching first within the default search path and the paths given
#   in PATH_CPPFLAGS appended, by calling AC_CHECK_HEADER. If found, set
#   tl_cv_header_path_HEADER to ac_cv_header_HEADER. Otherwise, search
#   in the PATHS given in the blank-separated list of SEARCH-PATHS.
#   Append `-IPATH' to the shell variable PATH_CPPFLAGS for the first
#   PATH in which the header file was found. Set
#   tl_cv_header_path_HEADER to `yes' if the HEADER-FILE was
#   found, otherwise to `no'.
#
#   TODO: Does not append `-IPATH' to PATH_CPPFLAGS, if the results
#   are already available, i.e., they are cached. Therefore, running
#   TL_CHECK_HEADER_PATH twice does not result in
#   PATH_CPPFLAGS="$PATH_CPPFLAGS -IPATH -IPATH".
#   This is different from the standard behaviour of autoconf macros.
#
#   Example:
#
#   To find the png.h header file on Mac Darwin, where it might be
#   installed via Macports or Fink to unusual locations,
#
#   TL_CHECK_HEADER_PATH([png.h], [/opt/local/include /sw/include])
#
#   # In the configure.ac file, you would write
#   AC_SUBST([PATH_CPPFLAGS])
#   # To have the standard autoconf macros, trivially AC_CHECK_HEADER
#   # but, more importantly, AC_PATH_X, also profit from the extended
#   # search path
#   my_save_CPPFLAGS=$CPPFLAGS
#   CPPFLAGS="CPPFLAGS $PATH_CPPFLAGS"
#   AC_PATH_X
#   CPPFLAGS=$my_save_CPPFLAGS
#   # and in the Makefile.am,
#   AM_CPPFLAGS = $(PATH_CPPFLAGS) ...
#
# LICENSE
#
#   Copyright (c) 2008 Duncan Simpson <dps@simpson.demon.co.uk>
#   Copyright (c) 2016 Thomas Loimer <thomas.loimer@tuwien.ac.at>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
#
#   Modified from the AX_EXT_CHECK_HEADER macro by Duncan Simpson, see
#   http://www.gnu.org/software/autoconf-archive/ax_ext_check_header.html

AC_DEFUN([TL_CHECK_HEADER_PATH],
[tl_header_path_save_cppflags=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $PATH_CPPFLAGS"
AS_VAR_PUSHDEF([TL_header_path], [tl_cv_header_path_$1])dnl
AC_CHECK_HEADER([$1],
    [AS_VAR_COPY([TL_header_path], [AS_TR_SH([ac_cv_header_$1])])],
    [AC_CACHE_CHECK([for $1 in $2],
	[TL_header_path],
	[AS_VAR_SET([TL_header_path], [no])
	 AC_LANG_CONFTEST([AC_LANG_SOURCE([@%:@include <$1>])])
	 AS_FOR([TL_dir], [tl_dir], $2, dnl do not quote $2!
	    [AS_IF([test -d TL_dir],
		[CPPFLAGS="$tl_header_path_save_cppflags $PATH_CPPFLAGS -I[]TL_dir"
		 AC_COMPILE_IFELSE([],
		    [AS_IF([test -n "$PATH_CPPFLAGS"],
			[PATH_CPPFLAGS="$PATH_CPPFLAGS -I[]TL_dir"],
			[PATH_CPPFLAGS="-I[]TL_dir"])
		     AS_VAR_SET([TL_header_path], [yes])
		     break])])])
	 rm conftest.$ac_ext])])dnl
AS_VAR_POPDEF([TL_header_path])dnl
CPPFLAGS=$tl_header_path_save_cppflags
])
