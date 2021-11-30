
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "uf2format.h"

#define APP_START_ADDRESS   0x02000
#define APP_END_ADDRESS     0x40000

int main(int argc, char **argv) {
    const char *outname = argc > 1 ? argv[1] : "flash.uf2";

    FILE *fout = fopen(outname, "wb");

    uint32_t sz = (APP_END_ADDRESS - APP_START_ADDRESS)/256;

    UF2_Block bl;
    memset(&bl, 0, sizeof(bl));

    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.magicEnd = UF2_MAGIC_END;
    bl.targetAddr = APP_START_ADDRESS;
    bl.numBlocks = (sz + 255) / 256;
    bl.payloadSize = 256;
    bl.reserved = 0x68ed2b88;
    int numbl = 0;

    while (bl.targetAddr < APP_END_ADDRESS) {
        bl.blockNo = numbl++;
        memset(bl.data, 0xFF, 256);
        fwrite(&bl, 1, sizeof(bl), fout);
        bl.targetAddr += bl.payloadSize;
        // clear for next iteration, in case we get a short read
        memset(bl.data, 0, sizeof(bl.data));
    }

    fclose(fout);
    printf("Wrote %d blocks to %s\n", numbl, outname);
    return 0;
}
