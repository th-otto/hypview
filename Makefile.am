AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = include rcintl potools pic icons po
if NEED_ZLIB
SUBDIRS += zlib
endif
if HAVE_PNG
SUBDIRS += libpng
endif
if WITH_PDF
SUBDIRS += hpdf
endif
if NEED_GEMLIB
SUBDIRS += rsc
endif
SUBDIRS += hyp gtk win32 tos macos tools doc www

EXTRA_DIST = \
	defaults skins \
	intltool-merge.in intltool-extract.in intltool-update.in \
	COPYING \
	langid/model.c \
	$(empty)

DISTCLEANFILES = intltool-merge intltool-extract intltool-update


if OS_TOS
HYP_VERSION := $(shell sed -n 's/\#define PACKAGE_VERSION "\(.*\)"/\1/p' include/config.h | tr -d '.')
else
HYP_VERSION := $(shell sed -n 's/\#define PACKAGE_VERSION "\(.*\)"/\1/p' include/config.h)
endif
DISTDIR = hcp-$(HYP_VERSION)

bindist:
	-rm -rf $(DISTDIR)
	mkdir $(DISTDIR)
	mkdir $(DISTDIR)/bin
	mkdir $(DISTDIR)/defaults
	mkdir $(DISTDIR)/doc
	cp -a BUGS hyp/README NEWS READPDF.txt $(DISTDIR)
	cp -a hyp/reflink$(EXEEXT) hyp/hypfind$(EXEEXT) hyp/hypinfo$(EXEEXT) hyp/refcheck$(EXEEXT) hyp/stool$(EXEEXT) hyp/hcp$(EXEEXT) $(DISTDIR)/bin
	cp -a tools/pc_conv/pc_conv$(EXEEXT) $(DISTDIR)/bin
	cp -a tools/1st_conv/1st_conv$(EXEEXT) $(DISTDIR)/bin
	cp -a tools/help_rc/help_rc$(EXEEXT) $(DISTDIR)/bin
	$(STRIP) $(DISTDIR)/bin/*
	cp -a $(srcdir)/defaults/{catalog.cfg,katalog.cfg,catalog.txt,katalog.txt,hypview.ini} $(DISTDIR)/defaults
	for lang in en de; do \
		mkdir $(DISTDIR)/doc/$$lang; \
		cp -a doc/output/$$lang/stguide/*.hyp $(DISTDIR)/doc/$$lang; \
		cp -a doc/output/$$lang/stguide/*.ref $(DISTDIR)/doc/$$lang; \
	done
if OS_WIN32
if WITH_GUI_WIN32
	cp -a win32/hypview$(EXEEXT) $(DISTDIR)/bin
	$(STRIP) $(DISTDIR)/bin/hypview$(EXEEXT)
endif
	zip=hcp-$(HYP_VERSION)-`if test "$(host_triplet)" = x86_64-w64-mingw32; then echo win64; else echo win32; fi`.zip; \
	$(RM) "$$zip"; \
	zip -r "$$zip" $(DISTDIR)
endif
if OS_TOS
	$(MAKE) -C hyp DESTDIR=../$(DISTDIR) bindir=/bin install-exec-hook
	for f in pc_conv 1st_conv help_rc; do \
		mv $(DISTDIR)/bin/$$f $(DISTDIR)/bin/$$f.ttp; \
	done
	cp -a tos/hypview.app tos/hyptree.app tos/utils/call-acc/call-acc.gtp $(DISTDIR)/bin
	$(STRIP) $(DISTDIR)/bin/*
	for f in BUGS README NEWS; do \
		mv $(DISTDIR)/$$f $(DISTDIR)/$$f.txt; \
		cr2nl -d $(DISTDIR)/$$f.txt; \
	done
	for f in READPDF.txt; do \
		cr2nl -d $(DISTDIR)/$$f; \
	done
	zip=hcp`echo $(HYP_VERSION)`.zip; \
	$(RM) "$$zip"; \
	zip -r "$$zip" $(DISTDIR)
endif
if OS_UNIX
if WITH_GUI_GTK
	cp -a gtk/hypview$(EXEEXT) $(DISTDIR)/bin
	$(STRIP) $(DISTDIR)/bin/hypview$(EXEEXT)
endif
	tar cvfj hcp-$(HYP_VERSION)-linux.tar.bz2 $(DISTDIR)
endif
if OS_MACOS
	tar cvfj hcp-$(HYP_VERSION)-macos.tar.bz2 $(DISTDIR)
endif
