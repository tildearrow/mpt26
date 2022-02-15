# mpt26 usage guide

you need ca65 from the popular cc65 compiler for 6502.

1. open template.mptm in OpenMPT.
  - yep, you have to use the template. it's the only thing that will work with the converter.
  - do not touch the speed. you may optionally change the tempo from 150 to 125 if you are working on PAL (but this isn't detected).
  - only use instruments 1 to 10.
  - only use notes from 0 to 31.
  - only use volumes from 0 to 15.
  - anything else may break the converter.
2. compose your song.
3. open a command prompt/terminal, `cd` to `export/build/` and run the converter.
  - Windows: `mpt26conv.exe <filename>`
  - Linux: `./mpt26conv <filename>`
  - this will generate some files that are required for the converter to work.
  - the files **have** to be in `export/build/`. otherwise it will fail.
4. go back to the root, and:
  - on Windows, type:
    - `ca65 -o main.o main.s`
    - `ld65 -C mpt26.cfg -o mpt26.bin main.o`
  - on Linux, just type `make`.
5. this will output a ROM file called `mpt26.bin` which may be loaded on an emulator or real hardware.

# notes

- don't make your song too long/complex, otherwise the converter will produce a file that is too large to fit in 4KB.
- if you need to, edit mpt26.cfg and change the start address and size of DATA and PRG to fit your needs.
- if you want to write a PAL ROM, uncomment `PAL=1` in `main.s`.
- make sure to have a note at the beginning of each pattern, otherwise it will be silent when the pattern changes (to be fixed).
- no bankswitching supported... yet.
- if your patterns are longer than 128, do not add too much complexity. it may cause the player to get stuck (to be fixed).

# attribution

thanks Cubby for the original TIA template!
