ifdef CROSS
override CROSS+=-
endif

CC=$(CROSS)gcc
CFLAGS=-O3
LD=$(CROSS)ld
LDFLAGS=-s -static
LIBS=-lkernel32 -luser32 -lshell32 -ladvapi32
RC=$(CROSS)windres

all: clickkeys.exe

clickkeys.res: clickkeys.rc
	$(RC) $< -o $@

clickkeys.obj: resource.h
clickkeys.rc: resource.h manifest.xml clickkeys.ico

clickkeys.exe: clickkeys.obj clickkeys.res
	$(LD) $(LDFLAGS) $(LIBS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -p $@

clean:
	del clickkeys.o clickkeys.res

distclean: clean
	del clickkeys.exe
