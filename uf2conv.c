#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>

#include <windows.h>
#include <io.h>

#include "uf2.h"

#define PROGRAM "uf2conv.exe"
#define VERSION "0.3.0"

#define APP_START_ADDRESS   0x2000

/*程序的名字*/
const char *program_name = PROGRAM;

/* 此参数用于承放指定的参数，默认为空 */
const char *input_filename = NULL;
const char *output_filename = "flash.uf2";

uint32_t app_start_addr = APP_START_ADDRESS;
uint32_t family_id = 0x0;

typedef struct {
    const char name[16];
    uint32_t id;
} families_t;

const families_t families[] = {
    {"SAMD21",      0x68ed2b88 },
    {"SAML21",      0x1851780a },
    {"SAMD51",      0x55114460 },
    {"NRF52",       0x1b57745f },
    {"STM32F1",     0x5ee21072 },
    {"STM32F2",     0x5d1a0a2e },
    {"STM32F3",     0x6b846188 },
    {"STM32F4",     0x57755a57 },
    {"STM32F7",     0x53b80f00 },
    {"STM32G0",     0x300f5633 },
    {"STM32G4",     0x4c71240a },
    {"STM32L0",     0x202e3a91 },
    {"STM32L1",     0x1e1f432d },
    {"STM32L4",     0x00ff6919 },
    {"STM32L5",     0x04240bdf },
    {"STM32H5",     0x4e8f1c5d },
    {"STM32H7",     0x6db66082 },
    {"ATMEGA32",    0x16573617 },
    {"MIMXRT10XX",  0x4FB2D5BD },
    {"ESP8266",     0x7eab61ed },
    {"ESP32",       0x1c5f21b0 },
    {"ESP32S2",     0xbfdd4eee },
    {"ESP32S3",     0xc47e5767 },
    {"ESP32C2",     0x2b88d29c },
    {"ESP32C3",     0xd42ba06c },
    {"ESP32C5",     0xf71c0343 },
    {"ESP32C6",     0x540ddf62 },
    {"ESP32C61",    0x77d850c4 },
    {"ESP32H2",     0x332726f6 },
    {"ESP32P4",     0x3d308e94 },
    {"RP2040",      0xe48bff56 },
    {"RP2040",      0xe48bff56 },
    {"RP2350_ARM_S",0xe48bff59 },
    {"RP2350_ARM_NS",0xe48bff5b },
    {"RP2350_RISCV",0xe48bff5a },
    {"CH32V",       0x699b62ec },
};

#define FAMILY_MAX      (sizeof(families) / sizeof(families_t))

uint32_t get_family_id(const char *name){
    uint32_t id = 0;
    int i;

    for (i = 0; i< FAMILY_MAX; i++){
        if(strncmp(name, families[i].name, strlen(families[i].name)) == 0){
            id = families[i].id;
            break;
        }
    }

    return id;
}

/* 打印程序参数 */
void print_usage (FILE *stream, int exit_code)
{
    fprintf (stream, "\n%s: version %s, a tools for convert bin to uf2 \n", program_name, VERSION);
    fprintf (stream, "\nusage: %s [-f family] [-b address] [-o flash.uf2] flash.bin \n", program_name);
    fprintf (stream, "Options:\n"
             "  -b --base address       Set base address of application for BIN format (default: 0x2000)\n"
             "  -f --family SAMD21/SAML21/SAMD51/NRF52/STM32F1/STM32F4/ATMEGA32/MIMXRT10XX\n"
             "                          Set the device family name, (default: 0)\n"
             "  -o --output flash.uf2   Set the output file name, default: flash.uf2\n"
             "  -h --help               Display help information\n"
             "  -v --version            Show the program version\n\n");
    if (exit_code != 0)
        exit (exit_code);
} /* procedure USAGE */

