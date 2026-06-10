/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2014      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include <common.h>
#include <private_boot0.h>	// BT0_head
#include <mmc_boot0.h>

static const int SD_CARD_NUM = 0;

// JDZ struct copied from nboot/load_image_mcc/load_image_sdmmc.c
typedef struct _boot_sdcard_info_t
{
	__s32	card_ctrl_num;                //总共的卡的个数
	__s32	boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
	__s32	card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
	__s32	speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	__s32	line_sel[4];                  //卡的线制，0: 1线，其它，4线
	__s32	line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void)
{
	boot_sdcard_info_t  *sdcard_info = (void *)BT0_head.prvt_head.storage_data;
	int bus_width = sdcard_info->line_sel[SD_CARD_NUM]; // 4 default/always?
	int unused = 0; // offset ignored if card# = 0(SD)
	if (sunxi_mmc_init(SD_CARD_NUM, bus_width, BT0_head.prvt_head.storage_gpio, unused) == -1)
		return STA_NOINIT;

	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* buff,		/* Pointer to the destination object */
	DWORD sector,	/* Sector number (LBA) */
	UINT offset,	/* Offset in the sector */
	UINT count		/* Byte count (bit15:destination) */
)
{
	char tmp[512];

	int nsector = mmc_bread(SD_CARD_NUM, sector, 1, tmp); // read 1 sector
	if (nsector != 1) return RES_ERROR; 	// confirm full sector read
	memcpy(buff, tmp + offset, count); 		// copy out desired nbytes at offset
	return 0;
}
