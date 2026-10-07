# SIGNAL//NULL 0.2 — cross-platform pure C FPS prototype

Explore an industrial maze and purge six floating signal drones. Match your
weapon frequency to each drone's color. Each drone needs two matching hits.
Drones approach when they see you and deal contact damage. Purge all six to win.

The game and software renderer are C11. SDL3 handles the window, mouse,
keyboard and framebuffer presentation on Linux, Windows and macOS.
This is a source package, not a precompiled binary. SDL3 3.2+ is required.

## Omarchy / Arch Linux (Hyprland / Wayland)

Install build tools and SDL3 (Arch's sdl3 package includes the development headers):

```sh
sudo pacman -Syu --needed base-devel cmake sdl3 unzip
```

Extract the ZIP, open a terminal in its `signal-null` directory, then:

```sh
sh build.sh
sh play.sh
```

The build script compiles the game and runs the core tests. No root access is
needed for building or playing. Use `sh` even if your terminal shell is zsh.

SDL chooses the available display backend. To explicitly request native Wayland:

```sh
SDL_VIDEO_DRIVER=wayland sh play.sh
```

If mouse capture or your Wayland backend fails, test XWayland (requires XWayland
in your session):

```sh
SDL_VIDEO_DRIVER=x11 sh play.sh
```

Arrow keys can turn without mouse capture. F11 toggles fullscreen.

## macOS

With Homebrew installed:

```sh
brew install cmake sdl3
sh build.sh
sh play.sh
```

If CMake cannot find SDL3: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
-DCMAKE_PREFIX_PATH="$(brew --prefix sdl3)"` (put the command on one line),
then `cmake --build build`.

## Windows

Install CMake and Visual Studio with the **Desktop development with C++**
workload (the compiler also compiles C). Download the SDL3 development package
for Visual C++ from https://github.com/libsdl-org/SDL/releases and extract it.
Open an x64 Native Tools Command Prompt in the game directory and run:

```bat
build.bat -DCMAKE_PREFIX_PATH="C:\path\to\SDL3-development-folder"
play.bat
```

Point to the extracted SDL3 directory containing its CMake configuration.
CMake copies SDL3.dll beside the executable when using the shared SDL target.
MinGW builds need the matching SDL3 MinGW development package instead of the
Visual C++ package.

## Controls

| Key | Action |
| --- | --- |
| W / S | Forward / backward |
| A / D | Strafe left / right |
| Click | Start / resume and capture mouse; subsequent clicks fire |
| Mouse left / right | Turn |
| Left / right arrows | Turn without mouse capture |
| Space | Fire; hold for automatic shots |
| 1 | LOW — amber, pistol: hurts the serpentipede and combat slug |
| 2 | MID — cyan, blaster: hurts the trilobite and technospider |
| 3 | HIGH — violet, shotgun: hurts the flesh worm and flame bringer |
| P | Pause / resume; the pause page lists the controls |
| Escape | Pause and release mouse |
| F11 | Toggle fullscreen |
| R | Restart |
| Q | Quit |

The game starts on the pause page, which lists these controls. The first click
resumes and captures the mouse without firing. Focus loss pauses and releases it.

Band keys are a switch, not a modifier: tap the number once, then fire. The three
slots at the top left show each key with the two monsters it hurts, and the crosshair
takes the colour of the enemy under it, showing the key to press when the band is
wrong. Wrong-frequency shots do no damage: the enemy flashes its own band colour
instead of white. Every shot is a bolt in the band's colour that flies out and bursts
where it lands, and a killed enemy plays its death animation. Blocks above an enemy
are its remaining hit points. Health is at the bottom left; purge count is top right. Win and lose messages appear over the
scene.

The clock at the top centre counts the seconds you have played; it stops while
paused. Clearing the sector shows your time against the five fastest clears, which
are saved in SDL's per-user data folder (`~/.local/share/signal-null/scores.txt` on
Linux). Delete that file to reset them. All text is drawn into the frame, anti-aliased,
from the Share Tech Mono TrueType font.

## Testing and scope

The portable game core was compiled with GCC using strict warnings and checked
with address/undefined-behavior sanitizers. No sanitizer issues were reported
(leak detection was disabled because it is incompatible with this environment).
The SDL3 desktop path was compiled and linked against SDL 3.2.28 on Linux.
The Release CMake build and CTest passed, and a three-frame presentation test
passed with SDL's dummy video driver/software renderer. Actual Wayland/XWayland
input, fullscreen, and Windows/macOS desktops still need local verification.

To test the core without installing SDL3:

```sh
cc -std=c11 -DSIGNAL_HEADLESS -O2 -Wall -Wextra -Wpedantic -fopenmp -Isrc -isystem third_party \
   src/main.c \
   src/game/game.c \
   src/game/player.c \
   src/game/weapon.c \
   src/game/enemy.c \
   src/game/scores.c \
   src/world/world.c \
   src/world/collision.c \
   src/renderer/renderer.c \
   src/renderer/texture.c \
   src/renderer/font.c \
   src/test/selftest.c \
   -lm -o signal-null-core
./signal-null-core --test
./signal-null-core --snapshot
```

Alternatively, use CMake with `-DSIGNAL_HEADLESS=ON` in a separate build directory.
For a quick desktop check after an SDL3 build, run `sh play.sh --smoke`; this
opens a window, presents three frames and exits (or reports an SDL error).

- 1280 x 720 CPU framebuffer with smoothed (bilinear) textures and scanlines. Rows are
  drawn in parallel when the compiler has OpenMP; without it the game still runs, slower.
- Raycast walls, textured floor, panoramic sky, fog, and animated sprite enemies and guns.
- Wall collision, line-of-sight pursuit, health, restart and victory.
- Horizontal aim; no jumping, vertical look, audio, saves or editor yet.
- 2.5D software raycasting, not yet the proposed full 3D OpenGL engine.

## References

- SDL3 CMake: https://wiki.libsdl.org/SDL3/README-cmake
- SDL3 API: https://wiki.libsdl.org/SDL3/QuickReference
- Arch SDL3 package: https://archlinux.org/packages/extra/x86_64/sdl3/

MIT licensed; see LICENSE.

## Assets

Wall and floor materials are 1K photographed textures from
[Poly Haven](https://polyhaven.com), and the sky is a strip of its Shanghai Bund panorama;
both are in `assets/polyhaven` under CC0 1.0.
The six monsters are [Freedoom](https://github.com/freedoom/freedoom) sprites upscaled 4x by
the [freedoom-hd](https://github.com/m4cd4r4/freedoom-hd) project, in `assets/freedoom-hd`.
Freedoom's art is BSD 3-Clause: keep `assets/freedoom-hd/COPYING.adoc` with the game when you
distribute it.
The guns are in `assets/sprites`. The blaster (CC0) and the shotgun (CC BY 4.0, so its credit
must stay with the game) are by W_K_Studio on itch.io; the pistol is a render of a
CC0 3D model from Poly Haven. Each folder's README links its sources and lists what was changed.
The font is [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono) by Carrois
Type Design, in `assets/fonts` under the SIL Open Font License (`OFL.txt` beside it).
PNG decoding and font rasterising use [stb_image and stb_truetype](https://github.com/nothings/stb)
(public domain / MIT) in `third_party/stb`. The game loads `assets/` relative to the working directory, falling
back to the source tree path recorded by CMake.
