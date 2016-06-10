
dnl ###########################################################################
dnl # Configure paths for libpurple
dnl # Gary Kramlich 2005
dnl #
dnl # Based off of glib-2.0.m4 by Owen Taylor
dnl ###########################################################################

dnl ###########################################################################
dnl # AM_PATH_PURPLE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl #
dnl # Test for purple and define PURPLE_CFLAGS, PURPLE_LIBS, PURPLE_DATADIR, and
dnl # PURPLE_LIBDIR
dnl ###########################################################################
AC_DEFUN([AM_PATH_PURPLE_CUSTOM],
[dnl

	no_purple=""

	min_version=ifelse([$1], ,2.0.0,$1)
	found_version=""

	AC_MSG_CHECKING(for purple - version >= $min_version)

	if test x"$no_purple" = x"" ; then
	   	if test x"$prefix" = x"NONE" ; then
			PURPLE_LIBDIR="$libdir"
			PURPLE_DATADIR="$datadir"
	   	else
		   	PURPLE_DATADIR=`$PKG_CONFIG --variable=datadir purple`
			PURPLE_LIBDIR=`$PKG_CONFIG --variable=libdir purple`
		fi

		PURPLE_CFLAGS=`$PKG_CONFIG --cflags purple`
		PURPLE_LIBS=`$PKG_CONFIG --libs purple`

		purple_version=`$PKG_CONFIG --modversion purple`
		purple_major_version=`echo $purple_version | cut -d. -f 1`
		purple_minor_version=`echo $purple_version | cut -d. -f 2`

		dnl # stash the micro version in a temp variable.  Then stash
		dnl # the numeric for it in purple_micro_version and anything
		dnl # else in purple_extra_version.
		purple_micro_version_temp=`echo $purple_version | cut -d. -f 3`
		purple_micro_version=`echo $purple_micro_version_temp | sed 's/[[^0-9]]//g'`
		purple_extra_version=`echo $purple_micro_version_temp | sed 's/[[0-9]]//g'`

		dnl # get the major, minor, and macro that the user gave us
		min_major_version=`echo $min_version | cut -d. -f 1`
		min_minor_version=`echo $min_version | cut -d. -f 2`
		min_micro_version=`echo $min_version | cut -d. -f 3`

		dnl # check the users version against the version from pkg-config
		if test $purple_major_version -eq $min_major_version -a \
			$purple_minor_version -ge $min_minor_version -a \
			$purple_micro_version -ge $min_micro_version
		then
			:
		else
			no_purple="yes"
			found_version="$purple_major_version.$purple_minor_version.$purple_micro_version$purple_extra_version"
		fi

		dnl # Do we want a compile test here?
	fi

	if test x"$no_purple" = x"" ; then
		AC_MSG_RESULT(yes (version $purple_major_version.$purple_minor_version.$purple_micro_version$purple_extra_version))
		ifelse([$2], , :, [$2])
	else
		AC_MSG_RESULT(no)
		if test x"$PKG_CONFIG" = x"no" ; then
			echo "*** A new enough version of pkg-config was not found."
			echo "*** See http://www.freedesktop.org/software/pkgconfig/"
		fi

		if test x"found_version" != x"" ; then
			echo "*** A new enough version of purple was not found."
			echo "*** You have version $found_version"
			echo "*** See http://pidgin.im/"
		fi

		PURPLE_CFLAGS=""
		PURPLE_LIBS=""
		PURPLE_DATADIR=""
		PURPLE_LIBDIR=""

		ifelse([$3], , :, [$3])
	fi

	AC_SUBST(PURPLE_CFLAGS)
	AC_SUBST(PURPLE_LIBS)
	AC_SUBST(PURPLE_DATADIR)
	AC_SUBST(PURPLE_LIBDIR)
])



dnl ###########################################################################
dnl see purple.m4
dnl ###########################################################################
AC_DEFUN([AM_PATH_PIDGIN_CUSTOM],
[dnl

	no_pidgin=""

	min_version=ifelse([$1], ,2.0.0,$1)
	found_version=""

	AC_MSG_CHECKING(for pidgin - version >= $min_version)

	if test x"$no_pidgin" = x"" ; then
	   	if test x"$prefix" = x"NONE" ; then
			PIDGIN_LIBDIR="$libdir"
			PIDGIN_DATADIR="$datadir"
	   	else
			PIDGIN_DATADIR=`$PKG_CONFIG --variable=datadir pidgin`
			PIDGIN_LIBDIR=`$PKG_CONFIG --variable=libdir pidgin`
		fi
		PIDGIN_REAL_LIBDIR=`$PKG_CONFIG --variable=libdir pidgin`

		PIDGIN_CFLAGS=`$PKG_CONFIG --cflags pidgin`
		PIDGIN_LIBS=`$PKG_CONFIG --libs pidgin`

		pidgin_version=`$PKG_CONFIG --modversion pidgin`
		pidgin_major_version=`echo $pidgin_version | cut -d. -f 1`
		pidgin_minor_version=`echo $pidgin_version | cut -d. -f 2`

		dnl # stash the micro version in a temp variable.  Then stash
		dnl # the numeric for it in pidgin_micro_version and anything
		dnl # else in pidgin_extra_version.
		pidgin_micro_version_temp=`echo $pidgin_version | cut -d. -f 3`
		pidgin_micro_version=`echo $pidgin_micro_version_temp | sed 's/[[^0-9]]//g'`
		pidgin_extra_version=`echo $pidgin_micro_version_temp | sed 's/[[0-9]]//g'`

		dnl # get the major, minor, and macro that the user gave us
		min_major_version=`echo $min_version | cut -d. -f 1`
		min_minor_version=`echo $min_version | cut -d. -f 2`
		min_micro_version=`echo $min_version | cut -d. -f 3`

		dnl # check the users version against the version from pkg-config
		if test $pidgin_major_version -eq $min_major_version -a \
			$pidgin_minor_version -ge $min_minor_version -a \
			$pidgin_micro_version -ge $min_micro_version
		then
			:
		else
			no_pidgin="yes"
			found_version="$pidgin_major_version.$pidgin_minor_version.$pidgin_micro_version$pidgin_extra_version"
		fi

		dnl # Do we want a compile test here?
	fi

	if test x"$no_pidgin" = x"" ; then
		AC_MSG_RESULT(yes (version $pidgin_major_version.$pidgin_minor_version.$pidgin_micro_version$pidgin_extra_version))
		ifelse([$2], , :, [$2])
	else
		AC_MSG_RESULT(no)
		if test x"$PKG_CONFIG" = x"no" ; then
			echo "*** A new enough version of pkg-config was not found."
			echo "*** See http://www.freedesktop.org/software/pkgconfig/"
		fi

		if test x"found_version" != x"" ; then
			echo "*** A new enough version of pidgin was not found."
			echo "*** You have version $found_version"
			echo "*** See http://pidgin.im/"
		fi

		PIDGIN_CFLAGS=""
		PIDGIN_LIBS=""
		PIDGIN_DATADIR=""
		PIDGIN_LIBDIR=""
		PIDGIN_REALLIBDIR=""

		ifelse([$3], , :, [$3])
	fi

	AC_SUBST(PIDGIN_CFLAGS)
	AC_SUBST(PIDGIN_LIBS)
	AC_SUBST(PIDGIN_DATADIR)
	AC_SUBST(PIDGIN_LIBDIR)
	AC_SUBST(PIDGIN_REAL_LIBDIR)
])
