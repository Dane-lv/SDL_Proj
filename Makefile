#om alla .c filer är i source

# Detect the OS. "uname -s" returns "Darwin" on macOS, "Linux" on Linux.
UNAME_S := $(shell uname -s)

# Default compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS =

# OS-specific overrides
ifeq ($(UNAME_S), Darwin)
    # macOS-specific flags
    CFLAGS  += -I/opt/homebrew/include/SDL2
    LDFLAGS += -L/opt/homebrew/lib -lSDL2 -lSDL2_ttf
endif

# If the environment variable OS is set to Windows_NT, we assume Windows
ifeq ($(OS), Windows_NT)
    # Windows-specific flags
    # Using vcpkg installation
    CC      = gcc
    CFLAGS  += -IC:/vcpkg/installed/x64-windows/include -IC:/vcpkg/installed/x64-windows/include/SDL2
    LDFLAGS += -LC:/vcpkg/installed/x64-windows/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
endif

SRCDIR = source
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Final executable name
TARGET  = game

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
