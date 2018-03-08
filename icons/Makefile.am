include ../Makefile.common

pixmapdir = $(datadir)/pixmaps

pixmap_DATA = hypview.png

EXTRA_DIST = \
	hypview.h  hypview.ico \
	back.h     back.ico \
	help.h     help.ico \
	home.h     home.ico \
	index.h    index.ico \
	info.h     info.ico \
	catalog.h  catalog.ico \
	load.h     load.ico \
	bookmark.h bookmark.ico \
	menu.h     menu.ico \
	history.h  history.ico \
	next.h     next.ico \
	nextphys.h nextphys.ico \
	previous.h previous.ico \
	prevphys.h prevphys.ico \
	first.h    first.ico \
	last.h     last.ico \
	referenc.h referenc.ico \
	save.h     save.ico \
	remarker.h remarker.ico \
	$(pixmap_DATA) \
	$(empty)

icons: \
	$(srcdir)/hypview.h \
	$(srcdir)/back.h \
	$(srcdir)/help.h \
	$(srcdir)/home.h \
	$(srcdir)/index.h \
	$(srcdir)/info.h \
	$(srcdir)/catalog.h \
	$(srcdir)/load.h \
	$(srcdir)/bookmark.h \
	$(srcdir)/menu.h \
	$(srcdir)/history.h \
	$(srcdir)/next.h \
	$(srcdir)/nextphys.h \
	$(srcdir)/previous.h \
	$(srcdir)/prevphys.h \
	$(srcdir)/first.h \
	$(srcdir)/last.h \
	$(srcdir)/referenc.h \
	$(srcdir)/save.h \
	$(srcdir)/remarker.h \
	$(empty)

GEN_PIXBUF_CSOURCE = gen-pixbuf-csource

$(srcdir)/hypview.h: $(srcdir)/hypview.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=hypview_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/back.h: $(srcdir)/back.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=back_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/help.h: $(srcdir)/help.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=help_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/home.h: $(srcdir)/home.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=home_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/index.h: $(srcdir)/index.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=index_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/info.h: $(srcdir)/info.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=info_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/catalog.h: $(srcdir)/catalog.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=catalog_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/load.h: $(srcdir)/load.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=load_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/bookmark.h: $(srcdir)/bookmark.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=bookmark_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/menu.h: $(srcdir)/menu.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=menu_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/history.h: $(srcdir)/history.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=history_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/next.h: $(srcdir)/next.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=next_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/nextphys.h: $(srcdir)/nextphys.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=nextphys_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/previous.h: $(srcdir)/previous.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=previous_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/prevphys.h: $(srcdir)/prevphys.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=prevphys_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/first.h: $(srcdir)/first.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=first_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/last.h: $(srcdir)/last.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=last_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/referenc.h: $(srcdir)/referenc.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=reference_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/save.h: $(srcdir)/save.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=save_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

$(srcdir)/remarker.h: $(srcdir)/remarker.ico
	$(AM_V_GEN)$(GEN_PIXBUF_CSOURCE) --raw --static --name=remarker_icon_data $< > $@.tmp && mv $@.tmp $@ || { $(RM) $@.tmp; false; }

