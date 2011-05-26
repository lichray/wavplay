# cxxconf 

TAGS := exctags
PROGRAM := wavplay
CFLAGS += -Wall
PREFIX := /usr/local

ifdef WITH_ALSA
CFLAGS += $(shell pkg-config --cflags alsa) -DUSE_ALSA
LDFLAGS += $(shell pkg-config --libs alsa)
endif

.PHONY : all clean
all : $(PROGRAM)
clean :
	rm -f $(PROGRAM) play.o wavplay.o tags
install : $(PROGRAM)
	install -m 755 $(PROGRAM) $(PREFIX)/bin/$(PROGRAM)
uninstall : $(PROGRAM)
	-rm $(PREFIX)/bin/$(PROGRAM)

tags : *.h play.c wavplay.c
	$(TAGS) *.h play.c wavplay.c

man : wavplay.3
wavplay.3 : README.rst
	rst2man.py README.rst wavplay.3

$(PROGRAM) : play.o wavplay.o
	$(CC) $(LDFLAGS) -o $(PROGRAM) play.o wavplay.o
play.o: play.c wavplay.h
wavplay.o: wavplay.c wavplay.h
