#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>

#include <Windows.h>
#include <fileapi.h>
#include <io.h>

#include "uf2format.h"

#define PROGRAM "uf2conv.exe"

#define APP_START_ADDRESS   0x2000

/*程序的名字*/
const char *program_name = PROGRAM;

/* 此参数用于承放指定的参数，默认为空 */
const char *input_filename = "flash.bin";
const char *output_filename = "flash.uf2";

uint32_t app_start_addr = APP_START_ADDRESS;

/* 打印程序参数 */
void print_usage (FILE *stream, int exit_code)
{
    fprintf (stream, "\nusage: %s [-s address] [-i flash.bin] [-o flash.uf2]\n", program_name);
    fprintf (stream, "Options:\n"
             "  -s --start address      Starting address in hex for binary file (default: 2000)\n"
             "  -i --input flash.bin    Set the input file name to be convert, default: flash.bin\n"
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
    char *stop;

    /* 包含短选项字符的字符串，注意这里的‘:’ */
    const char *const short_options = "s:i:o:vh";

    /* 标识长选项和对应的短选项的数组 */
    const struct option long_options[] = {
        { "help",       0,  NULL,   'h' },
        { "start",      1,  NULL,   's' },
        { "input",      1,  NULL,   'i' },
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

            case 's':                /* -s or --start */
                /* 此时optarg指向--start后的address, 16进制数 */
                app_start_addr = strtol (optarg, &stop, 16);
                haveargv = 1;
                break;

            case 'i':                /* -i or --input */
                /* 此时optarg指向--input后的filename */
                input_filename = optarg;
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

        printf ("uf2conf, a tool for convert binary to uf2 format.\nVersion 0.1.2\n");
        print_usage (stdout, 1);
    }
}

int main (int argc, char **argv)
{
    if (argc < 2) {
        print_usage (stdout, 1);
    }

    parse_options (argc, argv);

    /* argv[0]始终指向可执行的文件文件名 */
    program_name = argv[0];

    printf ("Input file: %s\n", input_filename);
    printf ("Output file: %s\n", output_filename);
    printf ("Start Address: %X\n", app_start_addr);

    FILE *fin = fopen (input_filename, "rb");
    if (!fin) {
        fprintf (stderr, "No such file: %s\n", input_filename);
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

    {
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

                    char cFileAddr[300];
                    struct _finddata_t fileinfo;    //文件存储信息结构体
                    long fHandle;                   //保存文件句柄

                    /* 检查驱动器中是否有INFO_UF2.TXT文件 */
                    strncpy (cFileAddr, szSingleDrive, 200);
                    strcpy (cFileAddr + strlen (szSingleDrive), "INFO_UF2.TXT");
                    if ((fHandle = _findfirst (cFileAddr, &fileinfo )) != -1L ) {
                        char pwd[128];
                        char cmd[1024];
                        printf ( "Find: %s, size %ld\n", fileinfo.name, fileinfo.size);
                        getcwd (pwd, 500);
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
    }

    return 0;
}
