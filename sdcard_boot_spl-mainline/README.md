To build a bootable SD card

Step 1: Build spl-mmc binary (there is a compiled binary in repo, can skip build and use our version as-is)
```
$ make CROSS_COMPILE=riscv64-unknown-elf- p=sun20iw1p1 mmc
```
Step 2: Format SD card with FAT 32 filesystem. 

Step 3: Write mmc image to boot sector of SD card.  We wrote a quick script to do this (but script works only on macOS).
```
$ jz_write_bootsector.sh
```

See README-sdcard.txt for rest of instructions about using the SD card for boot.
