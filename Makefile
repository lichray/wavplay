# cxxconf 

TAGS := exctags
PROGRAM := wavplay
CFLAGS += -Wall -Wmissing-prototypes
LDFLAGS += -lm

.PHONY : all clean
all : $(PROGRAM)
clean :
	rm -f $(PROGRAM) play.o wavplay.o tags

tags : *.h play.c wavplay.c
	$(TAGS) *.h play.c wavplay.c

man : $(PROGRAM).3
$(PROGRAM).3 : README.rst
	rst2man.py README.rst $(PROGRAM).3

$(PROGRAM) : play.o wavplay.o
	$(CC) $(LDFLAGS) -o wavplay play.o wavplay.o
play.o: play.c wavplay.h
wavplay.o: wavplay.c wavplay.h
