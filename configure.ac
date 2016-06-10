#
# Pidgin Send Screenshot plugin configure.ac
#
# Process this file with autoconf to produce a configure script.
#
# Some parts are stolen  rom the guification plugin, 
# which itself comes from pidgin and other plugins sources...
#  
#

AC_INIT([pidgin-sendscreenshot],[m4_esyscmd(build-aux/version-gen)],[contact@raoulito.info])

AC_PREREQ([2.67])

AC_CANONICAL_TARGET
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AC_CONFIG_MACRO_DIR([m4])

AC_PATH_PROG(sedpath, sed)
AC_DISABLE_STATIC

dnl This disables check of libtool for these compilers
CXX=no
F77=no
GCJ=no

dnl # Make sure we have pkg-config
PKG_PROG_PKG_CONFIG()
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

#
# Setup libtool
#
LT_INIT
LIBTOOL="$LIBTOOL --silent"

#
# Good o' gettext
#
IT_PROG_INTLTOOL
GETTEXT_PACKAGE=screenshot
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Define the gettext package to be used.])
ALL_LINGUAS="de es fr nl pl ru zh_CN"
AM_GLIB_GNU_GETTEXT

#
# Look for the C compiler
#
AC_PROG_CC_STDC
if test "x$GCC" = "xyes"; then
	CFLAGS="-Wall -Wextra -Wshadow -ggdb -O1"
fi
AC_SUBST(CFLAGS)

#
# check for gtk libs > 2.4
#
# --enable-upload : check for (glib > 2.14 and libcurl)
# --disable-upload : check for glib > 2.8
#

AC_ARG_ENABLE(upload, 
             [AS_HELP_STRING([--disable-upload],[compile without capability to upload a screenshot to a server.])],
	     enable_upload="$enableval", enable_upload="yes")

AM_CONDITIONAL(ENABLE_UPLOAD, test "x$enable_upload" = "xyes")

PKG_CHECK_MODULES(Gtk, [gtk+-2.0 >= 2.4.0])
PKG_CHECK_MODULES(Glib, [glib-2.0 >= 2.8.0])

if test "x$enable_upload" = "xyes" ; then 
   # check for libcurl first
   foundlibcurl="no"
   foundpcre="not found"
   LIBCURL_CHECK_CONFIG([yes], 
	   	        [7.19.1], # make sure CURLOPT_ERRORBUFFER is always filled in on errors 
 		        [foundlibcurl="yes"
			 PKG_CHECK_MODULES(GRegex, 
                                           [glib-2.0 >= 2.14.0], # glib with PCRE support
                                            foundpcre="found `$PKG_CONFIG --modversion glib-2.0`")]
                        ,foundpcre="not checked")  

   if test "x$foundlibcurl" = "xno" ; then

      AC_MSG_ERROR([
To compile this plugin *with* upload support, you must have :
 - libcurl dev headers >= 7.19.1 ($foundlibcurl) and glib>=2.14.0 ($foundpcre).

If you don't need this feature, please specify --disable-upload when running configure.])
   else
       AC_DEFINE(ENABLE_UPLOAD, 1,
	         [Compile with capability to upload a screenshot to an image hosting provider.])	
   
   fi
fi

AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)


#
# Check for libpurple and pidgin
#
AM_PATH_PURPLE_CUSTOM([2.4.0], , 
	AC_MSG_ERROR([
	Please upgrade to a newer version.])
)

AM_PATH_PIDGIN_CUSTOM([2.4.0], , 
	AC_MSG_ERROR([
	Please upgrade to a newer version.])
)

#
# Finish up
#
AC_CONFIG_FILES([Makefile
		 po/Makefile.in
		 src/Makefile
		 config/Makefile])
AC_OUTPUT

echo Configuration complete !
echo

echo Build with uploading capability............. : $enable_upload

echo
eval eval echo Send Screenshot v.$VERSION will be installed in $PIDGIN_LIBDIR/pidgin.
if test "x$PIDGIN_LIBDIR" != "x$PIDGIN_REAL_LIBDIR" ; then
	echo ***WARNING: Pidgin library directory isn\'t the same : $PIDGIN_REAL_LIBDIR/pidgin !
	echo ***Pidgin may not be able to detect the plugin.
	echo ***Please specify the --prefix option to configure.
fi
cat <<EOF

Type make to compile the plugin.
Questions ? <contact@raoulito.info>

EOF
