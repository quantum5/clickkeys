CC=cl /nologo
CFLAGS=/Ox /GS-
LD=link /nologo
LDFLAGS=/entry:RawEntryPoint /subsystem:windows /nodefaultlib:libcmt.lib /nodefaultlib:msvcrt.lib
LIBS=kernel32.lib user32.lib shell32.lib advapi32.lib
RC=rc /nologo

all: clickkeys.exe

clickkeys.res: clickkeys.rc
	$(RC) clickkeys.rc

clickkeys.obj: resource.h
clickkeys.rc: resource.h manifest.xml clickkeys.ico

clickkeys.exe: clickkeys.obj clickkeys.res
	$(LD) /out:$@ $(LDFLAGS) $(LIBS) $**

.c.obj::
	$(CC) $(CFLAGS) /c $<

clean:
	del clickkeys.obj clickkeys.res

distclean: clean
	del clickkeys.exe
