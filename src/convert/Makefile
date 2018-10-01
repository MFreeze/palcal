include ../Makefile.defs

INCLDIR = -I${prefix}/include `pkg-config --cflags glib-2.0`
LIBDIR  =


SRC     = ical2pal.c pal2ical.c

OBJ     = $(SRC:.c=.o)


help:
	@echo "Please use one of the following targets:"
	@echo "  ical           Make ical <--> pal conversion utilities (requires libical)"
	@echo "  ical-install   Install ical <--> pal conversion utilities (requires libical)"
	@echo ""



ical: ical2pal pal2ical

ical2pal: ical2pal.o
	$(CC) $(CFLAGS) ical2pal.o -lical -lpthread -o ical2pal
ifneq ($(DEBUG),1)
	strip ical2pal
endif


pal2ical: pal2ical.o
	cd .. && make libpal.a
	$(CC) $(CFLAGS) pal2ical.o -L.. -lpal  `pkg-config --libs glib-2.0` -lreadline -lncurses -o pal2ical
ifneq ($(DEBUG),1)
	strip pal2ical
endif


ical-debug: clean
	@$(MAKE) ical DEBUG=1


# Install binary file
ical-install:
	@echo " --- Installing binary --- "
	@mkdir -p ${DESTDIR}${prefix}/bin;
	install -m 0755 -o root ical2pal ${DESTDIR}${prefix}/bin;
	install -m 0755 -o root pal2ical ${DESTDIR}${prefix}/bin;
	@echo


# try to uninstall pal
ical-uninstall:
	@echo " --- Removing binary --- "
	rm -f ${prefix}/bin/ical2pal;
	rm -f ${prefix}/bin/pal2ical;
	@echo


# Remove binay, object files and emacs backup files
clean:
	rm -f ical2pal pal2ical $(OBJ) *~

# Remove object files and dependency files
cleandeps: clean
	rm -f $(SRC:.c=.d)

cleandep: cleandeps

splint:
	splint $(INCLDIR) $(SRC)

