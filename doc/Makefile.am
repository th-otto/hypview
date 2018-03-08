LANGUAGES=de en

SUBDIRS = de en

EXTRA_DIST = \
	macros.ui \
	hypview.css \
	Makefile.common

# create manual in specific format
# pdf, info, html are not included here
# because they are already standard automake recursive targets
help win winhelp3 win4 winhelp4 htmlhelp stg rtf tex latex lyx man nroff ps ascii txt src drc amg ipf pch pchelp hph helptag tvh vision ldoc linuxdoc aqv quickview pdflatex hah applehelp:
	for i in $(LANGUAGES); do (cd $$i && $(MAKE) $@); done

# create manual in all available formats
help-all:
	for i in $(LANGUAGES); do (cd $$i && $(MAKE) $@); done

# create standalone version of manual page
manpage:
	for i in $(LANGUAGES); do (cd $$i && $(MAKE) $@); done

# create archive of default formats
upload:	upload_src
	for i in $(LANGUAGES); do (cd $$i && $(MAKE) $@); done

# create archive of manual source
upload_src:
	$(RM) distlist
	for i in $(LANGUAGES); do (cd $$i && $(MAKE) $(PASS_FLAGS) distlist) | grep -v '^make' | sed -e 's|^\./|'$$i'/|' >> distlist; done
	echo $(DISTFILES) | tr ' ' '\n' >> distlist
	$(RM) $(UPLOAD_DIR)udo-doc.zip
	cat distlist | zip -X -@ $(UPLOAD_DIR)udo-doc.zip
	$(RM) $(UPLOAD_DIR)udo-doc.tgz
	tar cvfT - distlist | gzip -c > $(UPLOAD_DIR)udo-doc.tgz
	$(RM) $(UPLOAD_DIR)udo-doc.tar.bz2
	tar cvfT - distlist | bzip2 -c > $(UPLOAD_DIR)udo-doc.tar.bz2
	$(RM) distlist

clean-local:
	$(RM) distlist
