CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS =

# -------- Plattforms‑auto‑detektion ------------------------
ifeq ($(OS),Windows_NT)
    WINDOWS = 1
endif

ifneq ($(WINDOWS),1)
    UNAME_S := $(shell uname -s 2>/dev/null)
    ifeq ($(UNAME_S),Darwin)
        MACOS = 1
    else ifeq ($(UNAME_S),Linux)
        LINUX = 1
    endif
endif

# -------- macOS -------------------------------------------
ifeq ($(MACOS),1)
    CFLAGS  += -I/opt/homebrew/include/SDL2
    LDFLAGS += -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
endif

# -------- Linux -------------------------------------------
ifeq ($(LINUX),1)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS += $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
endif

# -------- Windows – vcpkg eller MSYS2 ---------------------
ifeq ($(WINDOWS),1)
    # vcpkg
    ifneq ("$(wildcard C:/vcpkg/installed/x64-windows/include/SDL2)","")
        SDL2_FOUND = 1
        CFLAGS  += -IC:/vcpkg/installed/x64-windows/include \
                   -IC:/vcpkg/installed/x64-windows/include/SDL2
        LDFLAGS += -LC:/vcpkg/installed/x64-windows/lib \
                   -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
    endif
    # MSYS2
    ifeq ($(SDL2_FOUND),)
        ifneq ("$(wildcard C:/msys64/mingw64/include/SDL2)","")
            SDL2_FOUND = 1
            CFLAGS  += -IC:/msys64/mingw64/include \
                       -IC:/msys64/mingw64/include/SDL2
            LDFLAGS += -LC:/msys64/mingw64/lib \
                       -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
        endif
    endif
    ifeq ($(SDL2_FOUND),)
        $(error SDL2 not found in known directories)
    endif
endif

# -------- Källfiler ---------------------------------------
SRCDIR   = source

CORE_SOURCES = $(SRCDIR)/game_core.c \
               $(SRCDIR)/maze.c \
               $(SRCDIR)/player.c \
               $(SRCDIR)/projectile.c \
               $(SRCDIR)/network.c \
               $(SRCDIR)/camera.c \
               $(SRCDIR)/menu.c \
               $(SRCDIR)/audio_manager.c \
               $(SRCDIR)/lobby.c 

GAME_SOURCES = $(SRCDIR)/client.c $(CORE_SOURCES)

OBJECTS = $(GAME_SOURCES:.c=.o)

TARGET  = game

# -------- Regler ------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(WINDOWS),1)
	@powershell -Command "if (Test-Path $(TARGET).exe) { Remove-Item $(TARGET).exe }"
	@powershell -Command "Get-ChildItem $(SRCDIR)/*.o -ErrorAction SilentlyContinue | Remove-Item"
else
	@rm -f $(TARGET) $(OBJECTS)
endif