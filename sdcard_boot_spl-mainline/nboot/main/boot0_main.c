/*
 * (C) Copyright 2018
* SPDX-License-Identifier:	GPL-2.0+
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>
#include <private_boot0.h>
#include <arch/clock.h>
#include <arch/uart.h>
#include <arch/dram.h>
#include <arch/gpio.h>

int load_image_from_sdcard_fat(const char *filename, uintptr_t start_addr);

static int boot0_clear_env(void);

// JDZ: used to confirm this boot program is the one that is running
// blink ACT led and count on uart for a while before start boot sequence
void blink(int nseconds) {
	unsigned *cfg = (void *)0x02000098; // PD cfg 2
	*cfg = (*cfg & ~0xf00) | 0x100;	 // config PD18 as output
	unsigned *data = (void *)0x020000a0; // PD data
	char *uart = (void *)0x02500000; // uart base
	for (int i = 0; i < nseconds; i++) {
		*uart = '0' + (i%10); // output digit
		*data ^= (1 << 18);  // toggle PD18 (blue ACT led)
		mdelay(1000);
	}
}

void main(void)
{
	sunxi_serial_init(BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6);
	mdelay(5*1000); // pause for terminal to catch up
	// blink(10);

	printf("HELLO! BOOT0 is starting!\n");
	printf("BOOT0 commit : %s\n", BT0_head.hash);
	printf("This is boot0 modified for CS107E Mango Pi (load mango.bin image from sdcard FAT32)\n");

	sunxi_set_printf_debug_mode(BT0_head.prvt_head.debug_mode);

	int status = sunxi_board_init();
	if(status)
		goto _BOOT_ERROR;

	int dram_size = init_DRAM(0, (void *)BT0_head.prvt_head.dram_para);
	if(!dram_size)
		goto _BOOT_ERROR;
	else {
		printf("dram size =%d\n", dram_size);
	}
	// JDZ Aug 2024
	// this version of boot0 will attempt to load program image from sdcard
	// looks for file named mango.bin, if found will load and exec
	// otherwise exits to FEL
	const char *kernel_file = "MANGO.BIN";
	uintptr_t start_addr = 0x40000000;
	int error = load_image_from_sdcard_fat(kernel_file, start_addr);
	if (error == 0) {
		printf("read file '%s' from SD card, load address @0x%x and exec...\n", kernel_file, start_addr);
		boot0_jmp(start_addr); // does not return
	} else {
		printf("unable to read file '%s' from SD card (error code %d), exiting to FEL\n", kernel_file, error);
	}

_BOOT_ERROR:
	boot0_clear_env();
	boot0_jmp(FEL_BASE);
}

static int boot0_clear_env(void)
{
	sunxi_board_exit();
	sunxi_board_clock_reset();
	mmu_disable();
	mdelay(10);

	return 0;
}
