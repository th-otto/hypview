include ../silent.am

INCLUDES = 

if OS_WIN32
noinst_LIBRARIES = librcintl.a
endif

librcintl_a_SOURCES = \
	gettextP.h libintl.h gmo.h loadinfo.h rcsysdep.h \
	dcigettext.c \
	gettext.c \
	dgettext.c \
	dngettext.c \
	ngettext.c \
	hash-string.c hash-string.h \
	plural-exp.c plural-exp.h eval-plural.h \
	plural.c plural.h \
	finddomain.c \
	loadmsgcat.c \
	explodename.c \
	l10nflist.c \
	bindtextdom.c \
	textdomain.c \
	tsearch.c tsearch.h \
	localenm.c localename.h \
	windows.rh \
	$(empty)
