# PWDoom

> A Doom-inspired game built with **C99** and **raylib 5.5**.

## Project Structure

```
ProjektZespolowy-PWDoom/
├── CMakeLists.txt          # Build configuration
├── CMakePresets.json        # Build presets (debug / release)
├── .clang-format            # Code formatting rules
├── .clang-tidy              # Static analysis config
├── .clangd                  # IDE clangd config
├── Dockerfile               # CI build image
├── .github/workflows/ci.yml # GitHub Actions CI
├── run.sh                   # Build + run
├── watch.sh                 # File watcher (auto-rebuild)
│
├── src/                     # Implementation (.c)
│   ├── main.c
│   └── game/
│       └── game.c
│
├── include/                 # Public headers (.h)
│   └── game/
│       └── game.h
│
├── assets/                  # Game assets (copied to build dir)
│   ├── textures/
│   ├── sounds/
│   └── maps/
└── docs/
```

## Requirements

| Tool    | Minimum Version |
|---------|----------------|
| CMake   | 3.25+          |
| Ninja   | 1.10+          |
| C Compiler (Clang/GCC) | C99 support |

> raylib is fetched automatically via CMake FetchContent.

## Quick Start

```bash
./run.sh              # build + run (debug)
./run.sh release      # build + run (release)
./watch.sh            # file watcher, auto-rebuild on save
```

### Manual Build

```bash
cmake --preset default
cmake --build --preset default
./build/debug/PWDoom
```

## Static Analysis

```bash
cmake --build --preset default --target tidy      # clang-tidy
cmake --build --preset default --target cppcheck  # cppcheck
```

## Build System

- **CMake** with **Ninja** generator
- **C99** standard (strict, no extensions)
- **raylib 5.5** via FetchContent (auto-downloaded)
- **AddressSanitizer + UBSan** enabled in Debug builds
- **Strict warnings** (NASA/CERT inspired): `-Wall -Wextra -Wpedantic -Wconversion` + more
- **clang-tidy** (CERT C, bugprone, clang-analyzer)
- **cppcheck** static analysis
- **GitHub Actions CI**: GCC + Clang builds, static analysis, format check