/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2007 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
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

#ifndef FIG_H
#define FIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ANCIENT /**************************************/

/* For the X stuff, include only Xlib.h and Intrinsic.h here -
   use figx.h for widget stuff */

#if defined(ultrix) || defined(__bsdi__) || defined(Mips) || defined(apollo) || defined(__hpux)
#if defined(__hpux)
#define _HPUX_SOURCE /* for typedef caddr_t :-(( */
#endif
#include <sys/types.h>	/* for stat structure */
#endif
#include <sys/stat.h>

#if defined(__convex__) && defined(__STDC__)
#define S_IFDIR _S_IFDIR
#define S_IWRITE _S_IWRITE
#endif

#ifndef SYSV
#ifndef SVR4
#include <fcntl.h>
#endif
#endif

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#ifndef HAVE_STRERROR
extern char	*strerror();
#  if !defined(__bsdi__) && !defined(__NetBSD__) && !defined(__GNU_LIBRARY__)
      extern int	errno;
      extern int	sys_nerr;
#     if ( !(defined(BSD) && (BSD - 0 >= 199306)) && !defined(__NetBSD__) && \
	   !defined(__GNU_LIBRARY__) && !defined(__FreeBSD__) && !defined(__GLIBC__))
	 extern char    *sys_errlist[];
#     endif
#  endif
#endif /* HAVE_STRERROR */

#include <math.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#include <X11/Xos.h>

/* for those who have an older (R4) Xos.h, we need to include unistd.h here */

/*
 * Get open(2) constants
 */
#ifdef X_NOT_POSIX
#include <fcntl.h>
#ifdef USL
#include <unistd.h>
#endif /* USL */
#ifdef CRAY
#include <unistd.h>
#endif /* CRAY */
#ifdef MOTOROLA
#include <unistd.h>
#endif /* MOTOROLA */
#ifdef SYSV386
#include <unistd.h>
#endif /* SYSV386 */
#include <sys/file.h>
#else /* X_NOT_POSIX */
#if !defined(_POSIX_SOURCE) && defined(macII)
#define _POSIX_SOURCE
#include <fcntl.h>
#undef _POSIX_SOURCE
#else
#include <fcntl.h>
#endif
#include <unistd.h>
#endif /* X_NOT_POSIX else */

#if defined(SYSV) && defined(SYSV386)
#if defined(__STDC__)
#ifdef ISC
extern double atof(char const *);
#endif  /* ISC */
#ifdef SCO
#ifdef SCO324
#include <stdlib.h>   /* for atof() and getenv(), maybe required for all SCO's ? */
#else /* NOT SCO 3.2r4 */
extern double atof(const char *);
#endif /* SCO 3.2r4 */
#else  /* NOT SCO */
extern double atof();
#endif /* SCO */
#else  /* NOT __STDC__ */
extern double atof();
#endif /* __STDC__ */
#else  /* NOT defined(SYSV) && defined(SYSV386) */
#ifdef X_NOT_STDC_ENV
#if defined(ultrix) || defined(sun) && !defined(sparc) || defined(titan) || \
	(defined(ibm032) && !defined(_AIX))
extern double atof();
extern char *getenv();
#endif /* (sun) !(sparc) (titan) */
#else  /* NOT X_NOT_STDC_ENV */
#include <stdlib.h>	/* for atof() and getenv() */
#endif /* X_NOT_STDC_ENV */
#endif /* defined(SYSV) && defined(SYSV386) */

#if defined(SYSV) || defined(SVR4)
extern	void		srand48();
extern	long		lrand48();
extern	double		drand48();
#define	srandom(seed)	srand48((seed))
#define	random()	lrand48()
#define	frandom()	drand48()

#elif defined(BSD)  /* not SYSV/SVR4, check for BSD */
#define	srandom(seed)	srand48((long)(seed))
#define	random()	lrand48()
#define	frandom()	drand48()

#elif (defined(linux) && !defined(glibc) && !defined(__GLIBC__))
extern	long		random();
extern	void		srandom(unsigned int);

#elif !defined(__osf__) && !defined(__CYGWIN__) && !defined(linux) && !defined(__FreeBSD__) && !defined(__GLIBC__)
extern	void		srandom(int);

#endif

#ifndef frandom
#define	frandom()	(random()*(1./2147483648.))
#endif

#else /********************* ANCIENT *********************/

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#include <fcntl.h>
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#include <errno.h>
#include <sys/types.h>	/* for stat structure */
#include <sys/stat.h>

#if defined HAVE_DECL_S_IFDIR && !HAVE_DECL_S_IFDIR
#define S_IFDIR _S_IFDIR
#endif
#if defined HAVE_DECL_S_IWRITE && !HAVE_DECL_S_IWRITE
#define S_IWRITE _S_IWRITE
#endif

#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> /* info autoconf: On Darwin, stdio.h is a prerequisite. */
#include <pwd.h>
#include <signal.h>
#include <math.h>

#define	frandom()	drand48()

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#endif /********************* ANCIENT *********************/

#if XtSpecificationRelease > 4
#include <X11/Xfuncs.h>
#else
#include "Xosdefs.h"
#include "Xfuncs.h"
#endif /* XtSpecificationRelease > 4 */

#ifdef I18N
#include <locale.h>
#endif

/* use my own PI because GNUC has a long double and others have something else */
#undef M_PI
#undef M_PI_2
#undef M_2PI
#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923
#define M_2PI	6.28318530717958647692

#define		min2(a, b)	(((a) < (b)) ? (a) : (b))
#define		max2(a, b)	(((a) > (b)) ? (a) : (b))
#define		min3(a,b,c)	((((a<b)?a:b)<c)?((a<b)?a:b):c)
#define		max3(a,b,c)	((((a>b)?a:b)>c)?((a>b)?a:b):c)
#define		round(a)	(int)(((a)<0.0)?(a)-.5:(a)+.5)
#define		signof(a)	(((a) < 0) ? -1 : 1)

#endif /* FIG_H */
