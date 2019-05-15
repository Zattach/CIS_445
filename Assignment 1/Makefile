CFLAGS=-W -Wall
PROGRAMS=mm1 mm2 mm1alt mm2alt

all: $(PROGRAMS)

mm1: mm1.o lcgrand.o
	gcc -o mm1 $(CFLAGS) $^

mm1alt: mm1alt.o lcgrand.o
	gcc -o mm1alt $(CFLAGS) $^

mm2: mm2.o lcgrand.o
	gcc -o mm2 $(CFLAGS) $^

mm2alt: mm2alt.o lcgrand.o
	gcc -o mm2alt $(CFLAGS) $^

clean:
	rm -f *.o *.out $(PROGRAMS)

mm1.o: mm1.c
mm1alt.o: mm1alt.c
mm2.o: mm2.c
mm2alt.o: mm2alt.c
lcgrand.o: lcgrand.c lcgrand.h
