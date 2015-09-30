ifeq ($(DEBUG), 1)
    CFLAGS:=-g
    LDFLAGS:=
else
    CFLAGS:=-O2 -DNDEBUG
    LDFLAGS:=-s -mwindows
endif

CC:=gcc
WINDRES:=windres
EXE:=magnifier.exe
OBJS:=magnifier.o mainwnd.o viewport.o resource.o
LIBS:=-lgdi32 -lopengl32

.PHONY: all clean

all: $(EXE)

clean:
	del $(EXE) $(OBJS) magnifier.cfg

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.SUFFIXES: .rc

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

.rc.o:
	$(WINDRES) -o $@ $<
