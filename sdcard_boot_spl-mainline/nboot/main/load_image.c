#include "pff.h"

// Using Petit Fatfs to read named file from sdcard, write contents to start_addr
int load_image_from_sdcard_fat(const char *filename, uintptr_t start_addr) {
	FATFS fs;
    FRESULT rc;
    UINT n;

    rc = pf_mount(&fs);
    if (rc != 0) return rc;

    rc = pf_open(filename);
    if (rc != 0) return rc;

    uint8_t *cur = (void *)start_addr;
    for (;;) {
        rc = pf_read(cur, 512, &n);  // read one sector (512 bytes)
        if (rc != 0) return rc;      // exit if error
        if (n == 0) return 0;        // EOF, nothing more to read
        cur += n;
    }
}
