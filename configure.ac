dnl -*- automake -*-
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)

dnl Update version here and below at LIB_CURRENT, ..., if needed.
AC_INIT(bes, 3.20.4, support@opendap.org)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])

PACKAGE_MAJOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\1@'`
PACKAGE_MINOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\2@'`
PACKAGE_SUBMINOR_VERSION=`echo $PACKAGE_VERSION | sed 's@^\([[0-9]]\)*\.\([[0-9]]*\)\.\([[0-9]]*\)$@\3@'`
AC_MSG_NOTICE(Package Major Version:     $PACKAGE_MAJOR_VERSION)
AC_MSG_NOTICE(Package Minor Version:     $PACKAGE_MINOR_VERSION)
AC_MSG_NOTICE(Package SubMinor Version:  $PACKAGE_SUBMINOR_VERSION)
AC_SUBST(PACKAGE_MAJOR_VERSION)
AC_SUBST(PACKAGE_MINOR_VERSION)
AC_SUBST(PACKAGE_SUBMINOR_VERSION)

dnl Add valgrind options
DODS_GCOV_VALGRIND

AC_CONFIG_AUX_DIR([conf])
AC_CONFIG_MACRO_DIR([conf])

AM_INIT_AUTOMAKE([1.10 tar-pax])
LT_INIT

dnl flags for the compilers and linkers - set these before locating the
dnl actual tools since some of the AC_PROG macros set these 'flag variables'
dnl to default values otherwise.
AC_CANONICAL_HOST
AC_SUBST(host)

dnl This is the default. If the caller uses --enable-developer, that option
dnl will override these values
cxx_debug_flags="-g -O2"

