quickerNES
============

quickerNES is an attempt to modernizing and improving the performance of quickNES, the fastest NES emulator in the interwebs (as far as I know). The goals for this project are, in order of importance:

- Improve overall emulation performance even more
- Modernize the code base with best programming practices, including CI tests, benchmarks, and coverage analysis
- Add support for more mappers, controllers, and features supported by other emulators
- Improve accuracy, if possible

The main aim is to improve the performance of skip (non-rendering, no-audio) frame advances for brute force botting. (See: [JaffarPlus](https://github.com/SergioMartin86/jaffarPlus)). However, if this work might help with homebrew emulation and other people having more fun, then much better!

Changes
=========

- Optimizations made in the CPU emulation core, including:
  + Forced alignment at the start of a page to prevent crossing cache line boundaries
  + Simplifying instruction decode
- Minimize compiled code size to reduce pressure on L1i cache
- Reduce heap allocations

Credits
=========

- quickNES was originally by Shay Green (a.k.a. [Blaarg](http://www.slack.net/~ant/)) under the GNU GPLv2 license. The source code is still located [here](https://github.com/kode54/QuickNES) 
- The code was later improved and maintained by Christopher Snowhill (a.k.a. [kode54](https://kode54.net/))
- I could trace further contributions (e.g., new mappers) by retrowertz, CaH4e3, some adaptations from the [FCEUX emulator](https://github.com/TASEmulators/fceux) (see mapper021)
- The latest version of the code is maintained by Libretro's community [here](https://github.com/libretro/QuickNES_Core)
- For the interactive player, this project uses a modified version of [HeadlessQuickNES (HQN)](https://github.com/Bindernews/HeadlessQuickNes) by Drew (Binder News)
- We use some of the [NES test rom set](https://github.com/christopherpow/nes-test-roms) made by multiple authors and gathered by Christopher Pow et al.
- We also use some movies from the [TASVideos](tasvideos.org) website for testing. These movies are copied into this repository with authorization under the Creative Commons Attribution 2.0 license.

All base code for this project was found under open source licenses, which I preserved in their corresponding files/folders. Any non-credited work is unintentional and shall be immediately rectfied.

