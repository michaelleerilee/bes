# Tests

AUTOMAKE_OPTIONS = foreign

# Headers in 'tests' are used by the arrayT unit tests.

AM_CPPFLAGS = -I$(top_srcdir)/xmlcommand -I$(top_srcdir)/dispatch $(DAP_CFLAGS)
AM_LDADD =  $(top_builddir)/dispatch/libbes_dispatch.la -ltest-types $(DAP_LIBS) $(LIBS)

if CPPUNIT
AM_CPPFLAGS += $(CPPUNIT_CFLAGS)
AM_LDADD += $(CPPUNIT_LIBS)
endif

# These are not used by automake but are often useful for certain types of
# debugging. Set CXXFLAGS to this in the nightly build using export ...
CXXFLAGS_DEBUG = -g3 -O0  -Wall -W -Wcast-align
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

# This header file is used for configuration location
noinst_HEADERS = test_config.h

# This determines what gets built by make check
check_PROGRAMS = $(UNIT_TESTS)

# This determines what gets run by 'make check.'
TESTS = $(UNIT_TESTS)

DIRS_EXTRA = 

EXTRA_DIST = $(DIRS_EXTRA) test_config.h.in bes.conf

CLEANFILES = bes.log

DISTCLEANFILES = test_config.h

############################################################################
# Unit Tests

test_config.h: test_config.h.in Makefile
	sed -e "s%[@]abs_srcdir[@]%${abs_srcdir}%" $< > test_config.h

if CPPUNIT
UNIT_TESTS = propsT buildT
else
UNIT_TESTS =

check-local:
	@echo ""
	@echo "**********************************************************"
	@echo "You must have cppunit 1.12.x or greater installed to run *"
	@echo "check target in xmlcommand unit-tests directory          *"
	@echo "**********************************************************"
	@echo ""
endif


buildT_SOURCES = buildT.cc BuildTInterface.cc BuildTCmd1.cc BuildTCmd2.cc BuildTInterface.h BuildTCmd1.h BuildTCmd2.h
buildT_CPPFLAGS = $(XML2_CFLAGS) $(AM_CPPFLAGS)
buildT_LDADD = $(top_builddir)/xmlcommand/libbes_xml_command.la $(top_builddir)/dispatch/libbes_dispatch.la $(XML2_LIBS) $(AM_LDADD)

propsT_SOURCES = propsT.cc
propsT_CPPFLAGS = $(XML2_CFLAGS) $(AM_CPPFLAGS)
propsT_LDADD = $(top_builddir)/xmlcommand/libbes_xml_command.la $(top_builddir)/dispatch/libbes_dispatch.la $(XML2_LIBS) $(AM_LDADD)

