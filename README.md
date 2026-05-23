# Game Boy Emulator

A custom Game Boy emulator written in C. 

**Note on Audio:** The APU (Audio Processing Unit) is currently not implemented, and there are no plans to implement it in the future. This emulator focuses strictly on the core CPU, memory mapping (MMU), graphics (PPU), timers, and input (Joypad).

**Note on Cartridge Compatibility (MBC):** This emulator currently has its Memory Management Unit (MMU) hardcoded to support **MBC5** cartridges (specifically tailored to run *Pokémon Yellow*). While ROM-only games (like *Tetris*) will work flawlessly, attempting to run games that use other Memory Bank Controllers (such as MBC1 or MBC3) will require small modifications to the MMU's bank-switching logic.

## Directory Structure
* **`src/`** - Contains all C source code and header files.
* **`roms/`** - Directory for placing Game Boy ROMs (e.g., `Boot.gb`, `Tetris.gb`, `PokemonYellow.gb`).
* **`saves/`** - Directory where `.sav` files are automatically stored.

## Compilation
This project requires **SDL2** for video rendering and input handling. 

To compile the emulator, navigate to the root directory of the project and use `gcc` to compile all the source files in the `src` folder, linking the SDL2 library:

```bash
gcc src/*.c -o gameboy -lSDL2
```

## How to Run
To play Tetris:
```bash
./gameboy roms/Tetris.gb
```

To play Pokemon Yellow:
```bash
./gameboy roms/PokemonYellow.gb
