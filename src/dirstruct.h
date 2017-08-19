/*
 *  dirstruct.h
 *  This file is part of FIG : Facility for Interactive Generation of figures
 *
 *  Copyright (c) 2016 by Thomas Loimer
 *
 *  This file is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DIRSTRUCT

/* See info autoconf, AC_HEADER_DIRENT: All current systems have dirent.h
 * -- but, for ancient systems. See info autoconf AC_HEADER_DIRENT */
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#define DIRSTRUCT struct dirent
#else /* HAVE_DIRENT_H */
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif
#define DIRSTRUCT struct direct
#endif /* HAVE_DIRENT_H */

#endif