void parse_options (int argc, char *argv[])
{
    int next_option;    // 下一个要处理的参数符号
    int haveargv = 0;   // 是否有我们要的正确参数，一个标识

    /* 包含短选项字符的字符串，注意这里的‘:’ */
    const char *const short_options = "f:b:o:vh";

    /* 标识长选项和对应的短选项的数组 */
    const struct option long_options[] = {
        { "help",       0,  NULL,   'h' },
        { "family",     1,  NULL,   'f' },
        { "base",       1,  NULL,   'b' },
        { "output",     1,  NULL,   'o' },
        { "version",    0,  NULL,   'v' },
        { NULL,         0,  NULL,    0  }   // 最后一个元素标识为NULL
    };

    /* 一个标志，是否显示版本号 */
    int verbose = 0;

    do {
        next_option = getopt_long (argc, argv, short_options,
                                   long_options, NULL);
        switch (next_option) {
            case 'h':                /* -h or --help */
                haveargv = 1;
                print_usage (stdout, 1);
                break;

            case 'f':
                family_id = get_family_id(optarg);
                haveargv = 1;
                break;

            case 'b':                /* -s or --start */
                /* 此时optarg指向--base后的address, 16进制数 */
                app_start_addr = strtol (optarg, NULL, 16);
                haveargv = 1;
                break;

            case 'o':                /* -o or --output */
                /* 此时optarg指向--output后的filename */
                output_filename = optarg;
                haveargv = 1;
                break;

            case 'v':                /* -v or --version */
                verbose = 1;
                haveargv = 1;
                break;

            case ':':
                /* 缺乏长选项内容 */
                break;

            case '?':
                /* 出现一个未指定的参数*/
                print_usage (stdout, 1);
            case -1:
                /* 处理完毕后返回-1 */
                if (!haveargv) {
                    print_usage (stdout, 1);
                }
                break;
            default:
                /* 未指定的参数出现，出错处理 */
                print_usage (stdout, 1);
                break;
        }
    }   while (next_option != -1);

    if (verbose) {
        int i;
        for (i = optind; i < argc; ++i)
            printf ("Argument: %s\n", argv[i]);

        printf ("\nuf2conf, for Windows only, version 1.1.0\n");
        printf ("A tools for convert bin file to uf2 format.\nIf target board already in bootloader mode, auto copy uf2 into the bootloader\n");
        print_usage (stdout, 0);
    }
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        print_usage (stdout, 1);
    }

    parse_options (argc, argv);

    if(argc > optind){
        /* 此时optarg指向--input后的filename */
        input_filename = argv[optind];
    }

    /* argv[0]始终指向可执行的文件文件名 */
    program_name = argv[0];

    if (input_filename == NULL) {
        fprintf (stdout, "Need set input file, exit\n");
        print_usage (stdout, 1);
    }

    printf ("Input file: %s\n", input_filename);
    printf ("Output file: %s\n", output_filename);
    printf ("Base Address: %X\n", app_start_addr);

    FILE *fin = fopen (input_filename, "rb");
    if (!fin) {
        fprintf (stdout, "No such file: %s\n", input_filename);
        return 1;
    }

    fseek (fin, 0L, SEEK_END);
    uint32_t sz = ftell (fin);
    fseek (fin, 0L, SEEK_SET);

    FILE *fout = fopen (output_filename, "wb");

    UF2_Block bl;
    memset (&bl, 0, sizeof (bl));

    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.magicEnd = UF2_MAGIC_END;
    bl.targetAddr = app_start_addr;
    bl.numBlocks = (sz + 255) / 256;
    if(family_id != 0) {
        printf ("Family ID: %X\n", family_id);
        bl.flags |= UF2_FLAG_FAMILY_ID;
        bl.reserved = family_id;
    }
    bl.payloadSize = 256;
    int numbl = 0;
    while (fread (bl.data, 1, bl.payloadSize, fin)) {
        bl.blockNo = numbl++;
        fwrite (&bl, 1, sizeof (bl), fout);
        bl.targetAddr += bl.payloadSize;
        // clear for next iteration, in case we get a short read
        memset (bl.data, 0, sizeof (bl.data));
    }

    fclose (fout);
    fclose (fin);
    printf ("Wrote %d blocks to %s\n", numbl, output_filename);

    do {
        DWORD dwSize = MAX_PATH;
        char szLogicalDrives[MAX_PATH] = {0};
        //获取逻辑驱动器号字符串
        DWORD dwResult = GetLogicalDriveStrings (dwSize, szLogicalDrives);
        //处理获取到的结果
        if (dwResult > 0 && dwResult <= MAX_PATH) {
            char *szSingleDrive = szLogicalDrives;  //从缓冲区起始地址开始
            while (*szSingleDrive) {
                /* 获取驱动器的类型，只处理可移动盘 */
                if (GetDriveType (szSingleDrive) == DRIVE_REMOVABLE) {
                    printf ("Removable Disk %s\n", szSingleDrive); //输出单个驱动器的驱动器号和类型

                    char cFileAddr[2048];
                    struct _finddata_t fileinfo;    //文件存储信息结构体
                    long fHandle;                   //保存文件句柄

                    /* 检查驱动器中是否有INFO_UF2.TXT文件 */
                    strncpy (cFileAddr, szSingleDrive, strlen (szSingleDrive));
                    strcpy (cFileAddr + strlen (szSingleDrive), "INFO_UF2.TXT");
                    if ((fHandle = _findfirst (cFileAddr, &fileinfo )) != -1L ) {
                        char pwd[2048];
                        char cmd[2048];
                        printf ( "Find: %s, size %ld\n", fileinfo.name, fileinfo.size);
                        getcwd (pwd, 512);
                        sprintf (cmd, "copy %s\\%s %s", pwd, output_filename, szSingleDrive);
                        printf ( "%s\n", cmd);
                        system (cmd);
                    }

                    _findclose ( fHandle ); //关闭文件
                }

                // 获取下一个驱动器号起始地址
                szSingleDrive += strlen (szSingleDrive) + 1;
            }
        }
    } while (0);

    return 0;
}
