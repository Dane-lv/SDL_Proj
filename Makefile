# Set default flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS =

# Windows detection (no nesting)
ifeq ($(OS),Windows_NT)
    WINDOWS = 1
endif

# Try macOS detection if not Windows
ifneq ($(WINDOWS),1)
    UNAME_S := $(shell uname -s 2>/dev/null)
    ifeq ($(UNAME_S),Darwin)
        MACOS = 1
    endif
endif

# Try Linux detection if not Windows and not macOS
ifneq ($(WINDOWS),1)
ifneq ($(MACOS),1)
    ifeq ($(UNAME_S),Linux)
        LINUX = 1
    endif
endif
endif

# Apply macOS settings (if detected)
ifeq ($(MACOS),1)
    CFLAGS  += -I/opt/homebrew/include/SDL2
    LDFLAGS += -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
endif

# Apply Linux settings (if detected)
ifeq ($(LINUX),1)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS += $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_net -lSDL2_mixer
endif

# Windows SDL2 detection - first option (vcpkg)
ifeq ($(WINDOWS),1)
    ifneq ("$(wildcard C:/vcpkg/installed/x64-windows/include/SDL2)","")
        SDL2_FOUND = 1
        CFLAGS  += -IC:/vcpkg/installed/x64-windows/include -IC:/vcpkg/installed/x64-windows/include/SDL2
        LDFLAGS += -LC:/vcpkg/installed/x64-windows/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
    endif
endif

# Windows SDL2 detection - second option (MSYS2)
ifeq ($(WINDOWS),1)
    ifeq ($(SDL2_FOUND),)
        ifneq ("$(wildcard C:/msys64/mingw64/include/SDL2)","")
            SDL2_FOUND = 1
            CFLAGS  += -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/SDL2
            LDFLAGS += -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_net -lSDL2_mixer
        endif
    endif
endif

# Error if Windows and SDL2 not found
ifeq ($(WINDOWS),1)
    ifeq ($(SDL2_FOUND),)
        $(error SDL2 not found in known directories)
    endif
endif

SRCDIR   = source
CORE_SOURCES = $(SRCDIR)/game_core.c $(SRCDIR)/maze.c $(SRCDIR)/player.c $(SRCDIR)/projectile.c $(SRCDIR)/network.c $(SRCDIR)/camera.c $(SRCDIR)/text_renderer.c $(SRCDIR)/audio_manager.c
SERVER_SOURCES = $(SRCDIR)/server.c
CLIENT_SOURCES = $(SRCDIR)/client.c

SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o) $(CORE_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o) $(CORE_SOURCES:.c=.o)

SERVER_TARGET = server
CLIENT_TARGET = client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(WINDOWS),1)
	powershell -Command "if (Test-Path $(SERVER_TARGET).exe) { Remove-Item $(SERVER_TARGET).exe }"
	powershell -Command "if (Test-Path $(CLIENT_TARGET).exe) { Remove-Item $(CLIENT_TARGET).exe }"
	powershell -Command "Get-ChildItem $(SRCDIR)/*.o -ErrorAction SilentlyContinue | Remove-Item"
else
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
endif