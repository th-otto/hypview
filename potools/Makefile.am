include ../silent.am

#
# programs here are compiled for the build system.
# setup variables to clear all target related variables,
# and to use the native compiler
#
LIBS =
DEFAULT_INCLUDES = -I. -I$(top_srcdir)/src
DEFS =

CC = $(CC_FOR_BUILD)
EXEEXT = $(BUILD_EXEEXT)
#CFLAGS = $(CFLAGS_FOR_BUILD)
CFLAGS = -O2 -g -Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wwrite-strings -Wcast-qual -Wdeclaration-after-statement -Wundef -Wc++-compat -Werror
CPPFLAGS = $(CPPFLAGS_FOR_BUILD) -DCATOBJEXT=\"$(CATOBJEXT)\"
LDFLAGS = $(LDFLAGS_FOR_BUILD)

AM_CPPFLAGS = -I$(top_srcdir)/rcintl -I$(top_srcdir)/include

noinst_PROGRAMS = po2rc molist polinguas

po2rc_SOURCES = \
	po2rc.c \
	$(empty)
po2rc_LDADD = -lgettextpo

molist_SOURCES = \
	molist.c \
	$(empty)

polinguas_SOURCES = \
	polinguas.c \
	$(empty)
polinguas_LDADD = -lgettextpo