dnl I moved this up to here from the tail end of the file
dnl because it needs to run before the AC_PROG_CXX macro
dnl which will also set CXXFLAGS
dnl jhrg 3/4/15
AC_ARG_ENABLE([developer], [AS_HELP_STRING([--enable-developer], [Developer mode; won't override CXXFLAGS on the command line])],
  [AM_CONDITIONAL([BES_DEVELOPER], [true])
   cxx_debug_flags="-g3 -O0  -Wall -W -Wcast-align"],   
  [AM_CONDITIONAL([BES_DEVELOPER], [false])
   AC_DEFINE([NDEBUG], [1], [Define this to suppress assert() statements.])])

AC_ARG_ENABLE([asan], [AS_HELP_STRING([--enable-asan], [Use the address sanitizer; won't override CXXFLAGS on the command line])], 
    [AM_CONDITIONAL([USE_ASAN], [true])
     cxx_debug_flags="$cxx_debug_flags -fsanitize=address -fno-omit-frame-pointer -fno-common"], 
    [AM_CONDITIONAL([USE_ASAN], [false])])

AC_ARG_WITH([cmr], [AS_HELP_STRING([--with-cmr], [Build and include the CMR Module (built by default in developer mode)])], 
    [AM_CONDITIONAL([WITH_CMR], [true])],
    [AM_CONDITIONAL([WITH_CMR], [AM_COND_IF([BES_DEVELOPER], [true], [flase])])])

AS_IF([test "$CC" = "gcc"], [AM_CONDITIONAL([COMPILER_IS_GCC],[true])],
    [AM_CONDITIONAL([COMPILER_IS_GCC],[false])])

dnl Only set CXXFLAGS if the caller didn't supply a value
dnl AS_IF([test -z "${CXXFLAGS+set}"], [CXXFLAGS="$cxx_debug_flags"])
AS_IF([test -z "${CXXFLAGS+set}"], [CXXFLAGS=])

dnl : ${CXXFLAGS=""}

dnl library visioning: Update these when the interface changes.
dnl
dnl How to set these SO variables:
dnl No interfaces changed, only implementations (good): ==> Increment REVISION.
dnl Interfaces added, none removed (good): ==> Increment CURRENT,
dnl increment AGE, set REVISION to 0.
dnl Interfaces removed or changed (BAD, breaks upward compatibility):
dnl ==> Increment CURRENT, set AGE and REVISION to 0.

LIB_DIS_CURRENT=18
LIB_DIS_AGE=3
LIB_DIS_REVISION=1
AC_SUBST(LIB_DIS_CURRENT)
AC_SUBST(LIB_DIS_AGE)
AC_SUBST(LIB_DIS_REVISION)
LIBDISPATCH_VERSION="$LIB_DIS_CURRENT:$LIB_DIS_REVISION:$LIB_DIS_AGE"
AC_SUBST(LIBDISPATCH_VERSION)

LIB_PPT_CURRENT=5
LIB_PPT_AGE=1
LIB_PPT_REVISION=0
AC_SUBST(LIB_PPT_CURRENT)
AC_SUBST(LIB_PPT_AGE)
AC_SUBST(LIB_PPT_REVISION)
LIBPPT_VERSION="$LIB_PPT_CURRENT:$LIB_PPT_REVISION:$LIB_PPT_AGE"
AC_SUBST(LIBPPT_VERSION)

LIB_XML_CMD_CURRENT=5
LIB_XML_CMD_AGE=4
LIB_XML_CMD_REVISION=0
AC_SUBST(LIB_XML_CMD_CURRENT)
AC_SUBST(LIB_XML_CMD_AGE)
AC_SUBST(LIB_XML_CMD_REVISION)
LIBXMLCOMMAND_VERSION="$LIB_XML_CMD_CURRENT:$LIB_XML_CMD_REVISION:$LIB_XML_CMD_AGE"
AC_SUBST(LIBXMLCOMMAND_VERSION)

AC_CHECK_LIB([pthread], [pthread_kill],
    [PTHREAD_LIBS="-lpthread"],
    [AC_MSG_ERROR([I could not find pthreads])])
    
AC_SUBST([PTHREAD_LIBS])

dnl This seems like it doesn't belong here but in the module, instead.
dnl TODO. jhrg 11/28/17
AC_DEFINE([DAPREADER_PACKAGE], ["dapreader_module"], [dapreader_module])
AC_DEFINE([DAPREADER_VERSION], ["0.0.1"], [0.0.1])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_INSTALL
dnl AC_PROG_LEX
AM_PROG_LEX

dnl NB: CentOS 6 does not support C++-11 but does support some features:
dnl https://gcc.gnu.org/gcc-4.4/cxx0x_status.html.
dnl For now, I'm counting 0x as supporting C++-11 in our code.

CXX11_FLAG=""

CXX_FLAGS_CHECK([--std=c++11], [CXX11_FLAG=--std=c++11], [])

AS_IF([test -z "$CXX11_FLAG"],
      [CXX_FLAGS_CHECK([--std=c++0x], [CXX11_FLAG=--std=c++0x], [])])

AS_IF([test -z "$CXX11_FLAG"],
      [AC_MSG_ERROR([Not using C++-11 (or C++0x)])],
      [AC_MSG_NOTICE([Using $CXX11_FLAG for C++-11 support])
       cxx_debug_flags="$cxx_debug_flags --pedantic $CXX11_FLAG"])

AC_SUBST(CXX11_FLAG)

AS_IF([test -n "$cxx_debug_flags"], [CXXFLAGS="$cxx_debug_flags"])

dnl We really need bison and not yacc. If you use AC_PROG_YACC, the resulting 
dnl Makefile will call bison -y which does not know how to make the parsers 
dnl we require. jhrg 6/15/05
AC_CHECK_PROG(YACC,[bison],[bison])

dnl Use libtool instead?
dnl AC_PROG_RANLIB
AM_PROG_LIBTOOL

AC_SEARCH_LIBS([uuid_generate], [uuid],
    [],
    [AC_MSG_ERROR([I could not find a library with uuid_generate])])

AC_CHECK_PROG(VALGRIND, valgrind, [valgrind --logfile=memcheck])

dnl Perl is used by www-interface during the build
dnl TODO: Really? jhrg 11/28/17
AC_CHECK_PROG([PERL], [perl], [`which perl`])

dnl We need the *-config scripts to build, so the AC_CHECK_LIB 
dnl macro is not needed.
AC_PATH_PROG([CURL], [curl], [Where is curl?], [/bin:/usr/bin:/usr/local/bin])

dnl The DMR++ handler uses, conditionally, the Multi API. These tests determine
dnl if that API is present (iff the multi.h header is present). jhrg 8/27/18
AC_CHECK_HEADERS([curl/curl.h])
AC_CHECK_HEADERS([curl/multi.h])

AC_SEARCH_LIBS([curl_multi_wait], [curl],
    [AC_DEFINE([HAVE_CURL_MULTI_API],[1],[Does libcurl have the multi API])], [], [])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS_ONCE(fcntl.h float.h malloc.h stddef.h stdlib.h limits.h unistd.h pthread.h bzlib.h string.h strings.h)

dnl AC_CHECK_HEADERS_ONCE([uuid/uuid.h uuid.h])
dnl Do this because we have had a number of problems with the UUID header/library
AC_CHECK_HEADERS([uuid/uuid.h],[found_uuid_uuid_h=true],[found_uuid_uuid_h=false])
AC_CHECK_HEADERS([uuid.h],[found_uuid_h=true],[found_uuid_h=false])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_STDBOOL
AC_CHECK_TYPES([ptrdiff_t])
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_CHECK_FUNCS(strdup strftime strtol strcasecmp strcspn strerror strncasecmp)
AC_CHECK_FUNCS(strpbrk strchr strrchr strspn strtoul)
AC_CHECK_FUNCS(timegm mktime atexit floor isascii memmove memset pow sqrt)

# Make sure we have the cctype library
AC_SEARCH_LIBS([isdigit], [cctype])

AC_SYS_LARGEFILE
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STRTOD
AC_FUNC_VPRINTF

BES_OLD_LIBS=$LIBS
dnl z and bz2 library?
AC_CHECK_LIB( bz2, BZ2_bzReadOpen,
    [
	AM_CONDITIONAL([BZ2UNCOMPRESS], [true])
	BES_BZ2_LIBS=-lbz2
	AC_DEFINE([HAVE_LIBBZ2], [1], [libbz2])
    ],
    [ AM_CONDITIONAL([BZ2UNCOMPRESS], [false]) ] )
    
AC_CHECK_LIB( z, gzopen, [BES_ZLIB_LIBS=-lz])

dnl dl lib?
AC_CHECK_FUNC(dlclose, [], [ AC_CHECK_LIB(dl, dlopen, [BES_DL_LIBS=-ldl]) ])

LIBS=$BES_OLD_LIBS

AC_SUBST(BES_DL_LIBS)
AC_SUBST(BES_ZLIB_LIBS)
AC_SUBST(BES_BZ2_LIBS)

dnl Checks for libraries.

# check for readline
VL_LIB_READLINE
AS_IF([test "$vl_cv_lib_readline" = "no"], [AC_MSG_ERROR([I could not find the readline library!])])

xmlprivatereq=
xmlprivatelibs=
xml_required_version=2.6.16
libdap_pkgconfig_libxml2=yes
libdap_libxml2_module='libxml-2.0 >= $xml_required_version'

PKG_CHECK_MODULES([XML2],[$libdap_libxml2_module], [], [libdap_pkgconfig_libxml2=no])
 
AS_IF([test $libdap_pkgconfig_libxml2 = 'yes'],
    [
    xmlprivatereq=$libdap_libxml2_module
    XML2_STATIC_LIBS="`$PKG_CONFIG --static --libs libxml-2.0`"
    AC_MSG_RESULT([yes; used pkg-config])
    ],
    [xml2-config --version > /dev/null 2>&1],
    [
    AC_MSG_RESULT([no; used pkg-config])
    AC_MSG_CHECKING([for libxml2 version >= $xml_required_version])
    version_libxml2=`xml2-config --version`
    AS_VERSION_COMPARE(["$version_libxml2"], ["$xml_required_version"], 
        [
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([I could not find libxml2 $xml_required_version or newer, found $version_libxml2])
        ])
    XML2_LIBS="`xml2-config --libs`"
    XML2_STATIC_LIBS=$XML2_LIBS
    XML2_CFLAGS="`xml2-config --cflags`"
    xmlprivatelibs="`xml2-config --libs`" # `
    AC_MSG_RESULT([yes; used xml2-config])
    ],
    [AC_MSG_ERROR([I could not find libxml2])])
    
AC_SUBST([xmlprivatereq])
AC_SUBST([xmlprivatelibs])
AC_SUBST([XML2_LIBS])
AC_SUBST([XML2_STATIC_LIBS])
AC_SUBST([XML2_CFLAGS])

SIC_VAR_SYS_ERRLIST

dnl jhrg 10/14/15 BES_CHECK_OPENSSL
dnl BES_CHECK_KERBEROS

AC_CHECK_LIBDAP([3.18.0], [AM_CONDITIONAL([LIBDAP], [true])], [AM_CONDITIONAL([LIBDAP], [false])])

dnl AS_CASE([$variable], [foo*], [run1], [bar*], [run2], [catchall])
    
dnl extra argument: --with-libwrap
AC_MSG_CHECKING(whether to use libwrap)
AC_ARG_WITH(libwrap,
    [AS_HELP_STRING([--with-libwrap], [Compile in libwrap (tcp_wrappers) support.])],
    [AS_CASE([$withval],
        [yes],  
        [
            AC_MSG_RESULT(yes)
            AC_CHECK_LIB(wrap, request_init, [LIBS="-lwrap $LIBS" AC_DEFINE([HAVE_LIBWRAP], [1], [Description])],
                [AC_MSG_ERROR(Could not find libwrap library. You have to insall tcp-wrappers before using --with-libwrap.)])
        ],
        [AC_MSG_RESULT(no)])],  dnl The default case (!= yes) is no.
    [AC_MSG_RESULT(no)])
    
AC_HEADER_TR1_FUNCTIONAL

# Check for cppunit to enable unit testing.
AM_PATH_CPPUNIT(1.12.0,
	[AM_CONDITIONAL([CPPUNIT], [true])],
	[
	    PKG_CHECK_MODULES(CPPUNIT, [cppunit >= 1.12.0],
		[AM_CONDITIONAL([CPPUNIT], [true])],
		[AM_CONDITIONAL([CPPUNIT], [false])]
	    )
	]
)

dnl Use this we add in SQL and DMR++. jhrg 10/9/17

dnl AC_ARG_WITH([dap-exp-modules],
dnl   [AS_HELP_STRING([--with-dap-exp-modules], [build the experimental dap modules (default is no)])],
dnl   [with_dap_exp_modules=${withval} ],
dnl   [with_dap_exp_modules=no])

dnl AM_CONDITIONAL([DAP_EXP_MODULES], [test x$with_dap_exp_modules = xyes])

dnl dap-builtin-mouldes is true by default; use --disable-dap-builtin-modules or
dnl --enable-dap-builtin-modules=no to turn this off. jhrg 11/17/13

AC_ARG_WITH([dap-builtin-modules],
  [AS_HELP_STRING([--with-dap-builtin-modules], [build the builtin dap modules (default is yes)])],
  [with_dap_builtin_modules=${withval} ],
  [with_dap_builtin_modules=yes])

AM_CONDITIONAL([DAP_BUILTIN_MODULES], [test x$with_dap_builtin_modules = xyes])

dnl These are tests that only apply if the modules are being built too.
AM_COND_IF([DAP_BUILTIN_MODULES], [

ac_bes_dependencies_prefix=
AC_ARG_WITH([dependencies],
	[AS_HELP_STRING([--with-dependencies],[The prefix for the dependencies])],
    [AS_IF([test -n "$with_dependencies"], 
           [ac_bes_dependencies_prefix=$with_dependencies
            dnl This conditional and symbol substitution pass the deps value to the 
            dnl top level makefile. jhrg 3/18/15
            AM_CONDITIONAL([WITH_DEPENDENCIES], [true])
            AC_SUBST([ac_bes_dependencies_prefix])
            AC_MSG_RESULT([Using ${ac_bes_dependencies_prefix} as the dependencies prefix])])],
           [AM_CONDITIONAL([WITH_DEPENDENCIES], [false])])

BES_DISPATCH_LIB="-L\$(top_builddir)/dispatch -lbes_dispatch"
AC_SUBST(BES_DISPATCH_LIB)
BES_XML_CMD_LIB="-L\$(top_builddir)/xmlcommand -lbes_xml_command"
AC_SUBST(BES_XML_CMD_LIB)
BES_PPT_LIB="-L\$(top_builddir)/ppt -lbes_ppt"
AC_SUBST(BES_PPT_LIB)
BES_DAP_LIB="-L\$(top_builddir)/dap -ldap_module"
AC_SUBST(BES_DAP_LIB)
BES_EXTRA_LIBS="$LIBS $openssl_libs"
AC_SUBST(BES_EXTRA_LIBS)

dnl These tests are for the dependecies of specific handlers. Added jhrg 11/17/13

# This is used by 'usage' which builds the .info response. It's likely 
# old and should be removed
AC_DEFINE([DAP_PROTOCOL_VERSION], ["3.2"], [What DAP version is supported?])
AC_SUBST([DAP_PROTOCOL_VERSION])

dnl DMR++ module support - only when BES_DEVELOPER is set do we need this.
dnl jhrg 11/28/17
dnl
dnl support the --with-curl option for specifying a particular version of curl.
dnl This will be used only when BES_DEVELOPER (aka --enable-developer) is set,
dnl since that is the only time the dmr++ module is build.

AC_ARG_VAR(CURL_LIBS, [libcurl required libraries])
AC_ARG_VAR(CURL_CFLAGS, [libcurl required compiler flags])

AM_COND_IF([BES_DEVELOPER], [

curl_is_ok=no
curl_req_version=7.20.0

AC_ARG_WITH([curl],
    AS_HELP_STRING([--with-curl], [Prefix where curl/libcurl is installed; any version found is used.]),
    with_curl_prefix="$withval",
    with_curl_prefix="")

AS_IF([test -n "$with_curl_prefix" -a -x $with_curl_prefix/bin/curl-config],
    [AC_MSG_CHECKING([for libcurl])
     CURL_LIBS="`$with_curl_prefix/bin/curl-config --libs`"
     CURL_CFLAGS="`$with_curl_prefix/bin/curl-config --cflags`"
     AC_MSG_RESULT([yes; used $with_curl_prefix/bin/curl-config])
     curl_is_ok=yes],
    
    dnl FAIL if $with_curl_prefix is not zero but the script is not executable
    [test -n "$with_curl_prefix"],
    [AC_MSG_ERROR([curl-prefix set to $with_curl_prefix, but curl-config is not there.])])
    
AC_ARG_VAR(PKG_CONFIG, [path to pkg-config utility])
AC_PATH_TOOL([PKG_CONFIG], [pkg-config])

AS_IF([test -n "$PKG_CONFIG"], 
    [AC_MSG_CHECKING([for libcurl using pkg-config])
     AS_IF([pkg-config --atleast-version=$curl_req_version curl],
        [CURL_CFLAGS=`pkg-config --cflags curl`
         CURL_LIBS=`pkg-config --libs curl`
         AC_MSG_RESULT([yes; found at least version $curl_req_version])
         curl_is_ok=yes],
        [AC_MSG_RESULT([no, needed $curl_req_version])])])
    
AC_ARG_VAR(CURL_CONFIG, [path to curl-config utility])
AC_PATH_TOOL([CURL_CONFIG], [curl-config])

AS_IF([test -n "$CURL_CONFIG"], 
    [AC_MSG_CHECKING([for libcurl using curl-config])
     version_libcurl=`curl-config --version | sed 's@libcurl \(.*\)@\1@'`
     AS_VERSION_COMPARE([$version_libcurl], [$curl_req_version], [curl_is_ok=no], [curl_is_ok=yes], [curl_is_ok=yes])
     AS_IF([test $curl_is_ok = yes],
        [CURL_LIBS="`curl-config --libs`"
         CURL_CFLAGS="`curl-config --cflags`"
         AC_MSG_RESULT([yes; found version $version_libcurl])],
        [AC_MSG_RESULT([no, found version $version_libcurl but needed $curl_req_version])])])

AS_IF([test $curl_is_ok = no],       
    [AC_MSG_ERROR([libcurl: insufficient version or not found])])
    
])  dnl End AM_COND_IF BES_DEVELOPER for DMR++/libcurl
    
dnl Freeform hacks

FF_CPPFLAGS=
AC_MSG_CHECKING([the OS type for the freeform code])
AS_CASE([$host],
    [*linux*], [FF_CPPFLAGS=-DLINUX],
    [*hp*], [FF_CPPFLAGS=-DHP9000],
    [*alpha*], [FF_CPPFLAGS=-DDEC_ALPHA],
    [*sun*], [FF_CPPFLAGS=-DSUN],
    [*darwin*], [FF_CPPFLAGS=-DLINUX],
    [*sgi*], [AS_IF([test $ac_cv_sizeof_long -eq 4],
                    [FF_CPPFLAGS=-DIRIS4],
                    [FF_CPPFLAGS=-DIRIX])],
    [AC_MSG_WARN([Either this machine is not supported by FreeForm or the configure script needs to be fixed])])
    
AC_MSG_RESULT([set to $FF_CPPFLAGS])
AC_SUBST(FF_CPPFLAGS)

dnl xml_data_handler hack
AC_DEFINE([ENABLE_UNIT_TESTS], [1], [Should the XML Data Handler unit-test support be compiled?])
AC_SUBST(ENABLE_UNIT_TESTS)

AC_MSG_CHECKING([the netCDF library])

dnl CHECK_NETCDF defines $NC_LIBS, $NC_LDFLAGS and $NC_CPPFLAGS
AC_CHECK_NETCDF(
   [AC_MSG_RESULT($NC_LIBS); BUILD_NETCDF=yes],
   [AC_MSG_WARN([netcdf library and/or netcdf headers not found; will not build the netcdf modules.]); BUILD_NETCDF=no],
   [3],
   $ac_bes_dependencies_prefix)

AM_CONDITIONAL([BUILD_NETCDF], [test "x$BUILD_NETCDF" = "xyes"]) 
	
AM_COND_IF([BUILD_NETCDF], [

save_LIBS=$LIBS
LIBS="$NC_LDFLAGS $NC_LIBS $LIBS"
AC_CHECK_LIB(netcdf, nc_inq_libvers, NETCDF_MAJOR_VERSION=4, NETCDF_MAJOR_VERSION=3, [])
LIBS=$save_LIBS

# save_CPPFLAGS=$CPPFLAGS
# CPPFLAGS="$NC_CPPFLAGS $CPPFLAGS"
# AC_EGREP_HEADER(NC_NETCDF4, netcdf.h, NETCDF_MAJOR_VERSION=4, NETCDF_MAJOR_VERSION=3)
# CPPFLAGS=$save_CPPFLAGS

AC_DEFINE_UNQUOTED(NETCDF_VERSION, $NETCDF_MAJOR_VERSION, [What version of netcdf were we built with?])

dnl This is a bit hokey, since the real test - for nc_lib_vers 
dnl was done above.
AC_MSG_CHECKING([the netCDF library version number])
AC_MSG_RESULT($NETCDF_MAJOR_VERSION)

dnl This is needed for the fileout_netcdf handler unit tests
NC_BIN=`echo $NC_LDFLAGS | sed 's@^-L\(.*\)/lib@\1/bin@g'`
AC_MSG_NOTICE([NC_BIN is $NC_BIN])
AC_SUBST(NC_BIN_PATH, $NC_BIN)

])	# AM_COND_IF BUILD_NETCDF

AM_CONDITIONAL([NETCDF4_TESTS], [test "x$NETCDF_MAJOR_VERSION" = "x4"])

dnl fileout_gdal tests

# Which copy of GDAL should be used for the build?
GDAL_FOUND=

AC_ARG_WITH(gdal, AS_HELP_STRING([--with-gdal], [Use the copy of GDAL at this location]),
            with_gdal_prefix="$withval", with_gdal_prefix="")

AS_IF([test -z "$with_gdal_prefix" -a -n "$ac_bes_dependencies_prefix"],
	[with_gdal_prefix=$ac_bes_dependencies_prefix])
 
AS_IF([test -n "$with_gdal_prefix" -a -x $with_gdal_prefix/bin/gdal-config],
    dnl then
    [AC_MSG_NOTICE([Using $with_gdal_prefix as the GDAL prefix directory.])
    GDAL_LDFLAGS="`$with_gdal_prefix/bin/gdal-config --libs` `$with_gdal_prefix/bin/gdal-config --dep-libs`"
    GDAL_CFLAGS="`$with_gdal_prefix/bin/gdal-config --cflags`"
    GDAL_FOUND="yes"
    AC_MSG_NOTICE([Set GDAL_LDFLAGS to $GDAL_LDFLAGS.])],
    
    dnl if
    [test -n "$with_gdal_prefix"],
    dnl then
    [AC_MSG_NOTICE([You set the gdal-prefix directory to $with_gdal_prefix,])
    AC_MSG_NOTICE([but gdal-config is not there. Not building GDAL-dependent modules.])],
    
    dnl else
    [AC_MSG_NOTICE([Looking for GDAL.])
    AX_LIB_GDAL([1.10.0])
    AS_IF([test ! -z "$GDAL_CFLAGS" -a ! -z "$GDAL_LDFLAGS"], [GDAL_FOUND=yes])])

AM_CONDITIONAL([BUILD_GDAL], [test x$GDAL_FOUND = xyes])

dnl Only look for the openjpeg2000 library if GDAL was found

AM_COND_IF([BUILD_GDAL], [

    dnl Which copy of OpenJPEG should be used for the build?
    OPENJPEG_FOUND=

    AC_ARG_WITH(openjpeg, AS_HELP_STRING([--with-openjpeg], [Use the copy of openjpeg at this location]),
            with_openjpeg_prefix="$withval", with_openjpeg_prefix="")

    AS_IF([test -z "$with_openjpeg_prefix" -a -n "$ac_bes_dependencies_prefix"],
	   [with_openjpeg_prefix=$ac_bes_dependencies_prefix])

    save_LIBS=$LIBS
    LIBS="-L$with_openjpeg_prefix/lib $LIBS"
    AC_CHECK_LIB(openjp2, opj_version, [OPENJPEG_FOUND="yes"], [OPENJPEG_FOUND="no"], [-lm])
    LIBS=$save_LIBS

    AM_CONDITIONAL([OPENJPEG_FOUND], [test "x$OPENJPEG_FOUND" = "xyes"])

    AS_IF([test "x$OPENJPEG_FOUND" = "xyes"], 
        [ AC_MSG_NOTICE([Found OPENJPEG; building GMLJP2 support.]) ],
        [ AC_MSG_NOTICE([I could not find OPENJPEG, not building GMLJP2 support.]) ])

],[ # else AN_CONF_IF([BUILD_GDAL]

    dnl if we are not building the gdal-related things, make sure that 
    dnl OPENJPEG_FOUND is defined and is false
    AM_CONDITIONAL([OPENJPEG_FOUND], [false])

]) # AN_CONF_IF([BUILD_GDAL] 

dnl Fits macro and test

dnl Check for the cfits library.
dnl Usage AC_CHECK_FITS action-if-found, action-if-not-found, default-path-to-check
dnl if default-path-to-check is given and the --with-cfits option(s) are not used,
dnl check the default path in addition to the current values of CPPFLAGS, etc.
dnl Defines CFITS_CPPFLAGS, CFITS_LDFLAGS, CFITS_LIBS.

AC_DEFUN([AC_CHECK_CFITS],
[

BES_DEPS_PREFIX=$3

AC_ARG_WITH([cfits],
            [AS_HELP_STRING([--with-cfits=ARG],[cfits directory])],
            [CFITS_PATH=${withval}], 
            [CFITS_PATH="$BES_DEPS_PREFIX"])

AC_ARG_WITH([cfits_inc],
            [AS_HELP_STRING([--with-cfits-inc=ARG],[cfits include directory])],
            [CFITS_PATH_INC=${withval}], 
            [CFITS_PATH_INC=""]) dnl was "/usr/include/cfitsio"

AC_ARG_WITH([cfits_libdir],
            [AS_HELP_STRING([--with-cfits-libdir=ARG],[cfits library directory])],
            [CFITS_PATH_LIBDIR=${withval}], 
            [CFITS_PATH_LIBDIR=""])  dnl was "/usr/lib"

AS_IF([test -n "$CFITS_PATH" -a -z "$CFITS_PATH_LIBDIR"],[CFITS_PATH_LIBDIR="$CFITS_PATH/lib"])
AS_IF([test -n "$CFITS_PATH" -a -z "$CFITS_PATH_INC"],[CFITS_PATH_INC="$CFITS_PATH/include"])


ac_cfits_save_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -L$CFITS_PATH_LIBDIR"
AC_CHECK_LIB(cfitsio, fits_is_url_absolute, 

[ac_cfits_ok='yes'
CFITS_LIBS="-lcfitsio"
CFITS_LDFLAGS="-L${CFITS_PATH_LIBDIR}" 
AC_SUBST([CFITS_LDFLAGS])
AC_SUBST([CFITS_LIBS])],

[ac_cfits_ok='no'],
[-lm]
)

LDFLAGS="$ac_cfits_save_LDFLAGS"

CFITS_CPPFLAGS=
ac_cfits_h='no'
ac_cfits_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -I$CFITS_PATH_INC"
AC_CHECK_HEADERS([fitsio.h],
  [ac_cfits_h='yes'],
  [ac_cfits_h='no']
)
CPPFLAGS=$ac_cfits_save_CPPFLAGS

AS_IF([test "$ac_cfits_h" = 'yes' ], [CFITS_CPPFLAGS="-I$CFITS_PATH_INC"])

AC_SUBST([CFITS_CPPFLAGS])

AS_IF([test "$ac_cfits_ok" = 'no' -o "$ac_cfits_h" = 'no'],
  [m4_if([$2], [], [:], [$2])],
  [m4_if([$1], [], [:], [$1])]
)

])

AC_CHECK_CFITS(
   [AM_CONDITIONAL([BUILD_FITS],[true])],
   [AC_MSG_WARN([The cfits library and/or cfits headers not found; not building the fits handler.])
    AM_CONDITIONAL([BUILD_FITS],[false])],
   $ac_bes_dependencies_prefix
)

dnl tests for HDF4 and HDFEOS2

AC_CHECK_HDF4(
   [AM_CONDITIONAL([BUILD_HDF4],[true])
   ],
   [AC_MSG_WARN([The hdf4 library and/or hdf4 headers not found; not building the hdf4 handler.])
    AM_CONDITIONAL([BUILD_HDF4],[false])
    ],
   $ac_bes_dependencies_prefix
)
                              
dnl Test for HDF-EOS2. If found, subst hdfeos2 symbols                                                                           

AC_ARG_WITH([hdfeos2],
            [AC_HELP_STRING([--with-hdfeos2=DIR], [Specify path to external hdfeos2 library.])],
            [HDFEOS2_DIR=${withval}], 
            [HDFEOS2_DIR="$ac_bes_dependencies_prefix"])

AS_IF([test -n "$HDFEOS2_DIR"],
    [HDFEOS2_CPPFLAGS="-I$HDFEOS2_DIR/include -DUSE_HDFEOS2_LIB"
    HDFEOS2_LDFLAGS="-L$HDFEOS2_DIR/lib"
    HDFEOS2_LIBS="-lhdfeos -lGctp"
    AC_MSG_NOTICE([HDFEOS2 set to true])
    AM_CONDITIONAL([HDFEOS2], [true])],

    [AC_MSG_NOTICE([HDFEOS2 set to false])
    AM_CONDITIONAL([HDFEOS2], [false])]
 )
    
AC_SUBST([HDFEOS2_CPPFLAGS])
AC_SUBST([HDFEOS2_LDFLAGS])
AC_SUBST([HDFEOS2_LIBS])
AC_SUBST([HDFEOS2_DIR])

dnl HDF5 tests; We pass null for the tird argument - the interface number
AC_CHECK_HDF5(
  [AM_CONDITIONAL([BUILD_HDF5],[true])
  ],
  [AC_MSG_WARN([The hdf5 library and/or hdf5 headers not found; not building the hdf5 handler.])
   AM_CONDITIONAL([BUILD_HDF5],[false])
  ],
  [],
  $ac_bes_dependencies_prefix
)

AC_CHECK_STARE(
  [AM_CONDITIONAL([BUILD_STARE],[true])
  ],
  [AC_MSG_WARN([STARE library not found.])
  AM_CONDITIONAL([BUILD_STARE],[false])
  ],
  $ac_bes_dependencies_prefix
)

dnl NCML module test
dnl
dnl Look for the minimum version of the icu libs and headers
dnl This sets: ICU_CPPFLAGS with -Iicu-include-path and ICU_LIBS with 
dnl -Licu-lib-path -licu-lib-1 ... 
dnl Note: it also handles --with-icu-prefix=path_to_icu_install
dnl in case there are multiple icu development environments on the machine
dnl NOTE: LD_LIBRARY_PATH or DYLD_LIBRARY_PATH may need to be set for this
dnl to work, however.
AC_CHECK_ICU([3.6],
	[AM_CONDITIONAL([BUILD_NCML],[true])
	AC_MSG_NOTICE([ICU_CPPFLAGS=$ICU_CPPFLAGS])
	AC_MSG_NOTICE([ICU_LIBS=$ICU_LIBS])
	],
	[AC_MSG_WARN([The icu library and/or icu headers not found; not building the NCML handler.])
 	AM_CONDITIONAL([BUILD_NCML],[false])
	],
	$ac_bes_dependencies_prefix
)

dnl Look for gridfields

dnl When pkgconfig support is added, use this
libdap_libgridfields_module="libgridfields >= 1.0.3"
libgf_needed="1.0.3"

libgf_ok="no"

dnl NB: Hackery: if --with-gridfields is not given and ac_bes_dependencies_prefix 
dnl is non-zero, use that as if --with-... was given. jhrg 12/1/14
AC_ARG_WITH([gridfields],
    [AC_HELP_STRING([--with-gridfields=path], [Use the gridfields library at this location.])],
	[AS_IF([test x"$withval" = x"yes"],
		[AC_MSG_ERROR([You must supply a path when using --with-gridfields; try --enable-gridfields.])],
		[AC_MSG_NOTICE([Using $withval as the GridFields prefix directory.])
         GF_LIBS="-L$withval/lib -lgridfields"
         GF_CFLAGS="-I$withval/include"
 	 GF_PATH=$withval
         AC_DEFINE([GRIDFIELDS],[1],[define if gridfields lib is present])
         libgf_ok="yes"])],
    [ libgf_ok="no" ])

AC_ARG_ENABLE([gridfields],
    AC_HELP_STRING([--enable-gridfields], [Enable gridfields (default is YES)]),
    [ ],
    [enable_gridfields=yes])
 
AS_IF([test "x$libgf_ok" = "xno" -a "x$enable_gridfields" = "xyes"],
    [

    dnl save PATH     
    gridfields_save_path=$PATH

    dnl if --with-dependencies was used, look there too
    AS_IF([test -n "$ac_bes_dependencies_prefix"], [PATH=$ac_bes_dependencies_prefix/bin:$PATH])

    AC_MSG_CHECKING([for libgridfields $libgf_needed])

    dnl Use AS_VERSION_COMPARE to test version numbers. jhrg 9/9/15

    AS_IF([gridfields-config --version > /dev/null 2>&1],
        [
        version_libgf=`gridfields-config --version | sed 's@gridfields \(.*\)@\1@'`
        AS_VERSION_COMPARE($version_libgf, $libgf_needed, [libgf_ok=no], [libgf_ok=yes], [libgf_ok=yes])
        AS_IF([test x$libgf_ok = xno],
            [
            AC_MSG_ERROR([FIXME must have libgf $libgf_needed or greater, found $version_libgf])
            ])

        GF_LIBS="`gridfields-config --libs`"
        GF_CFLAGS="`gridfields-config --cflags`"
        GF_PATH="`gridfields-config --prefix`"

        AC_DEFINE([GRIDFIELDS],[1],[define if gridfields lib is present])
        AC_MSG_RESULT([yes; used gridfields-config])
        ],
        [test -f $ac_bes_dependencies_prefix/lib/libgridfields.a -a -d $ac_bes_dependencies_prefix/include/gridfields/],
        [
        libgf_ok="yes"
        GF_LIBS="-L$ac_bes_dependencies_prefix/lib -lgridfields"
        GF_CFLAGS="-I$ac_bes_dependencies_prefix/include"
        GF_PATH=$ac_bes_dependencies_prefix
        AC_DEFINE([GRIDFIELDS],[1],[define if gridfields lib is present])
        AC_MSG_RESULT([yes; used --with-dependencies value ($ac_bes_dependencies_prefix)])
        ],
        [
        AC_MSG_NOTICE([I could not find libgridfields])
        ]
    )
    
    dnl Restore saved value of PATH
    export PATH=$gridfields_save_path
    ]
)

AC_SUBST([GF_LIBS])
AC_SUBST([GF_CFLAGS])
AC_SUBST([GF_PATH])

if test "$libgf_ok" = "yes"
then
    echo "Set gridfields CFLAGS to $GF_CFLAGS"
    echo "Set gridfields LIBS to $GF_LIBS"
    AM_CONDITIONAL([USING_GRIDFIELDS],[true])
else
    AM_CONDITIONAL([USING_GRIDFIELDS],[false])
fi

dnl End handler specific tests

],[ # else AM_COND_IF([DAP_BUILTIN_MODULES]

dnl These are needed because the symbols are used in each modules Makefile.am
dnl files. It might be that if the generation of those Makefiles was also conditional,
dnl then we could remove this block of code. jhrg 11/30/14
AM_CONDITIONAL([OPENJPEG_FOUND], [false])
AM_CONDITIONAL([BUILD_GDAL], [false])
AM_CONDITIONAL([NETCDF4_TESTS], [false])
AM_CONDITIONAL([BUILD_NETCDF], [false])
AM_CONDITIONAL([BUILD_FITS],[false])
AM_CONDITIONAL([BUILD_HDF4],[false])
AM_CONDITIONAL([HDFEOS2], [false])
AM_CONDITIONAL([BUILD_HDF5],[false])
AM_CONDITIONAL([BUILD_NCML],[false])
AM_CONDITIONAL([USING_GRIDFIELDS],[false])

AM_CONDITIONAL([WITH_DEPENDENCIES], [false])

])	# AM_COND_IF([DAP_BUILTIN_MODULES]

dnl autoheader macros; tack some text at the top and bottom of config.h.in
AH_TOP([#ifndef _config_h
#define _config_h])

AH_BOTTOM([

// These values are defined in configure.ac
#define ANNOTATION_SYSTEM 1

#if defined(__GNUG__) || defined(__GNUC__)
#define not_used __attribute__ ((unused))
#else
#define not_used 
#endif /* __GNUG__ || __GNUC__ */

#endif /* _config_h */])

AC_CONFIG_FILES([Makefile
		 abi_checker.xml
		 bes_dispatch.pc
		 bes_ppt.pc
		 bes_xml_command.pc
		 doxy.conf
		 main_page.doxygen
		 
		 dispatch/Makefile
		 dispatch/unit-tests/Makefile
		 dispatch/tests/Makefile
		 dispatch/tests/atlocal
		 
		 hello_world/Makefile
		 hello_world/tests/Makefile
		 hello_world/tests/atlocal
		  
		 docs/Makefile
		 
		 xmlcommand/Makefile
		 xmlcommand/unit-tests/Makefile
		 xmlcommand/tests/Makefile
		 xmlcommand/tests/atlocal
		 
		 ppt/Makefile
		 ppt/unit-tests/Makefile
		 
		 cmdln/Makefile
		 cmdln/unit-tests/Makefile
		 cmdln/tests/Makefile
		 cmdln/tests/atlocal
		 
		 standalone/Makefile
		 
		 server/Makefile

		 bin/Makefile
		 
		 templates/Makefile
		 
		 dap/Makefile
		 dap/unit-tests/Makefile
		 dap/tests/Makefile
		 dap/tests/atlocal

		 dapreader/Makefile
		 dapreader/tests/Makefile
		 dapreader/tests/atlocal

		 functions/Makefile
		 functions/tests/Makefile
		 functions/tests/atlocal
		 functions/unit-tests/Makefile
		 functions/stare/Makefile
		 ])
dnl These need to be moved into the AC_CONFIG_FILES above 
dnl when geo-functions is reenabled.
dnl		 functions/geo-functions/Makefile
dnl		 functions/geo-functions/unit-tests/Makefile

AC_CONFIG_FILES([bes-config], [chmod +x bes-config]) 
AC_CONFIG_FILES([besd], [chmod +x besd]) 
AC_CONFIG_FILES([server/besctl], [chmod +x server/besctl])

dnl A better way to control starting a stopping hyrax is needed.
dnl For now, use besd (which can be run from startup scripts or
dnl using besctl) and tomcat. both bes and tomcat can be set to
dnl start at boot time. jhrg 9/27/17 
AC_CONFIG_FILES([server/hyraxctl], [chmod +x server/hyraxctl]) 

AC_CONFIG_TESTDIR(cmdln/tests, [standalone])
AC_CONFIG_TESTDIR(hello_world/tests, [standalone])
AC_CONFIG_TESTDIR(dapreader/tests, [standalone])
AC_CONFIG_TESTDIR(dap/tests, [standalone])
AC_CONFIG_TESTDIR(functions/tests, [standalone])
AC_CONFIG_TESTDIR(dispatch/tests, [dispatch/tests])
AC_CONFIG_TESTDIR(xmlcommand/tests, [standalone])

dnl Output files if building the modules too
		 
AM_COND_IF([DAP_BUILTIN_MODULES],
    [
    AC_CONFIG_FILES([modules/Makefile
    modules/csv_handler/Makefile
    modules/csv_handler/tests/Makefile
    modules/csv_handler/tests/atlocal

    modules/usage/Makefile
    
    modules/asciival/Makefile
    modules/asciival/tests/Makefile
    modules/asciival/tests/atlocal
    modules/asciival/unit-tests/Makefile

    modules/cmr_module/Makefile
    modules/cmr_module/tests/Makefile
    modules/cmr_module/tests/atlocal
    modules/cmr_module/unit-tests/Makefile
    
    modules/httpd_catalog_module/Makefile
    modules/httpd_catalog_module/unit-tests/Makefile
    modules/httpd_catalog_module/tests/Makefile
    modules/httpd_catalog_module/tests/atlocal

    modules/www-interface/Makefile

    modules/debug_functions/Makefile
    modules/debug_functions/unit-tests/Makefile 
    modules/debug_functions/tests/Makefile 
    modules/debug_functions/tests/atlocal

    modules/dmrpp_module/Makefile
    modules/dmrpp_module/unit-tests/Makefile
    modules/dmrpp_module/tests/Makefile
    modules/dmrpp_module/tests/atlocal
    modules/dmrpp_module/data/Makefile

    modules/fileout_json/Makefile
    modules/fileout_json/unit-tests/Makefile 
    modules/fileout_json/tests/Makefile 
    modules/fileout_json/tests/atlocal

    modules/fileout_covjson/Makefile
    modules/fileout_covjson/unit-tests/Makefile 
    modules/fileout_covjson/tests/Makefile 
    modules/fileout_covjson/tests/atlocal
    
    modules/freeform_handler/Makefile
    modules/freeform_handler/tests/Makefile
    modules/freeform_handler/tests/atlocal
    modules/freeform_handler/FFND/Makefile

    modules/xml_data_handler/Makefile
    modules/xml_data_handler/unit-tests/Makefile
    modules/xml_data_handler/tests/Makefile
    modules/xml_data_handler/tests/atlocal
    
    modules/netcdf_handler/Makefile
    modules/netcdf_handler/tests/Makefile 
    modules/netcdf_handler/tests/atlocal 
	
    modules/fileout_netcdf/Makefile 	
    modules/fileout_netcdf/tests/Makefile 
    modules/fileout_netcdf/tests/atlocal
    modules/fileout_netcdf/data/build_test_data/Makefile 
	
    modules/gateway_module/Makefile 
    modules/gateway_module/tests/Makefile 
    modules/gateway_module/tests/atlocal 

    modules/fileout_gdal/Makefile
    modules/fileout_gdal/tests/Makefile 
    modules/fileout_gdal/tests/atlocal 

    modules/fits_handler/Makefile
    modules/fits_handler/tests/atlocal
    modules/fits_handler/tests/Makefile
    
    modules/gdal_handler/Makefile
    modules/gdal_handler/tests/Makefile
    modules/gdal_handler/tests/atlocal
    
    modules/hdf4_handler/Makefile
    modules/hdf4_handler/hdfclass/Makefile
    modules/hdf4_handler/bes-testsuite/Makefile
    modules/hdf4_handler/bes-testsuite/atlocal
    
    modules/hdf5_handler/Makefile
    modules/hdf5_handler/bes-testsuite/Makefile
    modules/hdf5_handler/bes-testsuite/atlocal
    modules/hdf5_handler/gctp/Makefile
    modules/hdf5_handler/gctp/src/Makefile
    
    modules/ncml_module/Makefile 
    modules/ncml_module/tests/Makefile 
    modules/ncml_module/tests/atlocal 

    modules/ugrid_functions/Makefile 
    modules/ugrid_functions/unit-tests/Makefile
    modules/ugrid_functions/tests/Makefile 
    modules/ugrid_functions/tests/atlocal
        
    modules/w10n_handler/Makefile 
    modules/w10n_handler/unit-tests/Makefile
    modules/w10n_handler/tests/Makefile 
    modules/w10n_handler/tests/atlocal])
        
    dnl Move these into the autotest.at files using an autotest option.
    AC_CONFIG_FILES([modules/xml_data_handler/tests/generate_metadata_baseline.sh],
	[chmod +x modules/xml_data_handler/tests/generate_metadata_baseline.sh])
		
    AC_CONFIG_FILES([modules/xml_data_handler/tests/generate_data_baseline.sh],
	[chmod +x modules/xml_data_handler/tests/generate_data_baseline.sh])

    AC_CONFIG_FILES([modules/fits_handler/tests/generate_metadata_baseline.sh],
	[chmod +x modules/fits_handler/tests/generate_metadata_baseline.sh])
    AC_CONFIG_FILES([modules/fits_handler/tests/generate_data_baseline.sh],
        [chmod +x modules/fits_handler/tests/generate_data_baseline.sh])
	
    AC_CONFIG_FILES([modules/hdf4_handler/bes-testsuite/generate_metadata_baseline.sh], 
        [chmod +x modules/hdf4_handler/bes-testsuite/generate_metadata_baseline.sh])
    
	dnl These can stay
    AC_CONFIG_FILES([modules/www-interface/js2h.pl], [chmod +x modules/www-interface/js2h.pl])

    AC_CONFIG_TESTDIR(modules/csv_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/asciival/tests, [standalone])
        
    AC_CONFIG_TESTDIR(modules/httpd_catalog_module/tests, [standalone])

    AC_CONFIG_TESTDIR(modules/cmr_module/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/fileout_json/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/fileout_covjson/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/debug_functions/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/dmrpp_module/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/freeform_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/xml_data_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/netcdf_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/fileout_netcdf/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/gateway_module/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/fileout_gdal/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/fits_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/gdal_handler/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/ncml_module/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/ugrid_functions/tests, [standalone])
    AC_CONFIG_TESTDIR(modules/w10n_handler/tests, [standalone])

    AC_CONFIG_TESTDIR(modules/hdf4_handler/bes-testsuite, [standalone])
    AC_CONFIG_TESTDIR(modules/hdf5_handler/bes-testsuite, [standalone])
    
    ]
)

AC_OUTPUT

