NAME    = pal

include Makefile.defs

INCLDIR = -I${prefix}/include `pkg-config --cflags glib-2.0`
LIBDIR  =
LIBS    = `pkg-config --libs glib-2.0` -lreadline -lncurses

SRC     = main.c colorize.c output.c input.c event.c rl.c html.c latex.c \
          add.c edit.c del.c remind.c search.c manage.c
OBJ     = $(SRC:.c=.o)

ifeq ($(DEBUG),1)
      TMPDIR = tmp.debug
else
      TMPDIR = tmp.opt
endif
OBJ   = $(addprefix $(TMPDIR)/,$(SRC:.c=.o))


$(NAME): info $(OBJ)
	@echo " [gcc] $(NAME)"
	@$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $(NAME)
ifneq ($(DEBUG),1)
	@echo " [strip] $(NAME)"
	@strip $(NAME)
endif

info:
	@echo "Using CFLAGS: $(CFLAGS)"
	@echo "Using CPPFLAGS: $(CPPFLAGS)"
	@echo "Using LDFLAGS: $(LDFLAGS)"

# make dependency files
tmp.deps/%.d: %.c
	@echo " [deps] $(@)"
	@mkdir -p tmp.deps
	@set -e; $(CC) -MM $(CFLAGS) $(CPPFLAGS) $< \
        | sed 's/\($*\)\.o[ :]*/tmp.opt\/\1.o $* : /g' > $@; \
        [ -s $@ ] ||  rm -f $@
	@set -e; $(CC) -MM $(CFLAGS) $(CPPFLAGS) $< \
        | sed 's/\($*\)\.o[ :]*/tmp.debug\/\1.o $* : /g' >> $@; \
        [ -s $@ ] ||  rm -f $@


# Include dependency files here.
# If the files don't exist, this will fail silently and remake the dep files.
-include $(addprefix tmp.deps/,$(SRC:.c=.d))


tmp.debug/%.o: Makefile Makefile.defs
	@mkdir -p tmp.debug
	@echo " [${CC} debug] $*.c"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $*.c $(OUTPUT_OPTION)

tmp.opt/%.o: Makefile Makefile.defs
	@mkdir -p tmp.opt
	@echo " [${CC}] $*.c"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $*.c $(OUTPUT_OPTION)



debug: clean
	@$(MAKE) DEBUG=1






# Use "install-no-rm" instead of "install" if you don't want to check
# for old files that should be removed before installing the new
# version.  This is useful for automated package managers that
# automatically remove files when a program is uninstalled (ie
# portage, rpm).
install-no-rm: install-mo install-man install-doc install-bin install-share

# Install binary file
install-bin:
	@echo " --- Installing binary --- "
	@mkdir -p ${DESTDIR}${prefix}/bin;
	install -m 0755 -o root pal ${DESTDIR}${prefix}/bin;
	install -m 0755 -o root convert/vcard2pal ${DESTDIR}${prefix}/bin;
	@echo

