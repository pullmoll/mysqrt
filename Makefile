# Version number
VER_MAJOR=0
VER_MINOR=3
VER_PATCH=0

CC=	gcc
LD=	gcc

CDEFS+=	-DVER_MAJOR=$(VER_MAJOR)
CDEFS+=	-DVER_MINOR=$(VER_MINOR)
CDEFS+=	-DVER_PATCH=$(VER_PATCH)
CFLAGS=	$(CDEFS) -W -Wall -O3 -ffast-math

LIBS=	-lgmp -lm
LDFLAGS=

PROG=mysqrt
OBJS=mysqrt.o

all:	$(PROG)

%.o%.c:
	$(CC) $(CFLAGS) -o $@ -c $<

$(PROG):	$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

factor.o:	factor.c Makefile

dist:	clean
	tar -C.. -c -v -z --exclude ".git" -f ../$(PROG)-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH).tgz $(PROG)

clean:
	rm -f $(PROG) $(OBJS) *~ *.i *.s *.core *.log
