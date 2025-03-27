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
    LDFLAGS += -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf -lSDL2_image
endif

# Apply Linux settings (if detected)
ifeq ($(LINUX),1)
    CFLAGS  += $(shell sdl2-config --cflags)
    LDFLAGS += $(shell sdl2-config --libs) -lSDL2_ttf
endif

# Windows SDL2 detection - first option (vcpkg)
ifeq ($(WINDOWS),1)
    ifneq ("$(wildcard C:/vcpkg/installed/x64-windows/include/SDL2)","")
        SDL2_FOUND = 1
        CFLAGS  += -IC:/vcpkg/installed/x64-windows/include -IC:/vcpkg/installed/x64-windows/include/SDL2
        LDFLAGS += -LC:/vcpkg/installed/x64-windows/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image
    endif
endif

# Windows SDL2 detection - second option (MSYS2)
ifeq ($(WINDOWS),1)
    ifeq ($(SDL2_FOUND),)
        ifneq ("$(wildcard C:/msys64/mingw64/include/SDL2)","")
            SDL2_FOUND = 1
            CFLAGS  += -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/SDL2
            LDFLAGS += -LC:/msys64/mingw64/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
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
SOURCES  = $(wildcard $(SRCDIR)/*.c)
OBJECTS  = $(SOURCES:.c=.o)
TARGET   = game

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(WINDOWS),1)
	del /Q $(TARGET).exe $(OBJECTS)
else
	rm -f $(TARGET) $(OBJECTS)
endif