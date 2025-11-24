(IMAGES ARE BELOW)
# Maze Mayhem

Maze Mayhem is a top-down multiplayer arena built with SDL2. Host a lobby, invite friends over TCP, and battle through a neon-lit maze where ricocheting projectiles, reactive audio, and a zoomable camera keep every match tense. The codebase is fully C-based and organized for cross-platform builds via the provided Makefile.

## Highlights
- Up to five simultaneous players with host/client roles negotiated in a lobby.
- Deterministic maze generation with fog-of-war style lighting per player.
- Mouse-aimed, bouncing projectiles with optional friendly fire after ricochets.
- Built-in menus for hosting, joining via IP, and adjusting SFX/music volume.
- Death screen with a spectate mode so eliminated players can keep watching.
- SDL_mixer-driven background music and death sound effects.

## Requirements
- C toolchain (GCC/Clang or MSVC via MinGW/MSYS2).
- SDL2 core plus the following extensions: `SDL2_image`, `SDL2_ttf`, `SDL2_net`, `SDL2_mixer`.
- Assets from the `resources/` directory (already included).

### Installing SDL2 Dependencies

**macOS (Homebrew)**
```
brew install sdl2 sdl2_image sdl2_ttf sdl2_net sdl2_mixer
```

**Ubuntu / Debian**
```
sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-image-dev \
     libsdl2-ttf-dev libsdl2-net-dev libsdl2-mixer-dev
```

**Windows**
- Via [vcpkg](https://github.com/microsoft/vcpkg): install the same SDL2 ports, then ensure `C:/vcpkg/installed/x64-windows` is present (the Makefile autodetects it).
- Or install MSYS2, add the `mingw64` environment to `PATH`, and install `mingw-w64-x86_64-SDL2` plus the matching `SDL2_image`, `SDL2_ttf`, `SDL2_net`, `SDL2_mixer` packages. The Makefile already knows both layouts.

## Build
```
cd /Users/dane/Documents/SDL_Proj
make            # builds the `game` executable
make clean      # removes objects and the binary
```
The Makefile auto-detects macOS, Linux, or Windows and sets include/library paths accordingly. Modify `CFLAGS`/`LDFLAGS` if your SDL installation lives elsewhere.

## Run
```
./game          # macOS / Linux
game.exe        # Windows (from PowerShell or MSYS2 shell)
```

1. Choose **Host Game** to start a server on TCP port `7777`.
2. Other players pick **Join Game**, type the host’s IP, and press Enter.
3. When everyone toggles ready in the lobby, the host starts the round.

You can also play solo by hosting and starting immediately; networking falls back gracefully if no peers connect.

## Gameplay & Controls
- `WASD` / Arrow keys: movement.
- Mouse: aim; the camera keeps your player centered unless spectating.
- `Space`: fire a projectile (shots bounce off walls and arena bounds).
- `Esc`: exit to desktop at any time.
- After dying, click the **Spectate** button to continue watching the match.

## Project Layout
- `source/`: implementation files (`client.c` entry point, gameplay, lobby, networking, audio, etc.).
- `include/`: public headers shared across modules.
- `resources/`: textures, fonts, music, SFX. Keep this folder next to the executable so relative paths resolve.
- `Makefile`: cross-platform build script and SDL2 auto-detection.

## Troubleshooting
- **Missing SDL headers/libraries**: confirm the packages above are installed and adjust the include/library paths in the Makefile if they live outside the default locations.
- **Firewall/port issues**: ensure TCP port `7777` is open on the host machine; clients must be able to reach it directly.
- **Choppy audio**: lower system output latency or verify your SDL_mixer backend is configured for the device you’re using.

<img width="3094" height="2296" alt="image" src="https://github.com/user-attachments/assets/7f8fa2c8-b95b-43c2-9310-9ac1f0722f7f" />
<img width="2874" height="1686" alt="image" src="https://github.com/user-attachments/assets/8b8cc08f-a76d-4e5c-bc8b-fe379cef4645" />
<img width="3224" height="1962" alt="image" src="https://github.com/user-attachments/assets/f56db9c9-0a9d-42e3-aa42-5f904acd0921" />
<img width="3078" height="2274" alt="image" src="https://github.com/user-attachments/assets/c1a9af73-641b-4633-a2e4-1fbaca422d6b" />
<img width="3050" height="2224" alt="image" src="https://github.com/user-attachments/assets/52f519bd-d1d6-4c89-8ef3-e2e1471dbe5c" />



