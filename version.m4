#
# This file is part of FIG: Facility for Interactive Generation of figures
#
# version.m4: Version information
# This file is included by configure.ac.
# Author: Thomas Loimer <thomas.loimer@tuwien.ac.at>, 2017

dnl The version information is kept separately from configure.ac.
dnl Thus, configure.ac can remain unchanged between different versions.
dnl The values in this file are set by update_version_m4 if
dnl ./configure is called with --enable_versioning.

m4_define([XFIG_VERSION], [3.2.6a])

dnl AC_INIT does not have access to shell variables.
dnl Therefore, define RELEASEDATE as a macro.
m4_define([RELEASEDATE], [Jan 2017])
