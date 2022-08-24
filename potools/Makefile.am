AUTOMAKE_OPTIONS = -Wnone

include $(top_srcdir)/silent.am

#
# programs here are compiled for the build system.
# setup variables to clear all target related variables,
# and to use the native compiler
#
# Note: automake will warn about some of this variables,
# suggesting to use AM_* etc. But that's nonsense,
# we must make sure that CFLAGS, LDFLAGS etc. do
# *not* use the values for the target system
#
LIBS =
DEFAULT_INCLUDES = -I. -I$(top_srcdir)/src
DEFS =
CC = $(CC_FOR_BUILD)
EXEEXT = $(BUILD_EXEEXT)
#CFLAGS = $(CFLAGS_FOR_BUILD)
CFLAGS =
CPPFLAGS =
LDFLAGS =

#AM_CFLAGS = $(CFLAGS_FOR_BUILD)
AM_CFLAGS = -O2 -g -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wwrite-strings -Wcast-qual -Wdeclaration-after-statement -Wundef -Wc++-compat -Werror
if OS_WIN32
AM_CFLAGS += -mwin32
endif
AM_CPPFLAGS = $(CPPFLAGS_FOR_BUILD) -DCATOBJEXT=\"$(CATOBJEXT)\"
AM_CPPFLAGS += -I$(top_srcdir)/rcintl -I$(top_srcdir)/include
AM_LDFLAGS = $(LDFLAGS_FOR_BUILD)

noinst_PROGRAMS = molist
if HAVE_GETTEXTPO
noinst_PROGRAMS += po2rc polinguas
endif

molist_SOURCES = \
	molist.c \
	$(empty)

po2rc_SOURCES = \
	po2rc.c \
	$(empty)
po2rc_LDADD = -lgettextpo

polinguas_SOURCES = \
	polinguas.c \
	$(empty)
polinguas_LDADD = -lgettextpo
