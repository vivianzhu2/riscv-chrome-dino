This is a Mango Pi bootable SD card (has custom boot0 in bootsector).

You can use this card to automatically boot the Mango Pi
into your custom program on reset. Here's how:

- Build program normally on laptop. Insert card into SD reader on laptop
and copy .bin file to card. Filename must be exactly MANGO.BIN
- Remove card from laptop and insert into SD slot of Mango Pi
- Reset Mango Pi. Custom boot0 runs, will attempt to load file named MANGO.BIN from card
- If file found, will load and exec
- If file not found or unable to read, will exit to FEL

To update program and re-run, repeat sequence: edit on laptop, recompile,
insert card on laptop, replace MANGO.BIN file on card with new build,
insert card on Mango Pi and reset.

Program image loaded via SD card should behave as when loaded via bootloader (xfel)
(at least that's the intention...)

This code is new and has been only lightly tested.
Let me know if you find issues with it and I can help investigate!

Julie Zelenski
CS107e December 2024