install-share:
	@echo " --- Installing global data --- "
	@mkdir -p ${DESTDIR}${prefix}/share/pal;
	install -m 0644 -o root ../share/*.pal ${DESTDIR}${prefix}/share/pal
	@mkdir -p ${DESTDIR}/etc;
	install -m 0644 -o root ../pal.conf ${DESTDIR}/etc/
	@echo

# "install" will install the files needed by pal and also check to see
# if there are old pal files that should be removed.  If you don't
# want the install process to attempt to remove old files, use
# "install-no-rm".
install: install-no-rm
	@echo " --- Checking for files to remove --- "
	@echo " NOTE: You might be prompted to remove some files that appear"
	@echo " to be from an old pal installation.  Unless you know that"
	@echo " that you need these files, you should probably delete them"
	@echo " by always answering 'y'."
	@# prompt about removing old doc directories
	@find ${prefix}/share/doc -name pal-[0-9].[0-9].[0-9]\* -a \! -name pal-$(PAL_VERSION) -a -type d -maxdepth 1 -exec rm -ri {} \;;
	@# Remove pal.conf from its old location
	@rm -f ${prefix}/share/pal/pal.conf;
	@echo

# Install man page
install-man:
	@echo " --- Installing man page --- "
	cd ../; sed 's/PAL_VERSION/$(PAL_VERSION)/' pal.1.template | gzip -9 > pal.1.gz
	@mkdir -p ${DESTDIR}${prefix}/share/man/man1/
	install -m 0644 -o root ../pal.1.gz ${DESTDIR}${prefix}/share/man/man1/
	install -m 0644 -o root convert/vcard2pal.1 ${DESTDIR}${prefix}/share/man/man1/
	rm ../pal.1.gz
	@echo

# Install documentation
install-doc:
	@echo " --- Installing docs --- "
	@mkdir -p ${DESTDIR}${prefix}/share/doc/pal-$(PAL_VERSION);
	cat ../COPYING | gzip -9 > ../COPYING.gz
	install -m 0644 -o root ../COPYING.gz ${DESTDIR}${prefix}/share/doc/pal-$(PAL_VERSION);
	rm ../COPYING.gz
	cat ../INSTALL | gzip -9 > ../INSTALL.gz
	install -m 0644 -o root ../INSTALL.gz ${DESTDIR}${prefix}/share/doc/pal-$(PAL_VERSION);
	rm ../INSTALL.gz
	cat ../ChangeLog | gzip -9 > ../ChangeLog.gz
	install -m 0644 -o root ../ChangeLog.gz ${DESTDIR}${prefix}/share/doc/pal-$(PAL_VERSION);
	rm ../ChangeLog.gz
	install -m 0644 -o root ../doc/example.css ${DESTDIR}${prefix}/share/doc/pal-$(PAL_VERSION);
	@echo


# install locale information
install-mo:
	@echo " --- Installing locale information --- "
	@# GERMAN
	mkdir -p ${DESTDIR}${prefix}/share/locale/de/LC_MESSAGES/
	cd ../po; msgfmt de.po -o de.mo
	install -m 0644 -o root ../po/de.mo ${DESTDIR}${prefix}/share/locale/de/LC_MESSAGES/pal.mo
	rm ../po/de.mo
	@# SWEDISH
	mkdir -p ${DESTDIR}${prefix}/share/locale/sv/LC_MESSAGES/
	cd ../po; msgfmt sv.po -o sv.mo
	install -m 0644 -o root ../po/sv.mo ${DESTDIR}${prefix}/share/locale/sv/LC_MESSAGES/pal.mo
	rm ../po/sv.mo
	@# SPANISH
	mkdir -p ${DESTDIR}${prefix}/share/locale/es/LC_MESSAGES/
	cd ../po; msgfmt es.po -o es.mo
	install -m 0644 -o root ../po/es.mo ${DESTDIR}${prefix}/share/locale/es/LC_MESSAGES/pal.mo
	rm ../po/es.mo
	@# POLISH
	mkdir -p ${DESTDIR}${prefix}/share/locale/pl/LC_MESSAGES/
	cd ../po; msgfmt pl.po -o pl.mo
	install -m 0644 -o root ../po/pl.mo ${DESTDIR}${prefix}/share/locale/pl/LC_MESSAGES/pal.mo
	rm ../po/pl.mo
	@# TURKISH
	mkdir -p ${DESTDIR}${prefix}/share/locale/tr/LC_MESSAGES/
	cd ../po; msgfmt tr.po -o tr.mo
	install -m 0644 -o root ../po/tr.mo ${DESTDIR}${prefix}/share/locale/tr/LC_MESSAGES/pal.mo
	rm ../po/tr.mo
	@echo

# try to uninstall pal
uninstall: uninstall-mo uninstall-man
	@echo " --- Removing binary --- "
	rm -f ${prefix}/bin/pal;
	@echo
	@echo " --- Removing global data --- "
	rm -rf ${prefix}/share/pal;
	@echo
	@echo " --- Removing doc directory --- "
	rm -rf ${prefix}/share/doc/pal-$(PAL_VERSION);
	@echo
	@echo " --- Removing /etc/pal.conf --- "
	rm -f /etc/pal.conf;
	@echo
	@echo " --- Check for doc directories for other versions of pal --- "
	@echo " NOTE: If you don't have other copies of pal installed, you can"
	@echo " safely delete the directories listed below:"
	@find ${prefix}/share/doc -name pal-[0-9].[0-9].[0-9]\* -type d -maxdepth 1;

# uninstall locale information
uninstall-mo:
	@echo " --- Removing locale info --- "
	cd ${prefix}/share/locale/; find -name pal.mo -exec rm {} \;;
	@echo

# uninstall man page
uninstall-man:
	@echo " --- Removing man page --- "
	rm -f ${prefix}/share/man/man1/pal.1.gz
	@echo


# Generates a new pot file from the .c files
pot: $(SRC)
	@echo Generating new pot file from .c files.
	@xgettext -k_ --copyright-holder="Scott Kuhl" $(SRC) -o ../po/pal.pot
	@sed 's/\(#.*\)\YEAR/\12005/' ../po/pal.pot | \
	 sed 's/\(#.*\)\SOME DESCRIPTIVE TITLE\./\1pal calendar/' | \
	 sed 's/\(#.*\)\FIRST AUTHOR <EMAIL@ADDRESS>/\1Scott Kuhl/' | \
	 sed 's/\("Project-Id-Version: \)PACKAGE VERSION\(\\n"\)/\1pal $(PAL_VERSION)\2/' | \
	 sed 's/\(#.*\)\PACKAGE/\1pal/' > ../po/pal.pot.tmp
	@mv ../po/pal.pot.tmp ../po/pal.pot


# Remove binay, object files and emacs backup files
clean:
	rm -rf $(NAME) libpal.a tmp.opt tmp.debug *.o *~

# Remove object files and dependency files
cleandeps: clean
	rm -rf tmp.deps *.d

cleandep: cleandeps
distclean: cleandeps
clobber: cleandeps

splint:
	splint $(INCLDIR) $(SRC)

# this rule creates a tgz with all of the needed source files for
# distribution
dist:	pot
	@echo
	rm -rf ../pal-$(PAL_VERSION)
	mkdir ../pal-$(PAL_VERSION)
	mkdir ../pal-$(PAL_VERSION)/src
	mkdir ../pal-$(PAL_VERSION)/src/convert
	mkdir ../pal-$(PAL_VERSION)/share
	mkdir ../pal-$(PAL_VERSION)/doc
	mkdir ../pal-$(PAL_VERSION)/po
	cd ../; \
	cp Makefile ChangeLog INSTALL COPYING pal.spec.template pal.1.template pal.conf pal-$(PAL_VERSION); \
	sed 's/Version:.*/Version: $(PAL_VERSION)/' pal.spec.template > pal-$(PAL_VERSION)/pal-$(PAL_VERSION).spec; \
	cp src/*.[ch] src/Makefile src/Makefile.defs pal-$(PAL_VERSION)/src; \
	cp src/convert/*.[ch] src/convert/vcard2pal{,.1} src/convert/Makefile pal-$(PAL_VERSION)/src/convert; \
	cp doc/example.css pal-$(PAL_VERSION)/doc; \
	cp share/*.pal pal-$(PAL_VERSION)/share; \
	cp po/*.po po/pal.pot po/README pal-$(PAL_VERSION)/po;
	@echo
	@echo " --- CREATING tgz file with the files listed below ---"
	cd ../; tar -czvf pal-$(PAL_VERSION).tgz pal-$(PAL_VERSION)
	@echo " --- DONE! (created pal-$(PAL_VERSION).tgz in parent directory) --- "
	rm -rf ../pal-$(PAL_VERSION)

srpm: dist
	rpmbuild -ts ../pal-$(PAL_VERSION).tgz

libpal.a: $(OBJ)
	rm -f libpal.a
	ar rcv libpal.a $(OBJ)
	strip --strip-symbol=main libpal.a
	ar s libpal.a
	ranlib libpal.a

