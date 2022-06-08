
# Gameboy

<div align="center">

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
#### DMG & GameBoy Color emulator

</div>

## Screenshots
| Menu                                              | In-Game                                                 |
|---------------------------------------------------|---------------------------------------------------------|
| ![Zelda Menu](screenshots/zelda-seasons-menu.png) | ![Zelda In-Game](screenshots/zelda-seasons-in-game.png) |
| ![Tetris Menu](screenshots/tetris-menu.png)       | ![Tetris In-Game](screenshots/tetris-in-game.png)       |
| ![Mario Menu](screenshots/mario-menu.png)         | ![Mario In-Game](screenshots/mario-in-game.png)         |
| ![Pokemon Menu](screenshots/pokemon-menu.png)     | ![Pokemon In-Game](screenshots/pokemon-in-game.png)     |

## Cloning
```sh
$ git clone --recursive https://github.com/coolq1000/gameboy.git
```

## Building
Dependencies: C/C++, CMake
```sh
$ cmake -B build -G "Unix Makefiles"
$ cd build && make
```

## Usage
```sh
$ ./gameboy <rom_path>
```

## Blargg's Test Report
![CPU Test](screenshots/cpu-test.png)

| Col 1  | Col 2  | Col 3  |
|--------|--------|--------|
| 01: ok | 02: ok | 03: ok |
| 04: ok | 05: ok | 06: ok |
| 07: ok | 08: ok | 09: ok |
| 10: ok | 11: 01 |        |

## Problems

### CPU - Processor
Currently, the emulator fails test #11 on `cpu_instrs.gb`.

### PPU - Graphics
There are some visual glitches/flickering on some sprites due to inaccurate frame timing.

### APU - Audio
The APU has been implemented fairly inaccurately, and causes some pops/crackles in some games. It works for the most part.
