include $(top_srcdir)/silent.am
include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -I$(srcdir)/../include $(Z_CFLAGS) -DSRCDIR=\"$(srcdir)\"
LIBS = -L../src -lhpdf -lm @LIBS@
if HAVE_PNG
AM_CPPFLAGS += $(PNG_CFLAGS)
LIBS += $(LIBPNG)
endif
LIBS += $(LIBZ)


noinst_PROGRAMS = \
	arc_demo \
	attach \
	character_map \
	chfont_demo \
	chunk_background \
	encoding_list \
	encryption \
	ext_gstate_demo \
	font_demo \
	image_demo \
	jpeg_demo \
	jpfont_demo \
	line_demo \
	link_annotation \
	make_rawimage \
	outline_demo \
	outline_demo_jp \
	permission \
	png_demo \
	raw_image_demo \
	slide_show_demo \
	text_annotation \
	text_demo \
	text_demo2 \
	ttfont_demo \
	ttfont_demo_jp \
	$(empty)

arc_demo_SOURCES = arc_demo.c grid_sheet.c grid_sheet.h
arc_demo_DEPENDENCIES = ../src/libhpdf.a

attach_SOURCES = attach.c
attach_DEPENDENCIES = ../src/libhpdf.a

chfont_demo_SOURCES = chfont_demo.c grid_sheet.c grid_sheet.h
chfont_demo_DEPENDENCIES = ../src/libhpdf.a

chunk_background_SOURCES = chunk_background.c
chunk_background_DEPENDENCIES = ../src/libhpdf.a

character_map_SOURCES = character_map.c
character_map_DEPENDENCIES = ../src/libhpdf.a

encoding_list_SOURCES = encoding_list.c
encoding_list_DEPENDENCIES = ../src/libhpdf.a

encryption_SOURCES = encryption.c
encryption_DEPENDENCIES = ../src/libhpdf.a

ext_gstate_demo_SOURCES = ext_gstate_demo.c
ext_gstate_demo_DEPENDENCIES = ../src/libhpdf.a

font_demo_SOURCES = font_demo.c
font_demo_DEPENDENCIES = ../src/libhpdf.a

image_demo_SOURCES = image_demo.c
image_demo_DEPENDENCIES = ../src/libhpdf.a

jpeg_demo_SOURCES = jpeg_demo.c
jpeg_demo_DEPENDENCIES = ../src/libhpdf.a

jpfont_demo_SOURCES = jpfont_demo.c
jpfont_demo_DEPENDENCIES = ../src/libhpdf.a

line_demo_SOURCES = line_demo.c
line_demo_DEPENDENCIES = ../src/libhpdf.a

link_annotation_SOURCES = link_annotation.c
link_annotation_DEPENDENCIES = ../src/libhpdf.a

make_rawimage_SOURCES = make_rawimage.c
make_rawimage_DEPENDENCIES = ../src/libhpdf.a

outline_demo_SOURCES = outline_demo.c
outline_demo_DEPENDENCIES = ../src/libhpdf.a

outline_demo_jp_SOURCES = outline_demo_jp.c
outline_demo_jp_DEPENDENCIES = ../src/libhpdf.a

permission_SOURCES = permission.c
permission_DEPENDENCIES = ../src/libhpdf.a

png_demo_SOURCES = png_demo.c
png_demo_DEPENDENCIES = ../src/libhpdf.a

raw_image_demo_SOURCES = raw_image_demo.c
raw_image_demo_DEPENDENCIES = ../src/libhpdf.a

slide_show_demo_SOURCES = slide_show_demo.c
slide_show_demo_DEPENDENCIES = ../src/libhpdf.a

text_annotation_SOURCES = text_annotation.c
text_annotation_DEPENDENCIES = ../src/libhpdf.a

text_demo_SOURCES = text_demo.c grid_sheet.c grid_sheet.h
text_demo_DEPENDENCIES = ../src/libhpdf.a

text_demo2_SOURCES = text_demo2.c grid_sheet.c grid_sheet.h
text_demo2_DEPENDENCIES = ../src/libhpdf.a

ttfont_demo_SOURCES = ttfont_demo.c
ttfont_demo_DEPENDENCIES = ../src/libhpdf.a

ttfont_demo_jp_SOURCES = ttfont_demo_jp.c
ttfont_demo_jp_DEPENDENCIES = ../src/libhpdf.a


CLEANFILES = \
	arc_demo.pdf \
	attach.pdf \
	character_map.pdf \
	chfont_demo.pdf \
	chunk_background.pdf \
	encoding_list.pdf \
	encryption.pdf \
	ext_gstate_demo.pdf \
	font_demo.pdf \
	grid_sheet.pdf \
	image_demo.pdf \
	jpeg_demo.pdf \
	jpfont_demo.pdf \
	line_demo.pdf \
	link_annotation.pdf \
	outline_demo.pdf \
	outline_demo_jp.pdf \
	permission.pdf \
	png_demo.pdf \
	raw_image_demo.pdf \
	slide_show_demo.pdf \
	text_annotation.pdf \
	text_demo.pdf \
	text_demo2.pdf \
	ttfont_demo.pdf \
	ttfont_demo_jp.pdf \
	$(empty)

EXTRA_DIST = \
	font_demo.cpp \
	images \
	mbtext \
	pngsuite \
	rawimage \
	ttfont \
	type1 \
	$(empty)
