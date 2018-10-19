
# Build bescmdln, part of the BES code.

AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/ppt -I$(top_srcdir)/xmlcommand -I$(top_srcdir)/cmdln -I$(top_srcdir)/dispatch -I$(java_home)/include -I$(java_home)/include/darwin
AM_CXXFLAGS = 

if BES_DEVELOPER
AM_CPPFLAGS += -DBES_DEVELOPER
endif

# These are not used by automake but are often useful for certain types of
# debugging. The best way to use these is to run configure as:
#     export CXXFLAGS='...'; ./configure --disable-shared
# the --disable-shared is not required, but it seems to help with debuggers.
CXXFLAGS_DEBUG = -g3 -O0  -Wall -W -Wcast-align
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

#bin_PROGRAMS = besstandalone

if BES_DEVELOPER
AM_CXXFLAGS += $(CXXFLAGS_DEBUG)
endif

lib_besdir=$(libdir)/bes
lib_bes_LTLIBRARIES = libbes_standalone.la

#besstandalone_SOURCES = StandAloneApp.cc \
#	StandAloneClient.cc \
#	StandAloneApp.h \
#	StandAloneClient.h

STANDALONE_SRCS = StandAloneApp.cc \
	StandAloneClient.cc 

STANDALONE_HDRS = StandAloneApp.h \
	StandAloneClient.h

# 			../cmdln/CmdTranslation.cc \
#			../cmdln/CmdTranslation.h \
# 			../xmlcommand/BESXMLUtils.cc \
#			../xmlcommand/BESXMLUtils.h

libbes_standalone_la_CPPFLAGS = $(XML2_CFLAGS) $(AM_CPPFLAGS)
libbes_standalone_la_SOURCES = $(STANDALONE_SRCS) $(STANDALONE_HDRS)
libbes_standalone_la_LDFLAGS = -avoid-version -module
libbes_standalone_la_LIBADD = $(DAP_LIBS) $(LIBS)

#besstandalone_CPPFLAGS = $(XML2_CFLAGS) $(AM_CPPFLAGS)
## Added libbes_dap.la jhrg 12/24/12 so besstandalone can call BESCatalogUtils::delete_all_catalogs()
#besstandalone_LDADD = $(top_builddir)/dispatch/libbes_dispatch.la  \
#$(top_builddir)/xmlcommand/libbes_xml_command.la $(top_builddir)/cmdln/CmdTranslation.o \
#$(READLINE_LIBS) $(XML2_LIBS)
