# FujiNet YAIL (Yet Another Image Loader) Client

A cc65-based image loading and viewing application for the Atari 8-bit platform that works with the FujiNet hardware. YAIL allows you to load and view images from network sources directly on your Atari computer.

## Features

- Load and display images from network sources via FujiNet
- Support for various graphics modes
- Console interface for commands and settings
- Persistent settings storage using FujiNet AppKey functionality
- Support for PBM image format

## Build Requirements

- cc65 compiler suite
- FujiNet library (version 4.7.4 recommended)
- `dir2atr` utility for creating Atari disk images (required for `make disk`)

## Building

To build the application, ensure you have the correct compiler/linker for your platform (e.g. cc65), and
make on your path, then simply run make.

```shell
# to clean all artifacts
make clean

# to generate the application for all targets
make all

# to generate a "disk" (ATR)
make disk
```

By default, the build will target multiple platforms (atari, apple2enh, apple2, c64), but you can specify just the Atari target:

```shell
# just the Atari target
make TARGETS="atari" all
```

### Important Build Notes

- The project uses a custom linker configuration file (`src/atari/yail.atari-xex.cfg`) that defines a `FRAMEBUFFER` memory area of at least 20KB
- FujiNet library version 4.7.4 is specified in the Makefile, but version 4.5.1 has been confirmed to work well

## Output Files

After a successful build:
- The executable will be in `build/yail.atari`
- Running `make disk` will create an ATR disk image in `dist/yail.atr`

## Usage

1. Boot the ATR disk image in an Atari computer with FujiNet hardware attached
2. The application will start and display the console interface
3. Use the interface to specify image URLs and view images

## Project Structure

- `src/` - Main source files
- `src/atari/` - Atari-specific implementation files
- `makefiles/` - Build system files
- `build/` - Build output directory
- `dist/` - Distribution files (ATR disk images)

## Makefile System

The build uses a modular Makefile system:
- `Makefile` - Main entry point
- `makefiles/build.mk` - Core build logic
- `makefiles/custom-atari.mk` - Atari-specific settings
- `makefiles/common.mk` - Common settings across platforms
- `makefiles/fujinet-lib.mk` - FujiNet library integration

## License

See the LICENSE file for details.
